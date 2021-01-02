#include "Preprocessor.h"
#include "Compiler.h"
#include "../Interface/Document.h"
#include <boost/algorithm/string.hpp>


namespace Cerite {

void Preprocessor::preprocess(Document& doc) {
    
    for(auto& func : doc.functions) {
        expandStatements(func, doc);
    }
}

void Preprocessor::expandOuterLoop(TokenString& tokens, Document& doc) {
    int loopstart = 0;
    int loopend = (int)tokens.size()-1;
    
    while(loopend > 0 && tokens[loopend]->type != tPreprocessor && tokens[loopend]->symbol != "END")
        loopend--;
    
    if(loopend == 0) return;
    
    loopstart = loopend - 1;
    
    int nest = 0;
    bool found = false;
    while(!found) {
        if(loopstart < 0)
            break;
        if(tokens[loopstart]->type == tPreprocessor && tokens[loopstart]->symbol == "END")
            nest++;
        else if(tokens[loopstart]->type == tPreprocessor)
            nest--;
        if(nest == -1)
            found = true;
        else
            loopstart--;
    }
    
    
    if(!found) {
        std::cerr << "Error: invalid for-loop" << std::endl;
        return;
    }
    
    TokenString loop = tokens.substr(loopstart, loopend);
    TokenString result;
    
    if(tokens[loopstart]->symbol == "FOR")
        expandLoop(doc, result, loop);
    else return;

    tokens.erase(loopstart, loopend + 1);
    tokens.insert(result.tokens, loopstart);
}


void Preprocessor::expandStatements(Function& codestr, Document& doc) {
    
    while(codestr.body.count("FOR")) {
        expandOuterLoop(codestr.body, doc);
    }
        
}


void Preprocessor::expandLoop(Document& doc, TokenString& result, TokenString& body) {
    
    std::string replace = body[0]->args[0].symbol;
    int start = valueFromDocument(doc, body[0]->args[1].symbol);
    int end = valueFromDocument(doc, body[0]->args[2].symbol);
    
    bool inverse = end < start;
    
    if(inverse) {
        for(int i = start - 1; i >= end ; i--) {
            TokenString copy(body.substr(1, body.size()));
            copy.replaceSymbol(replace, std::to_string(i), true);
            result.append(copy.tokens);
        }
    }
    else {
        for(int i = start; i < end ; i++) {
            TokenString copy(body.substr(1, body.size()));
            copy.replaceSymbol(replace, std::to_string(i), true);
            result.append(copy.tokens);
        }
    }
    
}


double Preprocessor::valueFromDocument(Document& doc, std::string& var) {
    double val;
    if(!var.empty() && std::all_of(var.begin(), var.end(), ::isdigit))
        val = std::stod(var);
    else if(doc.getVariable(var).predefined) {
        val = doc.getVariable(var).init;
    }
    else {
        val = evaluate(doc, var);
    }
    return val;
}

// Quick math evaluator
double Preprocessor::evaluate(Document& doc, std::string s) {

    TokenString tokens(s, doc.vtable);
    
    for(auto& token : tokens) {
        if(token.type == tVariable && doc.getVariable(token.symbol).predefined) {
            token.symbol = std::to_string(doc.getVariable(token.symbol).init);
            token.type = tConst;
        }
    }
    
    TCCState* state = tcc_new();
    
    tcc_set_output_type(state, TCC_OUTPUT_MEMORY);
    
    if(!state) {
        printf("Can’t create a TCC context\n");
        return 0;
    }
    
    std::string ccode = "double calc() { return " + tokens.toC(doc) + "; }";
    
    if (tcc_compile_string(state, ccode.c_str()) > 0) {
        printf("Compilation error !\n");
        return 0;
    }
    
    tcc_relocate(state, TCC_RELOCATE_AUTO);
    
    auto calc = (double(*)())tcc_get_symbol(state, "calc");

    return calc();
}
}
