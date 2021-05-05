//
//  AudioPlayer.cpp
//  Cerite_Light - ConsoleApp
//
//  Created by Timothy Schoen on 27/04/2021.
//


#include "AudioPlayer.hpp"
#include "Engine.hpp"
#include "Library.hpp"

AudioPlayer::AudioPlayer() {
    current_player = this;
}

void AudioPlayer::apply_settings(ValueTree settings)
{
    last_settings = settings;
    audio_settings = settings.getChildWithName("Audio").getChild(0).createXml();
    
    if (RuntimePermissions::isRequired (RuntimePermissions::recordAudio)
        && ! RuntimePermissions::isGranted (RuntimePermissions::recordAudio))
    {
        RuntimePermissions::request (RuntimePermissions::recordAudio,
                                     [&] (bool granted) { setAudioChannels (granted ? 2 : 0, 2, audio_settings.get()); });
    }
    else
    {
        // Specify the number of input and output channels that we want to open
        setAudioChannels (2, 2, audio_settings.get());
    }
}


void AudioPlayer::compile(Patch patch) {
    
    
    receive_callbacks.clear();
    
    auto tree = tree_from_patch(patch);
    auto patch_2 = patch_from_tree(tree);
   // auto path = File("/Users/timschoen/Cerite_Light/Builds/MacOSX/build/Debug"); //File::getSpecialLocation (File::SpecialLocationType::currentExecutableFile).getParentDirectory();
    
    // Generate code for patch
    auto objects = NodeConverter::create_objects(patch_2);
    
    auto formatted = NodeConverter::format_nodes(objects, Library::contexts);
    
    auto header = File::getSpecialLocation(File::SpecialLocationType::userDocumentsDirectory).getChildFile("Cerite").getChildFile(".exec").getChildFile("libcerite.h");

    String code = "#include \"" + header.getFullPathName() + "\" \n\n";
   
    code += Engine::combine_objects(formatted, Library::contexts);
    
    
    auto code_tree = last_settings.getChildWithName("Code");
    String compiler_name = code_tree.getProperty("Compiler");
    String optimization = code_tree.getProperty("Optimization");
    
    auto patch_c = File::createTempFile(".c");
    auto patch_o = File::createTempFile(".o");
    
    patch_c.replaceWithText(code);
    
    Uuid id;
    
   
    String compile_command = compiler_name + " " + optimization + " -c " +  patch_c.getFullPathName() + " -o " + patch_o.getFullPathName();
    compiler.start(compile_command);
    compiler.waitForProcessToFinish(-1);
    
    char compiler_error[2048];
    compiler.readProcessOutput(compiler_error, 2048);
    
    int compiler_exit = compiler.getExitCode();
    
#if JUCE_MAC
    String dll = " -fPIC -dynamiclib -o ";
    auto dll_file = File::createTempFile(".dylib");
#elif JUCE_LINUX
    String dll = " -fPIC -shared -o ";
    auto dll_file = File::createTempFile(".so");
    
#elif JUCE_WINDOWS
    String dll = " -fPIC -shared -o ";
    auto dll_file = File::createTempFile(".dll");
#endif
   
    String link_command = compiler_name + dll + dll_file.getFullPathName() + " " + patch_o.getFullPathName();
    compiler.start(link_command);
    compiler.waitForProcessToFinish(-1);
    
    char linker_error[2048];
    compiler.readProcessOutput((void*)linker_error, 2048);
    
    int linker_exit = compiler.getExitCode();
    
    bool success = dynlib.open(dll_file.getFullPathName());
    
    patch_o.deleteFile();
    dll_file.deleteFile();
    
#ifndef JUCE_DEBUG
    patch_c.deleteFile();
#endif
    
    if(!success || compiler_exit || linker_exit) {
        std::cout << "Error compiling patch:" << std::endl;
        std::cout << compiler_error << "\n\n\n" << linker_error << std::endl;
        throw;
    }
    
    device_manager.getAudioCallbackLock().enter();
    
    reset = (void(*)())dynlib.getFunction("reset");
    process = (void(*)())dynlib.getFunction("calc");
    prepare = (void(*)())dynlib.getFunction("prepare");
    
    get_output = (double*(*)())dynlib.getFunction("get_audio_output");
    
    register_gui = (void(*)(int port, void(*)(void*, libcerite::Data, int)))dynlib.getFunction("gui_external_register");
    send_gui = (void(*)(int port, libcerite::Data)) dynlib.getFunction("gui_external_send");
    
    if(!register_gui) register_gui = [](int port, void(*)(void*, libcerite::Data,  int)){};
    if(!send_gui) send_gui = [](int, libcerite::Data){};
    
    if(!get_output) get_output = [](){ return &audio_zero[0]; };
    
    reset();
    prepare();
    
    device_manager.getAudioCallbackLock().exit();
}

void AudioPlayer::setAudioChannels (int numInputChannels, int numOutputChannels, XmlElement* state)
{
    String audioError;
    
    //auto setup = device_manager.getAudioDeviceSetup();
    
    audioError = device_manager.initialise (numInputChannels, numOutputChannels, state, false);
    
    jassert (audioError.isEmpty());
    
    device_manager.addAudioCallback (&source_player);
    source_player.setSource (this);
}


void AudioPlayer::prepareToPlay (int samplesPerBlockExpected, double sampleRate) {

}

void AudioPlayer::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) {
    
    
    std::function<void()> update;
    if(queue.try_dequeue(update)) {
        update();
    }
    
    if(!enabled)  {
        bufferToFill.clearActiveBufferRegion();
        return;
    }
    
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

// std::vector<std::tuple<String, int, int, std::map<String, std::vector<std::vector<int>>>>>;
Patch AudioPlayer::patch_from_tree(ValueTree tree) {
    Patch result;
    
    for(auto obj : tree) {
        
        std::tuple<String, String, int, int, std::map<String, std::vector<std::vector<int>>>> patch_object;
        
        auto& [name, id, x, y, nodes] = patch_object;
        
        name = obj.getProperty("Name");
        id = obj.getProperty("ID");
        x = obj.getProperty("X");
        y = obj.getProperty("Y");
        
        for(auto type : obj.getChildWithName("Nodes")) {
            String type_name = type.getType().toString();
            nodes.insert({type_name, {}});
            auto& node_vector = nodes[type_name];
            for(auto nodes_i : type) {
                
                int num_nodes = nodes_i.getNumProperties();
                node_vector.push_back(std::vector<int>(num_nodes, 0));
                int i = type.indexOf(nodes_i);
                
                for(int j = 0; j < num_nodes; j++) {
                    node_vector[i][j] = (int)nodes_i.getProperty(String(j));
                }
            }
            
        }
        
        result.push_back(patch_object);
    }
    
    return result;
}

ValueTree AudioPlayer::tree_from_patch(Patch patch) {
    ValueTree result("Patch");
    
    for(auto& [name, id, x, y, nodes] : patch) {
        ValueTree obj_tree("Object");
        
        obj_tree.setProperty("Name", name, nullptr);
        obj_tree.setProperty("ID", id, nullptr);
        obj_tree.setProperty("X", x, nullptr);
        obj_tree.setProperty("Y", y, nullptr);
        
        ValueTree node_tree("Nodes");
        
        for(auto& node : nodes) {
            ValueTree n_list(node.first);
            
            for(int i = 0; i < node.second.size(); i++) {
                ValueTree ni_list((String(i)));
                
                for(int j = 0; j < node.second[i].size(); j++) {
                    ni_list.setProperty(String(j),  node.second[i][j], nullptr);
                }
                
                n_list.appendChild(ni_list, nullptr);
            }
            node_tree.appendChild(n_list, nullptr);
        }
        
        obj_tree.appendChild(node_tree, nullptr);
        result.appendChild(obj_tree, nullptr);
    }
    
    
    return result;
}

void AudioPlayer::set_receive_callback(int port, std::function<void(libcerite::Data)> callback)
{
    queue.enqueue([this, port, callback]() mutable {
        if(receive_callbacks.size() <= port) {
            receive_callbacks.resize(port + 1);
        }
        
        receive_callbacks[port] = callback;

        register_gui(port, [](void*, libcerite::Data data, int port) {
            receive_callbacks[port](data);
            
        });
    });
}

void AudioPlayer::send_data(int port, libcerite::Data data)
{
    queue.enqueue([this, port, data]() mutable {
        send_gui(port, data);
    });
}
