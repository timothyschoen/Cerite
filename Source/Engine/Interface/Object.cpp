#include <map>
#include <algorithm>
#include <assert.h>
#include "Object.h"
#include "../IO/Reader.h"
#include "../IO/Writer.h"

#include "../Interface/Library.h"
#include "../Compiler/CodeWriter.h"
#include "../Compiler/Combiner.h"

namespace Cerite {


std::string Object::getText() {
    return Writer::format(*this);
}

Object Object::withArgument(const std::string& name, double value) const {
    Object newdoc = Object(*this);
    newdoc.setArgument(name, value);
    
    return newdoc;
}

Object Object::withArgument(int idx, double value) const {
    Object newdoc = Object(*this);
    newdoc.setArgument(idx, value);
    return newdoc;
}

int Object::getArgumentPosition(int idx)
{
    auto pos = std::find_if(variables.begin(), variables.end(), [this, idx](const Variable& v) {
        return v.argpos == idx;
    });
    
    if(pos == variables.end()) return -1;

    return pos - variables.begin();
}

void Object::setArgument(const std::string& argname, double val, bool removeVariable) {
    std::ostringstream stream;
    stream << val;
    applyArgument(findVariable(argname), stream.str(), removeVariable);
}

void Object::setArgument(int idx, double val, bool removeVariable) {
    std::ostringstream stream;
    stream << val;
    setArgument(idx, stream.str(), removeVariable);
}

void Object::setArgument(int idx, const std::string& val, bool removeVariable) {
    applyArgument(getArgumentPosition(idx), val, removeVariable);
}

void Object::applyArgument(int position, std::string val, bool removeVariable) {
    
    if(position == -1)  {
        nArgs++;
        varArgs.push_back(val);
        return;
    }
    
    Variable& v = variables[position];
    // Force special cases where ints are used to create inlets...
    if(v.ctype == "int" && !val.empty() && std::all_of(val.begin(), val.end(), ::isdigit)) {
        v.predefined = true;
        v.init = std::stoi(val);
    }
    
    replaceVarName(v.name, val);
    
    if(removeVariable) {
        variables.erase(findVariable(v.name) + variables.begin());
    }
}

// Replace all occurances of a variables in a Object
void Object::replaceVarName(std::string oldVar, std::string newVar) {
    Document::replaceVarName(oldVar, newVar);
    
    for(auto& [key, ctx] : imports) {
        ctx.replaceVarName(oldVar, newVar);
    }    
}

Vector* Object::getVectorPtr(const std::string& name) {
    return const_cast<Vector*>(std::as_const(*this).getVectorPtr(name));
}
Function* Object::getFunctionPtr(const std::string& name) {
    return const_cast<Function*>(std::as_const(*this).getFunctionPtr(name));
}

Variable* Object::getVariablePtr(const std::string& name) {
    return const_cast<Variable*>(std::as_const(*this).getVariablePtr(name));
}


const Vector* Object::getVectorPtr(const std::string& name) const {
    const Vector* result = Document::getVectorPtr(name);
    if(result) return result;
    
    for(auto& [key, import] : imports) {
        result = import.getVectorPtr(name);
        if(result) return result;
    }
    
    return nullptr;
}
const Function* Object::getFunctionPtr(const std::string& name) const {
    const Function* result = Document::getFunctionPtr(name);
    if(result) return result;
    
    for(auto& [key, import] : imports) {
        result = import.getFunctionPtr(name);
        if(result) return result;
    }
    
    return nullptr;
}

const Variable* Object::getVariablePtr(const std::string& name) const {
    const Variable* result = Document::getVariablePtr(name);
    if(result) return result;
    
    for(auto& [key, import] : imports) {
        result = import.getVariablePtr(name);
        if(result) return result;
    }
    
    return nullptr;
}

void Object::combine(Object& result)
{
    
    for(auto& vec : vectors) {
        std::string newname = name + "_" + vec.name;
        vec.origin = name;
        result.replaceVarName(vec.name, newname);
        replaceVarName(vec.name, newname);
        
        int pos = result.findVector(newname);
        if(pos == -1) {
            result.addVector(vec);
        }
    }
    
    
    for(auto& var : variables) {
        std::string newname = name + "_" + var.name;
        var.origin = name;
        result.replaceVarName(var.name, newname);
        replaceVarName(var.name, newname);
       
        int pos = result.findVariable(newname);
        if(pos == -1)
        {
            result.addVariable(var);
        }
        else if(var.predefined && var.init != result.variables[pos].init) {
            std::cout << "Warning: confliting values for " << var.name << " in import " << name << std::endl;
        }
    }


    
    for(auto& func : functions) {
        std::string newname = name + "_" + func.name;
        func.origin = name;
        result.replaceVarName(func.name, newname);
        replaceVarName(func.name, newname);
        
        int pos = result.findFunction(func.name);
        if(pos == -1) {
            result.addFunction(func);
        }
    }
}
 Object Object::combineObjects(const std::string& name, std::vector<Object>& objects, NodeList& nodes, std::vector<int> ports)
{
    Object result;
    
    std::map<std::string, int> ctxnames = {{"mna", 0}, {"dsp", 1}, {"data", 2}};
    
    result.name = name;

    std::map<std::string, int> sizes;
    
    std::vector<int> nets(ctxnames.size(), 0);
    // find all unique node numbers excluding 0
    for(auto node : nodes) {
        for(auto [num, type] : node) {
            nets[type] = std::max(nets[type], num);
        }
    }
    
    std::map<std::string, std::vector<Context*>> contexts;
    std::map<std::string, std::vector<Object*>> sortedObjects;
    
    std::map<std::string, std::vector<std::vector<int>>> nodelist;
    
    for(int i = 0; i < objects.size(); i++)
    {
        CodeWriter::handleVariableInput(objects[i]);
        
        for(auto& key : objects[i].contexts)
        {
            auto& ctx = objects[i].imports[key];
            int d = ctxnames[key];
            int internal = ctx.size - ctx.getNodes(&objects[i]).size();
            
            for(int n = 0; n < internal; n++) {
                nets[d]++;
                nodes[i].push_back({nets[d], d});
            }
            
            if(contexts.count(key) == 0) {
                contexts.insert({key, std::vector<Context*>()});
                sortedObjects.insert({key, std::vector<Object*>()});
            }
            
            contexts[key].push_back(&ctx);
            sortedObjects[key].push_back(&objects[i]);
            std::vector<int> ports;
            
            for(auto& node : nodes[i]) {
                if(node.second == ctxnames[key]) {
                    ports.push_back(node.first - 1);
                }
            }
            
            nodelist[key].push_back(ports);
        }
    }
    
    std::vector<std::string> names;
    for(auto& obj : objects) {
        // Make sure each Object has a unique name
        std::string oldname = obj.internalName.substr(0, 3);
        
        obj.name = oldname + std::to_string(1);
        int count = 2;
        
        while(std::find(names.begin(), names.end(), obj.name) != names.end()) {
            obj.name = oldname + std::to_string(count);
            count++;
        }
        
        names.push_back(obj.name);
    }
    
    for(auto& obj : objects)
    {
        obj.combine(result);
    }

    
    for(auto& [key, ctx] : contexts)
    {
        auto context = Context::combine(key, ctx, sortedObjects[key], nodelist[key], &result, nets[ctxnames[key]]);
        
        result.imports[key] = context;
        result.contexts.push_back(key);
    }
    
    return result;
}

void Object::applyNodes(Vector& v, std::vector<int> nodes, std::string origin) {
    Document::applyNodes(v, nodes, origin);
    
    for(auto& [key, import] : imports) {
        import.applyNodes(v, nodes, "");
    }
}

double Object::parseExpression(std::string expr) {
    double val;
    
    
    size_t start_pos = 0;
    while((start_pos = expr.find("argc", start_pos)) != std::string::npos) {
        std::string to = std::to_string(nArgs);
        expr.replace(start_pos, 4, to);
        start_pos += to.length();
    }
    
    Variable* vptr = getVariablePtr(expr);
    
    if(!expr.empty() && std::all_of(expr.begin(), expr.end(), ::isdigit))
        val = std::stod(expr);
    else if(vptr && vptr->predefined) {
        val = vptr->init;
    }
    else {
        val = evaluate(expr);
    }
    return val;
}

// Quick math evaluator
double Object::evaluate(const std::string& s) {

    TokenString tokens(s, vtable);
    
    for(auto& token : tokens) {
        if(token.type == tVariable && getVariablePtr(token.symbol)->predefined) {
            token.symbol = std::to_string(getVariablePtr(token.symbol)->init);
            token.type = tConst;
        }
    }
    
    TCCState* state = tcc_new();
    
    tcc_set_output_type(state, TCC_OUTPUT_MEMORY);
    
    if(!state) {
        printf("Can’t create a TCC context\n");
        return 0;
    }
    
    std::string ccode = "double calc() { return " + tokens.toC(*this) + "; }";
    
    if (tcc_compile_string(state, ccode.c_str()) > 0) {
        printf("Compilation error !\n");
        return 0;
    }
    
    tcc_relocate(state, TCC_RELOCATE_AUTO);
    
    auto calc = (double(*)())tcc_get_symbol(state, "calc");

    return calc();
}


}
