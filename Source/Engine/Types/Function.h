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
    
    bool local = true;
    
    TokenString body;
    
    std::string args;
    
    Function() : body("") {};
        
    Function(const std::string& c, bool loc) : body(c) , local(loc) {};

    Function(const TokenString& body, bool loc) : body(body), local(loc) {};
    
    void fixIndices(const std::vector<std::pair<int, int>> indices, const std::string& name, int type);
    
    void replaceVariable(const std::string& oldname, const std::string& newname) {
        body.replaceSymbol(oldname, newname, true);
    }
    
    bool empty() const {
       return body.size() == 0;
    }
    
};

}
