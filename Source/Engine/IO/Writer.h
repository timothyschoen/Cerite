#pragma once
#include "../Interface/Document.h"

namespace Cerite {


struct Writer
{
    enum vartype {
        VARIABLE,
        STATIC
    };
    
    static std::string format(Document doc);
    
private:
    
    static std::string writeFunction(Function func) ;
    
    static std::string writeVariables(Document& doc, vartype type);
    
    static std::string writeData(Document& doc);
    
    
    static std::string writeVector(Vector vec);
    
};

}
