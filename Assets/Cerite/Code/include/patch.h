#pragma once
#include <iostream>
#include <string.h>

#include "klu_solver.h"

struct IPatch
{

        IPatch() {
        };

        virtual ~IPatch() {
        };

        virtual void init() {
        };

        virtual double process(double input) = 0;

        virtual void destroy() {
        };


};

struct Patch : public IPatch
{


        int maxiter = 10;
        int ticks = 0;

        int nets;

        double controls[16] = {0};
        KLU Solver;

        void init() override;

        double process(double input) override;

        void destroy() override;

        void setSlider(int index, double value) {
                controls[index] = value;
        }

        void setIterations(int q){
                std::cout << "Loaded! maxiter = " << maxiter  << '\n';
                maxiter = q;

        };

};


extern "C" IPatch* create() {
        //int i = 5;
        return new Patch;
}
