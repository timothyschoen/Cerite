#pragma once

#include <JuceHeader.h>
#include "ExternalProcessor.h"
#include "Message.h"

enum PlayerMessage {
    Playing,
    Loop,
    Move,
    Load
};

struct AudioPlayer : public ExternalProcessor, public Timer
{
    bool playing = false;
    bool looping = false;
    int numSamples;
    
    int ID;
    std::atomic<int> currentSample = 0;

    AudioFormatManager formatManager;
    std::unique_ptr<AudioFormatReaderSource> readerSource;
    
    double* target;
    int channels;
    
    AudioBuffer<float> buf;
    AudioSourceChannelInfo audiochannel;
    
    AudioPlayer();
    
    ~AudioPlayer() {
        stopTimer();
    }
    
    void init(double* destination) override;
    void tick(int channel = 0) override;
    
    int loadFile(String path);

    void setPosition(int numSamples);
    long long getPosition();

    void receiveMessage(MemoryInputStream& m) override;
    
    void timerCallback() override;
};
