#pragma once

#include <JuceHeader.h>
#include <boost/interprocess/ipc/message_queue.hpp>
#include "Processor.h"
#include "concurrentqueue.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/

using namespace boost::interprocess;


class MainComponent  : public AudioSource
{
public:
    
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    std::unique_ptr<Processor> processor = std::unique_ptr<Processor>(nullptr);
    
    ReferenceCountedObjectPtr<AudioDeviceManager::LevelMeter> outputLevelMeter;
    
    std::atomic<bool> poweredOn;
    std::atomic<float> volume;
    float lastvolume;
    float lastlevel;
    
    
    double phase = 0;
    double sampleRate;
    int sampsPerBlock;
    
    void sendMessage(MemoryBlock m) {
        
        if(m.getSize() > msg_size)
        {
            std::cout << "message too big!" << std::endl;
        }
        unsigned int priority;
       
        m.ensureSize(msg_size);
        // Use try to prevent blocking the audio thread
        sendQueue.try_send(m.getData(), msg_size, priority);
    }
    
    bool receiveMessage(MemoryBlock& m)
    {
        message_queue::size_type recvd_size;
        
        unsigned int priority;
        
        if(receiveQueue.try_receive(receivedData, msg_size, recvd_size, priority))
        {
            m.replaceWith(receivedData, msg_size);
            return true;
        }
        
        return false;
    }
    

    static inline MainComponent* main = nullptr;
    
    static void log(String message);
    
    const size_t msg_size = 1 << 10;
    
    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;
    
    void setAudioChannels (int numInputChannels, int numOutputChannels);
    void shutdownAudio();
    
    void compile(std::string code);

    JUCE_DECLARE_SINGLETON (MainComponent, true)
    
private:
    
    void* receivedData;
    
    // Names are inverted here so they look regular on the main process
    message_queue receiveQueue = message_queue(open_only ,"cerite_send");
    message_queue sendQueue = message_queue(open_only,"cerite_receive");
    
    
    String receivedCode;
    
    void handleMessage(const MemoryBlock& m);
    
    AudioDeviceManager deviceManager;
    AudioSourcePlayer audioSourcePlayer;
    
    bool pingReturned = true;
    int blockCount = 0;
};
