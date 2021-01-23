#pragma once
#include "../Types/Function.h"
#include "../Types/Variable.h"
#include "../Compiler/Compiler.h"
#include "../IO/VarTable.h"
#include "../IO/Reader.h"
#include "Document.h"
#include "Context.h"

namespace Cerite {


typedef std::vector<std::vector<std::pair<int, int>>> NodeList;
struct Object : public Document
{
    std::map<std::string, Context> imports;
    std::vector<std::string> contexts;
    
    int nArgs = 0;
    std::vector<std::string> varArgs;
    
    Object() {};
    
    std::string getText();
    
    // Replace all occurances of a variables in a Object
    void replaceVarName(std::string oldVar, std::string newVar) override;
    
    
    int getArgumentPosition(int idx);
    void setArgument(const std::string& name, double value, bool removeVariable = true);
    void setArgument(int idx, double value, bool removeVariable = true);
    void setArgument(int idx, const std::string& value, bool removeVariable = true);

    Object withArgument(const std::string& name, double value) const;
    Object withArgument(int idx, double value) const;
    
    void applyArgument(int position, std::string value, bool removeVariable = true);
    
    void combine(Object& result);
    
    static Object combineObjects(const std::string& name, std::vector<Object>& objects, NodeList& nodes, std::vector<int> ports = {});
    
    /*
    // SFINAE construction to allow any class derived from Object
    template <typename ObjType>
    auto compile(void(*logFunc)(const char*) = nullptr) -> typename std::enable_if<std::is_base_of<Object, ObjType>::value,ObjType*>::type
    {
        // Pass a copy of Object to make sure that this Object remains intact
        Object copy(*this);
        ObjType* newObj = new ObjType(copy);
        return static_cast<ObjType*>(Compiler::compile(*this, newObj, logFunc));
    } */
    
    void applyNodes(Vector& v, std::vector<int> nodes, std::string origin) override;
    
    

    Vector* getVectorPtr(const std::string& name) override;
    Function* getFunctionPtr(const std::string& name) override;
    Variable* getVariablePtr(const std::string& name) override;
    
    const Vector* getVectorPtr(const std::string& name) const override;
    const Function* getFunctionPtr(const std::string& name) const override;
    const Variable* getVariablePtr(const std::string& name) const override;
    
    double parseExpression(std::string expr);
private:
    
    double evaluate(const std::string& s);
};

}
