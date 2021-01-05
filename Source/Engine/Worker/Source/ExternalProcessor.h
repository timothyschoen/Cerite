#pragma once

#include <JuceHeader.h>
#include <libtcc.h>
#include "../../Utility/Solver.h"
#include "../../Types/Data.h"


enum ProcessorType {
    None,
    AudioPlayerType
};

struct ExternalProcessor
{
    
    virtual void init(double* target) = 0;
    
    virtual void tick(int channel = 0) = 0;
    
    virtual void receiveMessage(MemoryInputStream& m) = 0;
};

