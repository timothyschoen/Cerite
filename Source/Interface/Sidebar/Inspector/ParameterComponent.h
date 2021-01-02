#pragma once

#include <JuceHeader.h>
#include "../../Canvas/Canvas.h"
#include "../SidebarDialog.h"

class ParameterComponent : public Component, public Timer, public ValueTree::Listener
{
public:
    
    String name;
    ValueTree parameter;
    
    roundButton rlook;
    
    std::unique_ptr<Component> editor;
    
    TextButton advancedSettings = TextButton("m");
    
    bool dynamic = false;
    bool advanced;
    int type;
    
    Colour background;
    
    ParameterComponent(String n, ValueTree param, int idx, bool isAdvanced = true);
    
    ~ParameterComponent() {
        advancedSettings.setLookAndFeel(nullptr);
    }
    virtual void treeSetup();
    
    virtual void update(ValueTree param = ValueTree());
    
    
    void openAdvancedSettings();
    
    void paint(Graphics & g) override;
    
    void timerCallback() override;
    
    void resized() override;
    
    void valueTreePropertyChanged (ValueTree &treeWhosePropertyHasChanged, const Identifier &property) override;
    
    Slider* createSlider();
    
};

class SimpleParameterComponent : public ParameterComponent
{
public:
    
    SimpleParameterComponent(String n, ValueTree param, int idx, int t);
    
    void treeSetup() override;
    
    void update(ValueTree param = ValueTree()) override;
    
    void valueTreePropertyChanged (ValueTree &treeWhosePropertyHasChanged, const Identifier &property) override;
};
    
class CollapseablePanel : public Component
{
public:
    String title;

    ValueTree paramTree;
    OwnedArray<ParameterComponent> items;
    
    
    int itemheight = 25;
    bool expanded = true;
    
    std::function<bool(ValueTree)> filter;
    
    CollapseablePanel(String name);
    
    void clear() {
        items.clear();
    }
    
    void refresh() {};
    
    void addItem(ValueTree parentTree, String itemname, int type) {
        addAndMakeVisible(items.add(new SimpleParameterComponent(itemname, parentTree, items.size(), type)));
    }
    
    void addValueTree(ValueTree treeToEvaluate) {
        clear();
        paramTree = treeToEvaluate;
        for(auto child : treeToEvaluate) {
            if(filter(child))
                addAndMakeVisible(items.add(new ParameterComponent(child.getType().toString(), child, items.size())));
        }
    }
    
    void update() {
        int i = 0;
        for(auto child : paramTree) {
            if(filter(child) && i < items.size()) {
                items[i]->update(child);
                i++;
            }
        }
    }
    
    void setItemHeight(int h) {
        itemheight = h;
    }
    
    int getBestHeight() {
        return (expanded * itemheight * items.size()) + 30;
    }
    
    void drawTriangle (Graphics& g, float x1, float y1, float x2, float y2, float x3, float y3, Colour fill, Colour outline);

    
    void mouseDown(const MouseEvent& e) override;
    
    void paint(Graphics & g) override;
    
    void resized() override;
    
    void setPower(bool state);
    
};


class ParamEditDialog : public DialogBase
{
public:
    
    TextEditor minEditor;
    TextEditor maxEditor;
    TextEditor skewEditor;
    
    TextButton saveButton;
    TextButton cancelButton;
    
    Rectangle<int> window;
    
    ValueTree parameters;
    bool invalid = false;
    
    ParamEditDialog(ValueTree param) : parameters(param) {
        
        minEditor.setText(param.getProperty("Minimum").toString());
        maxEditor.setText(param.getProperty("Maximum").toString());
        skewEditor.setText(param.getProperty("Skew").toString());
        
        saveButton.setButtonText("Save");
        cancelButton.setButtonText("Cancel");
        
        addAndMakeVisible(cancelButton);
        addAndMakeVisible(saveButton);
        addAndMakeVisible(minEditor);
        addAndMakeVisible(maxEditor);
        addAndMakeVisible(skewEditor);
        
        cancelButton.onClick = [this]() {
            cancelled();
        };
        
        saveButton.onClick = [this]() {
            if(check())
                confirmed();
            else {
                invalid = true;
                repaint();
            }
        };
        
    }
    
    bool check() {
        if(minEditor.getText().getDoubleValue() < maxEditor.getText().getDoubleValue()) {
            parameters.setProperty("Minimum", minEditor.getText().getDoubleValue(), nullptr);
            parameters.setProperty("Maximum", maxEditor.getText().getDoubleValue(), nullptr);
            parameters.setProperty("Skew", skewEditor.getText().getDoubleValue(), nullptr);
            return true;
        }
        
        return false;
    }
    
    ~ParamEditDialog() {
        setLookAndFeel(nullptr);
    }
    
    
    void paint(Graphics & g) override {
        g.setColour(Colour(70, 70, 70));
        g.drawRect(getLocalBounds());
        g.setColour(Colour(50, 50, 50));
        g.fillRect(getLocalBounds());
        
        
        if(invalid) {
            g.setColour(Colours::orange);
            g.drawFittedText("Invalid settings!", 5, getHeight()-25, 85, 20, Justification::left, 1);
        }
        
        g.setColour(Colours::white);
        g.drawFittedText("Minimum", 10, 5,  40, 20, Justification::left, 1);
        g.drawFittedText("Maximum", 10, 30, 40, 20, Justification::left, 1);
        g.drawFittedText("Skew", 10, 55,    40, 20, Justification::left, 1);
        
    }
    void resized() override {
        minEditor.setBounds(60, 5, getWidth() - 80, 20);
        maxEditor.setBounds(60, 30, getWidth() - 80, 20);
        skewEditor.setBounds(60, 55, getWidth() - 80, 20);
        saveButton.setBounds(getWidth()-60, getHeight()-25, 50, 20);
        cancelButton.setBounds(getWidth()-120, getHeight()-25, 50, 20);
    }
    
};
