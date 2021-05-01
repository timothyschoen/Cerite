#pragma once

#include <JuceHeader.h>
#include "Box.h"
//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/

using Patch = std::vector<std::tuple<String, int, int, std::map<String, std::vector<std::vector<int>>>>>;

class Box;
class Edge;
class Canvas  : public Component, public gin::ValueTreeObject, public KeyListener
{
public:
    //==============================================================================
    Canvas(ValueTree tree);
    ~Canvas() override;
    

    //==============================================================================
    void paintOverChildren (Graphics&) override;
    void resized() override;
    
    void mouseDown(const MouseEvent& e) override;
    void mouseDrag(const MouseEvent& e) override;
    void mouseUp(const MouseEvent& e) override;
    void mouseMove(const MouseEvent& e) override;
    
    Patch create_patch();
    
    bool keyPressed(const KeyPress &key, Component *originatingComponent) override;


private:
    //==============================================================================
    // Your private member variables go here...

    MultiComponentDragger<Box> dragger = MultiComponentDragger<Box>(this);

    LassoComponent<Box*> lasso;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Canvas)
};
