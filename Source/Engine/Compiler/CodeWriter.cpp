#include "../Interface/Document.h"
#include "CodeWriter.h"
#include <fstream>
#include <cmath>
#include <JuceHeader.h>

namespace Cerite {


typedef double*(*vecptr)();

CodeWriter::CodeWriter() {

}


std::string CodeWriter::exportCode(Document doc) {
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

void CodeWriter::prepare(Document& doc) {
    // inverse for-loop for removing variables
    for(int i = (int)doc.variables.size()-1; i >= 0; i--) {
        Variable var = doc.variables[i];
        if(var.isStatic) {
            if(!var.predefined) {
            std::cerr << "Warning: Static variable " + var.name + " not defined!" << std::endl;
            }
            else {
                Document d = doc.withArgument(var.name, var.init);
                // Set the argument
                doc = d;
            }
        }
    }
    
    Variable srvar("sr", true, 44100);
    doc.addVariable(srvar);
    
    Variable dtvar("dt", true, 1./44100.);
    doc.addVariable(dtvar);
    
    Variable gmvar("gMin", true, 1e-12);
    doc.addVariable(gmvar);
    
    Variable donevar("done", true, 1);
    doc.addVariable(donevar);
    
    if(doc.size.count("mna")) {
        Variable mnasize("mna_size", true, doc.size["mna"]);
        doc.addVariable(mnasize);
    }
    
    Variable outvar("out", true, 0);
    doc.addVariable(outvar);
    
    

}

// Function that handles generating the code
std::string CodeWriter::writeC(Document& doc) {
    
    std::string result;
    
    
    addUpdateFunc(doc);
    
    result += writeVariables(doc) + "\n\n";

    result += writeFunctions(doc);
    
    std::string code = result;
    
    return code;
}

// Writes C variable definitions for all our variables
std::string CodeWriter::writeVariables(Document& doc) {
    
    std::string result;
    
    for(auto& var : doc.variables) {
        
        size_t funcptr = var.ctype.find("(*)");
        if(funcptr != std::string::npos) {
            std::string definition = var.ctype;
            definition.insert(funcptr + 2, var.name);
            result += definition + ";\n";
        }
        else {
            result += var.ctype + " " + var.name;
                
            if(var.predefined) {
                std::ostringstream stream;
                stream << var.init; // formats it to scientific notation if necessary
                result += " = " + stream.str();
            }
            else {
                result += " = 0";
            }
            result += ";\n";
        }
    }
    
    
    for(auto& vec : doc.vectors) {
        
        // handle vector of func ptrs
        size_t funcptr = vec.ctype.find("(*)");
        if(funcptr != std::string::npos) {
            std::string definition = vec.ctype;
            definition.insert(funcptr + 2, vec.name + "[" + std::to_string((int)pow(vec.size, vec.dims)) + "]");
            result += definition + ";\n";
        }
        else {
            result += vec.ctype + " " + vec.name + "[" + std::to_string((int)pow(vec.size, vec.dims)) + "];\n";
        }

        
    }
    
    return result;
}


// Writes C functions for all our functions
std::string CodeWriter::writeFunctions(Document& doc) {
    std::string result;
    std::string appendix;
    
    for(auto& vec : doc.vectors) {
        if(vec.isFuncptr()) continue;
        std::string arrsize = std::to_string((int)pow(vec.size, vec.dims)) + " * sizeof(" + vec.ctype + ")";
        result += vec.ctype + "* " + vec.name + "_ptr() {\n";
        
        // TODO: add more number types!
        //if(vec.ctype == "double" || vec.ctype == "int")
        // result += "memset(" + vec.name + ", 0, " + arrsize + ");\n";
        
        
        result += "return " + vec.name + "; }\n";
    }
    
    for(auto& func : doc.functions) {
        
        std::string funcstr;
        
        funcstr += func.body.toC(doc) + "\n";
        
        // fix indentation
        boost::replace_all(funcstr, ";", ";\n\t");

        funcstr += "\n}\n\n\n";
        
        // TODO: what happens with multiple args?
        std::string argstr;
        if(!func.args.empty()) {
            argstr += func.args + " data";
        }
        
        if(func.name != "prepare" && func.name != "calc")
            result += "void " + func.name + "( " + argstr + ") {\n\t" + funcstr;
        else
            appendix.insert(0, "void " + func.name + "() {\n\t" + funcstr);
        
    }
    
    return result + appendix;
}

void CodeWriter::addUpdateFunc(Document& doc) {
    std::string fullExpr;
    
    for(auto& vec : doc.vectors) {
        
        //if(vec.local || !vec.update) continue;

        for(int i = 0; i < vec.definition.size(); i++) {
            //vec.definition[i].removeSemicolons();
            if(vec.isFuncptr()) {
                if(vec.definition[i].empty()) continue;
                
                Function* initfunc = doc.getFunctionPtr("prepare");
                
                if(i != 0) {
                    TokenString vecdef("\t" + vec.name + "[" + std::to_string(i) + "]" + " = " + vec.definition[i].toC(doc) + ";\n");
                    initfunc->body.append(vecdef.tokens);
                }
                else {
                    TokenString vecdef("\t" + vec.name + "[0] = do_nothing;\n");
                    initfunc->body.append(vecdef.tokens);
                }
                continue;
            }
            
            if(vec.definition[i].empty()) {
                
                vec.definition[i] = TokenString("0", vec.definition[i].chars);
            }

            fullExpr += "\t" + vec.name + "[" + std::to_string(i) + "]" + " = " + vec.definition[i].toC(doc) + ";\n";
            
        }
    }
    
    Function update(TokenString(fullExpr, doc.vtable), false);
    update.name = "update";
    doc.functions.push_back(update);
}

}

