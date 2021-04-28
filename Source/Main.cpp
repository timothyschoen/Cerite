#include <JuceHeader.h>
#include "Engine.hpp"
#include "AudioPlayer.hpp"
#include "NodeConverter.hpp"
//==============================================================================
int main (int argc, char* argv[]) {
    
    auto object_dir = File::getSpecialLocation(juce::File::userDocumentsDirectory).getChildFile("Cerite/Objects");
    
    auto context_dir = object_dir.getChildFile("Contexts");
    
    auto context_files = context_dir.findChildFiles(File::findFiles, false);
    
    ContextMap contexts;
    for(auto& file : context_files) {
        String name = file.getFileName().upToFirstOccurrenceOf(".", false, false);
        contexts[name] = Engine::parse_object(file.loadFileAsString(), name, contexts, true);
    }
    
    
    NodeConverter::initialise(object_dir, contexts);
    

    // TODO: automate this!
    auto sine = Engine::parse_object(object_dir.getChildFile("Signal/sine~.obj").loadFileAsString(), "sine", contexts);
    auto mul  = Engine::parse_object(object_dir.getChildFile("Signal/*~.obj").loadFileAsString(), "mulsig", contexts);
    auto add  = Engine::parse_object(object_dir.getChildFile("Signal/+~.obj").loadFileAsString(), "addsig", contexts);
    auto out  = Engine::parse_object(object_dir.getChildFile("Signal/output~.obj").loadFileAsString(), "output", contexts);
    
    
    auto metro = Engine::parse_object(object_dir.getChildFile("Glue/metro.obj").loadFileAsString(), "metro", contexts);
    
    auto print = Engine::parse_object(object_dir.getChildFile("Glue/print.obj").loadFileAsString(), "print_o", contexts);
    auto print_2 = Engine::parse_object(object_dir.getChildFile("Glue/print_2.obj").loadFileAsString(), "print_o2", contexts);
    
    auto snapshot = Engine::parse_object(object_dir.getChildFile("Signal/snapshot~.obj").loadFileAsString(), "snapshotsig", contexts);
    
    /*
    NodeList n_list = {{sine, {{"dsp", {{0}, {1}}}}, 10, 10},
                       {mul,  {{"dsp", {{1}, {0}, {2, 3}}}}, 10, 10},
                       {add,  {{"dsp", {{3}, {1}, {0}}}}, 10, 10},
                       {out,  {{"dsp", {{2}}}}, 10, 10}
    }; */
    
    /*
    NodeList n_list = { {metro, {{"data", {{0}, {0}, {1, 2}}}}, 60, 10},
                        //{metro, {{"data", {{0}, {0}, {2}}}}, 10, 10},
                        {print,  {{"data", {{1}}}}, 10, 10},
                        {print_2,  {{"data", {{2}}}}, 15, 10}
    }; */
    
    NodeList n_list = {{sine, {
        {"dsp", {{0}, {1}}}}, 10, 10},
        {mul,  {{"dsp", {{1}, {0}, {2, 3}}}}, 10, 10},
        {out,  {{"dsp", {{2}}}}, 10, 10},
        {snapshot, {{"dsp", {{3}}}, {"data", {{1}}}}, 10, 10},
        {print,  {{"data", {{1}}}}, 10, 10}
    };

    auto formatted = NodeConverter::format_nodes(n_list, contexts);
    
    
    String code = "#include \"libcerite.h\" \n\n";
    
    //Engine::set_arguments(objects.getReference(0), "300");
   
    code += Engine::combine_objects(formatted, contexts);
    
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

