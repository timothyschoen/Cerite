#include <map>
#include <algorithm>
#include <assert.h>
#include "Document.h"
#include "../IO/Reader.h"
#include "../IO/Writer.h"

#include "../Interface/Library.h"
#include "../Compiler/Compiler.h"

namespace Cerite {


std::string Document::getText() {
    return Writer::format(*this);
}

Document Document::withArgument(std::string name, double value) const {
    Document newdoc = Document(*this);
    newdoc.setArgument(name, value);
    return newdoc;
}

Document Document::withArgument(int idx, double value) const {
    Document newdoc = Document(*this);
    newdoc.setArgument(idx, value);
    return newdoc;
}

void Document::setArgument(std::string argname, double val, bool removeVariable) {
    
    std::ostringstream stream;
    stream << val;
    
    int idx = -1;
    for(int i = 0; i < variables.size(); i++) {
        if(variables[i].name == argname) {
            idx = i;
            break;
        }
    }

    applyArgument(idx, stream.str(), removeVariable);
}

void Document::setArgument(int idx, double val, bool removeVariable) {
    
    std::ostringstream stream;
    stream << val;
    
    int pos = -1;
    for(int i = 0; i < variables.size(); i++) {
        if(variables[i].argpos == idx) {
            pos = i;
            break;
        }
    }
    
    applyArgument(pos, stream.str(), removeVariable);
}

void Document::setArgument(int idx, std::string val, bool removeVariable) {
    int pos = -1;
    for(int i = 0; i < variables.size(); i++) {
        if(variables[i].argpos == idx) {
            pos = i;
            break;
        }
    }
    
    applyArgument(pos, val, removeVariable);
}

void Document::applyArgument(int idx, std::string val, bool removeVariable) {
   
    std::string name = variables[idx].name;
    
    if(idx == -1 || idx >= variables.size()) {
        std::cout << "Argument no." + std::to_string(idx) + "for object " +  name + " not found!!" << std::endl;
        return;
    }
    
    if(removeVariable) variables.erase(variables.begin() + idx);
    
    for(auto& func : functions)
        func.replaceVariable(name, val);
    
    for(auto& vec : vectors)
        for(auto& cell : vec.definition)
            cell.replaceSymbol(name, val);
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
    
    for(auto& func : functions)
        func.body.replaceSymbol(oldVar, newVar, true);
    
    for(auto& vec : vectors)
        for(auto& cell : vec.definition)
            cell.replaceSymbol(oldVar, newVar);
    
}

void Document::uniqueNamer(std::vector<Document>& documents) {
    std::vector<std::string> names;
    for(auto& doc : documents) {
        // Make sure each component has a unique name
        std::string oldname = doc.name.substr(0, 3);
        
        
        // Some objects like multiply cause invalid variable names
        // Base64 encoding them solves it while also ensuring uniqueness
        // doesn't look pretty tho...
        bool alphanumeric = find_if_not(oldname.begin(), oldname.end(), [](char c){ return isalpha(c) || isdigit(c); }) == oldname.end();
        if(!alphanumeric) oldname = encode(oldname);
        
        doc.name = oldname + std::to_string(1);
        int count = 2;
        
        while(std::find(names.begin(), names.end(), doc.name) != names.end()) {
            doc.name = oldname + std::to_string(count);
            count++;
        }
        
        names.push_back(doc.name);
    }
    
    // Give prefixes to all variables to prevent double vars and also show to origin of variables
    for(auto& doc : documents) {
        for(auto& var : doc.variables) {
            if(var.local) {
                if(!var.isVector)
                    doc.replaceVarName(var.name, var.name + "_" + doc.name);
                else
                    doc.replaceVarName(var.name, var.name.substr(0, 7) + doc.name + "_" + var.name.substr(7));
            }
        }
        
        for(auto& vec : doc.vectors) {
            doc.replaceVarName(vec.name, vec.origin + "_" + vec.name);
            vec.name = vec.origin + "_" + vec.name;
            if(!vec.local && doc.size[vec.origin] != 0)
                vec.size = doc.size[vec.origin];
            
        }
        
        for(auto& func : doc.functions) {
            if(func.local) {
                doc.replaceVarName(func.name, doc.name + "_" + func.name);
                func.name = doc.name + "_" + func.name;
            }
        }
    }
    
}

// Combine multiple Cerite documents into one using a node list
Document Document::concat(std::string name, std::vector<Document> documents, NodeList& nodes, std::vector<int> ports) {
    
    Document result;
    
    std::map<std::string, Function> functable;
    
    result.name = name;
    
    for(auto& doc : documents)
        Preprocessor::preprocess(doc);
    
    // Assign unique names to each sub-document
    uniqueNamer(documents);
    calcCombinedSize(result, documents, nodes);
    combineMatrices(result, documents, nodes);
    
    for(int n = 0; n < documents.size(); n++) {
        Document doc = documents[n];
        
        for(auto& func : doc.functions) {
            if(func.body.empty()) continue;
            for(auto& vec : doc.vectors)
                if(!vec.local)
                    func.fixIndices(nodes[n], vec.name);
            
            if(functable.count(func.name) == 0)
                functable[func.name] = func;
            else
                functable[func.name].body.append(func.body.tokens);
        }
        
        
        for(auto& var : doc.variables) {
            if(var.local) {
                result.variables.push_back(var);
            }
            else if(std::find_if(result.variables.begin(), result.variables.end(), [&doc, var] (Variable& v) mutable {
                bool equal = v.name == var.name;
                if(equal && v.init != var.init)
                    std::cout << "Warning: conflicting values for " << v.name << std::endl;
                return equal;
            }) == result.variables.end()) {
                
                result.variables.push_back(var);
            }
        }
    }
    
    for(auto& func : functable)
        result.functions.push_back(func.second);
    
    
    return result;
    
}

void Document::calcCombinedSize(Document& result, std::vector<Document>& documents, NodeList& nodes) {
    
    std::vector<std::vector<int>> uniqueNodes = {{0}, {0}, {0}};
    std::vector<std::string> domains = {"mna", "dsp", "data"};
    
    
    std::vector<int> max(3, 0);
    // find all unique node numbers excluding 0
    for(auto node : nodes) {
        for(auto num : node) {
            if(std::find(uniqueNodes[num.second].begin(), uniqueNodes[num.second].end(), num.first) == uniqueNodes[num.second].end())
                uniqueNodes[num.second].push_back(num.first);
            
            max[num.second] = std::max(max[num.second], num.first);
        }
    }
    
    
    std::vector<int> nets = max;
    
    for(size_t n = 0; n < nodes.size(); n++) {
        Document doc = documents[n];
        
        for(auto& domain : doc.ports) {
            int d = (domain.first == "dsp") + (2 * (domain.first == "data")); // Find a better way to do this!!
            // Number of internal nodes is equal to the total size minus the size of the ports
            int internal = doc.size[domain.first] - domain.second.size();
            
            for(int i = 0; i < internal; i++) {
                nets[d]++;
                nodes[n].push_back({(int)(nets[d]), d});
            }
            
            result.size[domain.first] = nets[d];
        }
        
        for(auto& node : nodes[n])
            node.first--;
    }
}

void Document::combineMatrices(Document& result, std::vector<Document>& documents, NodeList& nodes) {
    
    //int nets = result.size;
    
    // Map to find all vectors with the same name
    std::map<std::string, std::vector<std::pair<std::vector<int>, Vector>>> vectormap;
    std::vector<Vector> finalvector;
    
    for(int n = 0; n < nodes.size(); n++) {
        Document doc = documents[n];
        
        // Check for each vector/matrix if it already exists or if we need to create a new entry
        for(int i = 0; i < doc.vectors.size(); i++) {
            Vector& v = doc.vectors[i];
            int type = (v.origin == "dsp") + (2 * (v.origin == "data"));
            int size = doc.size[v.origin];
            std::vector<int> ports = doc.ports[v.origin];
            std::vector<int> node(size, 0);
            
            int stepOver = 0;
            for(int j = 0; j < size + stepOver; j++) {
                
                // Add internal ports
                if(std::find(ports.begin(), ports.end(), j) == ports.end())
                    ports.push_back(j);
                
                // Calculate final node position
                if(type == nodes[n][ports[j]].second)
                    node[j - stepOver] = nodes[n][ports[j]].first;
                else
                    stepOver++;
            }
            
            if(vectormap.count(v.name) > 0 && !v.local)
                vectormap[v.name].push_back({node, v});
            else if (vectormap.count(v.name) == 0)
                vectormap.insert({v.name, {{node, v}}});
        }
    }
    
    finalvector.resize(vectormap.size());
    
    for(int i = 0; i < vectormap.size(); i++) {
        auto vector = *std::next(vectormap.begin(), i);
        int size = result.size[vector.second[0].second.origin];
        finalvector[i] = {vector.first, size, vector.second[0].second.dims, vector.second[0].second.local};
        
        for(auto& subvector : vector.second) {
            std::vector<int> ports = subvector.first;
            Vector entry = subvector.second;
            finalvector[i].ctype = entry.ctype;
            finalvector[i].update = entry.update;
            
            if(!entry.definition.empty() && !entry.local)
                entry.overlap(finalvector[i], size, ports);
            else if(entry.local) {
                finalvector[i].size = entry.size;
            }
        }
    }
    
    // Add the new vectors to the result
    for(auto& vec : finalvector)
        result.vectors.push_back(vec);
    
}


void Document::addVariable(Variable var) {
    variables.push_back(var);
}

void Document::addVector(Vector vec) {
    vec.local = true;
    vectors.push_back(vec);
}

void Document::addPort(int num, std::string domain, bool type) {
    std::vector<int>& portlist = ports[domain];
    
    if(std::find(portlist.begin(), portlist.end(), num) != portlist.end()) {
        std::cout << "Port already exists!" << std::endl;
        return;
    }
    
    if(!type) {
        portlist.insert(portlist.begin() + outstart[domain], num);
        outstart[domain]++;
    }
    else {
        portlist.push_back(num);
    }
}



const Vector& Document::getVector(std::string name) const {
    auto pos = std::find_if(vectors.begin(), vectors.end(), [&name](const Vector &f) -> bool {
        return f.name == name;
    });
    assert(pos != vectors.end());
    return vectors[pos - vectors.begin()];
}


const Function& Document::getFunction(std::string name) const {
    auto pos = std::find_if(functions.begin(), functions.end(), [&name](const Function &f) -> bool {
        return f.name == name;
    });
    assert(pos != functions.end());
    return functions[pos - functions.begin()];
    
}

const Variable& Document::getVariable(int idx) const {
    return variables[idx];
}
const Variable& Document::getVariable(std::string name) const {
    for(auto& var : variables)
        if(var.name == name)
            return var;
    return Variable("", false);
}

Vector* Document::getVectorPtr(std::string name) {
    auto pos = std::find_if(vectors.begin(), vectors.end(), [&name](Vector &f) -> bool {
        return f.name == name;
    });
    assert(pos != vectors.end());
    
    return &vectors[pos - vectors.begin()];
}
Function* Document::getFunctionPtr(std::string name) {
    auto pos = std::find_if(functions.begin(), functions.end(), [&name](Function &f) -> bool {
        return f.name == name;
    });
    assert(pos != functions.end());
    return &functions[pos - functions.begin()];
}

Variable* Document::getVariablePtr(int idx) {
    return &variables[idx];
}

Variable* Document::getVariablePtr(std::string name) {
    for(auto& var : variables)
        if(var.name == name)
            return &var;
    
    return nullptr;
}

// Function for encoding special characters to a C-friendly variable name
std::string Document::encode(const std::string &input) {
    
    std::string result;
    for(auto& chr : input) {
        if(chr == '*') result += "mul";
        else if(chr == '/') result += "div";
        else if(chr == '+') result += "add";
        else if(chr == '-') result += "sub";
        else if(chr == '%') result += "mod";
        else if(chr == '&') result += "and";
        else if(chr == '=') result += "eq";
        else if(chr == '~') result += "sig";
        else result += chr;
    }
    return result;
}

/* Unnecessary for now, but these algorithms might still be useful at some point
 int Document::getNumPorts(std::string domain, int type) {
 if(type == 0) {
 return outstart[domain];
 }
 else if(type == 1) {
 return (int)ports[domain].size() - outstart[domain];
 }
 else {
 return (int)ports[domain].size();
 }
 }
 
 int Document::getPortType(int position) {
 
 int idx = 0;
 // First check inputs
 for(int i = 0; i < contexts.size(); i++) {
 int incr = outstart[contexts[i]];
 if(incr + idx > position) {
 return i;
 }
 idx += incr;
 }
 
 // Check outputs
 for(int i = 0; i < contexts.size(); i++) {
 int incr = (int)ports[contexts[i]].size() - outstart[contexts[i]];
 if(incr + idx > position) {
 return i;
 }
 idx += incr;
 }
 } */


}
