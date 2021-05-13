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

struct TabComponent : public TabbedComponent
{
    
    std::function<void(int)> on_tab_change = [](int){};
    
    TabComponent() : TabbedComponent(TabbedButtonBar::TabsAtTop) {
        
    }
    
    void currentTabChanged(int newCurrentTabIndex, const String& newCurrentTabName) override
    {
        on_tab_change(newCurrentTabIndex);
    }

    
};


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
    
    void add_tab(Canvas* cnv);
    
    Canvas* get_current_canvas();
    
    void triggerChange();
    
    ValueTreeObject* factory (const Identifier&, const ValueTree&) override;
    
    TabComponent& get_tabbar()  { return tabbar; };
    
    static inline File home_dir = File::getSpecialLocation(File::SpecialLocationType::userDocumentsDirectory).getChildFile("Cerite");
    static inline File app_dir = File::getSpecialLocation(File::SpecialLocationType::userApplicationDataDirectory);
    
private:
    
    FileChooser save_chooser =  FileChooser("Select a save file", File::getSpecialLocation(File::SpecialLocationType::userDocumentsDirectory).getChildFile("Cerite").getChildFile("Saves"), "*.crpat");
    FileChooser open_chooser = FileChooser("Choose file to open", File::getSpecialLocation( File::SpecialLocationType::userDocumentsDirectory).getChildFile("Cerite").getChildFile("Saves"), "*.crpat");
    
    std::unique_ptr<AudioPlayer> player;
    std::unique_ptr<SettingsDialog> settings_dialog;
    
    int toolbar_height = 50;
    int statusbar_height = 35;
    int sidebar_width = 300;
    int dragbar_width = 35;
    
    bool sidebar_hidden = false;


    std::array<TextButton, 6> toolbar_buttons = {TextButton("C"), TextButton("D"), TextButton("P"), TextButton("G"), TextButton("H"), TextButton("F")};
    std::array<TextButton, 2> sidebar_buttons = {TextButton("B"), TextButton("A")};
    
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
    
    TabComponent tabbar;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
