#include <iomanip>
#include "Writer.h"

namespace Cerite {

std::string Writer::format(Document doc) {
    std::string result;
    
    result += "\n\nname: " + doc.name;
   
    
    result += writeVariables(doc, VARIABLE);
    result += writeVariables(doc, STATIC);
    
    /*
    for(auto domain : doc.size) {
        result += "\n\n " + domain.first + "_size: " + std::to_string(domain.second);
    } */
    
    /*
    std::vector<int> nodes = getNodes()
    if(doc.getNodes().size() > 0) {
        for(auto domain : doc.ports) {
            result += "\n\n" + domain.first + "_ports: ";
            for(auto port : domain.second) {
                result += std::to_string(port) + ", ";
            }
        }
                
                
            
        result.erase(result.length()-2, 2);
    } */

    for(auto& vec : doc.vectors)
        if(!vec.local)
            result += writeVector(vec);
    
    for(auto& func : doc.functions)
        result += writeFunction(func);
    
    return result;
    
}


std::string Writer::writeFunction(Function func) {
    std::string result;
          
    if(func.empty()) return "";
    
    
    result += "\n\n" + func.name + ":\n\t";
    result += func.body.toString() + "\n";
    
    boost::replace_all(result, ";", ";\n\t");

    return result;
}


std::string Writer::writeVector(Vector vec) {
    
    /*
    if(!vec.update) return "";
    
    std::string result;
    
    
    if(vec.dims == 2) {
        std::vector<std::string> matrix = vec.toString();
        
        std::vector<int> maxLengths(vec.size, 0);

        int minimumSize = 10;
        // Find the biggest entry for each column
        for(int i = 0; i < vec.size; i++) {
            int maximum = std::min(minimumSize, 15);
            for(int j = 0; j < vec.size; j++) {
                
                if((int)matrix[j * vec.size + i].size() > 20) matrix[i * vec.size + j];
                maximum = std::max(maximum, (int)matrix[j * vec.size + i].size());
            }
            maximum += maximum % 2;
            maxLengths[i] = maximum + 2;
        }
         
        std::ostringstream formatted;
        
        // Matrix formatting
        for(int i = 0; i < vec.size; i++) {
            for(int j = 0; j < vec.size; j++) {
                formatted << std::setw(maxLengths[j]) << matrix[i * vec.size + j] << ", ";
            }
            formatted << std::endl;
        }
        result += formatted.str();
    }
    
    else {
        // Vector formatting
        std::vector<std::string> vector = vec.toString();
        for(auto str : vector) {
            //str.trimSpace();
            if(str.empty())
                str = "0";
            result += str + ", ";
        }
    }
    
    if(result.size() > 2 && !std::all_of(result.begin(), result.end(), isspace)) {
        // Remove last comma
        result.erase(result.length()-2, 2);
        result.insert(0, "\n\n " + vec.name + ":\n");
    }
    
    return result; */
    
    return "";
}


std::string Writer::writeVariables(Document& doc, vartype type) {
    
    std::string result;
    std::vector<Variable> variables = doc.variables;
    for(auto& var : variables) {
        if(!var.isVector && var.isStatic && type == STATIC)
            result += var.name + ", ";
        else if (!var.isVector && !var.isStatic && type != STATIC)
            result += var.name + ", ";
    }
    
    if(type == STATIC)
        for(auto& vec : doc.vectors)
            if(vec.local)
                result += vec.name + "[" + std::to_string(vec.size) + "]" + ", ";
        
    if(result.size() > 2 && !std::all_of(result.begin(), result.end(), isspace)) {
        result.erase(result.length()-2, 2);
        
        if(type == 0)
            result.insert(0, "\n\nvariables: ");
        else if(type == 1)
            result.insert(0, "\n\nstatic: ");
    }
    
    return result;
}
     

}
