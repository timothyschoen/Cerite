#pragma once

#include <cmath>
#include <functional>
#include "Object.h"
#include "../Utility/Solver.h"

namespace Cerite {

class Patch : public Object
{
    
public:
    
    Solver solver;
    
    fptr initfunc;
    fptr calcfunc;
    fptr newtonfunc;
    fptr solvefunc;
    fptr updatefunc;
    
    std::vector<double> io;
    double* A;
    double* x;
    double* b;

    Patch(Document& doc);
    
    ~Patch();
    
    void init(double deltaTime); // initialize simulation
    
    void tick(); // advance time
    
};

}
