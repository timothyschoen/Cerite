#pragma once

#include <JuceHeader.h>
#include "Edge.h"
//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class Connection  : public Component, public gin::ValueTreeObject, public ComponentListener
{
public:
    
    SafePointer<Edge> start, end;
    Path path;
    
    bool is_selected = false;
    
    //==============================================================================
    Connection(ValueTree tree);
    ~Connection() override;

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;
    
    void mouseDown(const MouseEvent& e) override;
    void mouseDrag(const MouseEvent& e) override;
    
    void delete_listeners();
    
    void componentMovedOrResized (Component &component, bool wasMoved, bool wasResized) override;
    
    virtual void componentBeingDeleted(Component& component);


private:
    //==============================================================================
    // Your private member variables go here...


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Connection)
};
