//
//  NodeConverter.hpp
//  Cerite_Light - ConsoleApp
//
//  Created by Timothy Schoen on 28/04/2021.
//

#pragma once
#include <JuceHeader.h>
#include "Engine.hpp"

struct Library
{
    inline static ObjectMap contexts, objects;
    
    inline static File object_location, context_location;
    
    static void initialise(File object_dir) {
        context_location = object_dir.getChildFile("Contexts");
        object_location = object_dir;
        refresh();
    }
    
    static void refresh() {
        
        contexts.clear();
        objects.clear();
        
        auto context_files = context_location.findChildFiles(File::findFiles, false);
        for(auto& file : context_files) {
            if(file.getFileExtension() != ".ctx") continue;
            
            String name = file.getFileName().upToFirstOccurrenceOf(".", false, false);
            contexts[name] = Engine::parse_object(file.loadFileAsString(), name, contexts, true);
        }
        
        auto object_dirs = object_location.findChildFiles(File::findDirectories, false);
        
        for(auto & dir : object_dirs) {
            auto object_files = dir.findChildFiles(File::findFiles, false);
            
            for(auto& file : object_files) {
                if(file.getFileExtension() != ".obj") continue;
                
                String name = file.getFileName().upToFirstOccurrenceOf(".", false, false);
                objects[name] = Engine::parse_object(file.loadFileAsString(), encode(name) + "_obj", contexts);
            }
            
        }
    }
    
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
    
};
