#pragma once
#include <boost/algorithm/string.hpp>
#include <libtcc.h>

namespace Cerite {

class Object;
struct CodeWriter
{

    std::vector<Variable> variables;
    
    CodeWriter();
    
    static void prepare(Object& doc);
    
    static std::string exportCode(Object doc);
    
private:
    
    static std::string writeC(Object& doc);
    
    static std::string writeVariables(Object& doc);
    
    static std::string writeFunctions(Object& doc);
    
    static void addUpdateFunc(Object& doc);
    
    static void linearizeConstants(Object& obj, std::vector<Variable>& variables);

};
}
