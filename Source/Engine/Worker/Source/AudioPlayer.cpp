#include "AudioPlayer.h"
#include "MainComponent.h"

AudioPlayer::AudioPlayer() : readerSource(nullptr)
{
    channels = 1; //nChannels;
    formatManager.registerBasicFormats();
    buf = AudioBuffer<float>(channels, 1024);
    audiochannel = AudioSourceChannelInfo(buf);
}

void AudioPlayer::init(double* destination) {
    target = destination;
}


int AudioPlayer::loadFile(String path)
{

    File file = File(path);
    auto* reader = formatManager.createReaderFor (file);

    if (reader)
    {
        readerSource.reset (new AudioFormatReaderSource (reader, true));
        readerSource->setLooping(true);
        readerSource->prepareToPlay (1024, 44100.);
        numSamples = reader->lengthInSamples;
        currentSample = 0;
        readerSource->setNextReadPosition(currentSample);
        readerSource->getNextAudioBlock(audiochannel);
        return numSamples;
    }

    return 0;
}


void AudioPlayer::setPosition(int numSamples)
{
    if (readerSource.get())
        readerSource->setNextReadPosition(numSamples);
    currentSample = numSamples;
}

long long AudioPlayer::getPosition()
{
    if (!readerSource.get()) return 0;
    return readerSource->getNextReadPosition();
}

void AudioPlayer::tick(int channel)  {
    if (!playing || !readerSource.get()) {
        *target = 0;
        return;
    }

    int bufSample = currentSample & 1023;
    
    if (bufSample == 0) {
        //setParameter("CurrentSample", currentSample);
        readerSource->getNextAudioBlock(audiochannel);
        //oversampledBlock = oversampler->processSamplesUp(buf);
    }
    
    channel %= channels;
    currentSample++;
    
    if(currentSample >= numSamples) {
        //setParameter("CurrentSample", currentSample + 1);
        currentSample = 0;
        if(!looping) {
            playing = false;
            stopTimer();
        }
        
    }
    *target = buf.getSample(channel, bufSample);
}

void AudioPlayer::receiveMessage(MemoryInputStream& m) {
    int messageType = m.readInt();
    
    switch(messageType) {
        case PlayerMessage::Playing: {
            playing = m.readBool();
            playing ? startTimer(200) : stopTimer();
            break;
        }
        case PlayerMessage::Loop: {
            looping = m.readBool();
            break;
            
        }
        case PlayerMessage::Move: {
            setPosition(m.readInt());
            break;
        }
        case PlayerMessage::Load: {
            loadFile(m.readString());
            break;
        }
    }
}


void AudioPlayer::timerCallback() {
    MemoryOutputStream update;
    update.writeInt(MessageID::SendProcessor);
    update.writeInt(ID);
    update.writeInt(ProcessorType::AudioPlayerType);
    update.writeInt(PlayerMessage::Move);
    update.writeInt(currentSample);
    MainComponent::getInstance()->sendMessage(update.getMemoryBlock());
}
