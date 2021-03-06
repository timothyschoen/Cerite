#include "MainComponent.h"
#include "Message.h"
//==============================================================================
MainComponent::MainComponent()
{
    main = this;
    poweredOn = false;
    
    
    receivedData = malloc(msg_size);
    
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
    free(receivedData);
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
    
    outputLevelMeter = deviceManager.getOutputLevelGetter();
    
}

void MainComponent::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
    if(!processor) poweredOn = false;
    
    // Dequeue to make sure we catch the start message
    MemoryBlock memblock;
    while(receiveMessage(memblock)) {
        handleMessage(memblock);
        memblock.reset();
    }
    
    /*
    // The host get 500 blocks (5 seconds) to return ping
    if(blockCount > 800) {
        if(!pingReturned) {
            //exit(0);
        }
        pingReturned = false;
        MemoryOutputStream pingStream;
        pingStream.writeInt(MessageID::Ping);
        sendMessage(pingStream.getMemoryBlock());
        blockCount = 0;
    }
    
    blockCount++;
    */

    
    if(poweredOn) {
        
        float** writeptr = bufferToFill.buffer->getArrayOfWritePointers();
        const float** readptr = bufferToFill.buffer->getArrayOfReadPointers();
        
        for(int s = 0; s < bufferToFill.buffer->getNumSamples(); s++) {
            
            double output = processor->process(readptr[0][s]);
            for(int c = 0; c < bufferToFill.buffer->getNumChannels(); c++) {
                writeptr[c][s] = output;
            }
            
        }
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

        
        float lvl = outputLevelMeter->getCurrentLevel();
        
        if(lvl != lastlevel) {
            MemoryOutputStream memstream;
            memstream.writeInt(MessageID::Volume);
            memstream.writeFloat(lvl);
            sendMessage(memstream.getMemoryBlock());
            
        }
        
        lastlevel = lvl;
    }
    else {
        if(processor) processor.reset(nullptr);
        
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
    main->sendMessage(memstream.getMemoryBlock());
    
}
    
void MainComponent::handleMessage(const MemoryBlock& m) {
    MemoryInputStream memstream(m, false);
    MessageID id = (MessageID)memstream.readInt();
    
    switch (id) {
        case Ping: {
            pingReturned = true;
            break;
        }
        case Code: {
            receivedCode += memstream.readString();
            break;
        }
        case Start: {
            processor.reset(new Processor(receivedCode.toStdString()));
            receivedCode.clear();
            
            processor->log = [](void *opaque, const char *msg) {
                log(String(msg));
            };
            poweredOn = processor->prepareToPlay(sampsPerBlock, sampleRate);
            
            if(poweredOn) {
                MemoryOutputStream response;
                response.writeInt(MessageID::Ready);
                sendMessage(response.getMemoryBlock());
            }
            else {
                MemoryOutputStream response;
                response.writeInt(MessageID::Stop);
                sendMessage(response.getMemoryBlock());
            }
            break;
        }
        case Stop: {
            poweredOn = false;
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
                player->init(processor->getVariablePtr(memstream.readString().toStdString() + "_attach"));
                
            }
        }
        case SendProcessor: {
            if(!poweredOn) break;
            int idx = memstream.readInt();
            if(idx >= 0 && idx < processor->external.size()) {
                processor->external[idx]->receiveMessage(memstream);
            }
            break;
        }
        default:
            break;
            
   
    }
};


JUCE_IMPLEMENT_SINGLETON(MainComponent);
