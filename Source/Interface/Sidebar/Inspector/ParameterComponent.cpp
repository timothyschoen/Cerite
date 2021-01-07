#include "ParameterComponent.h"
#include "Inspector.h"
#include "../Sidebar.h"

ParameterComponent::ParameterComponent(String n, ValueTree param, int idx, bool isAdvanced) : name(n), parameter(param)  {
    
    advanced = isAdvanced;
    background = idx % 2 ? Colour(42, 42, 42) : Colour(50, 50, 50);
    
    advancedSettings.setLookAndFeel(&rlook);
    
    if(advanced) treeSetup();
}

void ParameterComponent::openAdvancedSettings() {
    Sidebar* sbar = findParentComponentOfClass<Sidebar>();
    std::unique_ptr<DialogBase> paramdialog = std::make_unique<ParamEditDialog>(parameter);
    int action = sbar->createDialog(paramdialog.get(), this);
    
    Slider* s = dynamic_cast<Slider*>(editor.get());
    //if(s && action)
        //s->setNormalisableRange({double(parameter.getProperty("Minimum")), double(parameter.getProperty("Maximum")), type == ParameterType::tInt ? 1 : 1e-12, double(parameter.getProperty("Skew"))});
}


void ParameterComponent::treeSetup() {
    parameter.addListener(this);
    
    dynamic = bool(parameter.getProperty("Dynamic"));
    type = int(parameter.getProperty("Type"));
    
    if(type == ParameterType::tStr) {
        TextEditor* texteditor = new TextEditor();
        editor.reset(texteditor);
        texteditor->setText(parameter.getProperty("Value").toString());
    }
    else if(type == ParameterType::tInt) {
        Slider* slider = createSlider();
        
        slider->onValueChange = [this, slider]() {
            parameter.setProperty("Value", slider->getValue(), nullptr);
        };
        
        addAndMakeVisible(advancedSettings);
        
        advancedSettings.onClick = [this]() {
            openAdvancedSettings();
        };

        editor.reset(slider);
    }
    else if(type == ParameterType::tDouble) {
        Slider* slider = createSlider();
        
        slider->onValueChange = [this, slider]() {
            parameter.setProperty("Value", slider->getValue(), nullptr);
        };
        
        addAndMakeVisible(advancedSettings);
        
        advancedSettings.onClick = [this]() {
            openAdvancedSettings();
        };

        editor.reset(slider);
    }
    else if(type == ParameterType::tBool) {
        ToggleButton* toggle = new ToggleButton();
        editor.reset(toggle);
        toggle->setToggleState(bool(parameter.getProperty("Value")), sendNotification);

        toggle->onClick = [this, toggle]() {
            parameter.setProperty("Value", toggle->getToggleState(), nullptr);
        };

    }
    
    if(!dynamic)
        editor->setEnabled(!bool(parameter.getRoot().getProperty("Power")));
    else
        editor->setEnabled(true);
    
    addAndMakeVisible(editor.get());
     
}

void ParameterComponent::paint(Graphics & g) {
    g.setColour(background);
    g.fillAll();
    
    g.setColour(Colours::white);
    g.drawFittedText(name, 10, 0, getWidth() / 2., getHeight(), Justification::left, 1);
    
}

void ParameterComponent::timerCallback() {
            
    update();
    stopTimer();
}

void ParameterComponent::update(ValueTree param) {
    
    if(param.isValid()) {
        parameter = param;
        parameter.addListener(this);
        dynamic = param.getProperty("Dynamic");
    }
    
    
    if(type == ParameterType::tStr)
        static_cast<TextEditor*>(editor.get())->setText(parameter.getProperty("Value").toString());
    else if(type == ParameterType::tInt)
        static_cast<Slider*>(editor.get())->setValue(int(parameter.getProperty("Value")));
    else if(type == ParameterType::tDouble)
        static_cast<Slider*>(editor.get())->setValue(double(parameter.getProperty("Value")));
    else if(type == ParameterType::tBool)
        static_cast<ToggleButton*>(editor.get())->setToggleState(bool(parameter.getProperty("Value")), sendNotification);
    
    
    if(!dynamic)
        editor->setEnabled(!bool(parameter.getRoot().getProperty("Power")));
    else
        editor->setEnabled(true);

}

void ParameterComponent::resized() {
    
    advancedSettings.setBounds((getWidth() / 2.) - (getHeight() + 8), 2, getHeight() - 4, getHeight() - 4);
    editor->setBounds(getLocalBounds().removeFromLeft(getWidth() / 2.).withX((getWidth() / 2.) - 10));

};

void ParameterComponent::valueTreePropertyChanged (ValueTree &treeWhosePropertyHasChanged, const Identifier &property)
{
    if(property == Identifier("Value"))
       startTimer(100);
}

Slider* ParameterComponent::createSlider() {
    /*
    Slider* slider = new Slider();
    
    type = int(parameter.getProperty("Type"));
    
    slider->setTextBoxStyle(Slider::TextBoxRight, false, 60, 18);

    if(type == ParameterType::tDouble) {
        if(parameter.hasProperty("Skew")) slider->setNormalisableRange({double(parameter.getProperty("Minimum")), double(parameter.getProperty("Maximum")), 1e-12, double(parameter.getProperty("Skew"))});
        slider->textFromValueFunction = [this](double d) -> String {
            std::stringstream ss;
            ss << d;
            ss.precision(4);
            return String(ss.str());
        };
        
        slider->valueFromTextFunction = [this](String str) -> double {
            std::stringstream ss;
            ss << str;
            ss.precision(4);
            double outval;
            ss >> outval;
            return outval;
        };
    }
    else if (type == ParameterType::tInt) {
        if(parameter.hasProperty("Skew")) slider->setNormalisableRange({double(parameter.getProperty("Minimum")), double(parameter.getProperty("Maximum")), 1, double(parameter.getProperty("Skew"))});
        slider->textFromValueFunction = [this](double d) -> String {
            return String(int(d));
        };
        
        slider->valueFromTextFunction = [this](String str) -> double {
            std::stringstream ss;
            ss << str;
            ss.precision(4);
            int outval;
            ss >> outval;
            return outval;
        };
    }
    
    //slider->setEnabled(false);
    
    slider->setValue(double(parameter.getProperty("Value")));
    slider->setScrollWheelEnabled(false);
    slider->setColour(Slider::trackColourId, Colour(20, 20,20));
    slider->setColour(Slider::backgroundColourId, Colour(30, 30, 30));
     return slider;
     */
    return nullptr;
}


SimpleParameterComponent::SimpleParameterComponent(String n, ValueTree param, int idx, int t) : ParameterComponent(n, param, idx, false) {
    type = t;
    treeSetup();
}

void SimpleParameterComponent::treeSetup() {
    parameter.addListener(this);
    
    if(type == ParameterType::tStr) {
        TextEditor* texteditor = new TextEditor();
        editor.reset(texteditor);
        texteditor->setText(parameter.getProperty(name).toString());
    }
    else if(type == ParameterType::tInt) {
        Slider* slider = createSlider();
        slider->onValueChange = [this, slider]() {
            parameter.setProperty(name, slider->getValue(), nullptr);
        };
        editor.reset(slider);
    }
    else if(type == ParameterType::tDouble) {
        Slider* slider = createSlider();
        slider->onValueChange = [this, slider]() {
            parameter.setProperty(name, slider->getValue(), nullptr);
        };
        editor.reset(slider);
    }
    else if(type == ParameterType::tBool) {
        ToggleButton* toggle = new ToggleButton();
        editor.reset(toggle);
        toggle->setToggleState(bool(parameter.getProperty(name)), sendNotification);
        toggle->onClick = [this, toggle]() {
                parameter.setProperty(name, toggle->getToggleState(), nullptr);
        };
    }
    
    addAndMakeVisible(editor.get());
}

void SimpleParameterComponent::update(ValueTree param) {
    
    if(param.isValid()) {
        parameter = param;
        parameter.addListener(this);
    }
    
    if(type == ParameterType::tStr)
        static_cast<TextEditor*>(editor.get())->setText(parameter.getProperty(name).toString(), dontSendNotification);
    else if(type == ParameterType::tInt)
        static_cast<Slider*>(editor.get())->setValue(int(parameter.getProperty(name)), dontSendNotification);
    else if(type == ParameterType::tDouble)
        static_cast<Slider*>(editor.get())->setValue(double(parameter.getProperty(name)), dontSendNotification);
    else if(type == ParameterType::tBool)
        static_cast<ToggleButton*>(editor.get())->setToggleState(bool(parameter.getProperty(name)), dontSendNotification);
}

void SimpleParameterComponent::valueTreePropertyChanged (ValueTree &treeWhosePropertyHasChanged, const Identifier &property)
{
    
    if(property == Identifier(name))
       startTimer(100);
}



CollapseablePanel::CollapseablePanel(String name) : title(name) {
    // By default, apply no filter
    filter = [this](ValueTree) {return true; };
    
}

void CollapseablePanel::drawTriangle (Graphics& g, float x1, float y1, float x2, float y2, float x3, float y3, Colour fill, Colour outline)
{
    Path p;
    p.addTriangle (x1, y1, x2, y2, x3, y3);
    g.setColour (fill);
    g.fillPath (p);

    g.setColour (outline);
    g.strokePath (p, PathStrokeType (0.3f));
}

void CollapseablePanel::mouseDown(const MouseEvent& e)
{
   if(e.getMouseDownX() < 25.) {
       expanded = !expanded;
       findParentComponentOfClass<Inspector>()->resized();
   }
}

void CollapseablePanel::paint(Graphics & g) {
    g.setColour(Colour(35, 35, 35));
    g.fillRect(0, 0, getWidth(), 30);
    
    g.setColour(Colours::white);
    g.drawFittedText(title, 30, 0, getWidth(), 30, Justification::left, 1);
    
    Rectangle<int> arrowZone (8, 10, 12, 12);
    
    if(expanded) {
        drawTriangle(g, 11.f, 14.f, 14.f, 19.f, 17.f, 14.f, Colours::white, Colours::white);
    }
    else {
        
        drawTriangle(g, 12.f, 12.f, 12.f, 18.f, 17.f, 14.5f, Colours::white, Colours::white);
    
    }

}

void CollapseablePanel::resized() {
    for(int i = 0; i < items.size(); i++)
        items[i]->setBounds(0, i * itemheight + 30, getWidth(), itemheight);
    
    repaint();
}

void CollapseablePanel::setPower(bool state) {
    for(auto item : items) {
        if(!item->dynamic) {
            item->editor->setEnabled(!state);
        }
        else {
            item->editor->setEnabled(true);
        }
    }
}
