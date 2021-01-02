#include "Vector.h"

namespace Cerite {


// TODO: make this work recusively?
void Vector::overlap(Vector& target, int newSize, std::vector<int> ports) {
    target.update = update;
    target.ctype = ctype;
    if(dims == 1) {
        for(int n = 0; n < size; n++) {
            if(n >= ports.size() || ports[n] < 0) continue;
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


}
