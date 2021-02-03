#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
 This component lives inside our window, and this is where you should put all
 your controls and content.
 */

enum MessageID {
    Start,
    Code,
    Stop,
    Log,
    Volume,
    LoadParam,
    SetParam,
    GetParam,
    Ready,
    AddProcessor,
    SendProcessor,
    Ping
};
