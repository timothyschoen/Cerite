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
    std::string path;
    
    std::vector<std::string> contexts;
    std::vector<Variable> variables;
    std::vector<Vector> vectors;
    std::vector<Function> functions;
    
    std::map<std::string, int> size;
    std::map<std::string, std::vector<int>> ports;
    std::map<std::string, int> outstart;
    
    VarTable vtable;
    
    bool local = true;
    
    Document() {};

    std::string getText();
    
    // Replace all occurances of a variables in a document
    void replaceVarName(std::string oldVar, std::string newVar);
    
    void setArgument(std::string name, double value, bool removeVariable = true);
    
    void setArgument(int idx, double value, bool removeVariable = true);
    
    void setArgument(int idx, std::string value, bool removeVariable = true);
    
    Document withArgument(std::string name, double value) const;
    
    Document withArgument(int idx, double value) const;
    
    inline static Document fromFile(std::string fileToLoad)  {
        Reader reader;
        return reader.parse(fileToLoad);
    }
    
    // SFINAE construction to allow any class derived from Object
    template <typename ObjType>
    auto compile(void(*logFunc)(const char*) = nullptr) -> typename std::enable_if<std::is_base_of<Object, ObjType>::value,ObjType*>::type
    {
        // Pass a copy of document to make sure that this document remains intact
        Document copy(*this);
        ObjType* newObj = new ObjType(copy);
        return static_cast<ObjType*>(Compiler::compile(*this, newObj, logFunc));
    }
    
    static void uniqueNamer(std::vector<Document>& documents);
    // Combine multiple Cerite documents into one using a node specification
    static Document concat(std::string name, std::vector<Document> documents, NodeList& nodes, std::vector<int> ports = {});
    
    
    void addVariable(Variable var);
    
    void addVector(Vector vec);
    
    void addPort(int num, std::string domain, bool type);
    
    const Vector& getVector(std::string name) const;
    const Function& getFunction(std::string name) const;
    const Variable& getVariable(int idx) const;
    const Variable& getVariable(std::string name) const;
    

    Vector* getVectorPtr(std::string name);
    Function* getFunctionPtr(std::string name);
    Variable* getVariablePtr(int idx);
    Variable* getVariablePtr(std::string name);
    
    static std::string encode(const std::string &input);
    
private:
    
    void applyArgument(int idx, std::string value, bool removeVariable = true);
    
    static void calcCombinedSize(Document& result, std::vector<Document>& documents, NodeList& nodes);
    
    static void combineMatrices(Document& result, std::vector<Document>& documents, NodeList& nodes);
    
};

}
