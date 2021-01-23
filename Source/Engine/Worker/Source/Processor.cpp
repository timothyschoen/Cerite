#include "Processor.h"
#include "Message.h"
#include "MainComponent.h"
#include "../../Compiler/Compiler.h"

Processor::Processor(std::string code) {
    c_code = code;
    
}

bool Processor::prepareToPlay(int samplesPerBlockExpected, double sr) {
    
    main = MainComponent::getInstance();
    
    state = Cerite::Compiler::compile(c_code, print);
    
    if(state == nullptr) return false;
    
    initializeVariables();
    
    //setVariable("dt", 1. / sr);
    //setVariable("sr", sr);
    
    all_prepare();
    mna_prepare();
    dsp_prepare();
    data_prepare();
    all_update();
    
    solver.init(A, mna_size);
    

    
    
    return true;
}

double Processor::process(const double& input) {
    
    for(auto& ext : external)
        ext->tick();

   
    mna_calc();
    dsp_calc();
    data_calc();

    double returnvalue = output[0];
    output[0] = 0;
    return returnvalue;
}

void Processor::initializeVariables() {
    
    A = getVectorPtr("mna_A");
    b = getVectorPtr("mna_b");
    x = getVectorPtr("mna_x");
    
    
    done = getVariablePtr("done");
    
    double* sizeptr = getVariablePtr("mna_size");
    
    if(sizeptr != NULL)
        mna_size = *sizeptr;
    else
        mna_size = 0;
    
    dsp_prepare = getFunctionPtr("dsp_prepare");
    dsp_calc = getFunctionPtr("dsp_calc");
    
    
    mna_prepare = getFunctionPtr("mna_prepare");
    mna_calc = getFunctionPtr("mna_calc");
    
    data_prepare = getFunctionPtr("data_prepare");
    data_calc = getFunctionPtr("data_calc");
    
    all_update = getFunctionPtr("update");
    all_prepare = getFunctionPtr("prepare");
    //solvefunc = getFunctionPtr("solve");
    //updatefunc = getFunctionPtr("update");
    //trfunc = getFunctionPtr("tr");
    
    output = getVariablePtr("out");
    
}

void Processor::loadData(String name, int idx) {
    if(datafunctions.size() <= idx)
        datafunctions.resize(idx + 5);
    
    datafunctions[idx] = (datafunc)tcc_get_symbol(state, (name + "_attach").toRawUTF8());
    
    int* guiindex = (int*)tcc_get_symbol(state, (name + "_guiIdx").toRawUTF8());
    
    *guiindex = idx;
    
    auto assignfunc = (void(*)(void(*)(int, Data)))tcc_get_symbol(state, "registerGui");
    
    assignfunc([](int idx, Data d) {
        MemoryOutputStream memstream;
        memstream.writeInt(MessageID::SetParam);
        memstream.writeInt(idx);
        DataStream::writeToStream(d, memstream);
        MainComponent::main->sendMessage(memstream.getMemoryBlock());
    });
    
}


void Processor::setData(int idx, Data data) {
    datafunctions[idx](data);
}
