#pragma once
#include "../Types/Function.h"
#include "../Types/Variable.h"
#include "../Compiler/Compiler.h"
#include "../IO/VarTable.h"
#include "../IO/Reader.h"

namespace Cerite {


typedef std::vector<std::vector<std::pair<int, int>>> NodeList;
class Object;
struct Document
{
    //int size = 0;
    std::string name;
    std::string internalName;
    std::string path;
    
    std::vector<Variable> variables;
    std::vector<Vector> vectors;
    std::vector<Function> functions;
    
    
    VarTable vtable;

    std::vector<std::string> aliases;
    
    Document() {};
    virtual ~Document() {};

    std::string getText();
    
    // Replace all occurances of a variables in a document
    virtual void replaceVarName(std::string oldVar, std::string newVar);
    
    inline static Document fromFile(std::string fileToLoad)  {
        Reader reader;
        return reader.parse(fileToLoad);
    }
    
    virtual void applyNodes(Vector& v, std::vector<int> nodes, std::string origin);

    void addVariable(Variable var);
    void addVector(Vector vec);
    void addFunction(Function func);
    
    int findVector(const std::string& name) const;
    int findFunction(const std::string& name) const;
    int findVariable(const std::string& name) const;
    
    // These need to be overridden to search in contexts in Object class
    virtual Vector* getVectorPtr(const std::string& name);
    virtual Function* getFunctionPtr(const std::string& name);
    virtual Variable* getVariablePtr(const std::string& name);
    
    virtual const Vector* getVectorPtr(const std::string& name) const;
    virtual const Function* getFunctionPtr(const std::string& name) const;
    virtual const Variable* getVariablePtr(const std::string& name) const;

};

}
