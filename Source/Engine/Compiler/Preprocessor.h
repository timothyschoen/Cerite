#pragma once
#include <iostream>
#include <libtcc.h>
#include "../Types/Function.h"

namespace Cerite {

class Document;
class Preprocessor
{

    
public:
    
    static double valueFromDocument(Document& doc, std::string& var);
    
    static void preprocess(Document& doc);
    
    static void expandStatements(Function& code, Document& doc);
    static void expandLoop(Document& doc, TokenString& result, TokenString& body);
    
    static double evaluate(Document& doc, std::string code);
    
    static void expandOuterLoop(TokenString& tokens, Document& d);
    
    static std::pair<size_t, size_t> getInnerLoop(std::vector<size_t>& start, std::vector<size_t>& end);
};

}
