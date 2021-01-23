#include "../Interface/Document.h"
#include "../Interface/Object.h"
#include "../Interface/Library.h"
#include "CodeWriter.h"
#include <fstream>
#include <cmath>
#include <JuceHeader.h>

namespace Cerite {


typedef double*(*vecptr)();

CodeWriter::CodeWriter() {

}


std::string CodeWriter::exportCode(Object doc) {
    prepare(doc);
    
    std::string externs;
    
    /*
    // Add regular C imports, since this won't be compiled with TCC
    externs += "#include <stdio.h>\n";
    externs += "#include <stdlib.h>\n";
    externs += "#include <stddef.h>\n";
    externs += "#include <math.h>\n";
    externs += "void print(const char* txt) { printf(txt); }\n";
    externs += datatype;
    
     */
    std::string ccode = externs + "\n" + writeC(doc);
    
    return ccode;
}

void CodeWriter::linearizeConstants(Object& obj, std::vector<Variable>& variables)
{
    for(int i = (int)variables.size()-1; i >= 0; i--) {
        Variable var = variables[i];
        if(var.isStatic) {
            if(!var.predefined) {
            std::cerr << "Warning: Static variable " + var.name + " not defined!" << std::endl;
            }
            else {
                obj.setArgument(var.name, var.init);
            }
        }
        
    }
    
}

void CodeWriter::prepare(Object& doc) {
    // inverse for-loop for removing variables

    linearizeConstants(doc, doc.variables);
    
    for(auto& [key, import] : doc.imports)
    {
        linearizeConstants(doc, import.variables);
    }
    
    Variable srvar("sr", true, 44100);
    doc.addVariable(srvar);
    
    Variable dtvar("dt", true, 1./44100.);
    doc.addVariable(dtvar);
    
    for(auto& import : doc.imports) {
        doc.addVariable(Variable(import.first + "_size", true, import.second.size));
    }
    
    Variable outvar("out", true, 0);
    doc.addVariable(outvar);
    
    

}

// Function that handles generating the code
std::string CodeWriter::writeC(Object& doc) {
    
    std::string result; 
    
    addUpdateFunc(doc);
    
    result += writeVariables(doc) + "\n\n";

    result += writeFunctions(doc);
    
    std::string code = result;
    
    return code;
}

// Writes C variable definitions for all our variables
std::string CodeWriter::writeVariables(Object& doc) {
    
    std::string result;
    
    for(auto& var : doc.variables) {
        result += var.writeInitialiser();
    }
    
    // Write forward declarations for functions
    // so all functions can be found independent of order
    for(auto& func : doc.functions) {
        result += func.writeInitialiser() + ";\n";
    }
    
    
    for(auto& [key, import] : doc.imports) {
        for(auto& func : import.functions) {
            result += func.writeInitialiser() + ";\n";
        }
    }
    
    for(auto& [key, import] : doc.imports) {
        for(auto& var : import.variables) {
            result += var.writeInitialiser();
        }
    }
    
    for(auto& vec : doc.vectors) {
        result += vec.writeInitialiser() + ";\n";
    }
    
    for(auto& [key, import] : doc.imports) {
        for(auto& vec : import.vectors) {
            result += vec.writeInitialiser() + ";\n";
        }
    }

    return result;
}


// Writes C functions for all our functions
std::string CodeWriter::writeFunctions(Object& doc) {
    
    std::vector<std::string> result;
    
    for(auto& vec : doc.vectors) {
        if(vec.isFuncptr()) continue;
        result.push_back(vec.ctype + "* " + vec.name + "_ptr() {\n return " + vec.name + "; }\n");
    }
    
    for(auto& func : doc.functions) {
        bool atEnd = func.name.substr(4) == "prepare" || func.name.substr(4) == "calc";
        result.insert(result.end(), func.toString(doc));
    }
    
    for(auto& [key, import] : doc.imports) {
        
        
        
        for(int i = 0; i < import.functions.size(); i++) {
            
            bool atEnd = import.functions[i].name.substr(4) == "prepare" || import.functions[i].name.substr(4) == "calc";
            
            
            import.functions[i].body.append(import.defaultFunctions[i].body.tokens);
            result.insert(atEnd ? result.end() : result.begin(), import.functions[i].toString(doc));
        }
    }
    
    return boost::join(result, "\n\n");
}

void CodeWriter::addUpdateFunc(Object& doc) {
    std::string updateFunc;
    std::string initFunc;
    
    for(auto& [key, import] : doc.imports) {
        for(auto& vec : import.vectors) {
            if(!vec.update) continue;
            
            if(!vec.isFuncptr()) {
                updateFunc += vec.toString(doc) + "\n";
            }
            else {
                initFunc += vec.toString(doc) + "\n";
            }
        }
    }
    
    for(auto& vec : doc.vectors) {
        
        if(!vec.update) continue;
        
        if(!vec.isFuncptr()) {
            updateFunc += vec.toString(doc) + "\n";
        }
        else {
            initFunc += vec.toString(doc) + "\n";
        }
    }
    
    
    Function* initfunc = doc.getFunctionPtr("prepare");
    if(!initfunc) {
        Function newfunc = Function(TokenString(initFunc));
        newfunc.name = "prepare";
        doc.addFunction(newfunc);
    }
    else {
        initfunc->body.append(TokenString(initFunc).tokens);
    }
    
    
    Function update(TokenString(updateFunc, doc.vtable));
    update.name = "update";
    doc.functions.insert(doc.functions.begin(), update);
}

}

