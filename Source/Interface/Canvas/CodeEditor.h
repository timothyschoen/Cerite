/*
  ==============================================================================

   This file is part of the JUCE examples.
   Copyright (c) 2020 - Raw Material Software Limited

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES,
   WHETHER EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR
   PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             CodeEditorWindow
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Displays a code editor.

 dependencies:     juce_core, juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics, juce_gui_extra
 exporters:        xcode_mac, vs2019, linux_make, xcode_iphone

 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        CodeEditorWindow

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once
#include "../Looks.h"
#include "Box.h"
#include "../SaveDialog.h"
#include "../../Utility/FSManager.h"
#include "../CodeEditor/TextEditor.hpp"
#include "../Sidebar/Console.h"
//==============================================================================
class CodeEditor  : public Component
{
public:
    CodeEditor();
   
    std::function<void(File&)> refreshBox;
    
    inline static Console* consolePtr = nullptr;

    ~CodeEditor() override;
    
    void openFile(std::string newpath) {
        File content(newpath);
        //editor->moveCaretToEnd(false);
        
        editor->setText(content.loadFileAsString());
        path = newpath;
        currentFile = File(path);
        refreshBox(content);
    };

    bool keyPressed(const KeyPress& k) override {
        return editor->keyPressed(k);
    }
    void paint (Graphics& g) override;
    
    void paintOverChildren(Graphics& g) override;

    void resized() override;
    
    void compile();
    
    bool askToSave() {
        std::unique_ptr<SaveDialog> sdialog = std::make_unique<SaveDialog>();
        sdialog->setBounds(getScreenX()  + (getWidth() / 2.) - 200., getScreenY(), 400, 130);
        int action = sdialog->runModalLoop(); // appearantly this is a bad method to use but it does what i want...
        sdialog->setVisible(false);
        
        if(action == 1)
            savebutton.onClick();
        
        return action != 0;
    }
    
    std::string path;
    File currentFile;
    
    // the editor component
    std::unique_ptr<mcl::TextEditor> editor;
   
private:
    CeriteLookAndFeel clook;
    roundButton rlook;

    Console console;
    FileChooser saveChooser =  FileChooser("Select a save file", FSManager::home.getChildFile("Objects"), "*.obj");
    FileChooser openChooser = FileChooser("Choose save location", FSManager::home.getChildFile("Objects"), "*.obj");
    
    TextButton newbutton = TextButton("p", "New");
    TextButton openbutton = TextButton("o", "Open");
    TextButton savebutton = TextButton("O", "Save");
    
    TextButton undobutton = TextButton("x", "Undo");
    TextButton redobutton = TextButton("6", "Redo");
    TextButton compilebutton = TextButton("4", "Compile");

    CodeEditorComponent::ColourScheme getDarkCodeEditorColourScheme()
    {
        struct Type
        {
            const char* name;
            juce::uint32 colour;
        };

        const Type types[] =
        {
            { "Error",              0xffe60000 },
            { "Comment",            0xff72d20c },
            { "Keyword",            0xffee6f6f },
            { "Operator",           0xffc4eb19 },
            { "Identifier",         0xffcfcfcf },
            { "Integer",            0xff42c8c4 },
            { "Float",              0xff885500 },
            { "String",             0xffbc45dd },
            { "Bracket",            0xff058202 },
            { "Punctuation",        0xffcfbeff },
            { "Preprocessor Text",  0xfff8f631 }
        };

        CodeEditorComponent::ColourScheme cs;

        for (auto& t : types)
            cs.set (t.name, Colour (t.colour));

        return cs;
    }

    CodeEditorComponent::ColourScheme getLightCodeEditorColourScheme()
    {
        struct Type
        {
            const char* name;
            juce::uint32 colour;
        };

        const Type types[] =
        {
            { "Error",              0xffcc0000 },
            { "Comment",            0xff00aa00 },
            { "Keyword",            0xff0000cc },
            { "Operator",           0xff225500 },
            { "Identifier",         0xff000000 },
            { "Integer",            0xff880000 },
            { "Float",              0xff885500 },
            { "String",             0xff990099 },
            { "Bracket",            0xff000055 },
            { "Punctuation",        0xff004400 },
            { "Preprocessor Text",  0xff660000 }
        };

        CodeEditorComponent::ColourScheme cs;

        for (auto& t : types)
            cs.set (t.name, Colour (t.colour));

        return cs;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CodeEditor)
};

class Canvas;
struct CodeEditorWindow : public DocumentWindow
{
    
    std::unique_ptr<CodeEditor> editor;
    
    File boxfile;
    
    Box* box;
    Canvas* canvas;
    
    CodeEditorWindow(Canvas* cnv, Box* box);
    
    void closeButtonPressed() override;
    
    void resized() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CodeEditorWindow)
};
