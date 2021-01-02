#pragma once
#include "Document.h"
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
    
    static std::unordered_map<std::string, Document> traverseFolder(std::string path, std::string extension) {
        
        Reader reader;
        std::unordered_map<std::string, Document> result;
        std::vector<Document> docs;
        for (const auto & entry : fs::directory_iterator(path)) {
            if(extension.empty() || entry.path().extension() != extension || entry.path().string().empty())
                continue;            
            Document doc = reader.parse(entry.path().string());

            result.insert({doc.name, doc});
        }

        return result;
        
    }
    
    static void refresh() {
        currentLibrary->contexts = traverseFolder(currentLibrary->path, ".ctx");
        currentLibrary->components = traverseFolder(currentLibrary->path, ".obj");
    }
    
    
    static std::string readFile(std::string path);
    
    
    static inline std::unordered_map<std::string, Document> components = {};
    static inline  std::unordered_map<std::string, Document> contexts;
    
    static bool isObject(std::string name) {
        return components.count(name) > 0;
    }
    
    static bool isContext(std::string name) {
        return contexts.count(name) > 0;
    }
    
    Library(std::string location) : path(location) {
        currentLibrary = this;
        contexts = traverseFolder(location, ".ctx");
        components = traverseFolder(location, ".obj");
        
    }
    
    static Document get(std::string name, std::vector<double> args = {});
    
    Document& operator[](std::string name)
    {
        return components[name];
    }
    
    
 
};

}
