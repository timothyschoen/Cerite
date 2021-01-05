#include "Processor.h"
#include "Message.h"
#include "MainComponent.h"

Processor::Processor(std::string code) {
    c_code = code;
    
}

bool Processor::prepareToPlay(int samplesPerBlockExpected, double sr) {
    

    state = compile(c_code);
    
    if(state == nullptr) return false;
    
    initializeVariables();
    
    //setVariable("dt", 1. / sr);
    //setVariable("sr", sr);
    
    initfunc();
    updatefunc();
    
    solver.init(A, mna_size);
    
    return true;
}

void Processor::processBlock(AudioBuffer<float>& bufferToFill) {
    
    for(int i = 0; i < bufferToFill.getNumSamples(); i++) {
        for(auto& ext : external)
            ext->tick();
        
        calcfunc();
        trfunc();
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
        
        float outval = output[0];
        
        for(int c = 0; c < bufferToFill.getNumChannels(); c++) {
            bufferToFill.setSample(c, i, output[0]);
        }
    }
}

TCCState* Processor::compile(std::string code) {
    
    TCCState* cstate = tcc_new();
    
    // Register error func and set output type
    tcc_set_error_func(cstate, nullptr, log);
    tcc_set_output_type(cstate, TCC_OUTPUT_MEMORY);
    
    // Register the custom print function
    tcc_add_symbol(state, "print", (void*)print);
    
    // Add std library functions and write extern statements
    std::string externs = "extern void print(const char*);\n" + addStdLibrary();
    
    std::string fullCode = externs + code;
    
    File output = File::getSpecialLocation(File::userHomeDirectory).getChildFile("ctestserver.c");
    
    output.create();
    output.replaceWithText(String(fullCode));
    
    
    // Check for compilation errors
    if (tcc_compile_string(cstate, fullCode.c_str()) == -1) {
        print("Compilation error!\n");
        return nullptr;
    }
    else {
        print("Compilation successful!\n");
    }
    
    tcc_relocate(cstate, TCC_RELOCATE_AUTO);
    
    return cstate;
}

void Processor::initializeVariables() {
    
    A = getVectorPtr("mna_A");
    b = getVectorPtr("mna_b");
    x = getVectorPtr("mna_x");
    
    double* sizeptr = getVariablePtr("mna_size");
    
    if(sizeptr != NULL)
        mna_size = *sizeptr;
    else
        mna_size = 0;
    
    calcfunc = getFunctionPtr("calc");
    initfunc = getFunctionPtr("prepare");
    newtonfunc = getFunctionPtr("newton");
    solvefunc = getFunctionPtr("solve");
    updatefunc = getFunctionPtr("update");
    trfunc = getFunctionPtr("tr");
    
    output = getVariablePtr("out");
    
}

std::string Processor::addStdLibrary(){
    std::string externs;

    // math.h definitions
    externs +=  includeFunction<double, double>         (state, "sin",  (void*)(double(*)(double))&sin);
    externs +=  includeFunction<double, double>         (state, "cos",  (void*)(double(*)(double))&cos);
    externs +=  includeFunction<double, double>         (state, "tan",  (void*)(double(*)(double))&tan);
    externs +=  includeFunction<double, double>         (state, "sinh", (void*)(double(*)(double))&sinh);
    externs +=  includeFunction<double, double>         (state, "cosh", (void*)(double(*)(double))&cosh);
    externs +=  includeFunction<double, double>         (state, "tanh", (void*)(double(*)(double))&tanh);
    externs +=  includeFunction<double, double>         (state, "asin", (void*)(double(*)(double))&asin);
    externs +=  includeFunction<double, double>         (state, "acos", (void*)(double(*)(double))&acos);
    externs +=  includeFunction<double, double>         (state, "atan", (void*)(double(*)(double))&atan);
    externs +=  includeFunction<double, double, double> (state, "atan2",(void*)(double(*)(double, double))&atan2);
    externs +=  includeFunction<double, double>         (state, "exp",  (void*)(double(*)(double))&exp);
    externs +=  includeFunction<double, double>         (state, "log",  (void*)(double(*)(double)) &log);
    externs +=  includeFunction<double, double>         (state, "log10",(void*)(double(*)(double)) log10);
    externs +=  includeFunction<double, double, double> (state, "pow",  (void*)(double(*)(double, double))&pow);
    externs +=  includeFunction<double, double>         (state, "sqrt", (void*)(double(*)(double))&sqrt);
    externs +=  includeFunction<double, double>         (state, "ceil", (void*)(double(*)(double))&ceil);
    externs +=  includeFunction<double, double>         (state, "floor",(void*)(double(*)(double))&floor);
    externs +=  includeFunction<double, double>         (state, "fabs", (void*)(double(*)(double))&fabs);
    externs +=  includeFunction<double, double, int>    (state, "ldexp",(void*)(double(*)(double, int))&ldexp);
    externs +=  includeFunction<double, double, int*>   (state, "frexp",(void*) (double(*)(double, int*))&frexp);
    //externs +=  includeFunction<double, double  double*>(state, "modf", (void*)(double(*)(double, double*))&modf);
    externs +=  includeFunction<double, double, double> (state, "fmod", (void*)(double(*)(double, double))&fmod);
    
    externs += includeVariable<double>(state, "M_LOG2E", M_LOG2E);
    externs += includeVariable<double>(state, "M_LOG10E", M_LOG10E);
    externs += includeVariable<double>(state, "M_LN2", M_LN2);
    externs += includeVariable<double>(state, "M_LN10", M_LN10);
    externs += includeVariable<double>(state, "M_PI", M_PI);
    externs += includeVariable<double>(state, "M_PI_2", M_PI_2);
    externs += includeVariable<double>(state, "M_PI_4", M_PI_4);
    externs += includeVariable<double>(state, "M_1_PI", M_1_PI);
    externs += includeVariable<double>(state, "M_2_PI", M_2_PI);
    externs += includeVariable<double>(state, "M_2_SQRTPI", M_2_SQRTPI);
    externs += includeVariable<double>(state, "M_SQRT2", M_SQRT2);
    externs += includeVariable<double>(state, "M_SQRT1_2", M_SQRT1_2);
    
    externs +=  "#include <tcclib.h>\n";
    
    // Add data processing library
    externs += datatype;
    
    return externs;

};

void Processor::loadData(String name, int idx) {
    if(datafunctions.size() <= idx)
        datafunctions.resize(idx + 5);
    
    datafunctions[idx] = (datafunc)tcc_get_symbol(state, (name + "_attach").toRawUTF8());
    
    int* guiindex = (int*)tcc_get_symbol(state, ("guiIdx_" + name).toRawUTF8());
                                         
     *guiindex = idx;
                            
    auto assignfunc = (void(*)(void(*)(int, Data)))tcc_get_symbol(state, "registerGui");
    
    assignfunc([](int idx, Data d) {
        MemoryOutputStream memstream;
        memstream.writeInt(MessageID::SetParam);
        memstream.writeInt(idx);
        DataStream::writeToStream(d, memstream);
        MainComponent::main->sendMessageToMaster(memstream.getMemoryBlock());
    });
    
}


void Processor::setData(int idx, Data data) {
    datafunctions[idx](data);
}
