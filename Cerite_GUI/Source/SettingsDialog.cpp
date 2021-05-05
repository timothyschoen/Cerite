#include "SettingsDialog.h"
#include "MainComponent.h"

SettingsComponent::SettingsComponent() {
    for(auto& button : toolbar_buttons) {
        button->setClickingTogglesState(true);
        button->setRadioGroupId(0110);
        button->setLookAndFeel(&look_and_feel);
        button->setConnectedEdges(12);
        addAndMakeVisible(button);
        
    }
    

    addAndMakeVisible(audioSetupComp.get());
    
    toolbar_buttons[0]->setToggleState(true, dontSendNotification);
    
    toolbar_buttons[0]->onClick = [this](){
        audioSetupComp->setVisible(true);
        code_panel.setVisible(false);
        resized();
    };
    
    toolbar_buttons[1]->onClick = [this]()
    {
        audioSetupComp->setVisible(false);
        code_panel.setVisible(true);
        // make other panel visible
        resized();
        
    };
    
    update_device_manager(AudioPlayer::current_player == nullptr);
    
    
    use_sandbox.onClick = [this](){
        bool state = use_sandbox.getToggleState();
        MainComponent::current_main->set_remote(state);
        update_device_manager(state);
    };
    
    
    
    compiler_selector.addItemList({"Clang", "GCC", "TCC"}, 1);
    optimization_selector.addItemList({"-O1", "-O2", "-O3"}, 1);
    
    
    code_panel.addAndMakeVisible(use_sandbox);
    code_panel.addAndMakeVisible(sandbox_label);
    code_panel.addAndMakeVisible(compiler_label);
    code_panel.addAndMakeVisible(compiler_selector);
    code_panel.addAndMakeVisible(optimization_label);
    code_panel.addAndMakeVisible(optimization_selector);
    addChildComponent(&code_panel);
    
    // TODO: load settings from xml, saved in user application data!
    optimization_selector.setSelectedItemIndex(0);
    compiler_selector.setSelectedItemIndex(0);
    
}

ValueTree SettingsComponent::get_settings() {
    
    
    ValueTree settings_tree = ValueTree("Settings");
    
    ValueTree code_tree = ValueTree("Code");
    
    
    int selected_compiler = compiler_selector.getSelectedItemIndex();
    int selected_optimization = optimization_selector.getSelectedItemIndex();
    
    code_tree.setProperty("Compiler", (String[3]){"clang", "gcc", "tcc"}[selected_compiler], nullptr);
    code_tree.setProperty("Optimization", (String[3]){"-O1", "-O2", "-O3"}[selected_optimization], nullptr);
    
    
    auto xml = device_manager->createStateXml();
    
    ValueTree audio_settings("Audio");
    
    if(xml) {
        audio_settings.appendChild(ValueTree::fromXml(*xml), nullptr);
    }
    
    auto setup = device_manager->getAudioDeviceSetup();
    
    settings_tree.appendChild(code_tree, nullptr);
    settings_tree.appendChild(audio_settings, nullptr);
    
    return settings_tree;
}


void SettingsComponent::paint(Graphics& g) {
    auto base_colour = Colour(41, 41, 41);
    auto highlight_colour = Colour (0xff42a2c8).darker(0.3);
    
    // Toolbar background
    g.setColour(base_colour);
    g.fillRect(0, 0, getWidth(), toolbar_height);
    
    g.setColour(highlight_colour);
    g.fillRect(0, 42, getWidth(), 4);
}

void SettingsComponent::resized() {
    int toolbar_position = 0;
    for(auto& button : toolbar_buttons) {
        button->setBounds(toolbar_position, 0, 70, toolbar_height);
        toolbar_position += 70;
    }
    
    if(audioSetupComp) {
        audioSetupComp->setBounds(0, toolbar_height, getWidth(), getHeight() - toolbar_height);
    }
    
    code_panel.setBounds(0, toolbar_height, getWidth(), getHeight() - toolbar_height);
    
    
    sandbox_label.setBounds(15, 22, 140, 20);
    use_sandbox.setBounds(157, 20, 24, 24);
    
    compiler_label.setBounds(15, 52, 140, 24);
    compiler_selector.setBounds(160, 52, 80, 24);
    
    optimization_label.setBounds(15, 80, 140, 24);
    optimization_selector.setBounds(160, 80, 80, 24);
}

void SettingsComponent::update_device_manager(bool state)
{
    // If there is a local player, use that device manager
    if(!state) {
        
        auto* old_manager = device_manager;
        
        if(manager_allocated && old_manager) {
            old_manager->closeAudioDevice();
        }

        
        
            
        device_manager = &AudioPlayer::current_player->device_manager;
        
        audioSetupComp.reset(new AudioDeviceSelectorComponent(AudioPlayer::current_player->device_manager, 1, 2, 1, 2, false, false, false, false));
            
        if(manager_allocated && old_manager) {
            delete old_manager;
            manager_allocated = false;
        }
        
    }
    // Otherwise create a virtual one
    else {
        if(!manager_allocated)  {
            device_manager = new AudioDeviceManager;
            manager_allocated = true;
        }
    }
    
    device_manager->initialiseWithDefaultDevices(2, 2);
    audioSetupComp.reset(new AudioDeviceSelectorComponent(*device_manager, 1, 2, 1, 2, false, false, false, false));
    
    addAndMakeVisible(audioSetupComp.get());
}
