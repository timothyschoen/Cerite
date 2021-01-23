#include "Token.h"
#include "../Interface/Document.h"

namespace Cerite {


void TokenString::tokenize(std::vector<Token>& result, VarTable chars, std::string str) {
   
    
    for (size_t s=0; s < str.length();)
    {
        size_t jump = 0;
        size_t e = s;
        TokenType type = tUnknown;
        std::vector<Token> args;
        
        if (e < str.size() && str[e] == '/') {
            type = tComment;
            
            if(str[e + 1] == '/')
                while(str[e] != '\n')
                    e++;
            else if (str[e + 1] == '*')
                while(e < str.size() && str[e] != '*' && str[e + 1] != '/')
                    e++;
        }
        
        if (str[e] == '#') {
            type = tPreprocessor;
            s++;
            
            if(str.substr(s, 3) == "FOR") {
                
                e = s + 3;
                size_t it = e;
                size_t start;
                size_t end;
                std::vector<std::string> arguments(1);
                
                while(it < str.size() && chars.space.find(str[it]) != npos) it++;
                
                while(it < str.size() && chars.isVariableChar(str[it])) {
                    arguments[0] += str[it];
                    it++;
                }
                
                while(it < str.size() && str[it] != '(') it++;
                start = it + 1;
                
                while(it < str.size() && str[it] != ')') it++;
                end = it;
                
                while(it < str.size() && str[it] != '\n') it++;
                jump = it - e;
                
                std::string range = str.substr(start, end - start);
                boost::erase_all(range, " ");
                std::vector<std::string> split;
                boost::split(split, range, boost::is_any_of(","));
                
                
                arguments.insert(arguments.end(), split.begin(), split.end());
                
                for(size_t i = 0; i < arguments.size(); i++)
                args.push_back(Token(arguments[i], tPreprocessor));
                
            }
            
            if(str.substr(s, 2) == "IF") {
                e = s + 2;
                size_t it = e;
                
                while(it < str.size() && str[it] != '\n') it++;
                args.push_back(Token(str.substr(e, it), tPreprocessor));
            }
            
            // you can write ENDIF and ENDFOR, it will be ignored
            if(str.substr(s, 3) == "END") {
                e = s + 3;
                size_t it = e;
                while(it < str.size() && str[it] != '\n') it++;
                jump = it - e;
            }
        }
        
        else if (std::isdigit(str[e])) {
            type = tConst;
            while(e < str.size() && chars.digits.find(str[e]) != npos)
                e++;
        }
        // if it starts with a letter, it's a variable of sorts
        else if(std::isalpha(str[e]) || str[e] == '_') {
            while(e < str.size() && chars.isVariableChar(str[e]))
                e++;
            
            type = chars.getType(str.substr(s, e - s));

            if(type == tVector) {
                
                int brackpos = e;
                while(::isspace(str[brackpos])) {
                    brackpos++;
                }
                
                bool hasSubscript = str[brackpos] == '[';
                
                if(hasSubscript) {
                size_t lbrack = str.find("[", e) + 1;
                size_t split = str.find("][", lbrack);
                size_t rbrack = matchNests(str.substr(lbrack), {"[", "]"}) + lbrack;
                
                bool twodim = split <= rbrack;
                
                if(twodim) {
                    rbrack = matchNests(str.substr(split + 2), {"[", "]"}) + split + 2;
                    std::string arg1 = str.substr(lbrack, split - lbrack);
                    std::string arg2 = str.substr(split + 2, rbrack - (split + 2));
                    
                    boost::erase_all(arg1, " ");
                    boost::erase_all(arg2, " ");
                    
                    args.push_back(Token(arg1, std::isdigit(arg1[0]) ? tConst : tVariable));
                    args.push_back(Token(arg2, std::isdigit(arg2[0]) ? tConst : tVariable));
                }
                else {
                    std::string arg = str.substr(lbrack, rbrack - lbrack);
                    boost::erase_all(arg, " ");
                    args.push_back(Token(arg, std::isdigit(arg[0]) ? tConst : tVariable));
                }
                jump = (rbrack + 1) - e;
            }
          }
        }
        
        else if(chars.operators.find(str[e]) != npos) {
            type = tOperator;
            while(e < str.size() && chars.operators.find(str[e]) != npos)
                e++;
        }
        else if(e < str.size() && chars.parens.find(str[e]) != npos) {
            type = tOperator;
            e++;
        }
        
        else if(str[e] == ';') {
            type = tEnd;
            e++;
        }

        else if(chars.space.find(str[e]) != npos) {
            type = tSpacing;
            // we want to completely ditch spaces with newlines and reconstruct them later using the positions of semicolons
            bool newline = false;
            while(e < str.size() && chars.space.find(str[e]) != npos) {
                if(str[e] == '\n')
                    newline = true;
                e++;
            }
            if(newline)  {
                jump = e - s;
                e = s;
            }
        }
        
        else if (e < str.size()){
            type = tUnknown;
            e++;
        }
        
        std::string symbol = str.substr(s, e - s);

        if(symbol == "=" || symbol == "+=" || symbol == "*=" || symbol == "-=" ||  symbol == "/=" ||  symbol == "%=" || symbol == "<<=" || symbol == ">>=" || symbol == "|=" || symbol == "&=") {
            for(int i = result.size()-1; i >= 0; i--) {
                if(result[i].type == tVariable || result[i].type == tVector) {
                    result[i].rvalue = false;
                    break;
                }
            }
        }
        
        result.push_back(Token(symbol, type));
        result.back().args = args;
        
        
        s = e + jump;
    }
    return;
}


std::vector<Token*> TokenString::getSymbols(const std::string& name) {
    std::vector<Token*> result;
    
    for(auto& token : tokens) {
        if(token.symbol == name)
            result.push_back(&token);
        
        for(auto& arg : token.args)
            if(arg.symbol == name)
                result.push_back(&arg);
    }
    return result;
}
// Replace all occurances of a name
void TokenString::replaceSymbol(const std::string& oldname, const std::string& newname, bool deepReplaceArgs) {
        
    chars.replaceSymbol(oldname, newname);
    
    for(auto& token : tokens)
        token.replaceSymbol(oldname, newname);
    
    if(!deepReplaceArgs) return;
    
    for(auto& token : tokens) {
        for(auto& arg : token.args) {
            TokenString tokenized = TokenString(arg.symbol, chars);
            tokenized.replaceSymbol(oldname, newname);
            arg = Token(tokenized.toString(), token.type, arg.args);
        }
    }
}

std::string TokenString::printVector(const Token& vector, const Document& doc) const {
    std::string result = vector.symbol;
    
    const Vector& vec = *doc.getVectorPtr(result);

    if(vector.args.size() == 1 && vector.args[0].type == tConst) {
        result += vector.args[0].symbol;
    }
    else if(vector.args.size() == 2 && vector.args[0].type == tConst && vector.args[1].type == tConst) {
        result += vector.args[0].symbol + "_" + vector.args[1].symbol;
    }
    else if (vector.args.size() == 1){
        TokenString argument(vector.args[0].symbol, chars);
        
        std::string idx = argument.tokens[0].type == tVector ? printVector(argument.tokens[0], doc) : vector.args[0].symbol;
        result += "0[(" + idx + ")";
        
    }
    else if(vector.args.size() == 2) {
        TokenString first(vector.args[0].symbol, chars);
        TokenString second(vector.args[1].symbol, chars);
        
        std::string idx1 = first.tokens[0].type == tVector ? printVector(first.tokens[0], doc) : vector.args[0].symbol;
        std::string idx2 = second.tokens[0].type == tVector ? printVector(second.tokens[0], doc) : vector.args[1].symbol;
            
        result += "0_0[(" + idx1 + "*" + std::to_string(vec.size) + "+" + idx2 + ")";
    }
    
    return result;
}


std::string TokenString::toString() const {
    std::string result;
    if(!tokens.size()) return "";
    
    for(auto& token : tokens) {
        if(token.type == tVector) {
            if(token.args.size() == 2)
                result += token.symbol + "[" + token.args[0].symbol + "][" + token.args[1].symbol + "]";
            else
                result += token.symbol + "[" + token.args[0].symbol + "]";
        }
        else {
            result += token.symbol;
            
        }
    }

    return result;
    
}

std::string TokenString::toString(const Document& doc) const {
    std::string result;
    for(auto& token : tokens) {
        if(token.type == tVector)
            result += printVector(token, doc);
        else
            result += token.symbol;
    }

    return result;
    
}


std::string TokenString::toC(const Document& doc) const {
    std::string result;
    for(auto& token : tokens) {
        if(token.type == tVector && token.args.size() > 0) {
            const Vector& vec = *doc.getVectorPtr(token.symbol);
            if(token.args.size() == 2) {
                if(token.args[0].type == tConst && token.args[1].type == tConst) {
                    result += token.symbol + "[" + std::to_string(std::stoi(token.args[0].symbol) * vec.size +  std::stoi(token.args[1].symbol)) + "]";
                }
                else {
                    result += token.symbol + "[" + token.args[0].symbol + " * " + std::to_string(vec.size) + " + " + token.args[1].symbol + "]";
                }
            }
            else {
                result += token.symbol + "[" + token.args[0].symbol + "]";
            }
        }
        else {
            result += token.symbol;
            
        }
    }

    return result;
    
}

}
