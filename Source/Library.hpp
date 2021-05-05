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
    
    inline static File object_location, context_location;
    
    static void initialise(File object_dir) {
        context_location = object_dir.getChildFile("Contexts");
        object_location = object_dir;
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
            
            if(is_ctx) {
                
                auto colour = Colour::fromString(StringArray::fromTokens(Engine::find_section(file.loadFileAsString(), "colour"), ",", "")[0]);
                
                auto ctx_order = StringArray::fromTokens(Engine::find_section(file.loadFileAsString(), "order"), ",", "")[0];
                
                OrderType ordering_type = (OrderType)StringArray({"none", "rtl", "signal"}).indexOf(ctx_order);

                ordering[name] = ordering_type;
                colours[name] = colour;
            }
            auto aliases = StringArray::fromTokens(Engine::find_section(file.loadFileAsString(), "alias"), ",", "");
            for(auto& alias : aliases) {
                destination[alias.removeCharacters(" \n\r\t")] = destination[name];
            }
        }
        
        auto object_dirs = location.findChildFiles(File::findDirectories, false);

        for(auto & dir : object_dirs) {
            search_folder(dir);
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
