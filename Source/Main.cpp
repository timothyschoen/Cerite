#include <JuceHeader.h>
#include "Engine.hpp"
#include "AudioPlayer.hpp"
#include "NodeConverter.hpp"
#include "Library.hpp"


//==============================================================================
int main (int argc, char* argv[]) {
    
    auto object_dir = File::getSpecialLocation(juce::File::userDocumentsDirectory).getChildFile("Cerite/Objects");
    
   
    Library::initialise(object_dir);
    NodeConverter::initialise(object_dir, Library::contexts);
    
    auto& contexts = Library::contexts;
    auto& objects = Library::objects;

    

    /*
    NodeList n_list = {{objects["sine~"], {{"dsp", {{0}, {1}}}}, 10, 10},
                       {objects["mul~"],  {{"dsp", {{1}, {0}, {2, 3}}}}, 10, 10},
                       {objects["add~"],  {{"dsp", {{3}, {1}, {0}}}}, 10, 10},
                       {objects["out~"],  {{"dsp", {{2}}}}, 10, 10}
    }; */
    
    /*
    NodeList n_list = { {objects["metro"], {{"data", {{0}, {0}, {1, 2}}}}, 60, 10},
                        {objects["print"],  {{"data", {{1}}}}, 10, 10},
                        {objects["print"],  {{"data", {{2}}}}, 15, 10}
    }; */
    
    /*
    NodeList n_list = {
        {objects["sine~"], {{"dsp", {{0}, {1}}}}, 10, 10},
        {objects["mul~"],  {{"dsp", {{1}, {0}, {2, 3}}}}, 10, 10},
        {objects["output~"],  {{"dsp", {{2}}}}, 10, 10},
        {objects["snapshot~"], {{"dsp", {{3}}}, {"data", {{1}}}}, 10, 10},
        {objects["print"],  {{"data", {{1}}}}, 10, 10}
    };*/
    
    /*
    NodeList n_list = {
        {objects["metro"],     {{"data", {{0}, {0}, {1}}}}, 10, 10},
        {objects["pack"],      {{"data", {{1}, {0}, {2}}}}, 60, 10},
        {objects["unpack"],    {{"data", {{2}, {3}, {0}}}}, 10, 10},
        {objects["print"],     {{"data", {{3}}}}, 15, 10}
     
     Engine::set_arguments(std::get<0>(n_list[1]), "b i");
     Engine::set_arguments(std::get<0>(n_list[2]), "b i");
    }; */
    
    NodeList n_list = {
        {objects["metro"],     {{"data", {{0}, {0}, {1}}}}, 10, 10},
        {objects["select"],       {{"data", {{1}, {2}}}}, 60, 10},
        {objects["trigger"],   {{"data", {{2}, {3}, {4}}}}, 60, 10},
        {objects["print"],     {{"data", {{3}}}}, 15, 10},
        {objects["delay"],     {{"data", {{4}, {5}}}}, 15, 10},
        {objects["print"],   {{"data", {{5}}}}, 15, 10}
    };
        
    Engine::set_arguments(std::get<0>(n_list[1]), "bang");
    Engine::set_arguments(std::get<0>(n_list[2]), "b b");
        
    auto formatted = NodeConverter::format_nodes(n_list, contexts);
    
    String code = "#include \"libcerite.h\" \n\n";
   
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

