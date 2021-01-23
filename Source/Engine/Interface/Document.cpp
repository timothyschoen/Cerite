#include <map>
#include <algorithm>
#include <assert.h>
#include "Document.h"
#include "../IO/Reader.h"
#include "../IO/Writer.h"

#include "../Interface/Library.h"
#include "../Compiler/CodeWriter.h"
#include "../Compiler/Combiner.h"

namespace Cerite {

std::string Document::getText() {
    return Writer::format(*this);
}

// Replace all occurances of a variables in a document
void Document::replaceVarName(std::string oldVar, std::string newVar) {
    
    vtable.replaceSymbol(oldVar, newVar);
    
    
    for(auto& var : variables) {
        if(var.name == oldVar) {
            var.name = newVar;
            var.getOrigin();
        }
    }
    
    for(auto& func : functions) {
        if(func.name == oldVar) {
            func.name = newVar;
        }
        func.body.replaceSymbol(oldVar, newVar, true);
    }
    
    for(auto& vec : vectors) {
        if(vec.name == oldVar) {
            vec.name = newVar;
        }
        for(auto& cell : vec.definition) {
            cell.replaceSymbol(oldVar, newVar);
        }
    }
}

void Document::applyNodes(Vector& v, std::vector<int> nodes, std::string origin) {
    for(auto& func : functions)
    {
        if(func.origin == origin || origin.empty()) {
            func.fixIndices(nodes, v.name, (v.origin == "dsp") + 2 * (v.origin == "data"));
        }
    }
}

void Document::addVariable(Variable var) {
    variables.push_back(var);
}

void Document::addVector(Vector vec) {
    vectors.push_back(vec);
}

void Document::addFunction(Function func) {
    functions.push_back(func);
}


int Document::findVector(const std::string& name) const {
    for(int i = 0; i < vectors.size(); i++) {
        if(vectors[i].name == name) {
            return i;
        }
    }
    return -1;
}
int Document::findFunction(const std::string& name) const {
    for(int i = 0; i < functions.size(); i++) {
        if(functions[i].name == name) {
            return i;
        }
    }
    return -1;
}
int Document::findVariable(const std::string& name) const {
    for(int i = 0; i < variables.size(); i++) {
        if(variables[i].name == name) {
            return i;
        }
    }
    return -1;
}

Vector* Document::getVectorPtr(const std::string& name) {
    return const_cast<Vector*>(std::as_const(*this).getVectorPtr(name));
}
Function* Document::getFunctionPtr(const std::string& name) {
    return const_cast<Function*>(std::as_const(*this).getFunctionPtr(name));
}

Variable* Document::getVariablePtr(const std::string& name) {
    return const_cast<Variable*>(std::as_const(*this).getVariablePtr(name));
}


const Vector* Document::getVectorPtr(const std::string& name) const {
    int pos = findVector(name);
    
    if(pos != -1) {
        return &vectors[pos];
    }
    
    return nullptr;
}
const Function* Document::getFunctionPtr(const std::string& name) const {
    int pos = findFunction(name);
    
    if(pos != -1) {
        return &functions[pos];
    }
    
    return nullptr;
}

const Variable* Document::getVariablePtr(const std::string& name) const {
    int pos = findVariable(name);
    
    if(pos != -1) {
        return &variables[pos];
    }
    
    return nullptr;
}

}
