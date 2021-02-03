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
    
    static std::string addStdLibrary(TCCState* obj);
    
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

    
    // string that contains the Data type for C code
    static inline const std::string datatype = "typedef enum {\n"
    "      tBang,\n"
    "      tNumber,\n"
    "      tString,\n"
    "      tList\n"
    "    } Type;\n"
    "\n"
    "\n"
    "    typedef struct {\n"
    "\n"
    "        Type type;\n"
    "        double number;\n"
    "        const char* string;\n"
    "        struct Data* list;\n"
    "        unsigned int listlen;\n"
    "\n"
    "    } Data;\n"
    "\n"
    "    void freeData(Data data) {\n"
    "        if(data.listlen > 0) {\n"
    "            free(data.list);\n"
    "            data.listlen = 0;\n"
    "        }\n"
    "    }\n"
    "\n"
    "    #define createList(len) create_list(f_arg0, len)\n"
    "\n"
    "    void create_list(Data data, int len) {\n"
    "        if(data.listlen > 0) {\n"
    "            free(data.list);\n"
    "        }\n"
    "        data.list = malloc(len * sizeof(Data));\n"
    "        data.listlen = len;\n"
    "    }\n"
    "\n"
    "\n"
    "    Data Bang() {\n"
    "      Data bangdata = {tBang, 0, \"bang\", 0, 0};\n"
    "      //bangdata.getBang\n"
    "      return bangdata;\n"
    "    }\n"
    "    Data Number(double value) {\n"
    "      Data numdata = {tNumber, value, \"\", 0, 0};\n"
    "      return numdata;\n"
    "    }\n"
    "    Data String(const char* str) {\n"
    "      Data stringdata = {tString, 0, str, 0, 0};\n"
    "      return stringdata;\n"
    "    }\n"
    "    Data List(Data* values, unsigned int size) {\n"
    "      Data listdata = {tList, 0, \"\", values, size};\n"
    "      listdata.list = malloc(size * sizeof(Data));\n"
    "      memcpy(listdata.list, values, size * sizeof(Data));\n"
    "      return listdata;\n"
    "    }\n"
    "    #define getType() get_type(f_arg0)\n"
    "    Type get_type(Data data) {\n"
    "      return data.type;\n"
    "    }\n"
    "    #define isString() is_string(f_arg0)\n"
    "    int is_string(Data data) {\n"
    "    return data.type == tString;\n"
    "    }\n"
    "    #define isNumber() is_number(f_arg0)\n"
    "    int is_number(Data data) {\n"
    "    return data.type == tNumber;\n"
    "    }\n"
    "    #define isBang() is_bang(f_arg0)\n"
    "    int is_bang(Data data) {\n"
    "    return data.type == tBang;\n"
    "    }\n"
    "    #define getString() get_string(f_arg0)\n"
    "    const char* get_string(Data data) {\n"
    "      if(data.type == tString) {\n"
    "        return data.string;\n"
    "      }\n"
    "      else {\n"
    "        print(\"No Type String\");\n"
    "        return \"undefined\";\n"
    "      }\n"
    "\n"
    "    }\n"
    "\n"
    "    #define getNumber() get_number(f_arg0)\n"
    "    double get_number(Data data) {\n"
    "      if(data.type == tNumber) {\n"
    "        return data.number;\n"
    "      }\n"
    "      else {\n"
    "        print(\"No Type Number\");\n"
    "        return 0;\n"
    "      }\n"
    "    }\n"
    "\n"
    "    #define getList() get_list(f_arg0)\n"
    "    Data* get_list(Data data) {\n"
    "     return data.list;\n"
    "    }\n"
    "    void do_nothing(Data data) {\n"
    "     if(data.listlen != 0) free(data.list);\n"
    "    }\n"
    "    void do_nothing2(int num, Data data) {\n"
    "     if(data.listlen != 0) free(data.list);\n"
    "    }\n"
    "    void(*sendGui)(int, Data) = do_nothing2;\n"
    "    void registerGui(void(*func)(int, Data)) {\n"
    "     sendGui = func;\n"
    "    };\n";
};
}
