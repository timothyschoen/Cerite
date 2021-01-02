#pragma once

#include <JuceHeader.h>


enum OrderingRule {
    NoOrdering,
    StartToEnd,
    RightToLeft
};

enum SplitRule {
    NoSplit,
    OutToIn,
    InToOut
};

struct DomainInformation
{
    String name;
    int position;
    Colour colour;
    OrderingRule ordering;
    SplitRule split;
};
