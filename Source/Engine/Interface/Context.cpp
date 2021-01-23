#include <map>
#include <algorithm>
#include <assert.h>
#include "Context.h"
#include "../IO/Reader.h"
#include "../IO/Writer.h"

#include "Object.h"
#include "Library.h"
#include "../Compiler/CodeWriter.h"
#include "../Compiler/Combiner.h"

namespace Cerite {

std::vector<int> Context::getNodes(Object* obj) const
{
    std::pair<int, int> ports = getPorts(obj);
    std::vector<int> nodes;
    int total = 0;
    
    for(int i = 0; i < ports.first; i++) {
        nodes.push_back(total);
        total++;
    }
    
    for(int i = 0; i < ports.second; i++) {
        nodes.push_back(total);
        total++;
    }
    
    return nodes;
}

void Context::setSize(int newSize) {
    size = newSize;
    
    for(auto& vector : vectors)
    {
        vector.size = newSize;
    }
    
}

std::pair<int, int> Context::getPorts(Object* obj) const {
    
    std::pair<int, int> ports;
    
    TokenString pstr = portstring;
    std::vector<std::string> split;
   
    for(auto& token : pstr)
    {
        if(token.type == tVariable || token.type == tConst && !token.empty()) {
            split.push_back(token.symbol);
        }
    }
    assert(split.size() == 2);
    
    if(obj != nullptr) {
        ports = {obj->parseExpression(split[0]), obj->parseExpression(split[1])};
    }
    else {
        ports = {std::stoi(split[0]), std::stoi(split[1])};
    }
    
    return ports;
}

void Context::addToContext(Context& result, std::vector<int> nodes, Object* obj, Object* resultObj)
{

    
    for(auto& vec : vectors) {
        std::string newname = name + "_" + vec.name;
        
        resultObj->replaceVarName(vec.name, newname);
        obj->replaceVarName(vec.name, newname);
    
        resultObj->applyNodes(vec, nodes, obj->name);
        obj->applyNodes(vec, nodes, obj->name);
        
        int pos = result.findVector(newname);
        if(pos == -1) {
            result.addVector({newname, result.size, vec.dims, false});
            vec.overlap(result.vectors.back(), result.size, nodes);
        }
        else {
            vec.overlap(result.vectors[pos], result.size, nodes);
        }
    }
    
    for(auto& var : variables) {
        std::string newname = name + "_" + var.name;
        obj->replaceVarName(var.name, newname);
        
        
        int pos = result.findVariable(newname);
        if(pos == -1) {
            var.name = newname;
            result.addVariable(var);
        }
        else if(var.predefined && var.init != result.variables[pos].init) {
            std::cout << "Warning: confliting values for " << var.name << " in import " << name << std::endl;
        }
    }
    
    for(auto& func : functions) {
        std::string newname = name + "_" + func.name;
        obj->replaceVarName(func.name, newname);
        int pos = result.findFunction(newname);
        if(pos == -1) {
            func.name = newname;
            result.addFunction(func);
        }
        else {
            result.functions[pos].body.append(func.body.tokens);
            
        }
    }
    
    
    for(auto& func : defaultFunctions) {
        int pos = -1;
        for(int i = 0; i < result.defaultFunctions.size(); i++) {
            if(result.defaultFunctions[i].name == func.name) {
                pos = i;
            }
        }
        if(pos == -1) {
            result.defaultFunctions.push_back(func);
        }
    }
    
}


Context Context::combine(std::string type, std::vector<Context*> contexts, std::vector<Object*> objects, std::vector<std::vector<int>> nodes, Object* resultObj, int newSize)
{
    Context result;
    
    result.size = newSize;

    for(int i = 0; i < contexts.size(); i++)
    {
        if(contexts[i]->name == type) {
            contexts[i]->addToContext(result, nodes[i], objects[i], resultObj);
        }
    }
    
    return result;
}

void Context::replaceVarName(std::string oldVar, std::string newVar)
{
    Document::replaceVarName(oldVar, newVar);
    
    for(auto& func : defaultFunctions) {
        if(func.name == oldVar) {
            func.name = newVar;
        }
        func.body.replaceSymbol(oldVar, newVar, true);
    }
    
    portstring.replaceSymbol(oldVar, newVar, true);
}

}
