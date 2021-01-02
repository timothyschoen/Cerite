#pragma once
#include <JuceHeader.h>
#include "../../Canvas/Canvas.h"
#include "../SidebarDialog.h"

class PresetComponent : public Component
{

public:
    TextEditor presetLabel;
    ComboBox presetSelector;
    
    TextEditor descriptionLabel;
    TextEditor descriptionEditor;
    
    TextButton deleteButton  = TextButton("Delete");
    TextButton saveButton = TextButton("Save");
    
    ValueTree currentTree;
    String currentType;
    
    Array<Colour> colours = {Colour(42, 42, 42), Colour(50, 50, 50)};
    
    PresetComponent();
    
    void resized() override;

    void loadTree(String name, ValueTree boxTree);
    
    void refreshParameters();
    
    void savePreset();
    
    void deletePreset();
};

class ParamSaveDialog : public DialogBase
{
public:
    
    TextEditor nameEditor;
    TextEditor descriptionEditor;
    
    TextButton saveButton;
    TextButton cancelButton;
    
    Rectangle<int> window;
    
    
    String type;
    bool notUnique = false;
    
    ParamSaveDialog(String currentName, String currentDescription, String typeToCheck) : type(typeToCheck) {
        
        nameEditor.setText(currentName);
        descriptionEditor.setText(currentDescription);
        saveButton.setButtonText("Save");
        cancelButton.setButtonText("Cancel");
        
        addAndMakeVisible(cancelButton);
        addAndMakeVisible(saveButton);
        addAndMakeVisible(nameEditor);
        addAndMakeVisible(descriptionEditor);
        
        cancelButton.onClick = [this]() {
            cancelled();
        };
        
        saveButton.onClick = [this]() {
            if(checkType())
                confirmed();
            else {
                notUnique = true;
                repaint();
            }
        };
        
    }
    
    bool checkType() {
        
       // bool factoryExists = PresetManager::factoryPresets.getChildWithName(type).getChildWithProperty("Preset", nameEditor.getText()).isValid();
        //bool userExists = PresetManager::userPresets.getChildWithName(type).getChildWithProperty("Preset", nameEditor.getText()).isValid();
                
        return false; //!(factoryExists || userExists);
    }
    
    ~ParamSaveDialog() {
        setLookAndFeel(nullptr);
    }
    
    Array<String> getResult() override {
        return {nameEditor.getText(), descriptionEditor.getText()};
    };
    
    void paint(Graphics & g) override {
        g.setColour(Colour(70, 70, 70));
        g.drawRect(getLocalBounds());
        g.setColour(Colour(50, 50, 50));
        g.fillRect(getLocalBounds());
        
        
        if(notUnique) {
            g.setColour(Colours::orange);
            g.drawFittedText("Name already taken!", 5, getHeight()-25, 85, 20, Justification::left, 1);
        }
        
        g.setColour(Colours::white);
        g.drawFittedText("Name", 10, 10, 180, 15, Justification::left, 1);
        g.drawFittedText("Description", 10, 55, 180, 15, Justification::left, 1);
        
    }
    void resized() override {
        nameEditor.setBounds(10, 30, getWidth() - 20, 20);
        descriptionEditor.setBounds(10, 75, getWidth() - 20, 35);
        saveButton.setBounds(getWidth()-60, getHeight()-25, 50, 20);
        cancelButton.setBounds(getWidth()-120, getHeight()-25, 50, 20);
    }
    
};
