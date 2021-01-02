#pragma once
#include "Document.h"
#include "Library.h"
namespace Cerite {


struct Parser
{
    /*
    
    static Document parseNetlist(std::string stringToParse, Library& lib) {
        std::vector<std::string> lines;
        
        // First split by newline and semicolon
        boost::split(lines, stringToParse, boost::is_any_of("\n;"));
        
        std::string name = lines[0];
        lines.erase(lines.begin());
        
        std::vector<Document> parts(lines.size());
        std::vector<std::vector<int>> nodes(lines.size());
        
        for(int i = 0; i < lines.size(); i++) {
            std::vector<std::string> words;
            boost::split(words, lines[i], ::isspace);
            
            // remove empty lines
            for(int i = 0; i < words.size(); i++)
            if(words[i].empty())
                words.erase(words.begin() + i);
            
            parts[i] = lib.get(words[0]);
            
            size_t numPorts = parts[i].ports.size();
            size_t numArgs = words.size() - numPorts - 1;
            
            nodes[i].resize(numPorts);
            
            if(numPorts == 0) continue;
            
            // Get node indices
            for(int j = 0; j < numPorts; j++) {
                nodes[i][j] = std::stoi(words[j + 1]);
            }
            
            // Get arguments
            for(size_t j = numPorts; j < numPorts + numArgs; j++) {
                parts[i].setArgument(j - numPorts, std::stod(words[j + 1]));
            }
        }
        
        return Document::concat(name, parts, nodes);
    };
    
    
    // TODO
    static Document parseSpice(std::string stringToParse, Library& lib) {
        return Document();
    }
     */
};

}
