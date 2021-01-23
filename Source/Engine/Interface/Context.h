#pragma once
#include "../Types/Function.h"
#include "../Types/Variable.h"
#include "../Compiler/Compiler.h"
#include "../IO/VarTable.h"
#include "../IO/Reader.h"
#include "Document.h"

namespace Cerite {


typedef std::vector<std::vector<std::pair<int, int>>> NodeList;
class Object;
struct Context : public Document
{
    int size = -1;
    TokenString portstring;
    
    Context() {};
    
    inline static Context fromFile(std::string fileToLoad)  {
        Reader reader;
        //return reader.parse(fileToLoad);
    }
    
    void setSize(int newSize);
    
    std::vector<Function> defaultFunctions;
    
    
    static Context combine(std::string type, std::vector<Context*> contexts, std::vector<Object*> objects, std::vector<std::vector<int>> nodes, Object* resultObj, int newSize);
    
    void addToContext(Context& result, std::vector<int> nodes, Object* obj, Object* resultObj);
    
    std::vector<int> getNodes(Object* obj = nullptr) const;
    std::pair<int, int> getPorts(Object* obj = nullptr) const;
    
    void replaceVarName(std::string oldVar, std::string newVar) override;
    
private:
};

}
