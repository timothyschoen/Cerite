#pragma once

#include <JuceHeader.h>
#include <libtcc.h>
#include "../../Utility/Solver.h"
#include "../../Types/Data.h"
#include "AudioPlayer.h"
//==============================================================================
/*
 This component lives inside our window, and this is where you should put all
 your controls and content.
 */
class MainComponent;
struct Processor
{
    
    Cerite::Solver solver;
    
    OwnedArray<ExternalProcessor> external;
    
    
    TCCState* state;
    
    MainComponent* main;
    
    typedef void (*TCCErrorFunc)(void *opaque, const char *msg);
    inline static TCCErrorFunc log = [](void *opaque, const char *msg) {
        printf(msg);
    };
    
    typedef void (*PrintFunc)(const char *msg);
    
    static void print(const char* msg) {
        log(nullptr, msg);
    }
    
    typedef void (*fptr)();
    typedef double*(*vecptr)();
    typedef void(*datafunc)(Data);
    
    std::string c_code;
    

    
    fptr dsp_prepare;
    fptr dsp_calc;
    fptr mna_prepare;
    fptr mna_calc;
    fptr data_prepare;
    fptr data_calc;
    
    fptr all_prepare;
    fptr all_update;
    
    int mna_size = 0;
    double* output;
    
    double* A;
    double* b;
    double* x;
    
    double* done;
    
    std::vector<datafunc> datafunctions;
    
    Processor(std::string code);
    
    void loadData(String name, int idx);
    void setData(int idx, Data data);
    
    bool prepareToPlay(int samplesPerBlockExpected, double sampleRate);
    
    double process(const double& input);
    
    void initializeVariables();
    
    double* getVariablePtr(std::string name) {
        double* result = (double*)tcc_get_symbol(state, name.c_str());
        
        if(result != NULL)
            return result;
        else
            return nullptr;
    }
    
    fptr getFunctionPtr(std::string name) {
        fptr result = (fptr)tcc_get_symbol(state, name.c_str());
        
        if(result != NULL)
            return result;
        else
            return [](){};
    }
    
    double* getVectorPtr(std::string name) {
        double* vector = (double*)tcc_get_symbol(state, name.c_str());
        
        if(vector != NULL)
            return vector;
        else
            return nullptr;
    }
    
    void setVariable(std::string name, double value) {
        *(double*)tcc_get_symbol(state, name.c_str()) = value;
    }
    
};
