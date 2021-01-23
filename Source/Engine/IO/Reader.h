#pragma once

#include <string>
#include <algorithm>
#include "../IO/VarTable.h"
#include "../IO/Token.h"

namespace Cerite {


class Document;
struct Reader
{
    Document& parse(std::string path);
    
private:
    
    void addToVarTable(VarTable& vtable, Document& doc, std::string stringToParse);
    
    void handleSection(Document& doc, const std::string& selector, const std::string& content, const VarTable& table);
    
    void parseVariables(Document& doc, VarTable& table, std::string varstring, bool dynamic);
    
    void getImports(Document& doc, VarTable& table, std::string stringToParse);
    
    static std::string encode(const std::string &input);
    
};

}
