#pragma once


#include "../Looks.h"
#include "../../Engine/Cerite/src/Interface/Patch.h"
#include <JuceHeader.h>


struct ExternalProcessor
{
    std::string param;
    std::string name;
    ValueTree boxTree;
    ValueTree paramTree;
    
    ExternalProcessor(std::string paramname, ValueTree tree) {
        param = "attach_" + paramname;
        name = paramname;
        boxTree = tree;
        paramTree = tree.getChildWithName("Parameters");
    }
    
    virtual void tick(int channel = 0) {};
    virtual void init(Cerite::Object* obj) {};
    
    
    void setParameter(String name, String value)
    {
        ValueTree newparam = paramTree.getOrCreateChildWithName(name, nullptr);
        newparam.setProperty("Value", value, nullptr);
    };
    
    void setParameter(String name, double value)
    {
        ValueTree newparam = paramTree.getOrCreateChildWithName(name, nullptr);
        newparam.setProperty("Value", value, nullptr);
    };
    
};
class GUIContainer : public virtual Component
{
    
public:
    
    
    
    CeriteLookAndFeel clook;
    
    GUIContainer()
    {
        setLookAndFeel(&clook);
    }
    
    ~GUIContainer()
    {
        setLookAndFeel(nullptr);
    }
    
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
    
    virtual ExternalProcessor* createProcessor(std::string paramname) = 0;
    
};

