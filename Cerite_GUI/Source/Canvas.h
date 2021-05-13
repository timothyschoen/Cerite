#pragma once

#include <JuceHeader.h>
#include "Box.h"
#include "Utility/gin_valuetreeobject.h"
//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/

using Patch = std::vector<std::tuple<String, int, int, std::map<String, std::vector<std::vector<int>>>>>;

struct Identifiers
{
    // Object identifiers
    inline static Identifier canvas = Identifier("Canvas");
    inline static Identifier box = Identifier("Box");
    inline static Identifier edge = Identifier("Edge");
    inline static Identifier connection = Identifier("Connection");
    
    // Parameter identifiers
    
    // Box
    inline static Identifier box_x = Identifier("X");
    inline static Identifier box_y = Identifier("Y");
    inline static Identifier box_name = Identifier("Name");
    
    // Edge
    inline static Identifier edge_id = Identifier("ID");
    inline static Identifier edge_in = Identifier("Input");
    inline static Identifier edge_ctx = Identifier("Context");

    
    // Connection
    inline static Identifier start_id = Identifier("StartID");
    inline static Identifier end_id = Identifier("EndID");
    
};

class Edge;
class Canvas  : public Component, public ValueTreeObject, public KeyListener, public Timer
{
public:
    //==============================================================================
    Canvas(ValueTree tree);
    ~Canvas();
    
    ValueTreeObject* factory (const juce::Identifier&, const juce::ValueTree&) override;

    //==============================================================================
    void paintOverChildren (Graphics&) override;
    void resized() override;
    
    void mouseDown(const MouseEvent& e) override;
    void mouseDrag(const MouseEvent& e) override;
    void mouseUp(const MouseEvent& e) override;
    void mouseMove(const MouseEvent& e) override;
    
    Patch create_patch();
    
    bool keyPressed(const KeyPress &key, Component *originatingComponent) override;
    
    String copy_selection();
    void remove_selection();
    void paste_selection(String to_paste);
    
    void undo() {
        undo_manager.undo();
    }
    void redo() {
        undo_manager.redo();
    }
    
    void encapsulate(std::function<String(ValueTree)> encapsulate_func);

    
    void timerCallback() override;
    
    void valueTreeChanged() override {
        startTimer(250);
    }
    
    Edge* find_edge_by_id(String ID);
    
    Array<Edge*> get_all_edges();

    UndoManager undo_manager;
    
    Viewport viewport;
    
    bool connecting_with_drag = false;
    
private:

    MultiComponentDragger<Box> dragger = MultiComponentDragger<Box>(this);

    LassoComponent<Box*> lasso;
    
    Point<int> drag_start_position;
   
    PopupMenu popupmenu;
    
    FileChooser obj_save_chooser =  FileChooser("Save subpatch", File::getSpecialLocation(File::SpecialLocationType::userDocumentsDirectory).getChildFile("Cerite").getChildFile("Objects"), "*.obj");
    
    FileChooser patch_save_chooser =  FileChooser("Save subpatch", File::getSpecialLocation(File::SpecialLocationType::userDocumentsDirectory).getChildFile("Cerite").getChildFile("Saves"), "*.crpat");
    
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Canvas)
};
