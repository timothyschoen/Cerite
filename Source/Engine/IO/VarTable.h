#pragma once

#include <unordered_map>
#include <vector>
#include <string>
#include <algorithm>

namespace Cerite {

enum TokenType {
    tConst,
    tVector,
    tVariable,
    tFunction,
    tPreprocessor,
    tOperator,
    tSpacing,
    tComment,
    tEnd,
    tUnknown
};

struct VarTable
{

    int defaultSize = 0;
    
    std::vector<std::string> initializers = {"const", "var"};
    std::vector<std::pair<std::string, std::string>> functypes = {};
    std::vector<std::pair<std::string, std::string>> vectypes = {};
    std::vector<std::pair<std::string, std::string>> vartypes = {};
    std::vector<std::string> spectypes = {"name", "update"};
    
    static inline const std::string operators = "+-=*:!/%|&^";
    static inline const std::string space = "\f\n\r\t\v  ";
    static inline const std::string parens = "()";
    static inline const std::string digits = "1234567890.-e";
    static inline const std::string integers = "1234567890";
    
    VarTable() {};
    
    bool isVariableChar(const char c) const {
        return std::isdigit(c) || std::isalpha(c) || c == '_';
    }
    
    void replaceSymbol(const std::string& oldname, const std::string& newname) {
        for(auto& func : functypes)
            if(func.first == oldname) func.first = newname;
        
        for(auto& vec : vectypes)
            if(vec.first == oldname) vec.first = newname;
        
        for(auto& var : vartypes)
            if(var.first == oldname) var.first = newname;
    }
    
    TokenType getType(const std::string& name) const {
        if(isVariable(name))
            return tVariable;
        
        if(isVector(name))
            return tVector;
        
        if(isFunction(name))
            return tFunction;
        
        return tUnknown;
    }
    
    
    bool isInitializer(const std::string& selector) const {
        return std::find(initializers.begin(), initializers.end(), std::string(selector)) != initializers.end();
    }
    
    bool isFunction(const std::string& selector) const {
        return std::find_if(functypes.begin(), functypes.end(), [&selector] (const std::pair<std::string, std::string>& s) { return s.first == selector; }) != functypes.end();
    }
    
    bool isVector(const std::string& selector) const {
        return std::find_if(vectypes.begin(), vectypes.end(), [&selector] (const std::pair<std::string, std::string>& s) { return s.first == selector; }) != vectypes.end();
    }
    
    bool isVariable(const std::string& selector) const {
        return std::find_if(vartypes.begin(), vartypes.end(), [&selector] (const std::pair<std::string, std::string>& s) { return s.first == selector; }) != vartypes.end();
    }
    
    bool isSpecial(const std::string& selector) const {
        return selector.find("_size") != std::string::npos || std::find(spectypes.begin(), spectypes.end(), std::string(selector)) != spectypes.end();
    }
    
    std::string getOrigin(const std::string& selector) const {
        
        for(auto& func : functypes)
            if(func.first == selector) return func.second;
        
        for(auto& vec : vectypes)
            if(vec.first == selector) return vec.second;
        
        for(auto& var : vartypes)
            if(var.first == selector) return var.second;
    
        return "";
    }
    
    void combineWith(const VarTable& table) {
        for(auto& func : table.functypes) functypes.push_back(func);
        for(auto& vec : table.vectypes) vectypes.push_back(vec);
        for(auto& var : table.vartypes) vartypes.push_back(var);
    }
    
    
    std::vector<std::string> getAll() const {
        std::vector<std::string> sections = initializers;
        sections.insert(sections.end(), spectypes.begin(), spectypes.end());
        
        for(auto& func : functypes) sections.push_back(func.first);
        for(auto& vec : vectypes) sections.push_back(vec.first);
        
        
        for(int i = 0; i < 16; i++) sections.push_back(std::to_string(i));
        //for(auto& var : vartypes) sections.insert(sections.end(), var.first);
        
        
        return sections;
    }

};

}
