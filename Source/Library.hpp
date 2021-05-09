//
//  NodeConverter.hpp
//  Cerite_Light - ConsoleApp
//
//  Created by Timothy Schoen on 28/04/2021.
//

#pragma once
#include <JuceHeader.h>
#include "Engine.hpp"


enum OrderType
{
    none,
    rightToLeft,
    signalOrder
};

struct Library
{
    inline static ObjectMap contexts, objects;
    
    inline static std::map<String, Colour> colours;
    inline static std::map<String, OrderType> ordering;
    
    inline static std::map<std::pair<String, String>, String> conversions;
    
    inline static std::map<String, StringArray> subpatchers_connections;
    
    inline static File object_location, context_location;
    
    static void initialise(File object_dir) {
        context_location = object_dir.getChildFile("Contexts");
        object_location = object_dir;
        
        fileWatcher.addFolder(object_dir);
        fileWatcher.addListener(&fileListener);
        
        refresh();
    }
    
    static void deallocate() {
        contexts.clear();
        objects.clear();
    }
    
    static void refresh() {
        
        contexts.clear();
        objects.clear();
        
        search_folder(context_location);
        search_folder(object_location);
    }
    
    // Recursive folder search
    static void search_folder(File location) {
        
        auto object_files = location.findChildFiles(File::findFiles, false);
        for(auto& file : object_files) {
            String extension = file.getFileExtension();
            bool is_ctx = extension == ".ctx";
            
            auto& destination = is_ctx ? contexts : objects;
            
            if(extension != ".obj" && !is_ctx) continue;
            
            String name = file.getFileName().upToFirstOccurrenceOf(".", false, false);
            destination[name] = Engine::parse_object(file.loadFileAsString(), encode(name) + (is_ctx ? "" : "_obj"), contexts, is_ctx);
            
            String file_content = file.loadFileAsString();
            if(is_ctx) {
                
                auto colour = Colour::fromString(StringArray::fromTokens(Engine::find_section(file_content, "colour"), ",", "")[0]);
                
                auto ctx_order = StringArray::fromTokens(Engine::find_section(file_content, "order"), ",", "")[0];
                
                OrderType ordering_type = (OrderType)StringArray({"none", "rtl", "signal"}).indexOf(ctx_order);

                ordering[name] = ordering_type;
                colours[name] = colour;
                
                auto conversion_definition = Engine::parse_tokens(Engine::find_section(file_content, "convert", true).removeCharacters(" "), ",");
                for(auto& conversion : conversion_definition) {
                    int type_divide = conversion.indexOf("->");
                    int definition_divide = conversion.indexOf("=");
                    String out_type = conversion.substring(0, type_divide);
                    String in_type = conversion.substring(type_divide + 2, definition_divide);
                    String definition = conversion.substring(definition_divide + 1);
                    conversions[{out_type, in_type}] = definition;
                }
                subpatchers_connections[name] = Engine::parse_tokens(Engine::find_section(file_content, "subpatcher.ports", true).removeCharacters(" "), "|");
            }
            auto aliases = StringArray::fromTokens(Engine::find_section(file_content, "alias"), ",", "");
            for(auto& alias : aliases) {
                destination[alias.removeCharacters(" \n\r\t")] = destination[name];
            }
            
            

            
            
            
        }
        
        auto object_dirs = location.findChildFiles(File::findDirectories, false);

        for(auto & dir : object_dirs) {
            search_folder(dir);
        }
    }
    
    // this disables this feature for now as its not ready yet
    static bool can_convert(String type_1, String type_2) {
        return conversions.count({type_1, type_2});
        //return false;
    };
    
    static String encode(const String& input) {
        
        String result;
        for(auto chr : input) {
            if(chr == '*')      result += "mul";
            else if(chr == '/') result += "div";
            else if(chr == '+') result += "add";
            else if(chr == '-') result += "sub";
            else if(chr == '%') result += "mod";
            else if(chr == '&') result += "and";
            else if(chr == '=') result += "eq";
            else if(chr == '~') result += "sig";
            else if(chr == '<') result += "less";
            else if(chr == '>') result += "more";
            else result += chr;
        }
        return result;
    }
    
    
    class FileListener : public gin::FileSystemWatcher::Listener, public Timer
       {
       public:
           FileListener() {};
           // Since all other calls to the library class are made from the message thread,
           // this should secure it from data races
           void folderChanged (const File folder) override {
               startTimer(1000);
           }
           void fileChanged (const File file, gin::FileSystemWatcher::FileSystemEvent fsEvent) override {
               startTimer(1000);
           }
           
           // Use timer to group quick changes together
           void timerCallback() override
           {
               Library::refresh();
               stopTimer();
           }
       };

       inline static gin::FileSystemWatcher fileWatcher;
       inline static FileListener fileListener;

    
};
