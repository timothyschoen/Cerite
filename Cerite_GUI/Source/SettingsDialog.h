#pragma once

#include "../../Source/AudioPlayer.hpp"
#include "LookAndFeel.h"
#include <JuceHeader.h>


struct SettingsComponent : public Component
{
        
    Component code_panel;
    
    ToggleButton use_sandbox;
    Label sandbox_label = Label("sandbox_label", "Enable code sandbox:");
    Label compiler_label = Label("compiler_label", "Choose C compiler:");
    Label optimization_label = Label("optimization_label", "Compiler Optimization");
    
    ComboBox compiler_selector, optimization_selector;
    
    bool manager_allocated = false;
    AudioDeviceManager* device_manager = nullptr;
    
    std::unique_ptr<AudioDeviceSelectorComponent> audioSetupComp;
    
    int toolbar_height = 50;
    
    ToolbarLook look_and_feel = ToolbarLook(false);
    
    OwnedArray<TextButton> toolbar_buttons = {new TextButton("Audio"), new TextButton("Code")};
    
    SettingsComponent();
    
    ~SettingsComponent(){
        if(manager_allocated) delete device_manager;
    }
    
    ValueTree get_settings();

    void paint(Graphics& g);
        
    void resized();
    
    void update_device_manager(bool state);
    
};


struct SettingsDialog : public DocumentWindow
{
    
  
    MainLook main_look;
    SettingsComponent settings_component;
   
    
    SettingsDialog() : DocumentWindow("Settings",
                                      Colour(50, 50, 50),
                                      DocumentWindow::allButtons) {
        
        setUsingNativeTitleBar (true);
        
        setCentrePosition(400, 400);
        setSize(600, 400);
        
        setVisible (false);
        
        setResizable(false, false);
        
        setContentOwned (&settings_component, false);

        setLookAndFeel(&main_look);
    }
    
    ~SettingsDialog() {
        setLookAndFeel(nullptr);
    }
    
    void resized() {
        settings_component.setBounds(getLocalBounds());
    
    }
    
    ValueTree get_settings() {
        return settings_component.get_settings();
    }
    
    void closeButtonPressed()
    {
        setVisible(false);
    }
    
};
