#include "Connection.h"
#include "Edge.h"
#include "Box.h"
#include "GUIObjects.h"
#include "MainComponent.h"


GUIComponent::GUIComponent(Box* parent)  : box(parent), nodes(parent->gui_nodes)
{
    
}


GUIComponent* GUIComponent::create_gui(String name, Box* parent)
{
    if(name == "bng") {
        return new BangComponent(parent);
    }
    if(name == "tgl") {
        return new ToggleComponent(parent);
    }
    if(name == "hsl") {
        return new SliderComponent(false, parent);
    }
    if(name == "vsl") {
        return new SliderComponent(true, parent);
    }
    if(name == "hradio") {
        return new RadioComponent(false, parent);
    }
    if(name == "vradio") {
        return new RadioComponent(true, parent);
    }
    if(name == "msg") {
        return new MessageComponent(parent);
    }
    if(name == "nbx") {
        return new NumboxComponent(parent);
    }
    
    return nullptr;
}

void GUIComponent::register_receive(int port, std::function<void(libcerite::Data)> receive_callback) {
    if(port < nodes.size()) {
        MainComponent::current_main->receive_data(nodes[port], receive_callback);
    }
}

void GUIComponent::send_data(int port, libcerite::Data data) {
    if(port < nodes.size()) {
        MainComponent::current_main->send_data(nodes[port], data);
    }
}

// BangComponent

BangComponent::BangComponent(Box* parent) : GUIComponent(parent)
{
    addAndMakeVisible(bang_button);
}


void BangComponent::register_object()  {
    
    
    bang_button.onClick = [this](){
       send_data(0, libcerite::Bang());
    };
    
    register_receive(1, [this](libcerite::Data data) {
        if(libcerite::is_bang(data)) {
            MessageManager::callAsync([this](){
                bang_button.setToggleState(true, dontSendNotification);
                startTimer(50);
            });
        }
    });
}


void BangComponent::resized() {
    bang_button.setBounds(getWidth() / 4, getHeight() / 4, getWidth() / 2, getHeight() / 2);
}


// ToggleComponent

ToggleComponent::ToggleComponent(Box* parent) : GUIComponent(parent)
{
    toggle_button.setClickingTogglesState(true);
    addAndMakeVisible(toggle_button);
}


void ToggleComponent::register_object()  {
    
    
    toggle_button.onClick = [this](){
       send_data(0, libcerite::Number((double)toggle_button.getToggleState()));
    };
    
    register_receive(1, [this](libcerite::Data data) {
        if(libcerite::is_bang(data)) {
            MessageManager::callAsync([this](){
                toggle_button.setToggleState(!toggle_button.getToggleState(), dontSendNotification);
            });
        }
        else if(libcerite::is_number(data)) {
            int num = (int)libcerite::get_number(data);
            MessageManager::callAsync([this, num]() mutable {
                toggle_button.setToggleState(num, dontSendNotification);
            });
        }
    });
}


void ToggleComponent::resized() {
    toggle_button.setBounds(getWidth() / 4, getHeight() / 4, getWidth() / 2, getHeight() / 2);
}



// MessageComponent


MessageComponent::MessageComponent(Box* parent) : GUIComponent(parent)
{
    
    bang_button.setConnectedEdges(12);
   
    addAndMakeVisible(input);
    addAndMakeVisible(bang_button);
}


void MessageComponent::register_object()  {
    
    bang_button.onClick = [this](){
        send_data(0, {libcerite::tBang});
    };
    
    input.onTextChange = [this](){
        String new_text = input.getText();
        if(new_text.containsOnly("0123456789.-e")) {
            send_data(0, {libcerite::tNumber, new_text.getDoubleValue()});
        }

        else if(new_text.contains(" ")) {
            // list
            StringArray tokens;
            tokens.addTokens(new_text, " ", "\"\'");
            auto data = new libcerite::Data[tokens.size()];
            for(int i = 0; i < tokens.size(); i++) {
                if(tokens[i].containsOnly("0123456789.-e")) {
                    data[i] = {libcerite::tNumber, tokens[i].getDoubleValue()};
                }
                else if(tokens[i] == "bang") {
                    data[i] = {libcerite::tBang, 0};
                }
                else {
                    auto cstr = new char[tokens[i].length() + 1];
                    strcpy(cstr, tokens[i].toRawUTF8());
                    data[i] = {libcerite::tString, 0, cstr};
                }
            }
            send_data(0, {libcerite::tList, 0, "", data, (uint32_t)tokens.size()});
            
        }
        else {
            auto cstr = new char[new_text.length() + 1];
            strcpy(cstr, new_text.toRawUTF8());
            send_data(0, {libcerite::tString, 0, cstr});
        }
    };

    
    register_receive(1, [this](libcerite::Data data) {
        String str = parseData(data);
        MessageManager::callAsync([this, str]() mutable {
            input.setText(str);
        });
    });
}


void MessageComponent::resized() {
    input.setBounds(0, 0, getWidth() - 28, getHeight());
    bang_button.setBounds(getWidth() - 29, 0, 29, getHeight());
}

String MessageComponent::parseData(libcerite::Data d) {
    if(d.type == libcerite::tNumber)
    {
        return String(d.number);
        
    }
    else if(d.type == libcerite::tString)
    {
        
        return String(d.string);
    }
    else if(d.type == libcerite::tBang)
    {
        return "bang";
    }
    else if(d.type == libcerite::tList)
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

// NumboxComponent



NumboxComponent::NumboxComponent(Box* parent) : GUIComponent(parent)
{
    input.addMouseListener(this, false);
    
    input.setInputRestrictions(0, ".-0123456789");
    input.setText("0.");
    addAndMakeVisible(input);
}


void NumboxComponent::register_object()  {
    
    input.onTextChange = [this]() {
        send_data(0, libcerite::Number(input.getText().getDoubleValue()));
    };
    
    
    register_receive(1, [this](libcerite::Data data) {
        if(libcerite::is_number(data)) {
            MessageManager::callAsync([this, data]() mutable {
                input.setText(String(libcerite::get_number(data)));
            });
        }
    });
}


void NumboxComponent::resized() {
    input.setBounds(getLocalBounds());
}





// SliderComponent



SliderComponent::SliderComponent(bool is_vertical, Box* parent) : GUIComponent(parent)
{
    v_slider = is_vertical;
    addAndMakeVisible(slider);
    
    if(is_vertical) slider.setSliderStyle(Slider::LinearVertical);
    
    slider.setRange(0., 1., 0.001);
    slider.setTextBoxStyle(Slider::NoTextBox, 0, 0, 0);
}


void SliderComponent::register_object()  {
    
    slider.onValueChange = [this]()
    {
        double val = slider.getValue();
        send_data(0, libcerite::Number(val));
        
    };
    
    register_receive(1, [this](libcerite::Data data) {
        if(libcerite::is_number(data)) {
            double val = libcerite::get_number(data);
            MessageManager::callAsync([this, val]() mutable {
                slider.setValue(val);
            });
        }
    });
}


void SliderComponent::resized() {
    slider.setBounds(getLocalBounds().reduced(v_slider ? 0.0 : 6.0, v_slider ? 6.0 : 0.0));
}




// RadioComponent

RadioComponent::RadioComponent(bool is_vertical, Box* parent) : GUIComponent(parent)
{
    v_radio = is_vertical;
    
    for(int i = 0; i < 8; i++) {
        radio_buttons[i].onClick = [this, i]() mutable {
            //setValueOriginal(i);
        };
        radio_buttons[i].setConnectedEdges(12);
        radio_buttons[i].setRadioGroupId(1001);
        radio_buttons[i].setClickingTogglesState(true);
        addAndMakeVisible(radio_buttons[i]);
    }
}


void RadioComponent::register_object()  {
    
    for(int i = 0; i < radio_buttons.size(); i++) {
        radio_buttons[i].onClick = [this, i]() mutable {
            send_data(0, libcerite::Number(i));
        };
    }
    
    register_receive(1, [this](libcerite::Data data) {
        if(libcerite::is_number(data)) {
            int num = std::clamp((int)libcerite::get_number(data), 0, 7);
            MessageManager::callAsync([this, num]() mutable {
                radio_buttons[num].setToggleState(true, sendNotification);
            });
        }
    });
}


void RadioComponent::resized() {
    for(int i = 0; i < 8; i++) {
        radio_buttons[i].setBounds(v_radio ? getWidth() / 2 - 11 : i*20, v_radio ? (i*20) - 1 : -1, 21, 21);
    }
}


