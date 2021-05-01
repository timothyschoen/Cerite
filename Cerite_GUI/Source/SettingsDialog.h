#pragma once

#include "LookAndFeel.h"
#include <JuceHeader.h>


struct SettingsComponent : public Component
{
    
    int toolbar_height = 50;
    
    ToolbarLook look_and_feel;
    
    OwnedArray<TextButton> toolbar_buttons = {new TextButton("Audio"), new TextButton("Code")};
    
    SettingsComponent() {
        for(auto& button : toolbar_buttons) {
            button->setClickingTogglesState(true);
            button->setRadioGroupId(0110);
            button->setLookAndFeel(&look_and_feel);
            button->setConnectedEdges(12);
            addAndMakeVisible(button);
            
        }
    }
    
    void paint(Graphics& g) {
        auto base_colour = Colour(41, 41, 41);
        auto highlight_colour = Colour (0xff42a2c8).darker(0.3);
        
        // Toolbar background
        g.setColour(base_colour);
        g.fillRect(0, 0, getWidth(), toolbar_height);
        
        g.setColour(highlight_colour);
        g.fillRect(0, 42, getWidth(), 4);
    }
        
    void resized() {
        int toolbar_position = 0;
        for(auto& button : toolbar_buttons) {
            button->setBounds(toolbar_position, 0, 70, toolbar_height);
            toolbar_position += 70;
        }
        
        
    }
    
};


struct SettingsDialog : public DocumentWindow
{
    
  
    
    SettingsComponent settings_component;
   
    
    SettingsDialog() : DocumentWindow("Settings",
                                      Colour(50, 50, 50),
                                      DocumentWindow::allButtons) {
        
        setUsingNativeTitleBar (true);
        
        setCentrePosition(400, 400);
        setSize(500, 300);
        
        setVisible (false);
        
        setResizable(false, false);
        
        setContentOwned (&settings_component, false);

        
    }
    
    void resized() {
        settings_component.setBounds(getLocalBounds());
    }
    
    void closeButtonPressed()
    {
        setVisible(false);
    }
    
};
