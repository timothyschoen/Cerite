#pragma once
#include <functional>
#include <any>
#include <string.h>
#include <unordered_map>
#include <libtcc.h>

#include "../Types/Vector.h"
#include "../Types/Variable.h"


namespace Cerite {

typedef void (*fptr)();

class Document;
class Object
{
    
public:
    
    std::unordered_map<std::string, fptr> functions;
    std::vector<Vector> vectors;
    std::vector<Variable> variables;
    
    TCCState* state = nullptr;
    std::unique_ptr<Document> document;
    
    std::string finalcode;
    
    std::string name;
    int nets;
    
    Object(const Document& doc);
    
    ~Object();
    
    fptr loadFunction(const std::string& name) {
        if(functions.count(name) > 0) {
            return functions[name];
        }
        else {
            return [](){};
        }
    }
    
    void* getFunctionPtr(const std::string& name) {
        return state ? tcc_get_symbol(state, name.c_str()) : nullptr;
    }
    
    void setVariable(const std::string& name, double value) {
        for(auto& var : variables)
            if(var.name == name)
                *var.value = value;
    }
    
    double* getVariablePtr(const std::string& name) {
        for(auto& var : variables)
            if(var.name == name)
                return var.value;
        
        for(auto& vec : vectors)
            if(vec.name == name)
                return vec.start;
        
        return nullptr;
    }
};

}
