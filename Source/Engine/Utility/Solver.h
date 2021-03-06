#pragma once

#include <cstring>
#include <vector>
#include <numeric>
#include "../../../Libraries/KLU/Source/KLU/klu.h" // ... damn

// For now we use KLU as a solver since it's the fastest in most cases in my experience
// Later I might jit-compile a custom solver someday, but since it's hard to compete with KLU's algorithm, it's not a priority

namespace Cerite {

struct Solver
{

    ~Solver() {
        currentSolver = nullptr;
        /* this causes crashes, but right now it might leak...
        if(Symbolic != nullptr) {
            klu_free_symbolic(&Symbolic, &Common);
        }
        if(Numeric != nullptr) {
            klu_free_numeric(&Numeric, &Common);
        }
          */
    }
    
    
    void createCSR(double* A, int dim);
    
    void solve(double* A, double* b, double* x);
    
    static void callSolve(double* A, double* b, double* x);
    
    double* getAValues();
    
    void init(double* A, int size) {
        nets = size;
        createCSR(A, nets);
    }

    inline static Solver* currentSolver = nullptr;
  
private:
    
    std::vector<double> AVal;
    std::vector<int> AI;
    std::vector<int> AJ;

    std::vector<double*> nzpointers;

    int nets;
    int nonzero;
    
    klu_symbolic *Symbolic;
    klu_numeric *Numeric;
    klu_common Common;
    
};

}
