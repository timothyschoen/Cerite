#include "Reader.h"
#include "../Interface/Document.h"
#include "../Interface/Library.h"
#include <functional>
#include <boost/algorithm/string.hpp>


namespace Cerite {


Document Reader::parse(std::string path) {
    
    std::string stringToParse = Library::currentLibrary->readFile(path);
    VarTable vtable;
    Document document;
    document.path = path;
    
    document.name = fs::path(path).stem();
    
    // check if we're reading a context
    document.local = path.substr(path.rfind(".")) != ".ctx";
    
    getImports(document, vtable, stringToParse);
    
    // Create a table for locally defined variables
    addToVarTable(vtable, document, stringToParse);
    
    document.vtable = vtable;
    
    std::vector<std::string> segments;
    std::vector<size_t> sectionIndices = {stringToParse.length()};
    std::vector<std::string> sections = vtable.getAll();

    // Find definitions of those variables
    for(auto& str : sections) {
        size_t position = stringToParse.find(str + ":", 0);
        if (position != npos)
            sectionIndices.push_back(position);
    }
    
    // Sort them: definitions end where the next definition starts
    std::sort(sectionIndices.begin(), sectionIndices.end());
    
    for(int i = 1; i < sectionIndices.size(); i++) {
        size_t start = sectionIndices[i - 1];
        size_t end = sectionIndices[i];
        segments.push_back(stringToParse.substr(start, end - start));
    }
    
    for(int i = 0; i < segments.size(); i++) {
        size_t start = segments[i].find(":", 0);
        
        std::string selector = segments[i].substr(0, start);
        std::string content(segments[i].substr(start + 1));
        
        boost::replace_all(content, "\n\n", "\n");
        handleSection(document, selector, content, vtable);
    }
    
    for(auto& vec : document.vectors) {
        vec.ports = document.ports[vec.origin];
    }
    
    return document;
}

void Reader::getImports(Document& doc, VarTable& table, std::string importstr) {
    std::string stringToParse = importstr;
    size_t pos = stringToParse.find("import:", 0);
    if(pos == npos) return;

    
    size_t start = stringToParse.find(":", pos) + 1;
    size_t end = stringToParse.rfind("\n", stringToParse.find(":", start + 1));
    std::string definition = std::string(stringToParse.substr(start, end - start));
    
    definition.erase(std::remove_if(definition.begin(), definition.end(), ::isspace), definition.end());
    
    std::vector<std::string> split;
    boost::split(split, definition,boost::is_any_of(","));
    
    for(auto& import : split) {
        if(Library::currentLibrary->contexts.count(import) > 0) {
            Document& imported = Library::currentLibrary->contexts[import];
            table.spectypes.insert(table.spectypes.begin(), import + "_size");
            table.spectypes.insert(table.spectypes.begin(), import + "_ports");
            
            table.combineWith(imported.vtable);
            
            doc.contexts.push_back(imported.name);
            doc.vectors.insert(doc.vectors.begin(), imported.vectors.begin(), imported.vectors.end());
            doc.variables.insert(doc.variables.begin(), imported.variables.begin(), imported.variables.end());
            doc.functions.insert(doc.functions.begin(), imported.functions.begin(), imported.functions.end());
        }
        else
            std::cerr << "Could not find import " << import << std::endl;
        
    }
    
}

// Find initializer definitions
// Here you are allowed to define functions, vectors and variables that can later be used in code
void Reader::addToVarTable(VarTable& table, Document& doc, std::string stringToParse) {
    
    std::vector<std::pair<std::string, bool>> initializers = {{"var", true}, {"const", false}};
    
    if(doc.contexts.size() < 2 && doc.local){
        table.spectypes.push_back("ports");
        table.spectypes.push_back("size");
    }
    
      
    for(auto& key : initializers) {
        size_t pos = stringToParse.find(key.first + ":", 0);
        if(pos == npos)
            continue;
        
        size_t start = stringToParse.find(":", pos) + 1;
        size_t end = stringToParse.rfind("\n", stringToParse.find(":", stringToParse.find("\n", start + 1)));
        
        parseVariables(doc, table, stringToParse.substr(start, end - start), key.second);
    }
}

void Reader::parseVariables(Document& doc, VarTable& table, std::string varstring, bool dynamic) {
    
    // Split the variables
    
    std::vector<std::string> variables;
    boost::split(variables, varstring, boost::is_any_of(","));
    
    for(auto& var : variables) {
        TokenString::removeAllSpace(var);
        // Check if the static variables is function or vector
        size_t arg = var.find("{");
        size_t vector = var.find("[");
        size_t func = var.substr(1).find("(");
        size_t predef = var.find("=");
        
        
        bool isArgument = arg != npos;
        bool isFunction = func != npos;
        bool isVector = vector != npos;
        bool isDefined = predef != npos;
        bool typeSet = var[0] == '(' && var.find(")") != npos;
    
        std::string ctype = "double";
        size_t idx;
        int argpos = -1;
        
        if(isArgument) {
            size_t argend = var.find("}");
            std::vector<std::string> definition;
            std::string d = var.substr(arg + 1, argend - 1);
            
            boost::split(definition, d, boost::is_any_of(": "));
            argpos = std::stoi(definition[0]);
            var = definition[1];
            typeSet = var[0] == '(' && var.find(")") != npos;
        }
        if(typeSet) {
            int end = TokenString::matchNests(var.substr(1), {"(", ")"});
            ctype = var.substr(1, end);
            var = var.substr(end + 2);
            isFunction = false;
        }
        
        if(isVector) {
            vector = var.find("[");
            idx = vector;
            Vector newvec;
            size_t vectorend = var.find("]", vector);
            
            if(var.find("[]") == npos)
                newvec.size = std::stoi(var.substr(vector + 1, vectorend - vector - 1)); // Get vector size from string
            
            newvec.origin = Document::encode(doc.name);
            newvec.local = doc.local;
            newvec.dims = (int)std::count(var.begin(), var.end(), '[');
            //newvec.definition.resize(pow(newvec.size, newvec.dims));
            // Get name
            newvec.name = var.substr(0, idx);
            newvec.ctype = ctype;
            // Add vector to document
            doc.vectors.push_back(newvec);
            
            // Add vector to table to allow predefinition
            table.vectypes.push_back({newvec.name, doc.local});
        }
        else if(isFunction) {
            idx = func;
            
            Function newfunc("", doc.local);
            newfunc.args = var.substr(idx + 2, var.rfind(")") - (idx + 2));
            newfunc.name = var.substr(0, idx + 1);
            
            doc.functions.push_back(newfunc);
            // Add function to the table to allow definition
            table.functypes.push_back({var.substr(0, idx + 1), doc.local});
            
        }
        else if(isDefined) {
            idx = var.find("=");
            
            // Get predefined value from string
            double init = std::stod(var.substr(idx + 1));
            Variable v(var.substr(0, idx), !dynamic, init);
            v.predefined = true;
            v.local = doc.local;
            v.argpos = argpos;
            v.ctype = ctype;

            
            table.vartypes.push_back({var.substr(0, idx), doc.local});
            doc.variables.insert(doc.variables.begin(), v);
            

            
        }
        else {
            Variable v(var, !dynamic, 0);
            v.predefined = false;
            v.local = doc.local;
            v.argpos = argpos;
            v.ctype = ctype;
            
            table.vartypes.push_back({var, doc.local});
            doc.variables.insert(doc.variables.begin(), v);
            
            
        }
    }
    
}


void Reader::handleSection(Document& doc, const std::string& selector, const std::string& content, const VarTable& table) {
    
    if(table.isVector(selector))
    {
        // Get vector from document
        Vector* vec = doc.getVectorPtr(selector);
        bool local = table.isLocal(selector);
        
        // Get the definition
        std::vector<std::string> definition;
        boost::split(definition, content, boost::is_any_of(","));
        
        if(!vec) {
            doc.vectors.push_back({selector, (int)definition.size(), 1});
            vec = &doc.vectors.back();
        }
        
        vec->update = true;
        
        for(auto& cell : definition)
            vec->definition.push_back(TokenString(cell, table));

        vec->local = local;
        
        return;
    }

    if(table.isFunction(selector))
    {
        // Get function from document
        Function* func = doc.getFunctionPtr(selector);
        bool isLocal = table.isLocal(selector);
                
        TokenString tokenized(content, table);
        
        // If it exists, change its content
        if(func) {
            func->body = tokenized;
            func->origin = doc.name;
        }
        // Otherwise add a new function
        else {
            Function newfunc(tokenized, isLocal);
            newfunc.name = selector;
            newfunc.origin = doc.name;
            doc.functions.push_back(newfunc);
            
        }

        return;
    }
    
    if(table.isVariable(selector))
    {
    }
    
    if(table.isSpecial(selector))
    {
        for(auto& domain : doc.contexts) {
            
            if(selector == domain + "_size") {
                doc.size[domain] = std::stoi(content);
                return;
            }
        }

        if(selector == "size" && doc.local) {
            
            doc.size[doc.contexts[0]] = std::stoi(content);
            return;
        }
        
        for(auto& domain : doc.contexts) {
            if(selector == (domain + "_ports")) {
                doc.ports[domain] = parsePorts(doc, doc.outstart[domain], content);
                return;
            }
        }
        
        if(selector == "ports") {
            if(doc.contexts.size() > 1) {
                std::cerr << "Ambiguous definition of ports" << std::endl;
                return;
            }
            
            doc.ports[doc.contexts[0]] = parsePorts(doc, doc.outstart[doc.contexts[0]], content);
            return;
        }
    }
    
    if(selector == "const" || selector == "var" || selector.empty()) return;
    
    std::cerr << "Unknown identifier " << selector << std::endl;
}

std::vector<int> Reader::parsePorts(Document& doc, int& outstart, std::string content) {
    std::vector<int> ports;
    std::vector<std::string> split;
    boost::split(split, content, boost::is_any_of("|"));
    
    for(int i = 0; i < split.size(); i++) {
        std::vector<std::string> cells;
        boost::split(cells, split[i], boost::is_any_of(", "));
        
        for(auto& cell : cells) {
            if(cell.find_first_not_of(" \t\n\v\f\r") != npos)
                ports.push_back(std::stoi(cell));
        }
        
        if(i == 0) outstart = ports.size();
        
    }
    
    
    return ports;
    
}

}
