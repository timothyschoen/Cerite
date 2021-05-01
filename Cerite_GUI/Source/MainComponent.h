#pragma once

#include "LookAndFeel.h"
#include "Canvas.h"
#include "SettingsDialog.h"
#include <JuceHeader.h>

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/

class AudioPlayer;
class MainComponent  : public Component
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;
    
    std::unique_ptr<SettingsDialog> settings_dialog;
    
    Viewport canvas_port = Viewport("CanvasPort");
    Canvas canvas = Canvas(ValueTree("Canvas"));
    
    int toolbar_height = 50;
    int statusbar_height = 35;
    int sidebar_width = 150;
    int dragbar_width = 35;
    
    bool sidebar_hidden = false;
   
    
    ToolbarLook look_and_feel;
    
    TextButton compile_button = TextButton("Compile");

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;
    
    void mouseDown(const MouseEvent& e) override;
    void mouseDrag(const MouseEvent& e) override;
    void mouseUp(const MouseEvent& e) override;
    
    std::unique_ptr<AudioPlayer> player;
    
    

private:
    
    OwnedArray<TextButton> toolbar_buttons = {new TextButton("New"), new TextButton("Open"), new TextButton("Save"), new TextButton("Settings")};
    OwnedArray<TextButton> sidebar_buttons = {new TextButton("C"), new TextButton("I"), new TextButton("L")};
    
    TextButton hide_button = TextButton("H");
    
    int drag_start_width = 0;
    bool dragging_sidebar = false;
    //==============================================================================
    // Your private member variables go here...


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
