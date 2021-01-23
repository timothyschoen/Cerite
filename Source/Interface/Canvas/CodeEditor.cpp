#include "CodeEditor.h"
#include "Canvas.h"
#include "../ComponentDictionary.h"
#include "../../Engine/Compiler/CodeWriter.h"

CodeEditor::CodeEditor()
{
    setOpaque (true);
    
    // Create the editor..
    //
    editor.reset (new mcl::TextEditor());

    addAndMakeVisible (editor.get());
    
    setLookAndFeel(&clook);
    
    addAndMakeVisible(newbutton);
    addAndMakeVisible(openbutton);
    addAndMakeVisible(savebutton);
    addAndMakeVisible(undobutton);
    addAndMakeVisible(redobutton);
    addAndMakeVisible(compilebutton);
    addAndMakeVisible(console);
                      
    rlook.setColour(ComboBox::outlineColourId, Colour(0));
    
    newbutton.setLookAndFeel(&rlook);
    openbutton.setLookAndFeel(&rlook);
    savebutton.setLookAndFeel(&rlook);
    undobutton.setLookAndFeel(&rlook);
    redobutton.setLookAndFeel(&rlook);
    compilebutton.setLookAndFeel(&rlook);
    
    // has changed doesnt work yet!
    newbutton.onClick = [this]() {
        
        if(editor->changedSinceSave()) {
            if(askToSave())
                editor->setText("");
        }
        else {
            editor->setText("");
        }
            
    };
    openbutton.onClick = [this]() {
        if(editor->changedSinceSave()) {
            if(askToSave() && openChooser.browseForFileToOpen()) {
                currentFile = openChooser.getResult();
                editor->setText(currentFile.loadFileAsString());
                refreshBox(currentFile);
            }
        }
        else if(openChooser.browseForFileToOpen()) {
            editor->setText(openChooser.getResult().loadFileAsString());
            refreshBox(currentFile);
        }
    };
    savebutton.onClick = [this]() {
        if (saveChooser.browseForFileToSave(true))
        {
            currentFile = saveChooser.getResult();
            currentFile.replaceWithText(editor->getText());
            editor->setSavePoint();
            refreshBox(currentFile);
        }
    };
    
    undobutton.onClick = [this]() {
        editor->undoStep();
    };
    
    redobutton.onClick = [this]() {
        editor->undoStep();
    };
    
    compilebutton.onClick = [this]() {
        compile();
    };
    
    
    consolePtr = &console;
    
    lookAndFeelChanged();
    setSize (500, 500);
    
    editor->setColour(CodeEditorComponent::backgroundColourId, findColour(CodeEditorComponent::backgroundColourId).darker());
    editor->setColour(CaretComponent::caretColourId, Colours::white);
}

CodeEditor::~CodeEditor()
{
    setLookAndFeel(nullptr);
    newbutton.setLookAndFeel(nullptr);
    openbutton.setLookAndFeel(nullptr);
    savebutton.setLookAndFeel(nullptr);
    undobutton.setLookAndFeel(nullptr);
    redobutton.setLookAndFeel(nullptr);
    compilebutton.setLookAndFeel(nullptr);
}

void CodeEditor::paint (Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour(ResizableWindow::backgroundColourId).darker());
    
    g.setColour(getLookAndFeel().findColour(Toolbar::backgroundColourId).darker());
    g.fillRect(getLocalBounds().removeFromTop (40));

}

void CodeEditor::paintOverChildren(Graphics& g)
{
    g.setColour(findColour(Toolbar::backgroundColourId).darker());
    g.fillRect(getWidth() - 260, 40, 10, getHeight() - 40);
}

void CodeEditor::compile() {
    
    if(!currentFile.exists()) return;
    
    Reader reader;
    Document doc = reader.parse(currentFile.getFullPathName().toStdString());
    /*
    Compiler::compile(CodeWriter::exportCode(doc), [](const char* str){
        consolePtr->logMessage(str);
    }); */
    
}

void CodeEditor::resized()
{
    auto r = getLocalBounds().reduced (8);
    
    newbutton.setBounds(5, 5, 32, 32);
    openbutton.setBounds(45, 5, 32, 32);
    savebutton.setBounds(85, 5, 32, 32);
    
    undobutton.setBounds(150, 5, 32, 32);
    redobutton.setBounds(190, 5, 32, 32);
    compilebutton.setBounds(250, 5, 32, 32);
    
    editor->setBounds(0, 40, getWidth() - 250, getHeight() - 40);
    console.setBounds(getWidth() - 250, 40, 250, getHeight() - 40);
   
}



CodeEditorWindow::CodeEditorWindow(Canvas* cnv, Box* b) : DocumentWindow("test",
                                                      Colours::darkgrey,
                                                      DocumentWindow::allButtons), box(b), canvas(cnv)
{
    editor = std::make_unique<CodeEditor>();
    setUsingNativeTitleBar (true);
    setCentrePosition(400, 400);
    setSize(900, 700);
    editor->setSize(900, 700);
    setVisible (true);
    setResizable(true, false);
    setContentOwned (editor.get(), true);
    
    
    
    
    editor->refreshBox = [this](File& path) {
        StringArray tokens;
        String newname = path.getFileNameWithoutExtension();
        tokens.addTokens(box->name, " ", "\"");
        tokens.set(0, newname);
        setName(newname);
        
        ComponentDictionary::refresh();
        
        box->state.setProperty("Name", tokens.joinIntoString(" "), nullptr);
    };
    
    std::string path = Library::get(box->type.toStdString()).path;
    editor->openFile(path);
}

void CodeEditorWindow::closeButtonPressed()
{
    if(editor->editor->changedSinceSave()) {
        if(editor->askToSave())
            canvas->closeCode();
    }
    else {
        canvas->closeCode();
    }

}

void CodeEditorWindow::resized() {
    
    editor->setBounds(getLocalBounds());
    ResizableWindow::resized();
}

