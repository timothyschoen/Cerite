#pragma once

#include <JuceHeader.h>
#include <libtcc.h>
#include "../../Utility/Solver.h"
#include "../../Types/Data.h"
#include "AudioPlayer.h"
//==============================================================================
/*
 This component lives inside our window, and this is where you should put all
 your controls and content.
 */
struct Processor
{
    
    Cerite::Solver solver;
    
    
    OwnedArray<ExternalProcessor> external;
    
    
    TCCState* state;
    
    typedef void (*TCCErrorFunc)(void *opaque, const char *msg);
    inline static TCCErrorFunc log = [](void *opaque, const char *msg) {
        printf(msg);
    };
    
    typedef void (*PrintFunc)(const char *msg);
    
    static void print(const char* msg) {
        log(nullptr, msg);
    }
    
    typedef void (*fptr)();
    typedef double*(*vecptr)();
    typedef void(*datafunc)(Data);
    
    std::string c_code;
    
    fptr initfunc;
    fptr calcfunc;
    fptr trfunc;
    fptr newtonfunc;
    fptr solvefunc;
    fptr updatefunc;
    
    int mna_size = 0;
    double* output;
    
    double* A;
    double* b;
    double* x;
    
    std::vector<datafunc> datafunctions;
    
    
    
    Processor(std::string code);
    
    void loadData(String name, int idx);
    void setData(int idx, Data data);
    
    bool prepareToPlay(int samplesPerBlockExpected, double sampleRate);
    
    void processBlock(AudioBuffer<float>& bufferToFill);
    
    void initializeVariables();
    
    TCCState* compile(std::string code);
    
    std::string addStdLibrary();
    
    struct END {};
    
    // Recursive template type printer for common types
    // This allows me to quickly write extern declarations in C
    template<typename T, typename ...Args>
    constexpr void printArgs(std::string& arguments) {
        
        if(std::is_same<T, END>()) {
            arguments.resize (arguments.size() - 2);
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

    template<typename R, typename ...Args>
    std::string includeFunction(TCCState* state, std::string name, void* function) {
        
        std::string args;
        printArgs<Args...>(args);
        std::string rettype;
        printArgs<R, END>(rettype);
        
        tcc_add_symbol(state, name.c_str(), function);
        return "extern " + rettype + " " + name + "(" + args + ");\n";
    };
    
    template<typename T>
    std::string includeVariable(TCCState* state, std::string name, T value) {
        std::string rettype;
        printArgs<T, END>(rettype);
        
        std::ostringstream stream;
        stream << value;
        
        return  rettype + " " + name + " = " + stream.str() + ";\n";
    };
    
    double* getVariablePtr(std::string name) {
        double* result = (double*)tcc_get_symbol(state, name.c_str());
        
        if(result != NULL)
            return result;
        else
            return nullptr;
    }
    
    fptr getFunctionPtr(std::string name) {
        fptr result = (fptr)tcc_get_symbol(state, name.c_str());
        
        if(result != NULL)
            return result;
        else
            return [](){};
    }
    
    double* getVectorPtr(std::string name) {
        vecptr initfunc = (vecptr)tcc_get_symbol(state, (name + "_ptr").c_str());
        
        if(initfunc != NULL)
            return initfunc();
        else
            return nullptr;
    }
    
    void setVariable(std::string name, double value) {
        *(double*)tcc_get_symbol(state, name.c_str()) = value;
    }
    

    
    // string that contains the Data type for C code
    static inline const std::string datatype = "typedef enum {\n"
    "  tBang,\n"
    "  tNumber,\n"
    "  tString,\n"
    "  tList\n"
    "} Type;\n"
    "\n"
    "typedef struct {\n"
    "\n"
    "    Type type;\n"
    "    double number;\n"
    "    const char* string;\n"
    "    double* list;\n"
    "    unsigned int listlen;\n"
    "\n"
    "} Data;\n"
    "\n"
    "Data Bang() {\n"
    "  Data bangdata = {tBang, 0, \"bang\", 0, 0};\n"
    "  //bangdata.getBang\n"
    "  return bangdata;\n"
    "}\n"
    "Data Number(double value) {\n"
    "  Data numdata = {tNumber, value, \"\", 0, 0};\n"
    "  return numdata;\n"
    "}\n"
    "Data String(const char* str) {\n"
    "  Data stringdata = {tString, 0, str, 0, 0};\n"
    "  return stringdata;\n"
    "}\n"
    "Data List(double* values, unsigned int size) {\n"
    "  Data listdata = {tList, 0, \"\", values, size};\n"
    "  return listdata;\n"
    "}\n"
    "#define getType() get_type(data)\n"
    "Type get_type(Data data) {\n"
    "  return data.type;\n"
    "}\n"
    "#define isString() is_string(data)\n"
    "int is_string(Data data) {\n"
    "return data.type == tString;\n"
    "}\n"
    "#define isNumber() is_number(data)\n"
    "int is_number(Data data) {\n"
    "return data.type == tNumber;\n"
    "}\n"
    "#define isBang() is_bang(data)\n"
    "int is_bang(Data data) {\n"
    "return data.type == tBang;\n"
    "}\n"
    "#define getString() get_string(data)\n"
    "const char* get_string(Data data) {\n"
    "  if(data.type == tString) {\n"
    "    return data.string;\n"
    "  }\n"
    "  else {\n"
    "    print(\"No Type String\");\n"
    "    return \"undefined\";\n"
    "  }\n"
    "\n"
    "}\n"
    "\n"
    "#define getNumber() get_number(data)\n"
    "double get_number(Data data) {\n"
    "  if(data.type == tNumber) {\n"
    "    return data.number;\n"
    "  }\n"
    "  else {\n"
    "    print(\"No Type Number\");\n"
    "    return 0;\n"
    "  }\n"
    "}\n"
    "\n"
    "#define getList() get_list(data)\n"
    "double* get_list(Data data) {\n"
    " return 0;\n"
    "}\n"
    "void do_nothing(Data data) {\n"
    "}\n"
    "void do_nothing2(int num, Data data) {\n"
    "}\n"
    "void(*sendGui)(int, Data) = do_nothing2;\n"
    "void registerGui(void(*func)(int, Data)) {"
    " sendGui = func;"
    "}";
};
