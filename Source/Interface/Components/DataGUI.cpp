#include "DataGUI.h"

HSliderContainer::~HSliderContainer()
{
    slider.setLookAndFeel(nullptr);
    removeChildComponent(&slider);
}

HSliderContainer::HSliderContainer(ValueTree tree, Box* box) : boxTree(tree)
{
    addAndMakeVisible(slider);
    slider.setLookAndFeel(&clook);
    slider.setRange(0., 1., 0.001);
    slider.setTextBoxStyle(Slider::NoTextBox, 0, 0, 0);
    
    slider.onDragStart = [this]() {
        dragging = true;
        startTimerHz(30);
    };
    
    slider.onValueChange = [this]()
    {
        
        const float val = slider.getValue();
        /*
        if(gui.isLogScale())
        {
            setValueOriginal(exp(val * log(max / min)) * min);
        }
        else
        {
            setValueScaled(val);
        }*/
        
    };
    
    slider.onDragEnd = [this]() {
        stopTimer();
        dragging = false;
    };
}

void HSliderContainer::timerCallback() {
    if(dragging) {
        double newval = slider.getValue();
        if(newval != value) {
            send({tNumber, newval});
            value = newval;
        }
    }
    else {
        slider.setValue(value);
        stopTimer();
    }

    
    
}

void HSliderContainer::receive(Data d) {
    if(d.number != value) {
        value = d.number;
        startTimer(0);
    }
    freeData(d);
}

void HSliderContainer::resized()
{
    slider.setBounds(0, 0, 130, 30);
}

Point<int> HSliderContainer::getBestSize()
{
    return Point<int>(130, 30);
}


VSliderContainer::~VSliderContainer()
{
    
    slider.setLookAndFeel(nullptr);
    removeChildComponent(&slider);
}

VSliderContainer::VSliderContainer(ValueTree tree, Box* box) : boxTree(tree)
{
    addAndMakeVisible(slider);
    slider.setLookAndFeel(&clook);
    slider.setSliderStyle(Slider::LinearVertical);
    slider.setRange(0., 1., 0.001);
    slider.setTextBoxStyle(Slider::NoTextBox, 0, 0, 0);
    
    slider.onDragStart = [this]() {
        dragging = true;
        startTimerHz(30);
    };
    
    slider.onValueChange = [this]()
    {
        
        const float val = slider.getValue();
        /*
        if(gui.isLogScale())
        {
            setValueOriginal(exp(val * log(max / min)) * min);
        }
        else
        {
            setValueScaled(val);
        }*/
        
    };
    
    slider.onDragEnd = [this]() {
        stopTimer();
        dragging = false;
    };
}

void VSliderContainer::resized()
{
    slider.setBounds(10, 0, 30, 130);
}

Point<int> VSliderContainer::getBestSize()
{
    return Point<int>(50, 130);
}


void VSliderContainer::timerCallback() {
    if(dragging) {
        double newval = slider.getValue();
        if(newval != value) {
            send({tNumber, newval});
            value = newval;
        }
    }
    else {
        slider.setValue(value);
        stopTimer();
    }

    
    
}

void VSliderContainer::receive(Data d) {
    if(d.number != value) {
        value = d.number;
        startTimer(0);
    }
    freeData(d);
}

void BangButton::mouseDown (const MouseEvent &)  {
    onMouseDown();
}



BangContainer::~BangContainer()
{
    
    setLookAndFeel(nullptr);
    removeChildComponent(&bangbutton);
}

BangContainer::BangContainer(ValueTree tree, Box* box) : boxTree(tree)
{
    
    addAndMakeVisible(bangbutton);
    setLookAndFeel(&dlook);
    
    
    bangbutton.setColour (TextButton::buttonOnColourId, Colour(25, 25, 25));
    bangbutton.setColour (TextButton::buttonColourId, Colour(41, 41, 41));
    
    bangbutton.onMouseDown = [this]()
    {
        send({tBang, 0, "bang", 0, 0});
        //startEdition();
        //setValueOriginal(1);
        //stopEdition();
    };
    
    
}

void BangContainer::timerCallback() {
    if(bangbutton.getToggleState()) {
        bangbutton.setToggleState(false, dontSendNotification);
        stopTimer();
    }
    else {
        bangbutton.setToggleState(true, dontSendNotification);
        startTimer(100);
    }

    
}

void BangContainer::resized()
{
    bangbutton.setBounds(10, 10, 20, 20);
}

Point<int> BangContainer::getBestSize()
{
    return Point<int>(40, 40);
}

void BangContainer::receive(Data d) {
    startTimer(0);
    freeData(d);
}

void TglButton::mouseDown (const MouseEvent &)  {
    onMouseDown();
}


ToggleContainer::~ToggleContainer()
{
    
    setLookAndFeel(nullptr);
    removeChildComponent(&togglebutton);
}

ToggleContainer::ToggleContainer(ValueTree tree, Box* box) : boxTree(tree)
{
    addAndMakeVisible(togglebutton);
    setLookAndFeel(&dlook);
    
    togglebutton.setClickingTogglesState(true);
    togglebutton.setColour (TextButton::buttonOnColourId, Colour(25, 25, 25));
    togglebutton.setColour (TextButton::buttonColourId, Colour(41, 41, 41));
    togglebutton.onMouseDown = [this]()
    {
        send({tNumber, 1 - (double)togglebutton.getToggleState(), "", 0, 0});
        //startEdition();
        //setValueOriginal(1.f - getValueOriginal());
        //stopEdition();
    };
}

void ToggleContainer::resized()
{
    togglebutton.setBounds(10, 10, 20, 20);
}

Point<int> ToggleContainer::getBestSize()
{
    return Point<int>(40, 40);
}

void ToggleContainer::receive(Data d) {
    if(d.type == tNumber)
    {
        togglebutton.setToggleState(d.number != 1, dontSendNotification);
    }
    else if(d.type == tBang) {
        togglebutton.setToggleState(!togglebutton.getToggleState(), dontSendNotification);
    }
    freeData(d);
}


void NumComponent::mouseDown(const MouseEvent & e)  {
    oldval = getText().getFloatValue();
    //onDragStart();
    TextEditor::mouseDown(e);
}

void NumComponent::mouseUp(const MouseEvent & e)  {
    onDragEnd();
    TextEditor::mouseUp(e);
}


void NumComponent::focusGained(FocusChangeType type)  {
    //onDragEnd();
}

void NumComponent::mouseDrag(const MouseEvent & e)  {
    TextEditor::mouseDrag(e);
    int dist = -e.getDistanceFromDragStartY();
    if(abs(dist) > 2) {
        float newval = oldval + ((float)dist / 100.);
        setText(String(newval));
    }
    //onMouseDrag();
}


NumboxContainer::~NumboxContainer()
{
    
    input.setLookAndFeel(nullptr);
    removeChildComponent(&input);
}

NumboxContainer::NumboxContainer(ValueTree tree, Box* box) : boxTree(tree)
{
    addAndMakeVisible(input);
    input.setLookAndFeel(&clook);
    input.setInputRestrictions(0, ".-0123456789");
    
    input.setText("0.");
    
    input.onTextChange = [this]()
    {
        double newval = input.getText().getDoubleValue();
        if(newval != value) {
            send({tNumber, newval});
            value = newval;
        }
    };
    
    input.onDragEnd = [this]()
    {
        //stopTimer();
        dragging = false;
    };
    
    
    input.onFocusLost = [this]()
    {
        send({tNumber, input.getText().getDoubleValue()});
    };
}

void NumboxContainer::timerCallback() {
    
    if(dragging) {
        double newval = input.getText().getDoubleValue();
        if(newval != value) {
            send({tNumber, newval});
            value = newval;
        }
    }
    else {
        input.setText(String(value));
        stopTimer();
    }

}

void NumboxContainer::resized()
{
    input.setBounds(0, -1, 60, 23);
}

Point<int> NumboxContainer::getBestSize()
{
    return Point<int>(60, 22);
}

void NumboxContainer::receive(Data d) {
    if(d.number != value) {
        value = d.number;
        startTimer(0);
    }
    freeData(d);
}

MessageContainer::~MessageContainer()
{
    
    input.setLookAndFeel(nullptr);
    removeChildComponent(&input);
}

MessageContainer::MessageContainer(ValueTree tree, Box* box) : boxTree(tree)
{
    addAndMakeVisible(bangbutton);
    addAndMakeVisible(input);
    
    bangbutton.setConnectedEdges(12); // No rounded corners
    
    input.setLookAndFeel(&clook);
    bangbutton.setLookAndFeel(&clook);
    
    input.onTextChange = [this]()
    {
        String new_text = input.getText();
        
        if(new_text == "bang") {
            send({tBang});
        }
        else if(new_text.containsOnly("0123456789.-e")) {
            send({tNumber, new_text.getDoubleValue()});
        }

        else if(new_text.contains(" ")) {
            // list
            StringArray tokens;
            tokens.addTokens(new_text, " ", "\"\'");
            auto data = new Data[tokens.size()];
            for(int i = 0; i < tokens.size(); i++) {
                if(tokens[i].containsOnly("0123456789.-e")) {
                    data[i] = {tNumber, tokens[i].getDoubleValue()};
                }
                else if(tokens[i] == "bang") {
                    data[i] = {tBang, 0};
                }
                else {
                    auto cstr = new char[tokens[i].length() + 1];
                    strcpy(cstr, tokens[i].toRawUTF8());
                    data[i] = {tString, 0, cstr};
                }
            }
            send({tList, 0, "", data, (uint32_t)tokens.size()});
            
        }
        else {
            auto cstr = new char[new_text.length() + 1];
            strcpy(cstr, new_text.toRawUTF8());
            send({tString, 0, cstr});
        }
    };
    
    //bangbutton.setSliderStyle(SliderStyle::Vertical)
    bangbutton.onMouseDown = [this]()
    {
        input.onTextChange();
        send({tBang});
        //startEdition();
        //gui.setSymbol(input.getText().toStdString());
        //stopEdition();
    };
}


void MessageContainer::resized()
{
    input.setBounds(0, -1, 100, 23);
    bangbutton.setBounds(99, -1, 23, 23);
}

Point<int> MessageContainer::getBestSize()
{
    return Point<int>(122, 22);
}

void MessageContainer::updateValue()
{
    /*
    if(edited == false)
    {
        
        std::string const v = ""; //gui.getSymbol();
        if(v != last)
        {
            last = v;
            update();
        }
    } */
}

String MessageContainer::parseData(Data d) {
    if(d.type == tNumber)
    {
        return String(d.number);
        
    }
    else if(d.type == tString)
    {
        
        return String(d.string);
    }
    else if(d.type == tBang)
    {
        return "bang";
    }
    else if(d.type == tList)
    {
        String new_text;
        
        for(int i = 0; i < d.listlen; i++)
        {
            new_text += parseData(d.list[i]);
            if(i != d.listlen - 1) new_text += " ";
        }
        return new_text;
    }
    else {
        return "";
    }
    
    
}

void MessageContainer::receive(Data d) {
    String new_text = parseData(d);
    new_text.removeCharacters("\n\r");
    input.setText(new_text);
    
    freeData(d);
}


HRadioGroup::~HRadioGroup()
{
    
    for(int i = 0; i < 8; i++) {
        radiobuttons[i]->setLookAndFeel(nullptr);
        removeChildComponent(radiobuttons[i]);
    }
    
}

HRadioGroup::HRadioGroup(ValueTree tree, Box* box) : boxTree(tree)
{
    
    for(int i = 0; i < 8; i++) {
        TextButton* rbut = radiobuttons.add(new TextButton());
        rbut->onClick = [this, i]() mutable {
            //setValueOriginal(i);
        };
        rbut->setConnectedEdges(12); // No rounded corners
        rbut->setRadioGroupId(1001);
        rbut->setClickingTogglesState(true);
        addAndMakeVisible(rbut);
        rbut->setLookAndFeel(&clook);
        
    }
    
}

void HRadioGroup::resized()
{
    for(int i = 0; i < 8; i++) {
        radiobuttons[i]->setBounds(i*20, -1, 21, 21);
    }
    
}

Point<int> HRadioGroup::getBestSize()
{
    return Point<int>(161, 20);
}

void HRadioGroup::receive(Data d) {
    freeData(d);
}

VRadioGroup::~VRadioGroup()
{
    
    for(int i = 0; i < 5; i++) {
        radiobuttons[i]->setLookAndFeel(nullptr);
        removeChildComponent(radiobuttons[i]);
    }
    
}

VRadioGroup::VRadioGroup(ValueTree tree, Box* box) : boxTree(tree)
{
    
    for(int i = 0; i < 8; i++) {
        TextButton* rbut = radiobuttons.add(new TextButton());
        rbut->onClick = [this, i]() mutable {
           // setValueOriginal(i);
        };
        rbut->setConnectedEdges(12);
        rbut->setRadioGroupId(1001);
        rbut->setClickingTogglesState(true);
        addAndMakeVisible(rbut);
        rbut->setLookAndFeel(&clook);
    }
}

void VRadioGroup::resized()
{
    for(int i = 0; i < 8; i++) {
        radiobuttons[i]->setBounds(20, i*20, 20, 20);
    }
}

Point<int> VRadioGroup::getBestSize()
{
    return Point<int>(60, 160);
}

void VRadioGroup::receive(Data d) {
    freeData(d);
}
