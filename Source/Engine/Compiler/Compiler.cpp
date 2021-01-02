#include "../Interface/Document.h"
#include "../Interface/Patch.h"
#include "Compiler.h"
#include <fstream>
#include <cmath>


namespace Cerite {


typedef double*(*vecptr)();

Compiler::Compiler() {

}


std::string Compiler::exportCode(Document doc) {
    prepare(doc);
    
    std::string externs;
    
    // Add regular C imports, since this won't be compiled with TCC
    externs += "#include <stdio.h>\n";
    externs += "#include <stdlib.h>\n";
    externs += "#include <stddef.h>\n";
    externs += "#include <math.h>\n";
    externs += "void print(const char* txt) { printf(txt); }\n";
    externs += datatype;
    
    std::string ccode = externs + "\n" + writeC(doc);
    
    return ccode;
}

Object* Compiler::compile(Document doc, Object* obj, void(*print)(const char*)) {
    Compiler compiler;
    
    if(print != nullptr)
        compiler.print = print;
    
    prepare(doc);
    
    perform(doc, compiler, obj);
    
    return obj;
}

void Compiler::prepare(Document& doc) {
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
    
    Variable outvar("out", true, 0);
    doc.addVariable(outvar);

}


Object* Compiler::perform(Document& doc, Compiler& compiler, Object* object) {
    
    object->variables = doc.variables;

    // Create new tcc state
    object->state = tcc_new();
    
    // Register error func and set output type
    tcc_set_error_func(object->state, nullptr, compiler.errfunc);
    tcc_set_output_type(object->state, TCC_OUTPUT_MEMORY);
    
    // Check if context creation succeeded
    if(!object->state) {
        compiler.print("Can’t create a TCC context\n");
        return nullptr;
    }
    
    
    // Will link some C standard library functions from the current runtime
    // This improves portability because we're not dependent of TCC finding the libraries
    
    // Register the custom print function
    tcc_add_symbol(object->state, "print", (void*)compiler.print);
    
    // Add std library functions and write extern statements
    std::string externs = "extern void print(const char*);\n" + addStdLibrary(object);

    std::string ccode = externs + "\n" + writeC(doc);
    
    std::cout << ccode << std::endl;
    
    // Check for compilation errors
    if (tcc_compile_string(object->state, ccode.c_str()) > 0) {
        compiler.print("Compilation error !\n");
        return nullptr;
    }
    else {
        compiler.print("Compilation succesful!\n");
    }

    tcc_relocate(object->state, TCC_RELOCATE_AUTO);

    // Request pointers for all vectors, variables and functions
    // This allows for quick access from anywhere in the C++ program
    for(auto& func : doc.functions)
        object->functions[func.name] = (fptr)tcc_get_symbol(object->state, func.name.c_str());
    
    for(auto& var : object->variables)
        var.value = (double*)tcc_get_symbol(object->state, var.name.c_str());
    
    for(auto& vec : doc.vectors) {
        if(vec.isFuncptr()) continue;
        //double* start = *(double**)tcc_get_symbol(s, vec.name.c_str());
        vecptr initfunc = (vecptr)tcc_get_symbol(object->state, (vec.name + "_ptr").c_str());
        vec.start = initfunc();
    }
    
    object->vectors = doc.vectors;
    
    return object;
}

// Function for linking parts of the c-standard library from our c++ runtime
// This way we can prevent linker errors and reduce compile time
std::string Compiler::addStdLibrary(Object* obj) {
    std::string externs;

    // math.h definitions
    externs += includeFunction<double, double>(obj->state, "sin", (void*)(double(*)(double))&sin);
    externs += includeFunction<double, double>(obj->state, "cos", (void*)(double(*)(double))&cos);
    externs += includeFunction<double, double>(obj->state, "tan", (void*)(double(*)(double))&tan);
    
    externs +=  includeFunction<double, double>         (obj->state, "sin",  (void*)(double(*)(double))&sin);
    externs +=  includeFunction<double, double>         (obj->state, "cos",  (void*)(double(*)(double))&cos);
    externs +=  includeFunction<double, double>         (obj->state, "tan",  (void*)(double(*)(double))&tan);
    externs +=  includeFunction<double, double>         (obj->state, "sinh", (void*)(double(*)(double))&sinh);
    externs +=  includeFunction<double, double>         (obj->state, "cosh", (void*)(double(*)(double))&cosh);
    externs +=  includeFunction<double, double>         (obj->state, "tanh", (void*)(double(*)(double))&tanh);
    externs +=  includeFunction<double, double>         (obj->state, "asin", (void*)(double(*)(double))&asin);
    externs +=  includeFunction<double, double>         (obj->state, "acos", (void*)(double(*)(double))&acos);
    externs +=  includeFunction<double, double>         (obj->state, "atan", (void*)(double(*)(double))&atan);
    externs +=  includeFunction<double, double, double> (obj->state, "atan2",(void*)(double(*)(double, double))&atan2);
    externs +=  includeFunction<double, double>         (obj->state, "exp",  (void*)(double(*)(double))&exp);
    externs +=  includeFunction<double, double>         (obj->state, "log",  (void*)(double(*)(double)) &log);
    externs +=  includeFunction<double, double>         (obj->state, "log10",(void*)(double(*)(double)) log10);
    externs +=  includeFunction<double, double, double> (obj->state, "pow",  (void*)(double(*)(double, double))&pow);
    externs +=  includeFunction<double, double>         (obj->state, "sqrt", (void*)(double(*)(double))&sqrt);
    externs +=  includeFunction<double, double>         (obj->state, "ceil", (void*)(double(*)(double))&ceil);
    externs +=  includeFunction<double, double>         (obj->state, "floor",(void*)(double(*)(double))&floor);
    externs +=  includeFunction<double, double>         (obj->state, "fabs", (void*)(double(*)(double))&fabs);
    externs +=  includeFunction<double, double, int>    (obj->state, "ldexp",(void*)(double(*)(double, int))&ldexp);
    externs +=  includeFunction<double, double, int*>   (obj->state, "frexp",(void*) (double(*)(double, int*))&frexp);
    //externs +=  includeFunction<double, double  double*>(obj->state, "modf", (void*)(double(*)(double, double*))&modf);
    externs +=  includeFunction<double, double, double> (obj->state, "fmod", (void*)(double(*)(double, double))&fmod);
    
    externs += includeVariable<double>(obj->state, "M_LOG2E", M_LOG2E);
    externs += includeVariable<double>(obj->state, "M_LOG10E", M_LOG10E);
    externs += includeVariable<double>(obj->state, "M_LN2", M_LN2);
    externs += includeVariable<double>(obj->state, "M_LN10", M_LN10);
    externs += includeVariable<double>(obj->state, "M_PI", M_PI);
    externs += includeVariable<double>(obj->state, "M_PI_2", M_PI_2);
    externs += includeVariable<double>(obj->state, "M_PI_4", M_PI_4);
    externs += includeVariable<double>(obj->state, "M_1_PI", M_1_PI);
    externs += includeVariable<double>(obj->state, "M_2_PI", M_2_PI);
    externs += includeVariable<double>(obj->state, "M_2_SQRTPI", M_2_SQRTPI);
    externs += includeVariable<double>(obj->state, "M_SQRT2", M_SQRT2);
    externs += includeVariable<double>(obj->state, "M_SQRT1_2", M_SQRT1_2);
    
    externs +=  "#include <tcclib.h>\n";
    
    // Add data processing library
    externs += datatype;
    
    return externs;
    
}

// Function that handles generating the code
std::string Compiler::writeC(Document& doc) {
    
    std::string result;
    
    
    addUpdateFunc(doc);
    
    result += writeVariables(doc) + "\n\n";

    result += writeFunctions(doc);
    
    std::string code = result;
    
    return code;
}

// Writes C variable definitions for all our variables
std::string Compiler::writeVariables(Document& doc) {
    
    std::string result;
    
    for(auto& var : doc.variables) {
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
std::string Compiler::writeFunctions(Document& doc) {
    std::string result;
    std::string appendix;
    
    for(auto& vec : doc.vectors) {
        if(vec.isFuncptr()) continue;
        std::string arrsize = std::to_string((int)pow(vec.size, vec.dims)) + " * sizeof(" + vec.ctype + ")";
        result += vec.ctype + "* " + vec.name + "_ptr() {\n";
        
        // TODO: add more number types!
        if(vec.ctype == "double" || vec.ctype == "int")
            result += "memset(" + vec.name + ", 0, " + arrsize + ");\n";
        
        
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

void Compiler::addUpdateFunc(Document& doc) {
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

