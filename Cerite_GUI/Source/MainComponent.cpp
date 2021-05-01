#include "MainComponent.h"
#include "../../Source/NodeConverter.hpp"
#include "../../Source/Engine.hpp"
#include "../../Source/Library.hpp"
#include "../../Source/AudioPlayer.hpp"


//==============================================================================
MainComponent::MainComponent()
{
    
    setSize(1000, 700);
    
    auto& lnf = getLookAndFeel();
    
    lnf.setColour (ResizableWindow::backgroundColourId, Colour(50, 50, 50));
    lnf.setColour (TextButton::buttonColourId, Colour(41, 41, 41));
    lnf.setColour (TextButton::buttonOnColourId, Colour(25, 25, 25));
    lnf.setColour (juce::TextEditor::backgroundColourId, Colour(68, 68, 68));
    //lnf.setColour (Toolbar::backgroundColourId, Colour(41, 41, 41).withAlpha(alpha));
    lnf.setColour (SidePanel::backgroundColour, Colour(50, 50, 50));
    lnf.setColour (ComboBox::backgroundColourId, Colour(50, 50, 50));
    lnf.setColour (ListBox::backgroundColourId, Colour(50, 50, 50));
    lnf.setColour (Slider::backgroundColourId, Colour(60, 60, 60));
    lnf.setColour (Slider::trackColourId, Colour(50, 50, 50));
    lnf.setColour(CodeEditorComponent::backgroundColourId, Colour(50, 50, 50));
    lnf.setColour(CodeEditorComponent::defaultTextColourId, Colours::white);
    lnf.setColour(TextEditor::textColourId, Colours::white);
    //lnf.setColour(ColourScheme::widgetBackground, Colour(50, 50, 50));
    lnf.setColour(TooltipWindow::backgroundColourId, Colour(25, 25, 25).withAlpha(float(0.8)));
    lnf.setColour (PopupMenu::backgroundColourId, Colour(50, 50, 50));
    lnf.setColour (PopupMenu::highlightedBackgroundColourId, Colour(41, 41, 41));
    
    compile_button.setConnectedEdges(12);
    addAndMakeVisible(compile_button);
    
    canvas_port.setViewedComponent(&canvas);
    addAndMakeVisible(canvas_port);

    canvas.setVisible(true);
    canvas.setBounds(0, 0, 1000, 700);
    
    compile_button.onClick = [this]() {
        auto patch = canvas.create_patch();
        
        auto objects = NodeConverter::create_objects(patch);
        
        auto formatted = NodeConverter::format_nodes(objects, Library::contexts);
        
        String code = "#include \"libcerite.h\" \n\n";
       
        code += Engine::combine_objects(formatted, Library::contexts);
        
        
        Thread::launch([this, code]() mutable{
            if(player) player->deviceManager.getAudioCallbackLock().enter();
            player.reset(new AudioPlayer(code));
        });
        
        
        
    };
    
    for(auto& button : toolbar_buttons) {
        button->setLookAndFeel(&look_and_feel);
        button->setConnectedEdges(12);
        addAndMakeVisible(button);
    }
    
    settings_dialog.reset(new SettingsDialog);
    toolbar_buttons[3]->onClick = [this]() {
        settings_dialog->setVisible(true);
    };

    for(auto& button : sidebar_buttons) {
        button->setColour(ComboBox::outlineColourId, findColour(TextButton::buttonColourId));
        button->setClickingTogglesState(true);
        button->setRadioGroupId(1001);
        button->setConnectedEdges(12);
        addAndMakeVisible(button);
    }
    
    hide_button.setClickingTogglesState(true);
    hide_button.setColour(ComboBox::outlineColourId, findColour(TextButton::buttonColourId));
    hide_button.setConnectedEdges(12);
    
    hide_button.onClick = [this](){
        sidebar_hidden = hide_button.getToggleState();
        repaint();
        resized();
    };
    
    addAndMakeVisible(hide_button);
}

MainComponent::~MainComponent()
{
}

//==============================================================================
void MainComponent::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
    
    auto base_colour = Colour(41, 41, 41);
    auto highlight_colour = Colour (0xff42a2c8).darker(0.3);
    
    // Toolbar background
    g.setColour(base_colour);
    g.fillRect(0, 0, getWidth(), toolbar_height);
    
    g.setColour(highlight_colour);
    g.fillRect(0, 42, getWidth(), 4);
    
    // Statusbar background
    g.setColour(base_colour);
    g.fillRect(0, getHeight() - statusbar_height, getWidth(), statusbar_height);
    
    int s_width = sidebar_hidden ? dragbar_width : sidebar_width;
    
    // Sidebar
    g.setColour(base_colour.darker(0.1));
    g.fillRect(getWidth() - s_width, dragbar_width, s_width, getHeight() - toolbar_height);
    
    // Draggable bar
    g.setColour(base_colour);
    g.fillRect(getWidth() - s_width, dragbar_width, statusbar_height, getHeight() - toolbar_height);
    
}

void MainComponent::resized()
{
    int s_width = sidebar_hidden ? dragbar_width : sidebar_width;
    
    canvas_port.setBounds(0, toolbar_height, getWidth() - s_width, getHeight() - toolbar_height - statusbar_height);
    
    compile_button.setBounds(getWidth() - s_width - 80, getHeight() - 30, 60, 23);
    
    int toolbar_position = 0;
    for(auto& button : toolbar_buttons) {
        button->setBounds(toolbar_position, 0, 70, toolbar_height);
        toolbar_position += 70;
    }
    
    int button_height = 40;
    int sidebar_position = ((getHeight() - toolbar_height) / 2.0f) - (button_height * (sidebar_buttons.size() / 2)) + (toolbar_height / 2.0f);
    
    for(auto& button : sidebar_buttons) {
        button->setBounds(getWidth() - s_width, sidebar_position, dragbar_width, button_height);
        sidebar_position += button_height;
    }
    
    hide_button.setBounds(getWidth() - s_width, toolbar_height + 20, dragbar_width, button_height);
    // This is called when the MainComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
}

void MainComponent::mouseDown(const MouseEvent &e) {
    Rectangle<int> drag_bar(getWidth() - sidebar_width, dragbar_width, sidebar_width, getHeight() - toolbar_height);
    if(drag_bar.contains(e.getPosition()) && !sidebar_hidden) {
        dragging_sidebar = true;
        drag_start_width = sidebar_width;
    }
    else {
        dragging_sidebar = false;
    }
    
   
}

void MainComponent::mouseDrag(const MouseEvent &e) {
    if(dragging_sidebar) {
        sidebar_width = drag_start_width - e.getDistanceFromDragStartX();
        repaint();
        resized();
    }
}

void MainComponent::mouseUp(const MouseEvent &e) {
    dragging_sidebar = false;
}
