#include "NameSuggestions.h"
#include "Canvas.h"
#include <JuceHeader.h>

void CompleterTextBox::onChange(String& ttext) {
    
    nameSuggestions* parent = static_cast<nameSuggestions*>(suggestor);
    
    // Find start of highlighted region
    // This is the start of the last auto-completion suggestion
    // This region will automatically be removed after this function because it's selected
    int start = getHighlightedRegion().getLength() > 0 ? getHighlightedRegion().getStart() : getText().length();
    
    // Reconstruct users typing
    String typedText = getText().substring(0, start) + ttext;
    
    // Update suggestions
    parent->updateData(ComponentDictionary::finishName(typedText));
    
    // Get length of user-typed text
    int textlen = getText().substring(0, start).length();

    // Retrieve best suggestion
    fullName = parent->getCurrentSelection();
    
    if(!ttext.containsNonWhitespaceChars() || (getText() + ttext).contains(" ") || fullName.isEmpty()) {
        isCompleting = false;
        return;
    }
    
    isCompleting = true;
    ttext = fullName.substring(textlen);
    
    highlightStart = typedText.length();
    highlightEnd = fullName.length();
    
    /*


    // In case of non-text characters such as backspace, newline etc.
    if(typedText.contains(" ")) {
        moveCaretToEnd();
        highlightStart = typedText.length();
        highlightEnd = typedText.length();
        return;
    }
    
    // Retrieve best suggestion
    fullName = parent->getCurrentSelection();
    
    if(fullName.isEmpty())  {
        moveCaretToEnd();
        highlightStart = typedText.length();
        highlightEnd = typedText.length();
        return;
    }
    
    // Add suggestion at the end of the typed text
    ttext = fullName.substring(textlen);
    
    // Unfortunately we cant directly highlight the suggestion from here
    // This happens in the onTextChange lambda
    //highlightStart = typedText.length();
    //highlightEnd = fullName.length();
    //oldText = typedText;
    //typedText = fullName;
     */
}

nameSuggestions::nameSuggestions()
{
	// Set up the button list that contains our suggestions
	buttonholder = std::make_unique<Component>();

	for (int i = 0; i < 20; i++)
	{
        SuggestionComponent* but = buttons.add(new SuggestionComponent);
		buttonholder->addAndMakeVisible(buttons[i]);
        
        // Colour pattern
        but->setColour(TextButton::buttonColourId, colours[i % 2]);
        but->onStateChange = [this, i, but]() mutable {
            if(but->getToggleState()) move(0, i);
        };
	}

    // select the first button
	buttons[0]->setToggleState(true, sendNotification);
    
    // Set up name input
    addChildComponent(nameInput);
    nameInput.addKeyListener(this);
    
    // Set up viewport
    port = std::make_unique<Viewport>();
	port->setScrollBarsShown(true, false);
    port->setViewedComponent(buttonholder.get(), false);
    addAndMakeVisible(port.get());
    
    setLookAndFeel(&buttonlook);
    setInterceptsMouseClicks(true, true);
	setAlwaysOnTop(true);
    setVisible(false);
	
}

nameSuggestions::~nameSuggestions()
{
	buttons.clear();
    setLookAndFeel(nullptr);
    //nameInput.removeKeyListener(this);
}

void nameSuggestions::createCalloutBox(Box* caller)
{
	nameInput.setVisible(true);
	setBounds(caller->getX(), caller->getY(), 200, 100);
    openedEditor = caller;
	resized();
    String txt = caller->getButtonText();
	nameInput.setText(txt);
    oldInput = txt;
    updateData(ComponentDictionary::finishName(txt));
	nameInput.selectAll();

	for (int i = 0; i < buttons.size(); i++)
	{
		TextButton* but = buttons[i];
		but->onClick = [this, i]() mutable
		{
			currentidx = i;
		};
	}

    buttons[0]->setToggleState(true, sendNotification);
	setVisible(true);
	repaint();
    nameInput.grabKeyboardFocus();
}

void nameSuggestions::deleteCalloutBox(Component* parent)
{
	nameInput.setVisible(false);
	Canvas* cnv = static_cast<Canvas*>(parent);

	if(isVisible() && openedEditor != nullptr)
	{
		if(openedEditor->state.getProperty("Name").toString().compare(nameInput.getText())) cnv->startNewAction();

		setVisible(false);

		openedEditor->state.setProperty(Identifier("Name"), nameInput.getText(), &cnv->undoManager);
		openedEditor->state.setProperty(Identifier("Ground"), nameInput.getText().containsWholeWord("ground"), &cnv->undoManager);

		//if(openedEditor->GraphicalComponent == nullptr)
		//	openedEditor->changeWidthToFitText();
		//else
		//	openedEditor->GraphicalComponent->setVisible(true);

		openedEditor = nullptr;
		repaint();
	}
}

void nameSuggestions::move(int offset, int setto)
{
    // Calculate new selected index
    if(setto == -1)
        currentidx += offset;
    else
        currentidx = setto;
    
    if(numOptions == 0) return;
    
    // Limit it to minimum of the number of buttons and the number of suggestions
    int numButtons = std::min(20, numOptions);
	currentidx = (currentidx + numButtons) % numButtons;
    
    String newtext = listData[currentidx];
    TextButton* but = buttons[currentidx];
    
    but->setToggleState(true, dontSendNotification);
    
    nameInput.setText(newtext);
    nameInput.highlightEnd = newtext.length();
    nameInput.setHighlightedRegion({nameInput.highlightStart, nameInput.highlightEnd});
    
    // Auto-scroll item into viewport bounds
    if(port->getViewPositionY() > but->getY()) {
        port->setViewPosition(0, but->getY());
    }
    else if (port->getViewPositionY() + port->getMaximumVisibleHeight() < but->getY() + but->getHeight()) {
        port->setViewPosition(0, but->getY() - (but->getHeight() * 4));
    }
    
   // nameInput.grabKeyboardFocus();
}

String nameSuggestions::getCurrentSelection()
{
    return listData[currentidx];
}


void nameSuggestions::updateData(Array<String> data)
{
	listData.clear();
	listData = data;

	for(int i = 0; i < std::min(buttons.size(), listData.size()); i++)
		buttons[i]->setText(listData[i]);

	for(int i = listData.size(); i < buttons.size(); i++)
		buttons[i]->setText("     ");

	numOptions = listData.size();
	resized();
    
}

void nameSuggestions::paintOverChildren (Graphics& g)
{
    for(int i = 0; i < numOptions; i++) {
   
    }
	if(openedEditor != nullptr)
	{
		g.setColour(bordercolor);
		g.drawRect(port->getBounds());
	}
}

void nameSuggestions::resized()
{
	if(openedEditor != nullptr)
	{
		setBounds(openedEditor->getX(), openedEditor->getY(), 200, 100 + openedEditor->getHeight());
		port->setBounds(0, openedEditor->getHeight(), getWidth(), std::min(5, numOptions) * 20);
		nameInput.setTopLeftPosition(0, 0);
		nameInput.setSize(std::max(openedEditor->getWidth(), 75), openedEditor->getHeight());
		buttonholder->setBounds(0, 0, getWidth(), std::min(numOptions, 20) * 20);

		for (int i = 0; i < buttons.size(); i++)
			buttons[i]->setBounds(0, i * 20, getWidth(), 20);
	}
}

bool nameSuggestions::keyPressed(const KeyPress &key, Component *originatingComponent)
{
    bool moveUp = key.isKeyCode(63233);
    bool moveDown = key.isKeyCode(63232);
	if(moveUp || moveDown)
	{
		move(moveUp ? 1 : -1);
		return true;
	}
	return false;
}
