#include "MainComponent.h"
#include "Message.h"
//==============================================================================
MainComponent::MainComponent()
{
    main = this;
    poweredOn = false;
    m_mutex.lock();
    
    // Some platforms require permissions to open input channels so request that here
    if (RuntimePermissions::isRequired (RuntimePermissions::recordAudio)
        && ! RuntimePermissions::isGranted (RuntimePermissions::recordAudio))
    {
        RuntimePermissions::request (RuntimePermissions::recordAudio,
                                     [&] (bool granted) { setAudioChannels (granted ? 2 : 0, 2); });
    }
    else
    {
        // Specify the number of input and output channels that we want to open
        setAudioChannels (2, 2);
    }
}

MainComponent::~MainComponent()
{
    shutdownAudio();
}


void MainComponent::setAudioChannels (int numInputChannels, int numOutputChannels)
{
    String audioError;
    
    auto setup = deviceManager.getAudioDeviceSetup();
    
    audioError = deviceManager.initialise (numInputChannels, numOutputChannels, nullptr, true);
    
    jassert (audioError.isEmpty());
    
    deviceManager.addAudioCallback (&audioSourcePlayer);
    audioSourcePlayer.setSource (this);
}

void MainComponent::shutdownAudio()
{
    audioSourcePlayer.setSource (nullptr);
    deviceManager.removeAudioCallback (&audioSourcePlayer);
}


//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sr)
{
    sampleRate = sr;
    sampsPerBlock = samplesPerBlockExpected;
}

void MainComponent::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
    
    if(!processor) poweredOn = false;
    
    if(poweredOn && lock.tryEnter()) {
        processor->processBlock(*bufferToFill.buffer);
        
        // Apply and smooth volume changes
        if (volume == lastvolume)
        {
            bufferToFill.buffer->applyGain (volume);
        }
        else
        {
            bufferToFill.buffer->applyGainRamp (0, bufferToFill.buffer->getNumSamples(), lastvolume, volume);
            lastvolume = volume;
        }

        float lvl = bufferToFill.buffer->getRMSLevel(0, 0, bufferToFill.buffer->getNumSamples()) / 4.;
        
        MemoryOutputStream memstream;
        memstream.writeInt(MessageID::Volume);
        memstream.writeFloat(lvl);
        sendMessageToMaster(memstream.getMemoryBlock());
        lock.exit();
    }
    else {
        bufferToFill.clearActiveBufferRegion();
    }
    

}

void MainComponent::releaseResources()
{
    
}
void MainComponent::log(String message) {
    MemoryOutputStream memstream;
    memstream.writeInt(MessageID::Log);
    memstream.writeString(message);
    main->sendMessageToMaster(memstream.getMemoryBlock());
    
}

void MainComponent::handleMessageFromMaster (const MemoryBlock & m) {
    
    MemoryInputStream memstream(m, false);
    
    MessageID id = (MessageID)memstream.readInt();
    lock.enter();
    
    switch (id) {
        case Start: {
            processor.reset(new Processor(memstream.readString().toStdString()));
            processor->log = [](void *opaque, const char *msg) {
                log(String(msg));
            };
            poweredOn = processor->prepareToPlay(sampsPerBlock, sampleRate);
            
            if(poweredOn) {
                MemoryOutputStream response;
                response.writeInt(MessageID::Ready);
                sendMessageToMaster(response.getMemoryBlock());
            }
            else {
                MemoryOutputStream response;
                response.writeInt(MessageID::Stop);
                sendMessageToMaster(response.getMemoryBlock());
            }
            break;
        }
        case Stop: {
            poweredOn = false;
            processor.reset(nullptr);
            break;
        }
        case Volume: {
            volume = memstream.readFloat();
            break;
        }
        case LoadParam: {
            if(!poweredOn) break;
            processor->loadData(memstream.readString(), memstream.readInt());
            break;
        }
        case SetParam: {
            if(!poweredOn) break;
            Data destination;
            int idx = memstream.readInt();
            destination = DataStream::readFromStream(memstream);
            processor->setData(idx, destination);
            break;
        }
        case GetParam:
            // TODO: return the value
            break;
            
        case AddProcessor: {
            if(!poweredOn) break;
            if(memstream.readInt() == ProcessorType::AudioPlayerType){
                int IDA = memstream.readInt();
                int IDB = memstream.readInt();
                jassert(IDA == processor->external.size());
                AudioPlayer* player = new AudioPlayer;
                player->ID = IDB;
                processor->external.add(player);
                player->init(processor->getVariablePtr("attach_" + memstream.readString().toStdString()));
                
            }
        }
        case SendProcessor: {
            int idx = memstream.readInt();
            processor->external[idx]->receiveMessage(memstream);
            break;
        }
        default:
            break;
            
   
    }
    lock.exit();
};

void MainComponent::handleConnectionMade() {
};

void MainComponent::handleConnectionLost() {
    exit(0);
};

JUCE_IMPLEMENT_SINGLETON(MainComponent);
