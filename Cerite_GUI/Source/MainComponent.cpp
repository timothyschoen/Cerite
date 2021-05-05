#include "MainComponent.h"
#include "Canvas.h"
#include "../../Source/NodeConverter.hpp"
#include "../../Source/Engine.hpp"
#include "../../Source/Library.hpp"
#include "../../Source/AudioPlayer.hpp"


//==============================================================================
MainComponent::MainComponent() : ValueTreeObject(ValueTree("Main")), console(true, true)
{
    setSize(1000, 700);
        
    
    player.reset(nullptr);
    set_remote(false);
    
    setLookAndFeel(&main_look);
    
    compile_button.setClickingTogglesState(true);
    compile_button.setConnectedEdges(12);
    compile_button.setLookAndFeel(&statusbar_look);
    addAndMakeVisible(compile_button);
    
    
    autocompile_button.setClickingTogglesState(true);
    autocompile_button.setConnectedEdges(12);
    autocompile_button.setLookAndFeel(&statusbar_look);
    addAndMakeVisible(autocompile_button);
    
    autocompile_button.onClick = [this](){
        bool state = autocompile_button.getToggleState();
        compile_button.setEnabled(!state);
        compile_button.setToggleState(false, sendNotification);
        autocompile = state;
    };
    
    
    addAndMakeVisible(canvas_port);
    rebuildObjects();
    
    addAndMakeVisible(&console);
    addChildComponent(&code_editor);
    
    
    
    compile_button.onClick = [this]() {
        
        if(!compile_button.getToggleState()) {
            player->enabled = false;
            return;
        }
        auto patch = canvas->create_patch();
        
        player->enabled = false;
        player->apply_settings(settings_dialog->get_settings());
        player->compile(patch);
        
        for(auto& box : canvas->findChildrenOfClass<Box>()) {
            if(box->graphics) {
                box->graphics->register_object();
            }
        }
        
        player->enabled = true;
    };
    
    for(auto& button : toolbar_buttons) {
        button->setLookAndFeel(&toolbar_look);
        button->setConnectedEdges(12);
        addAndMakeVisible(button);
    }
    
    // New button
    toolbar_buttons[0]->onClick = [this]() {
        auto cnv_state = getState().getChildWithName("Canvas");
        if(cnv_state.isValid())  {
            getState().removeChild(cnv_state, nullptr);
            canvas_port.setViewedComponent(nullptr, false);
        }
        auto new_cnv = ValueTree("Canvas");
        getState().appendChild(new_cnv, nullptr);
    };
    
    // Open button
    toolbar_buttons[1]->onClick = [this]() {
        open_project();
    };
    
    // Save button
    toolbar_buttons[2]->onClick = [this]() {
        save_project();
    };
    
    sidebar_buttons[0]->onClick = [this]() {
        code_editor.setVisible(true);
        console.setVisible(false);
    };
    
    sidebar_buttons[1]->onClick = [this]() {
        code_editor.setVisible(false);
        console.setVisible(true);
    };
    
    sidebar_buttons[1]->setToggleState(true, dontSendNotification);
    
    // Settings dialog
    settings_dialog.reset(new SettingsDialog);
    toolbar_buttons[3]->onClick = [this]() {
        settings_dialog->setVisible(true);
    };

    for(auto& button : sidebar_buttons) {
        button->setLookAndFeel(&sidebar_look);
        button->setClickingTogglesState(true);
        button->setRadioGroupId(1001);
        button->setConnectedEdges(12);
        addAndMakeVisible(button);
    }
    hide_button.setLookAndFeel(&sidebar_look);
    hide_button.setClickingTogglesState(true);
    hide_button.setColour(ComboBox::outlineColourId, findColour(TextButton::buttonColourId));
    hide_button.setConnectedEdges(12);
    
    hide_button.onClick = [this](){
        sidebar_hidden = hide_button.getToggleState();
        hide_button.setButtonText(sidebar_hidden ? "J" : "K");
        
        repaint();
        resized();
    };
    
    addAndMakeVisible(hide_button);
    
    
    if(!getState().hasProperty("Canvas")) {
        getState().appendChild(ValueTree("Canvas"), nullptr);
    }
    
    
    current_main = this;
}

MainComponent::~MainComponent()
{
    current_main = nullptr;
    setLookAndFeel(nullptr);
}


ValueTreeObject* MainComponent::factory(const Identifier& id, const ValueTree& tree)
{
    if(id == Identifier("Canvas")) {
        canvas_port.setViewedComponent(nullptr, false);

        canvas = new Canvas(tree);
        
        canvas_port.setViewedComponent(canvas, false);
        canvas->setVisible(true);
        canvas->setBounds(0, 0, 1000, 700);
        
        return static_cast<ValueTreeObject*>(canvas);
    }
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
    
    int s_content_width = s_width - dragbar_width;
    
    console.setBounds(getWidth() - s_content_width, toolbar_height, s_content_width, getHeight() - toolbar_height);
    code_editor.setBounds(getWidth() - s_content_width, toolbar_height, s_content_width, getHeight() - toolbar_height);
    
    canvas_port.setBounds(0, toolbar_height, getWidth() - s_width, getHeight() - toolbar_height - statusbar_height);
    
    compile_button.setBounds(getWidth() - s_width - 40, getHeight() - 32, 30, 30);
    autocompile_button.setBounds(getWidth() - s_width - 80, getHeight() - 32, 30, 30);
    
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


void MainComponent::open_project() {
    open_chooser.launchAsync(FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles, [this](const FileChooser& f) {
          File openedfile = f.getResult();
          if(openedfile.exists() && openedfile.getFileExtension().equalsIgnoreCase(".crpat")) {
              try
              {
                  auto cnv_state = getState().getChildWithName("Canvas");
                  if(cnv_state.isValid())  {
                      getState().removeChild(cnv_state, nullptr);
                      canvas_port.setViewedComponent(nullptr, false);
                  }
                  
                  auto stream = openedfile.createInputStream();
                  auto new_cnv = ValueTree::readFromStream(*stream);
                  getState().appendChild(new_cnv, nullptr);
              }
              catch (...)
              {
                  std::cout << "Failed to open project" << std::endl;
              }
          }
    
    });
}

void MainComponent::save_project() {
    auto cnv = getState().getChildWithName("Canvas");
    
    if(cnv.isValid()) {
        save_chooser.launchAsync(FileBrowserComponent::saveMode | FileBrowserComponent::warnAboutOverwriting, [this](const FileChooser &f) {
            File result = f.getResult();
            
            FileOutputStream ostream(result);
            canvas->getState().writeToStream(ostream);
        });
    }
}


void MainComponent::set_remote(bool is_remote) {
    if(is_remote && player) {
        player->device_manager.closeAudioDevice();
        player.reset(nullptr);
    }
    else if (!is_remote && !player){
        player.reset(new AudioPlayer);
    }
}

void MainComponent::send_data(int port, libcerite::Data data) {
    
    if(player) {
        player->send_data(port, data);
    }
    else {
        
    }
}
void MainComponent::receive_data(int port, std::function<void(libcerite::Data)> callback) {
    if(player) {
        player->set_receive_callback(port, callback);
    }
    else {
        
    }
}
