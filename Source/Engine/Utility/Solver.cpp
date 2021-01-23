#include "Solver.h"


namespace Cerite {

void Solver::createCSR(double* A, int dim)
{
    // Reduce the number of nets by one since we index from 1 (0 is a fake index for ground)
    nets = dim;
    nonzero = 0;
    // Count the number of non-zero values
    for (int i = 0; i < nets; i++)
    for (int j = 0; j < nets; j++)
    if (A[(j  * nets + i)] != 0)
        nonzero++;
    
    // Allocate memory for CSR format
    AI.resize(nets + 1);
    AJ.resize(nonzero);
    AVal.resize(nonzero);
    
    // Pointers to the non-zero positions for updating the A matrix later on
    nzpointers.resize(nonzero);
    
    nonzero = 0;
    
    // Create our CSR format
    for (int i = 0; i < nets; i++) {
        for (int j = 0; j < nets; j++) {
            if (A[j  * nets + i] != 0) {
                AVal[nonzero] = A[j * nets + i];
                nzpointers[nonzero] = &A[j  * nets + i];
                AJ[nonzero] = j;
                nonzero++;
            }
        }
        AI[i+1] = nonzero;
    }
    
    // Initialize KLU
    klu_defaults (&Common);
    
    // Symbolic analysis
    Symbolic = klu_analyze (nets, &AI[0], &AJ[0], &Common);
    
    // Full numeric factorization: Only needed once!
    Numeric = klu_factor (&AI[0], &AJ[0], &AVal[0], Symbolic, &Common);
    
    currentSolver = this;
    
}

void Solver::callSolve(double* A, double* b, double* x)
{
    if(currentSolver) {
        currentSolver->solve(A, b, x);
    }
    
}
void Solver::solve(double* A, double* b, double* x)
{
    
    memcpy(x, b, nets * sizeof(double));
    
    
    // Update our factorization or refactor if the last factorization failed
    if(Numeric == nullptr || Symbolic == nullptr)
        createCSR(A, nets);
    else
        klu_refactor (&AI[0], &AJ[0], getAValues(), Symbolic, Numeric, &Common);
    
    // Solve the system!
    klu_solve (Symbolic, Numeric, nets, 1, x, &Common);
    
    std::vector<double> xread(x, x + nets);
    std::vector<double> bread(b, b + nets);
    
    auto g = xread.begin();
    auto d = bread.begin();
    
}


double* Solver::getAValues()
{
    for (size_t i = 0; i < nonzero; i++)
    {
        AVal[i] = *(nzpointers[i]);
    }
    
    return &AVal[0];
}

}
