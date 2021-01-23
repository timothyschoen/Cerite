#pragma once
#include "Document.h"

#include "Context.h"
#include "Object.h"

#include <unordered_map>
#include <iostream>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

namespace Cerite {

class Library
{

    std::string path;
    
public:
    
    inline static Library* currentLibrary = nullptr;
    
    static std::unordered_map<std::string, Document*> traverseFolder(std::string path, std::string extension) {
        
        Reader reader;
        std::unordered_map<std::string, Document*> result;
        std::vector<Document> docs;
        for (const auto & entry : fs::directory_iterator(path)) {
            if(extension.empty() || entry.path().extension() != extension || entry.path().string().empty())
                continue;
            // this is a cursed system
            Document* doc = &reader.parse(entry.path().string());

            result.insert({doc->name, doc});
            
            for(auto& alias : doc->aliases) {
                Document* copy = &reader.parse(entry.path().string());
                result.insert({alias, copy});
            }
        }

        return result;
        
    }
    
    static void refresh() {
        
        currentLibrary->components.clear();
        currentLibrary->contexts.clear();
        
        std::unordered_map<std::string, Document*> buffer = traverseFolder(currentLibrary->path, ".ctx");
        
        for(auto& ctx : buffer) {
            std::unique_ptr<Context> cptr(dynamic_cast<Context*>(ctx.second));
            
            if(cptr) {
                currentLibrary->contexts.insert({ctx.first, *cptr});
            }
            
        }
        
        buffer = traverseFolder(currentLibrary->path, ".obj");
        
        for(auto& obj : buffer) {
            std::unique_ptr<Object> cptr(dynamic_cast<Object*>(obj.second));
            
            if(cptr) {
                currentLibrary->components.insert({obj.first, *cptr});
            }
        }
    }
    
    
    static std::string readFile(std::string path);
    
    
    static inline std::unordered_map<std::string, Object> components = {};
    static inline  std::unordered_map<std::string, Context> contexts;
    
    static bool isObject(const std::string& name) {
        return components.count(name) > 0;
    }
    
    static bool isContext(const std::string& name) {
        return contexts.count(name) > 0;
    }
    
    Library(std::string location) : path(location) {
        currentLibrary = this;
        refresh();
        
    }
    
    static Document get(const std::string& name, std::vector<double> args = {});
    
    Document& operator[](std::string name)
    {
        return components[name];
    }
    
    
 
};

}
