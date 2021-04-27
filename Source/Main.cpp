#include <JuceHeader.h>
#include "Engine.hpp"
#include "AudioPlayer.hpp"
#include "Organizer.hpp"
//==============================================================================
int main (int argc, char* argv[]) {
    auto object_dir = File::getSpecialLocation(juce::File::userDocumentsDirectory).getChildFile("Cerite/Objects");
    
    auto context_dir = object_dir.getChildFile("Contexts");
    
    auto context_files = context_dir.findChildFiles(File::findFiles, false);
    
    ContextMap contexts;
    
    
    for(auto& file : context_files) {
        String name = file.getFileName().upToFirstOccurrenceOf(".", false, false);
        contexts[name] = Engine::parse_object(file.loadFileAsString(), name, contexts);
    }
    
    
    Organizer::split_object = Engine::parse_object(object_dir.getChildFile("Glue/split.obj").loadFileAsString(), "split", contexts);
    
    Organizer::splitsig_object = Engine::parse_object(object_dir.getChildFile("Signal/split~.obj").loadFileAsString(), "split", contexts);
    
    String code = "#include \"libcerite.h\" \n\n";
    
    ObjectList objects;
    Array<Array<int>> nodes;
    
    objects.add(Engine::parse_object(object_dir.getChildFile("Glue/metro.obj").loadFileAsString(), "metro", contexts));
    objects.add(Engine::parse_object(object_dir.getChildFile("Glue/print.obj").loadFileAsString(), "print_o", contexts));
    
    nodes.add(Array<int>({0, 0, 1}));
    nodes.add(Array<int>({1}));
    
    
    /* DSP test
     objects.add(parse_object(object_dir.getChildFile("Signal/sine~.obj").loadFileAsString(), "sine", contexts));
     objects.add(parse_object(object_dir.getChildFile("Signal/*~.obj").loadFileAsString(), "mulsig", contexts));
     objects.add(parse_object(object_dir.getChildFile("Signal/output~.obj").loadFileAsString(), "output", contexts));
     
     nodes.add(Array<int>({0, 1}));
     nodes.add(Array<int>({1, 0, 2}));
     nodes.add(Array<int>({2}));
     */
    
    code += Engine::combine_objects(objects, contexts, nodes);
    
    AudioPlayer player(code);
    
    ScopedJuceInitialiser_GUI libraryInitialiser;
    
    while(true) {
        JUCE_TRY
        {
            // loop until a quit message is received..
            MessageManager::getInstance()->runDispatchLoopUntil(1000);
        }
        JUCE_CATCH_EXCEPTION
    }
    return 0;
}

