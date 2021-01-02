#pragma once
#include <unordered_map>
#include <vector>
#include <string>
#include <iostream>
#include <boost/algorithm/string.hpp>
#include "VarTable.h"



namespace Cerite {

static const size_t npos = std::string::npos;

class Document;
struct Token
{
    std::string digits = "1234567890.-e";
    
    TokenType type;
    std::string symbol;
    std::vector<Token> args; // for any additional information
    bool rvalue = true;
    
    Token(std::string name, TokenType tokentype, std::vector<Token> arguments = {}) : symbol(name), type(tokentype) {
    }
    
    bool empty() const {
        return type == tSpacing || type == tComment || symbol == "";
    };
    
    void replaceSymbol(std::string find, std::string replace) {
        if(symbol == find)
            symbol = replace;
        
        for(auto& arg : args)
            arg.replaceSymbol(find, replace);
        
        if(symbol.empty() && std::all_of(symbol.begin(), symbol.end(), ::isdigit)) {
            type = tConst;
        }
    }
};


struct TokenString
{
    VarTable chars;
    
    std::vector<Token> tokens;
    
    TokenString() {};
    
    TokenString(std::vector<Token> tok, VarTable chartable = VarTable()) : tokens(tok), chars(chartable) {
    };
    

    TokenString(std::string code, VarTable chartable = VarTable()) : chars(chartable) {
        tokenize(tokens, chartable, code);
        
    }
    
    size_t size() const {
        return tokens.size();
    }
    
    bool empty() const {
        bool empty = true;
       
        for(auto& token : tokens)
            empty &= token.empty();
        
        return empty;
    }

    
    void append(std::vector<Token> newtokens) {
        for(auto& token : newtokens)
            tokens.push_back(token);
    }
    
    void insert(std::vector<Token> newtokens, int position = 0) {
        tokens.insert(tokens.begin() + position, newtokens.begin(), newtokens.end());
    }
    
    void insert(Token token, int position = 0) {
        tokens.insert(tokens.begin() + position, token);
    }
    void erase(size_t start, size_t end) {
        tokens.erase(tokens.begin() + start, tokens.begin() + end);
    }
    
    // Function for finding matching pairs of characters (like {}, () or [])
    static int matchNests(const std::string& str, std::pair<std::string, std::string> selectors) {
        int end = 0;
        int nest = 0;
        bool found = false;
        while(!found) {
            if(end >= str.size())
                break;
            
            std::string chr = std::string(&str[end], 1);
            if(chr == selectors.second)
                nest--;
            else if(chr == selectors.first)
                nest++;
            if(nest == -1)
                found = true;
            else
                end++;
        }
        
        if(!found)
            std::cerr << "Error: non-matching braces!" << std::endl;
        
        return end;
        
    }
    
    
    TokenString substr(size_t start, size_t end) const {
        return TokenString(std::vector<Token>(tokens.begin() + start, tokens.begin() + end));
    }
    
    
    Token* operator[](int index)
    {
        return &tokens[index];
    }
    
    const Token* operator[](int index) const
    {
        return &tokens[index];
    }
    
    
    std::vector<Token>::iterator begin() {
        return tokens.begin();
    }
    
    std::vector<Token>::iterator end() {
        return tokens.end();
    }
    
    size_t count(const std::string& name) const {
        int numFound = 0;
        for(auto& token : tokens)
            if(token.symbol == name)
                numFound++;
        
        return numFound;
    }

    

    std::vector<Token*> operator[](const std::string& name) {
        return getSymbols(name);
    }
    
    std::vector<Token*> getSymbols(const std::string& name);
    
    // Replace all occurances of a name
    void replaceSymbol(const std::string& oldname, const std::string& newname, bool deepReplaceArgs = false);
    
    std::string printVector(const Token& vector, const Document& doc) const;
    
    std::string toString(const Document& doc) const ;
    
    std::string toString() const;
    
    std::string toC(const Document& doc) const;
    
    static inline const std::string WHITESPACE = " \n\r\t\f\v";
    
    static void removeAllSpace(std::string& str) {
        str.erase(std::remove_if(str.begin(), str.end(), ::isspace), str.end());
    }
    
    static std::string ltrim(const std::string& s)
    {
        size_t start = s.find_first_not_of(WHITESPACE);
        return (start == std::string::npos) ? "" : s.substr(start);
    }
     
    static std::string rtrim(const std::string& s)
    {
        size_t end = s.find_last_not_of(WHITESPACE);
        return (end == std::string::npos) ? "" : s.substr(0, end + 1);
    }
     
    static std::string trim(const std::string& s)
    {
        return rtrim(ltrim(s));
    }
     
    
    
    void tokenize(std::vector<Token>& result, VarTable chars, std::string str);

};

}
