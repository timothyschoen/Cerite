#pragma once

#include <JuceHeader.h>
#include "Processor.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent  : public AudioSource, public ChildProcessSlave
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    std::unique_ptr<Processor> processor = std::unique_ptr<Processor>(nullptr);
    
    SpinLock audioLock;
    
    std::atomic<bool> poweredOn;
    std::atomic<float> volume;
    float lastvolume;
    std::mutex m_mutex;
    
    double phase = 0;
    double sampleRate;
    int sampsPerBlock;
    
    static inline MainComponent* main = nullptr;
    
    static void log(String message);
    
    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;
    
    void setAudioChannels (int numInputChannels, int numOutputChannels);
    void shutdownAudio();
    
    void compile(std::string code);

    JUCE_DECLARE_SINGLETON (MainComponent, true)
    
private:
    
    void handleMessageFromMaster (const MemoryBlock &) override;
    void handleConnectionMade() override;
    void handleConnectionLost() override;
    
    AudioDeviceManager deviceManager;
    AudioSourcePlayer audioSourcePlayer;
    
    SpinLock lock;
    
    
};
