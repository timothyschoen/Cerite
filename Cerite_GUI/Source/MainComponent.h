#pragma once

#include "LookAndFeel.h"
#include "Canvas.h"
#include "SettingsDialog.h"
#include "Utility/gin_valuetreeobject.h"

#include "Console.h"
#include "TextEditor.hpp"

#include <JuceHeader.h>

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/

class AudioPlayer;
class MainComponent : public Component, public ValueTreeObject
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;
    
    inline static MainComponent* current_main = nullptr;
    
    Canvas* canvas = nullptr;

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;
    
    void mouseDown(const MouseEvent& e) override;
    void mouseDrag(const MouseEvent& e) override;
    void mouseUp(const MouseEvent& e) override;
    
    void set_remote(bool is_remote);
    
    
    void open_project();
    void save_project();
    
    void send_data(int port, libcerite::Data data);
    void receive_data(int port, std::function<void(libcerite::Data)>);
    
    ValueTreeObject* factory (const Identifier&, const ValueTree&) override;
    
private:
    
    FileChooser save_chooser =  FileChooser("Select a save file", File::getSpecialLocation(File::SpecialLocationType::userDocumentsDirectory).getChildFile("Cerite").getChildFile("Saves"), "*.crpat");
    FileChooser open_chooser = FileChooser("Choose file to open", File::getSpecialLocation( File::SpecialLocationType::userDocumentsDirectory).getChildFile("Cerite").getChildFile("Saves"), "*.crpat");
    
    std::unique_ptr<AudioPlayer> player;
    std::unique_ptr<SettingsDialog> settings_dialog;
    
    Viewport canvas_port = Viewport("CanvasPort");
    
    int toolbar_height = 50;
    int statusbar_height = 35;
    int sidebar_width = 300;
    int dragbar_width = 35;
    
    bool sidebar_hidden = false;
    
    bool autocompile = false;

    
    OwnedArray<TextButton> toolbar_buttons = {new TextButton(CharPointer_UTF8("C")), new TextButton("D"), new TextButton("P"), new TextButton("F")};
    OwnedArray<TextButton> sidebar_buttons = {new TextButton("B"), new TextButton("A")};
    
    TextButton compile_button = TextButton("N");
    TextButton autocompile_button = TextButton("E");
    TextButton hide_button = TextButton("K");
    
    int drag_start_width = 0;
    bool dragging_sidebar = false;

    ToolbarLook toolbar_look;
    SidebarLook sidebar_look;
    SidebarLook statusbar_look = SidebarLook(1.4);
    MainLook main_look;
    
    Console console;
    mcl::TextEditor code_editor;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
