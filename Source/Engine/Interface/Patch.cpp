#include "Patch.h"
#include "Document.h"

namespace Cerite {

Patch::Patch(Document& doc) : Object(doc){
    nets = doc.size["mna"];
}


Patch::~Patch() {
}

// Prepare for audio playback
void Patch::init(double deltaTime) {

    initfunc    = loadFunction("prepare");
    calcfunc    = loadFunction("calc");
    newtonfunc  = loadFunction("newton");
    updatefunc  = loadFunction("update");
    solvefunc   = loadFunction("solve");
    
    // DC solution (optional)
    
    /*
    data[0] = 0;
    data[1] = 0;
     solve here
    */
    setVariable("dt", deltaTime);
    setVariable("sr", 1. / deltaTime);
    
    A = getVariablePtr("mna_A");
    b = getVariablePtr("mna_b");
    x = getVariablePtr("mna_x");
    
    
    initfunc();
    updatefunc();
    
    solver.init(A, nets);
    
    
}


void Patch::tick() {
    
    calcfunc();
    updatefunc();
    
    for(int i = 0; i < 1; i++) {

        solver.solve(A, b, x);
        //solvefunc();
         
        //data[2] = 1;
        newtonfunc();
        //updatefunc();
        
        // check if done
        //if(data[2]) break;
    }
}

}
