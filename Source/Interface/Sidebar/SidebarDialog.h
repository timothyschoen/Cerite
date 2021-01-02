#pragma once

#include <JuceHeader.h>
#include "../Looks.h"


// Base component for sidebar dialogs
// Any dialog implementations need to implement this as base
struct DialogBase : public Component
{
    std::function<void()> confirmed;
    std::function<void()> cancelled;
    
    virtual Array<String> getResult() {
        return Array<String>();
    };
    
};


// Wrapper around window that helps to black out the sidebar
class SidebarDialog : public Component, public ComponentListener
{
    CeriteLookAndFeel clook;
    
    DialogBase* dialogwindow;
    
    Component* main;
    Component* sidebar;
    
    Rectangle<int> windowbounds;
    Rectangle<int> callerbounds;
    
public:
    SidebarDialog(Component* mainwindow, Component* sbar, DialogBase* obj, Rectangle<int> bounds, Rectangle<int> callerBounds) : windowbounds(bounds), callerbounds(callerBounds), main(mainwindow), sidebar(sbar), dialogwindow(obj) {
        
        main->addComponentListener(this);
        
        dialogwindow->cancelled = [this]() {
            exitModalState(0);
        };
        
        dialogwindow->confirmed = [this]() {
            exitModalState(1);
        };

        setLookAndFeel(&clook);
        
        addAndMakeVisible(dialogwindow);
        
        setOpaque(false);
        addToDesktop (ComponentPeer::windowHasDropShadow);
    }
    
    ~SidebarDialog() {
        main->removeComponentListener(this);
        setLookAndFeel(nullptr);
    }
    
    void componentMovedOrResized(Component& comp, bool wasMoved, bool wasResized) override
    {
        setBounds(sidebar->getScreenBounds().withTrimmedLeft(30));
    }
    
    Array<String> getResult() {
            return dialogwindow->getResult();
    }
    void resized() override{
        dialogwindow->setBounds(windowbounds);
    }
    
    void paint(Graphics & g) override {
        
        g.setColour(Colour(20, 20, 20).withAlpha(0.5f));
        
        g.fillRect(0, 0, getWidth(), std::max(0, callerbounds.getY()));
        
        g.fillRect(0, std::max(0, windowbounds.getBottom()), getWidth(), getHeight());

    }
    
    
};
