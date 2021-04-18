
#include "Compiler.h"
#include "../Utility/Solver.h"
#include "../IO/Token.h"
#include "../Interface/Document.h"
#include <fstream>
#include <cmath>
#include <JuceHeader.h>

namespace Cerite {


typedef double*(*vecptr)();

Compiler::Compiler() {

}


TCCState* Compiler::compile(std::string code, void(*print)(const char*)) {
    Compiler compiler;
    
    compiler.print = print;
    compiler.errfunc = [](void* n, const char* msg) {
        Compiler::print(msg);
    };
    
    return perform(code, compiler);
}

TCCState* Compiler::perform(std::string code, Compiler& compiler) {
    
    // Create new tcc state
    TCCState* state = tcc_new();
    
    // Register error func and set output type
    tcc_set_error_func(state, nullptr, compiler.errfunc);
    tcc_set_output_type(state, TCC_OUTPUT_MEMORY);
    
    tcc_add_include_path(state, path.c_str());
    // Check if context creation succeeded
    if(!state) {
        compiler.print("Can’t create a TCC context\n");
        return nullptr;
    }
    
    // Will link some C standard library functions from the current runtime
    // This improves portability because we're not dependent of TCC finding the libraries
    
    
    addStdLibrary(state);

    // Add data processing library and tcc includes
    std::string includes =  "#define CERITE_GUI\n"
                            "#include \"libcerite.h\"\n";
    
    std::string ccode = includes + code;
    
    File output = File::getSpecialLocation(File::userHomeDirectory).getChildFile("Cerite_latest.c");
    output.create();
    output.replaceWithText(String(ccode));
    
    // Check for compilation errors
    if (tcc_compile_string(state, ccode.c_str()) == -1) {
        compiler.print("Compilation error !\n");
        return nullptr;
    }
    else {
        //compiler.print("Compilation succesful!\n");
    }

    tcc_relocate(state, TCC_RELOCATE_AUTO);

    
    return state;
}

// Function for linking parts of the c-standard library from our c++ runtime
// This way we can prevent linker errors and reduce compile time
void Compiler::addStdLibrary(TCCState* state) {
    std::string externs;
    
    // math.h definitions
    externs +=  includeFunction<double, double>         (state, "sin",  (void*)(double(*)(double))&sin);
    externs +=  includeFunction<double, double>         (state, "cos",  (void*)(double(*)(double))&cos);
    externs +=  includeFunction<double, double>         (state, "tan",  (void*)(double(*)(double))&tan);
    externs +=  includeFunction<double, double>         (state, "sinh", (void*)(double(*)(double))&sinh);
    externs +=  includeFunction<double, double>         (state, "cosh", (void*)(double(*)(double))&cosh);
    externs +=  includeFunction<double, double>         (state, "tanh", (void*)(double(*)(double))&tanh);
    externs +=  includeFunction<double, double>         (state, "asin", (void*)(double(*)(double))&asin);
    externs +=  includeFunction<double, double>         (state, "acos", (void*)(double(*)(double))&acos);
    externs +=  includeFunction<double, double>         (state, "atan", (void*)(double(*)(double))&atan);
    externs +=  includeFunction<double, double, double> (state, "atan2",(void*)(double(*)(double, double))&atan2);
    externs +=  includeFunction<double, double>         (state, "exp",  (void*)(double(*)(double))&exp);
    externs +=  includeFunction<double, double>         (state, "log",  (void*)(double(*)(double)) &log);
    externs +=  includeFunction<double, double>         (state, "log10",(void*)(double(*)(double)) log10);
    externs +=  includeFunction<double, double, double> (state, "pow",  (void*)(double(*)(double, double))&pow);
    externs +=  includeFunction<double, double>         (state, "sqrt", (void*)(double(*)(double))&sqrt);
    externs +=  includeFunction<double, double>         (state, "ceil", (void*)(double(*)(double))&ceil);
    externs +=  includeFunction<double, double>         (state, "floor",(void*)(double(*)(double))&floor);
    externs +=  includeFunction<double, double>         (state, "fabs", (void*)(double(*)(double))&fabs);
    externs +=  includeFunction<double, double, int>    (state, "ldexp",(void*)(double(*)(double, int))&ldexp);
    externs +=  includeFunction<double, double, int*>   (state, "frexp",(void*) (double(*)(double, int*))&frexp);
    //externs +=  includeFunction<double, double  double*>(state, "modf", (void*)(double(*)(double, double*))&modf);
    externs +=  includeFunction<double, double, double> (state, "fmod", (void*)(double(*)(double, double))&fmod);

    externs +=  includeFunction<int, END> (state, "rand", (void*)(int(*)())&rand);
    
    
    externs +=  includeFunction<double, double, double>         (state, "fmin",  (void*)(double(*)(double, double))&std::fmin);
    externs +=  includeFunction<double, double, double>         (state, "fmax",  (void*)(double(*)(double, double))&std::fmax);
    

    externs +=  includeFunction<int, const char*, const char*> (state, "strcmp", (void*)(int(*)(const char*, const char*))&strcmp);
    
    externs +=  includeFunction<void, const char*> (state, "print", (void*)Compiler::print);
    
    //externs +=  includeFunction<int, const char*, const char*> (state, "strlen", (void*)(int(*)(const char*))&strlen);
    
    
    externs += Cerite::Compiler::includeFunction<void, double*, double*, double*>(state, "solve",  (void*)(void(*)(double*, double*, double*))&Cerite::Solver::callSolve);
    
    externs += includeVariable<double>(state, "M_LOG2E", M_LOG2E);
    externs += includeVariable<double>(state, "M_LOG10E", M_LOG10E);
    externs += includeVariable<double>(state, "M_LN2", M_LN2);
    externs += includeVariable<double>(state, "M_LN10", M_LN10);
    externs += includeVariable<double>(state, "M_PI", M_PI);
    externs += includeVariable<double>(state, "M_PI_2", M_PI_2);
    externs += includeVariable<double>(state, "M_PI_4", M_PI_4);
    externs += includeVariable<double>(state, "M_1_PI", M_1_PI);
    externs += includeVariable<double>(state, "M_2_PI", M_2_PI);
    externs += includeVariable<double>(state, "M_2_SQRTPI", M_2_SQRTPI);
    externs += includeVariable<double>(state, "M_SQRT2", M_SQRT2);
    externs += includeVariable<double>(state, "M_SQRT1_2", M_SQRT1_2);

    File output = File(String(path)).getChildFile("externs.h");
    output.create();
    output.replaceWithText(String(externs));
    
}

}
