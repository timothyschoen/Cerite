#pragma once

#include <boost/algorithm/string.hpp>
#include "Preprocessor.h"
#include <libtcc.h>

namespace Cerite {

class Document;
struct CodeWriter
{

    std::vector<Variable> variables;
    
    CodeWriter();
    
    static void prepare(Document& doc);
    
    static std::string exportCode(Document doc);
    
private:
    
    static std::string writeC(Document& doc);
    
    static std::string writeVariables(Document& doc);
    
    static std::string writeFunctions(Document& doc);
    
    static void addUpdateFunc(Document& doc);

};
}
