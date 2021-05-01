#pragma once

#include "Utility/MultiComponentDragger.h"
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
    
    
    void mouseDown(const MouseEvent & e) override
    {
        is_down = true;
        dragger.handleMouseDown(box, e);
    }
    
    void mouseUp(const MouseEvent & e) override
    {
        is_down = false;
        dragger.handleMouseUp(box, e);
    }
    
    void mouseDrag(const MouseEvent & e) override
    {
        dragger.handleMouseDrag(e);
        Viewport* viewport = findParentComponentOfClass<Viewport>();
        Component* canvas = (Component*)findParentComponentOfClass<Canvas>();
        if(canvas) {
            auto pos = e.getEventRelativeTo(canvas).getPosition();
            
            /*
            if(!canvas->getBounds().contains(pos)) {
                if(canvas->getBounds().getRight() > pos.getX()) {
                    // Move all?
                    //canvas->setSize(pos.getX(), getHeight());
                    std::cout << "Out of left bound!" << std::endl;
                }
                if(canvas->getBounds().getX() < pos.getX()) {
                    viewport->setViewPosition(pos.getX() + getWidth(), viewport->getViewPositionY());
                    canvas->setSize(pos.getX() + getWidth() + 50, canvas->getHeight());
                    std::cout << "Out of right bound!" << std::endl;
                }
                if(canvas->getBounds().getY() > pos.getY()) {
                    // Move all?
                    //canvas->setSize(getWidth(), );
                    std::cout << "Out of top bound!" << std::endl;
                }
                if(canvas->getBounds().getBottom() < pos.getY()) {
                    viewport->setViewPosition(viewport->getViewPositionX(), pos.getY() + getHeight());
                    canvas->setSize(canvas->getWidth(), pos.getY() + getHeight() + 50);
                    std::cout << "Out of bottom bound!" << std::endl;
                }
            }
        } */
        
    }
    
    
    MultiComponentDragger<Box>& dragger;
};

class Box  : public Component, public gin::ValueTreeObject
{
    
public:
    //==============================================================================
    Box(ValueTree tree, MultiComponentDragger<Box>& multi_dragger);
    ~Box() override;
    
    std::map<String, std::pair<int, int>> ports;
    
    //==============================================================================
    void paint (Graphics&) override;
    
    void resized() override;
    void moved() override;
    
    void set_type (String new_type);
    
    void mouseMove(const MouseEvent& e) override;
    
    
    int total_in = 0;
    int total_out = 0;
    ClickLabel text_label;
    
private:
    //==============================================================================
    // Your private member variables go here...
    
    MultiComponentDragger<Box>& dragger;
    

    

    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Box)
};
