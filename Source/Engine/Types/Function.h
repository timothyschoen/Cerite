#pragma once
#include <string>
#include "Vector.h"
#include "../IO/VarTable.h"
#include "../IO/Token.h"

namespace Cerite {

struct Function
{
    
    std::string name;
    std::string origin;
    
    TokenString body;
    
    std::string args;
    
    Function() : body("") {};
        
    Function(const std::string& c) : body(c) {};

    Function(const TokenString& body) : body(body) {};
    
    void fixIndices(const std::vector<std::pair<int, int>> indices, const std::string& name, int type);
    
    void fixIndices(const std::vector<int> nodes, const std::string& name, int type);
    
    void replaceVariable(const std::string& oldname, const std::string& newname) {
        body.replaceSymbol(oldname, newname, true);
    }
    
    std::string toString(const Document& d) const;
    
    std::string writeInitialiser() const;
    
    bool empty() const {
       return body.size() == 0;
    }
    
};

}
