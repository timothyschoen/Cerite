//
//  AudioPlayer.cpp
//  Cerite_Light - ConsoleApp
//
//  Created by Timothy Schoen on 27/04/2021.
//

#include "AudioPlayer.hpp"

AudioPlayer::AudioPlayer(String code) {
    
    compile(code);
    
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

void AudioPlayer::compile(String code) {
    
    String compiler_name = "clang";
    
    auto path = File::getSpecialLocation (File::SpecialLocationType::currentExecutableFile).getParentDirectory();
    auto patch = path.getChildFile("patch.c");
    
    patch.replaceWithText(code);
    
    String object_location = path.getFullPathName() + "/patch.o ";
    
    String compile_command = compiler_name + " -O2 -c " +  patch.getFullPathName() + " -o " + object_location;
    compiler.start(compile_command);
    compiler.waitForProcessToFinish(-1);
    
    char compiler_error[2048];
    compiler.readProcessOutput(compiler_error, 2048);
    
    int compiler_exit = compiler.getExitCode();
    
#if JUCE_MAC
    String dll = " -fPIC -dynamiclib -o ";
    String location = path.getFullPathName() + "/patch.dylib";
#elif JUCE_LINUX
    String dll = " -fPIC -shared -o ";
    String location = path.getFullPathName() + "/patch.so";
    
#elif JUCE_WINDOWS
    String dll = " -fPIC -shared -o ";
    String location = path.getFullPathName() + "/patch.dll";
#endif
    
    String link_command = compiler_name + dll + location + " " + object_location;
    compiler.start(link_command);
    compiler.waitForProcessToFinish(-1);
    
    char linker_error[2048];
    compiler.readProcessOutput((void*)linker_error, 2048);
    
    int linker_exit = compiler.getExitCode();
    
    bool success = dynlib.open(location);
    
    
    if(!success || compiler_exit || linker_exit) {
        std::cout << "Error compiling patch:" << std::endl;
        std::cout << compiler_error << "\n\n\n" << linker_error << std::endl;
        throw;
    }
    
    reset = (void(*)())dynlib.getFunction("reset");
    process = (void(*)())dynlib.getFunction("calc");
    prepare = (void(*)())dynlib.getFunction("prepare");
    
    get_output = (double*(*)())dynlib.getFunction("get_audio_output");
    
    
    if(!get_output) get_output = [](){ return &audio_zero[0]; };
    
}

void AudioPlayer::setAudioChannels (int numInputChannels, int numOutputChannels)
{
    String audioError;
    
    auto setup = deviceManager.getAudioDeviceSetup();
    
    audioError = deviceManager.initialise (numInputChannels, numOutputChannels, nullptr, true);
    
    jassert (audioError.isEmpty());
    
    deviceManager.addAudioCallback (&audioSourcePlayer);
    audioSourcePlayer.setSource (this);
}


void AudioPlayer::prepareToPlay (int samplesPerBlockExpected, double sampleRate) {
    reset();
    prepare();
}

void AudioPlayer::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) {
    
    for(int i = 0; i < bufferToFill.numSamples; i++) {
        process();
        
        double* output = get_output();
        
        for(int ch = 0; ch < bufferToFill.buffer->getNumChannels(); ch++) {
            bufferToFill.buffer->setSample(ch, i, output[ch]);
        }
    }
}
void AudioPlayer::releaseResources() {
    
}
