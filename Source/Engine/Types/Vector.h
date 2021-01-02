#pragma once
#include <vector>
#include <cmath>
#include <boost/algorithm/string.hpp>
#include "Variable.h"
#include "../IO/Token.h"

namespace Cerite {


struct Vector
{
    
    std::string name;
    int size;
    int dims;
    std::vector<TokenString> definition;
    bool local;
    bool update = false;
    double* start;
    std::string origin;
    std::string ctype;
    std::vector<int> ports;
    
    
    Vector() {};
    
    Vector(const std::string& title, int numElts, int dim = 1, bool loc = true) : name(title), size(numElts), dims(dim), local(loc) {
        definition.resize(pow(numElts, dim));
    };
    

    void formatCell(std::string& m_string) {
        boost::erase_all(m_string, ";");
        boost::erase_all(m_string, " ");
        boost::erase_all(m_string, "\n");
    }
    
    bool isFuncptr() const{
        return ctype.find("(*)") != std::string::npos;
    }
    
    void overlap(Vector& target, int newSize, std::vector<int> ports);
    
    std::vector<std::string> toString() const {
        std::vector<std::string> result;
        
        
        for(auto& cell : definition) {
            if(cell.empty()) {
                result.push_back("0");
            }
            else {
                result.push_back(cell.toString());
            }
            
            
        }
        
        return result;
    }
    
    std::string combineCells(std::string expr1, std::string expr2) {
        formatCell(expr1);
        formatCell(expr2);
        
        bool firstIsValid = !expr1.empty() && (expr1 != std::to_string(0)) && !(std::all_of(expr1.begin(), expr1.end(),isspace));
        bool secondIsValid = !expr2.empty() && (expr2 != std::to_string(0)) && !(std::all_of(expr2.begin(), expr2.end(),isspace));
        
        if(!firstIsValid && secondIsValid)   return expr2;
        if(!secondIsValid && firstIsValid)   return expr1;
        if(!secondIsValid && !firstIsValid)  return std::string();
        
        return "(" + expr1 + ") + (" + expr2 + ")";
    }
    
};
 
}
