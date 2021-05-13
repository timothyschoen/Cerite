#pragma once

#define NO_CERITE_MACROS
#include "/Users/timschoen/Documents/Cerite/.exec/libcerite.h"

#include "Utility/MultiComponentDragger.h"
#include "Utility/gin_valuetreeobject.h"

#include "../../Source/Engine.hpp"

#include "GUIObjects.h"

#include <JuceHeader.h>



//==============================================================================
/*
 This component lives inside our window, and this is where you should put all
 your controls and content.
 */

class Box;
struct ClickLabel : Label
{
    
    Box* box;
    
    
    
    bool is_down = false;
    
    ClickLabel(Box* parent, MultiComponentDragger<Box>& multi_dragger) : dragger(multi_dragger), box(parent) {};
    
    void mouseDown(const MouseEvent & e) override;
    void mouseUp(const MouseEvent & e) override;
    void mouseDrag(const MouseEvent & e) override;
    
    MultiComponentDragger<Box>& dragger;
};

class Canvas;
class Box  : public Component, public ValueTreeObject
{
    
public:
    //==============================================================================
    Box(Canvas* parent, ValueTree tree, MultiComponentDragger<Box>& multi_dragger);
    ~Box() override;
    
    Canvas* cnv;
    
    std::map<String, std::pair<int, int>> ports;
    
    std::unique_ptr<GUIComponent> graphics = nullptr;
    
    std::vector<int> gui_nodes;
    
    ValueTreeObject* factory (const juce::Identifier&, const juce::ValueTree&) override;
    
    //==============================================================================
    void paint (Graphics&) override;
    
    void resized() override;
    void moved() override;
    
    void update_position();
    
    Ports update_subpatch(ValueTree tree);
    void update_ports(Ports ports);
    
    void set_type (String new_type);
    
    Ports load_subpatch (String title);
    
    void mouseMove(const MouseEvent& e) override;
    
    void valueTreePropertyChanged (ValueTree &treeWhosePropertyHasChanged, const Identifier &property) override;
    
    
    int total_in = 0;
    int total_out = 0;
    
    int hidden_in = 0;
    int hidden_out = 0;
    
    ClickLabel text_label;
    bool is_edge = false;
    
private:

    MultiComponentDragger<Box>& dragger;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Box)
};
