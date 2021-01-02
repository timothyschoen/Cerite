#include "PresetComponent.h"
#include "Inspector.h"
#include "../Sidebar.h"

PresetComponent::PresetComponent() {
    addAndMakeVisible(presetLabel);
    addAndMakeVisible(presetSelector);
    addAndMakeVisible(descriptionLabel);
    addAndMakeVisible(descriptionEditor);
    
    presetLabel.setText("Preset");
    presetLabel.setReadOnly(true);
    presetLabel.setCaretVisible(false);
    presetLabel.setColour(TextEditor::backgroundColourId, colours[0]);
    presetLabel.setColour(TextEditor::outlineColourId, colours[0]);
    
    descriptionLabel.setText("Description");
    descriptionLabel.setReadOnly(true);
    descriptionLabel.setCaretVisible(false);
    descriptionLabel.setColour(TextEditor::backgroundColourId, colours[1]);
    descriptionLabel.setColour(TextEditor::outlineColourId, colours[1]);
    
    descriptionEditor.setReadOnly(true);
    descriptionEditor.setCaretVisible(false);
    descriptionEditor.setMultiLine(true, false);
    descriptionEditor.setColour(TextEditor::backgroundColourId, colours[1]);
    descriptionEditor.setColour(TextEditor::outlineColourId, colours[1]);
    addAndMakeVisible(deleteButton);
    addAndMakeVisible(saveButton);
    
    deleteButton.onClick = [this]() {
        deletePreset();
    };
    
    saveButton.onClick = [this]() {
        savePreset();
    };
}

void PresetComponent::resized() {
    presetLabel.setBounds(0, 10, getWidth() / 2., 20);
    presetSelector.setBounds(getWidth() / 2., 11, (getWidth() / 2) - 20., 18);
    
    descriptionLabel.setBounds(0, 30, getWidth() / 2., 30);
    descriptionEditor.setBounds(getWidth() / 2., 30, (getWidth() / 2) - 20., 25);

    deleteButton.setBounds(getWidth() - 70, 57, 50, 18);
    saveButton.setBounds(getWidth() - 130, 57, 50, 18);
}

void PresetComponent::deletePreset() {
    /*
    ValueTree typetree = PresetManager::userPresets.getOrCreateChildWithName(currentType, nullptr);
    typetree.removeChild(typetree.getChildWithProperty("Preset", presetSelector.getText()), nullptr);
    
    currentTree.getChildWithName("Parameters").setProperty("Preset", "Default", nullptr);
    
    presetSelector.setText("Default");
    loadTree(currentType, currentTree);
    
    PresetManager::savePreset();
     */
}

void PresetComponent::savePreset() {

    Sidebar* sbar = findParentComponentOfClass<Sidebar>();
    std::unique_ptr<DialogBase> paramdialog = std::make_unique<ParamSaveDialog>(presetSelector.getText(), descriptionEditor.getText(), currentType);
    int action = sbar->createDialog(paramdialog.get(), this);
    
    if(action == 1) {
        Array<String> presetinfo = paramdialog->getResult();
        ValueTree newParameter("Preset");
        
        newParameter.copyPropertiesAndChildrenFrom(currentTree.getChildWithName("Parameters"), nullptr);
        newParameter.setProperty("Preset", presetinfo[0], nullptr);
        newParameter.setProperty("Description", presetinfo[1], nullptr);
        

        //ValueTree typetree = PresetManager::userPresets.getOrCreateChildWithName(currentType, nullptr);
        //typetree.appendChild(newParameter, nullptr);
        
        //PresetManager::savePreset();
        
        ValueTree parameters = currentTree.getOrCreateChildWithName("Parameters", nullptr);
        parameters.setProperty("Preset", presetinfo[0], nullptr);
        deleteButton.setEnabled(true);
        
        loadTree(currentType, currentTree);
    }

}

static ValueTree findPreset(ValueTree tree1, ValueTree tree2, String name) {
    return tree1.getChildWithProperty("Preset", name).isValid() ? tree1.getChildWithProperty("Preset", name) : tree2.getChildWithProperty("Preset", name);
}


void PresetComponent::loadTree(String name, ValueTree boxTree) {
    int id = 1;
    /*
    currentTree = boxTree;
    currentType = name;
    
    ValueTree parameters = boxTree.getOrCreateChildWithName("Parameters", nullptr);
    ValueTree factory = PresetManager::factoryPresets.getChildWithName(name);
    ValueTree user = PresetManager::userPresets.getChildWithName(name);
    
    presetSelector.clear();
    
    // Item for default presets
    presetSelector.addItem("Default", id);
    
    presetSelector.addSeparator();

    if(name.containsNonWhitespaceChars() && (factory.isValid() || user.isValid())) {
        
        // Fill preset combobox
        for (auto preset : user) {
            id++;
            presetSelector.addItem(preset.getProperty("Preset"), id);
        }
        
        presetSelector.addSeparator();
        
        for (auto preset : factory) {
            id++;
            presetSelector.addItem(preset.getProperty("Preset"), id);
        }
        
        // Make preset selector display correct value when selecting an object
        if(parameters.hasProperty("Preset") && parameters.getProperty("Preset").toString() == "Default") {
            presetSelector.setText("Default");
            descriptionEditor.setText("Default Preset for " + name);
        }
        else if(parameters.hasProperty("Preset") && !parameters.getProperty("Preset").isInt()) {
                ValueTree preset = findPreset(user, factory, parameters.getProperty("Preset"));
                presetSelector.setText(preset.getProperty("Preset"));
                descriptionEditor.setText(preset.getProperty("Description"));
        }
        else {
            presetSelector.setText("");
            descriptionEditor.setText("");
        }
        
        presetSelector.onChange = [this, name, parameters, factory, user]() mutable {
            
            std::vector<String> args;
            
            //for(int i = 0; i < ComponentDictionary::getInfo(name).numArguments; i++){
            //    args.push_back(parameters.getChild(i).getProperty("Value").toString());
            }
            
            ValueTree preset;
            
            if(presetSelector.getText() == "Default" && parameters.getProperty("Preset").toString() != "Default") {
                parameters.removeAllChildren(&Canvas::mainCanvas->undoManager);
                descriptionEditor.setText("Default Preset for " + name);
                //ComponentDictionary::getParameters(name, parameters, args);
                parameters.setProperty("Preset", "Default", nullptr);
                deleteButton.setEnabled(false);
                Canvas::mainCanvas->startNewAction();
                refreshParameters();
            }
            // load preset values if the preset has changed
            else if(presetSelector.getText() != parameters.getProperty("Preset").toString()) {
                preset = findPreset(factory, user, presetSelector.getText());
                if(user.getChildWithProperty("Preset", presetSelector.getText()).isValid()) {
                    deleteButton.setEnabled(true);
                }
                else {
                    deleteButton.setEnabled(false);
                }
                
                descriptionEditor.setText(preset.getProperty("Description"));
                parameters.copyPropertiesAndChildrenFrom(preset, &Canvas::mainCanvas->undoManager);
                Canvas::mainCanvas->startNewAction();
                refreshParameters();
            }

        };
        
    }
    else {
        descriptionEditor.setText(String());
        presetSelector.onChange = [this, user, factory, parameters]() mutable {
            
            ValueTree preset = findPreset(user, factory, presetSelector.getText());
            parameters.setProperty("Preset", presetSelector.getText(), &Canvas::mainCanvas->undoManager);
            Canvas::mainCanvas->startNewAction();
            refreshParameters();
        };
    }
    */
}

void PresetComponent::refreshParameters()
{
    
    findParentComponentOfClass<Inspector>()->update();

}
