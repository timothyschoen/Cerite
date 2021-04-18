#pragma once

#include <boost/algorithm/string.hpp>
#include <libtcc.h>

namespace Cerite {

class Document;
struct Compiler
{
    
    typedef void (*TCCErrorFunc)(void *opaque, const char *msg);

    static inline void(*print)(const char*) = nullptr;
    
    TCCErrorFunc errfunc = [](void *opaque, const char *msg){
        if(print) print(msg);
        else printf(msg);
    };
    
    Compiler();
    
    static TCCState* compile(std::string code, void(*print)(const char* text) = nullptr);
    
    static TCCState* perform(std::string code, Compiler& compiler);
    
    template<typename R, typename ...Args>
    static std::string includeFunction(TCCState* state, std::string name, void* function) {
        
        std::string args;
        printArgs<Args...>(args);
        std::string rettype;
        printArgs<R, END>(rettype);
        
        tcc_add_symbol(state, name.c_str(), function);
        return "extern " + rettype + " " + name + "(" + args + ");\n";
    };
    
    template<typename T>
    static std::string includeVariable(TCCState* state, std::string name, T value) {
        std::string rettype;
        printArgs<T, END>(rettype);
        
        std::ostringstream stream;
        stream << value;
        
        return  "static " + rettype + " " + name + " = " + stream.str() + ";\n";
    };
    
private:
    
    static void addStdLibrary(TCCState* obj);
    
    struct END {};
    
    // Recursive template type printer for common types
    // This allows me to quickly write extern declarations in C
    template<typename T, typename ...Args>
    static constexpr void printArgs(std::string& arguments) {
        
        if(std::is_same<T, END>()) {
            if(arguments.size() >= 2) {
                arguments.resize (arguments.size() - 2);
            }
            return;
        }
        
        if(std::is_same<T, int>())
            arguments += "int, ";
        else if(std::is_same<T, double>())
            arguments += "double, ";
        else if(std::is_same<T, long>())
            arguments += "long, ";
        else if(std::is_same<T, double*>())
            arguments += "double*, ";
        else if(std::is_same<T, int*>())
            arguments += "int*, ";
        else if(std::is_same<T, const char*>())
            arguments += "const char*, ";
        else if(std::is_same<T, char*>())
            arguments += "char*, ";
        else if(std::is_same<T, char>())
            arguments += "char, ";
        else if(std::is_same<T, void>())
            arguments += "void, ";
        else if(std::is_same<T, void*>())
            arguments += "void*, ";
        
         printArgs<Args..., END>(arguments);
        
    }
    
    const static inline std::string path = "/Users/Timothy/Documents/Cerite/.exec/";
};
}
