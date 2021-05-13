#include "MainComponent.h"
#include "Canvas.h"
#include "Edge.h"
#include "../../Source/NodeConverter.hpp"
#include "../../Source/Engine.hpp"
#include "../../Source/Library.hpp"
#include "../../Source/AudioPlayer.hpp"


//==============================================================================
MainComponent::MainComponent() : ValueTreeObject(ValueTree("Main")), console(true, true)
{
    setSize(1000, 700);
        
    tabbar.setColour(TabbedButtonBar::frontOutlineColourId, Colour(41, 41, 41));
    tabbar.setColour(TabbedButtonBar::tabOutlineColourId, Colour(41, 41, 41));
    tabbar.setColour(TabbedComponent::outlineColourId, Colour(41, 41, 41));
    
    player.reset(nullptr);
    set_remote(false);
    
    settings_dialog.reset(new SettingsDialog);
    
    setLookAndFeel(&main_look);
    
    tabbar.on_tab_change = [this](int idx) {
        Edge::connecting_edge = nullptr;
        triggerChange();
    };
    
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
    };
    
    addAndMakeVisible(tabbar);
    rebuildObjects();
    
    addAndMakeVisible(&console);
    addChildComponent(&code_editor);
    
    
    compile_button.onClick = [this]() {
        
        if(!compile_button.getToggleState()) {
            player->enabled = false;
            return;
        }
        
        auto patch = get_current_canvas()->create_patch();
        
        player->enabled = false;
        player->apply_settings(settings_dialog->get_settings());
        bool result = player->compile(patch);
        
        for(auto& box : get_current_canvas()->findChildrenOfClass<Box>()) {
            if(box->graphics && result) {
                box->graphics->register_object();
            }
        }
        
        player->enabled = result;
    };
    
    for(auto& button : toolbar_buttons) {
        button.setLookAndFeel(&toolbar_look);
        button.setConnectedEdges(12);
        addAndMakeVisible(button);
    }
    
    // New button
    toolbar_buttons[0].onClick = [this]() {
        auto new_cnv = ValueTree("Canvas");
        new_cnv.setProperty("Title", "Untitled Patcher", nullptr);
        getState().appendChild(new_cnv, nullptr);
    };
    
    // Open button
    toolbar_buttons[1].onClick = [this]() {
        open_project();
    };
    
    // Save button
    toolbar_buttons[2].onClick = [this]() {
        save_project();
    };
    
    toolbar_buttons[3].onClick = [this]() {
        get_current_canvas()->undo();
    };
    
    toolbar_buttons[4].onClick = [this]() {
        get_current_canvas()->redo();
    };
    
    toolbar_buttons[5].onClick = [this]() {
        settings_dialog->setVisible(true);
        settings_dialog->toFront(true);
    };
    
    sidebar_buttons[0].onClick = [this]() {
        code_editor.setVisible(true);
        console.setVisible(false);
    };
    
    sidebar_buttons[1].onClick = [this]() {
        code_editor.setVisible(false);
        console.setVisible(true);
    };
    
    sidebar_buttons[1].setToggleState(true, dontSendNotification);
    


    for(auto& button : sidebar_buttons) {
        button.setLookAndFeel(&sidebar_look);
        button.setClickingTogglesState(true);
        button.setRadioGroupId(1001);
        button.setConnectedEdges(12);
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
        ValueTree cnv_state("Canvas");
        cnv_state.setProperty("Title", "Untitled Patcher", nullptr);
        getState().appendChild(cnv_state, nullptr);
    }
    
    
    current_main = this;
}

MainComponent::~MainComponent()
{
    current_main = nullptr;
    setLookAndFeel(nullptr);
    compile_button.setLookAndFeel(nullptr);
    autocompile_button.setLookAndFeel(nullptr);
    hide_button.setLookAndFeel(nullptr);
    
    for(auto& button : toolbar_buttons) {
        button.setLookAndFeel(nullptr);
    }
    
    for(auto& button : sidebar_buttons) {
        button.setLookAndFeel(nullptr);
    }
}


ValueTreeObject* MainComponent::factory(const Identifier& id, const ValueTree& tree)
{
    if(id == Identifiers::canvas) {
        canvas = new Canvas(tree);
        add_tab(canvas);

        
        return static_cast<ValueTreeObject*>(canvas);
    }
    
    return nullptr;
}


//==============================================================================
void MainComponent::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
    
    auto base_colour = Colour(41, 41, 41);
    auto highlight_colour = Colour (0xff42a2c8).darker(0.2);
    
    // Toolbar background
    g.setColour(base_colour);
    g.fillRect(0, 0, getWidth(), toolbar_height);
    
    g.setColour(highlight_colour);
    g.fillRect(0, 42, getWidth(), 4);
    
    // Statusbar background
    g.setColour(base_colour);
    g.fillRect(0, getHeight() - statusbar_height, getWidth(), statusbar_height);
    
    int s_width = sidebar_hidden ? dragbar_width : std::max(dragbar_width, sidebar_width);
    
    // Sidebar
    g.setColour(base_colour.darker(0.1));
    g.fillRect(getWidth() - s_width, dragbar_width, s_width, getHeight() - toolbar_height);
    
    // Draggable bar
    g.setColour(base_colour);
    g.fillRect(getWidth() - s_width, dragbar_width, statusbar_height, getHeight() - toolbar_height);
    
}

void MainComponent::resized()
{
    int s_width = sidebar_hidden ? dragbar_width : std::max(dragbar_width, sidebar_width);
    
    int s_content_width = s_width - dragbar_width;
    
    int sbar_y = toolbar_height - 15;
    
    console.setBounds(getWidth() - s_content_width, sbar_y, s_content_width, getHeight() - sbar_y);
    code_editor.setBounds(getWidth() - s_content_width, sbar_y, s_content_width, getHeight() - sbar_y);
    
    tabbar.setBounds(0, toolbar_height, getWidth() - s_width, getHeight() - toolbar_height - statusbar_height);
    
    compile_button.setBounds(getWidth() - s_width - 40, getHeight() - 32, 30, 30);
    autocompile_button.setBounds(getWidth() - s_width - 80, getHeight() - 32, 30, 30);
    
    
    int jump_positions[2] = {3, 5};
    int idx = 0;
    int toolbar_position = 0;
    for(auto& button : toolbar_buttons) {
        int spacing = (25 * (idx >= jump_positions[0])) +  (25 * (idx >= jump_positions[1]));
        button.setBounds(toolbar_position + spacing, 0, 70, toolbar_height);
        toolbar_position += 70;
        idx++;
    }
    
    int button_height = 40;
    int sidebar_position = ((getHeight() - toolbar_height) / 2.0f) - (button_height * (sidebar_buttons.size() / 2)) + (toolbar_height / 2.0f);
    
    for(auto& button : sidebar_buttons) {
        button.setBounds(getWidth() - s_width, sidebar_position, dragbar_width, button_height);
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
                  auto stream = openedfile.createInputStream();
                  auto new_cnv = ValueTree::readFromStream(*stream);
                  new_cnv.setProperty("Title", openedfile.getFileName(), nullptr);
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
            get_current_canvas()->getState().writeToStream(ostream);
            get_current_canvas()->getState().setProperty("Title", result.getFileName(), nullptr);
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

void MainComponent::triggerChange() {
    if(!get_current_canvas())  {
        toolbar_buttons[3].setEnabled(false);
        toolbar_buttons[4].setEnabled(false);
        return;
    }
    
    toolbar_buttons[3].setEnabled(get_current_canvas()->undo_manager.canUndo());
    toolbar_buttons[4].setEnabled(get_current_canvas()->undo_manager.canRedo());
    
    if(autocompile_button.getToggleState()) {
        
        auto patch = get_current_canvas()->create_patch();
        Thread::launch([this, patch]() mutable {
        
            player->enabled = false;
            player->apply_settings(settings_dialog->get_settings());
            bool result = player->compile(patch);
            
            for(auto& box : get_current_canvas()->findChildrenOfClass<Box>()) {
                if(box->graphics && result) {
                    box->graphics->register_object();
                }
            }
            
            player->enabled = result;
        });
    }
}

Canvas* MainComponent::get_current_canvas()
{
    if(auto* viewport = dynamic_cast<Viewport*>(tabbar.getCurrentContentComponent())) {
        if(auto* cnv = dynamic_cast<Canvas*>(viewport->getViewedComponent())) {
            return cnv;
        }
    }
    return nullptr;
}

void MainComponent::add_tab(Canvas* cnv)
{
    tabbar.addTab(cnv->getState().getProperty("Title"), Colour(50, 50, 50), &cnv->viewport, false);
    
    int tab_idx = tabbar.getNumTabs() - 1;
    tabbar.setCurrentTabIndex(tab_idx);
    
    if(tabbar.getNumTabs() > 1) {
        tabbar.getTabbedButtonBar().setVisible(true);
        tabbar.setTabBarDepth(30);
    }
    else {
        tabbar.getTabbedButtonBar().setVisible(false);
        tabbar.setTabBarDepth(1);
    }
    
    auto* tab_button = tabbar.getTabbedButtonBar().getTabButton(tab_idx);
    
    auto* close_button = new TextButton("x");
   
    close_button->onClick = [this, tab_button]() mutable {
        
        int idx = -1;
        for(int i = 0; i < tabbar.getNumTabs(); i++) {
            if(tabbar.getTabbedButtonBar().getTabButton(i) == tab_button) {
                idx = i;
                break;
            }
        }
        if(idx == -1) return;
        
        if(tabbar.getCurrentTabIndex() == idx) {
            tabbar.setCurrentTabIndex(idx == 1 ? idx - 1 : idx + 1);
        }
        tabbar.removeTab(idx);
        
        if(tabbar.getNumTabs() == 1) {
            tabbar.getTabbedButtonBar().setVisible(false);
            tabbar.setTabBarDepth(1);
        }
    };
    
    close_button->setColour(TextButton::buttonColourId, Colour());
    close_button->setColour(TextButton::buttonOnColourId, Colour());
    close_button->setColour(ComboBox::outlineColourId, Colour());
    close_button->setColour(TextButton::textColourOnId, Colours::white);
    close_button->setColour(TextButton::textColourOffId, Colours::white);
    close_button->setConnectedEdges(12);
    tab_button->setExtraComponent(close_button, TabBarButton::beforeText);
    
    close_button->setVisible(true);
    close_button->setSize(28, 28);
    
    tabbar.repaint();
    
    canvas->setVisible(true);
    canvas->setBounds(0, 0, 1000, 700);
}
