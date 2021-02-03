#pragma once
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <string>
#include <vector>


namespace Cerite {


struct Variable
{
    std::string name;
    bool isStatic;
    int argpos = -1;
    
    Variable(const std::string& n, bool stat, double def = 0) : name(n), isStatic(stat), init(def), predefined(def != 0) {};
    
    std::string origin;
    std::string type;
    std::string ctype = "double";
    double init;
    bool predefined = false;
    bool isVector = false;
    bool local = true;
    double* value;
    bool vecstart = false;
    
    bool isFuncptr() {
        return ctype.find("(*)") != std::string::npos;
    }
    
    void getOrigin() {
        size_t pos = name.find("_");
        if(pos != std::string::npos) {
            type = std::string(name.substr(0, pos));
            origin = name.substr(pos + 1);
            
        }
    }
    
    std::string writeInitialiser() const
    {
        std::string result;
        size_t funcptr = ctype.find("(*)");
        if(funcptr != std::string::npos) {
            std::string definition = ctype;
            definition.insert(funcptr + 2, name);
            result += definition + ";\n";
        }
        else {
            result += ctype + " " + name;
            
            if(predefined) {
                std::ostringstream stream;
                stream << init; // formats it to scientific notation if necessary
                result += " = " + stream.str();
            }
            else if(ctype == "int" || ctype == "double" || ctype == "long" || ctype == "float"){
                result += " = 0";
            }
            result += ";\n";
        }
        return result;
    }
};

}
