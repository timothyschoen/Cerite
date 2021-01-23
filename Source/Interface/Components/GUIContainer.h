#pragma once


#include "../Looks.h"
#include "../../Engine/Types/Data.h"
#include "../../Engine/Worker/Source/ExternalProcessor.h"
#include "../../Engine/Worker/Source/Message.h"
#include <JuceHeader.h>

class GUIContainer : public virtual Component
{
    
public:
    
    ProcessorType type = ProcessorType::None;
    int ID;
    int processorID = -1;
    String parameterName;

    CeriteLookAndFeel clook;
    
    GUIContainer()
    {
        setLookAndFeel(&clook);
    }
    
    ~GUIContainer()
    {
        setLookAndFeel(nullptr);
    }
    
    virtual void setID(int newID, int procID = -1) {
        ID = newID;
    }
    
    virtual void init() {};
    virtual void close() {};
    
    void paint (Graphics & g) override
    {
        g.fillAll(Colour(41, 41, 41));
        g.setColour(Colours::grey);
        g.drawRect(0, -2, getWidth(), getHeight() + 2);
    }
    
    virtual Point<int> getBestSize ()
    {
        return Point<int>(70, 150);
    }
    
    virtual void receive(Data d) {};
    virtual void send(Data d);
    
};

