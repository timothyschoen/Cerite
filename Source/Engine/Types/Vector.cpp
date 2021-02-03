#include "Vector.h"
#include <iomanip>

namespace Cerite {


// TODO: make this work recusively?
void Vector::overlap(Vector& target, int newSize, std::vector<int> ports) {
    target.update = update;
    target.ctype = ctype;
    
    if(definition.empty()) return;
    
    if(dims == 1) {
        for(int n = 0; n < size; n++) {
            if(n >= ports.size() || ports[n] < 0 || n >= target.definition.size()) continue;
            else {
                target.definition[ports[n]] = TokenString(combineCells(target.definition[ports[n]].toString(), definition[n].toString()), target.definition[n].chars);
                
            }
        }
    }
    
    else if(dims == 2) {
        for(int m = 0; m < size; m++) {
            if(ports[m] < 0) continue;
            for(int n = 0; n < size; n++) {
                if(ports[n] < 0) continue;
                
                size_t newidx = ports[m] * newSize + ports[n];
                size_t oldidx = m * size + n;
                
                target.definition[newidx] = TokenString(combineCells(target.definition[newidx].toString(), definition[oldidx].toString()), target.definition[newidx].chars);
            }
            
        }
        
    }
    
}

std::string Vector::writeInitialiser() const {
    std::string result;
    size_t funcptr = ctype.find("(*)");
    if(funcptr != std::string::npos) {
        std::string initialiser = ctype;
        initialiser.insert(funcptr + 2, name + "[" + std::to_string((int)pow(size, dims)) + "]");
        result += initialiser;
    }
    else {
        result += ctype + " " + name + "[" + std::to_string((int)pow(size, dims)) + "]";
    }
    
    return result;
}

std::string Vector::toString(const Document& doc) const {
    std::string result;
    
    for(int i = 0; i < definition.size(); i++) {
        if(isFuncptr()) {
            if(definition[i].empty()) continue;
            if(i != 0) {
            result += "\t" + name + "[" + std::to_string(i) + "]" + " = " + definition[i].toC(doc) + ";\n";
            }
            else {
                result += "\t" + name + "[0] = do_nothing;\n";
            }
        }
        else {
            result += "\t" + name + "[" + std::to_string(i) + "]" + " = " + (definition[i].empty() ? "0" :  definition[i].toC(doc)) + ";\n";
        }
    }
    
    return result;
}

}
