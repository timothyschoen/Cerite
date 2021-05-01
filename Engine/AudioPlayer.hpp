//
//  AudioPlayer.hpp
//  Cerite_Light - ConsoleApp
//
//  Created by Timothy Schoen on 27/04/2021.
//

#pragma once
#include <JuceHeader.h>


struct AudioPlayer : public AudioSource
{
    AudioDeviceManager deviceManager;
    AudioSourcePlayer audioSourcePlayer;
    
    DynamicLibrary dynlib;
    ChildProcess compiler;
    
    void(*reset)();
    void(*prepare)();
    void(*process)();
    
    double*(*get_output)();
    
    std::array<double, 2> audio_output;
    
    inline static std::array<double, 2> audio_zero = {0, 0};
    
    AudioPlayer(String code);
    
    void compile(String code);
    
    void setAudioChannels (int numInputChannels, int numOutputChannels);
    
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;
};
