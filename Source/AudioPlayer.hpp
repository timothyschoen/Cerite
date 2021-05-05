//
//  AudioPlayer.hpp
//  Cerite_Light - ConsoleApp
//
//  Created by Timothy Schoen on 27/04/2021.
//

#pragma once


#define NO_CERITE_MACROS
#include "/Users/timschoen/Documents/Cerite/.exec/libcerite.h"

#include <JuceHeader.h>
#include "NodeConverter.hpp"
#include "concurrentqueue.h"

struct AudioPlayer : public AudioSource
{
    moodycamel::ConcurrentQueue<std::function<void()>> queue;
    
    AudioDeviceManager device_manager;
    AudioSourcePlayer source_player;
     
    std::atomic<bool> enabled = false;
    
    ValueTree last_settings;
    
    DynamicLibrary dynlib;
    ChildProcess compiler;
    
    void(*reset)();
    void(*prepare)();
    void(*process)();
    
    void(*register_gui)(int port, void(*)(void*, libcerite::Data, int));
    void(*send_gui)(int port, libcerite::Data);
    
    double*(*get_output)();
    
    std::array<double, 2> audio_output;
    
    std::unique_ptr<XmlElement> audio_settings;
    
    inline static std::array<double, 2> audio_zero = {0, 0};
    
    AudioPlayer();
    
    ~AudioPlayer() {
        receive_callbacks.clear();
    }
    
    void apply_settings(ValueTree settings);
    void compile(Patch patch);
    
    void setAudioChannels (int numInputChannels, int numOutputChannels, XmlElement* state);
    
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;
    
    
    static inline std::vector<std::function<void(libcerite::Data)>> receive_callbacks;
    
    void set_receive_callback(int port, std::function<void(libcerite::Data)> callback);
    void send_data(int port, libcerite::Data data);
    
    static Patch patch_from_tree(ValueTree tree);
    static ValueTree tree_from_patch(Patch patch);
    
    
    inline static AudioPlayer* current_player = nullptr;
};
