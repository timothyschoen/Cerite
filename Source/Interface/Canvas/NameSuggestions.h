#pragma once


#include <JuceHeader.h>
#include "Box.h"


// A box that will make typing suggestions when making a new object
// I had to waste a lot of code on this, to make sure the behavior is exactly right...


class SuggestionComponent : public TextButton
{

    int type = -1;
    Array<Colour> colours = {findColour(ScrollBar::thumbColourId), Colour(105, 83, 145).brighter(), Colour(246, 200, 46)};
    
    Array<String> letters = {"A", "D", "S"};
    
public:
    
    SuggestionComponent() {
        setText("");
        setWantsKeyboardFocus(false);
        setConnectedEdges(12);
        setClickingTogglesState(true);
        setRadioGroupId(1001);
        setColour(TextButton::buttonOnColourId, findColour(ScrollBar::thumbColourId));
    }
    void setText(String name) {
        setButtonText(name);
        
        //if (ComponentDictionary::getInfo(getButtonText()).pdObject)
         //   type = name.contains("~") ? 2 : 1;
        //else
         //   type = 0;
        
        repaint();
    }
    
    void paint(Graphics & g) override {
        TextButton::paint(g);
        
        if(type == -1) return;
        
        g.setColour(colours[type].withAlpha(float(0.8)));
        Rectangle<int> iconbound = getLocalBounds().reduced(3);
        iconbound.setWidth(getHeight() - 6);
        iconbound.translate(3, 0);
        g.fillRect(iconbound);
        
        g.setColour(Colours::white);
        g.drawFittedText(letters[type], iconbound.reduced(1), Justification::centred, 1);
    }
    
};

class CompleterTextBox : public TextEditor, public TextEditor::InputFilter
{
public:
    
    String oldText = "";
    String fullName = "";
    
    int highlightStart = 0;
    int highlightEnd = 0;
    
    CeriteLookAndFeel clook;
    
    bool isCompleting = false;
    
    Component* suggestor;
    
    CompleterTextBox(Component* sugg) : suggestor(sugg) {
        setAlwaysOnTop(true);
        setMultiLine(false, false);
        setJustification(4);
        setInputFilter(this, false);
        
        onTextChange = [this]() {
            if(isCompleting)
                setHighlightedRegion({highlightStart, highlightEnd});
        };
        
        setLookAndFeel(&clook);
    
    };
    
    ~CompleterTextBox() {
        setLookAndFeel(nullptr);
    }
    
    
    void onChange(String& newInput);

    
    String filterNewText(TextEditor& e, const String& newInput) override {
        String mutableInput = newInput;
        onChange(mutableInput);
        return mutableInput;
    }
    
    
};

class nameSuggestions : public Component, public KeyListener
{

public:

	bool selecting = false;

	nameSuggestions();

	~nameSuggestions();

	void createCalloutBox(Box* caller);
	void deleteCalloutBox(Component* parent);

	void updateData(Array<String> data);
	void move(int step, int setto = -1);

    CompleterTextBox nameInput = CompleterTextBox(this);

	Box* openedEditor = nullptr;
    
    String getCurrentSelection();

private:

	bool running = false;
	int numOptions = 0;
	int currentidx = 0;

	std::unique_ptr<Viewport> port;
	std::unique_ptr<Component> buttonholder;
	OwnedArray<SuggestionComponent> buttons;

	String oldInput;
	Array<String> listData;

	Array<Colour> colours = {Colour(42, 42, 42), Colour(55, 55, 55)};

	Colour bordercolor = Colour(142, 152, 155);
	nameSuggestionsButtons buttonlook;

	void paintOverChildren (Graphics& g) override;
    
	bool keyPressed(const KeyPress &key, Component *originatingComponent) override;

	void resized () override;

};



