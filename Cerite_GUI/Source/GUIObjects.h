#pragma once

#define NO_CERITE_MACROS
#include "/Users/timschoen/Documents/Cerite/.exec/libcerite.h"

#include <type_traits>
#include <utility>
#include <JuceHeader.h>



class Box;
struct GUIComponent : public Component
{
    
    std::unique_ptr<ResizableBorderComponent> resizer;
    
    Box* box;
    std::vector<int>& nodes;
    
    Array<int> num_registered;
    
    GUIComponent(Box* parent);
    
    virtual ~GUIComponent();
    
    virtual std::pair<int, int> get_best_size() = 0;
    
    virtual ComponentBoundsConstrainer get_constrainer() = 0;
    
    virtual void register_object() = 0;
    
    void register_receive(int port, std::function<void(libcerite::Data)> receive_callback);
    
    void send_data(int port, libcerite::Data data);
    
    void paint(Graphics& g) {
        g.setColour(findColour(TextButton::buttonColourId));
        g.fillRect(getLocalBounds().reduced(1));
    }
    
    void paintOverChildren(Graphics& g) {
        g.setColour(findColour(ComboBox::outlineColourId));
        g.drawLine(0, 0, getWidth(), 0);
    }
    
    static GUIComponent* create_gui(String name, Box* parent);
    
};

struct BangComponent : public GUIComponent, public Timer
{
    
    TextButton bang_button;
    
    BangComponent(Box* parent);
    
    std::pair<int, int> get_best_size() override {return {24, 46}; };
    
    ComponentBoundsConstrainer get_constrainer()  override {
        
    };
    
    void timerCallback() override {
        bang_button.setToggleState(false, dontSendNotification);
    }
    
    void resized() override;
    
    void register_object() override;
};

struct ToggleComponent : public GUIComponent, public Timer
{
    
    TextButton toggle_button;
    
    ToggleComponent(Box* parent);
    
    std::pair<int, int> get_best_size() override {return {24, 46}; };
    
    ComponentBoundsConstrainer get_constrainer()  override {
        
    };
    
    void timerCallback() override {
        toggle_button.setToggleState(false, dontSendNotification);
    }
    
    void resized() override;
    
    void register_object() override;
};


struct MessageComponent : public GUIComponent, public Timer
{
    
    TextEditor input;
    TextButton bang_button;
    
    MessageComponent(Box* parent);
    
    std::pair<int, int> get_best_size() override {return {120, 28}; };
    
    ComponentBoundsConstrainer get_constrainer()  override {
        
    };
    
    String parseData(libcerite::Data d);
    
    void timerCallback() override {
       // bang_button.setToggleState(false, dontSendNotification);
    }
    
    void resized() override;
    
    void register_object() override;
};


struct NumboxComponent : public GUIComponent, public Timer
{

    TextEditor input;
    
    NumboxComponent(Box* parent);
    
    std::pair<int, int> get_best_size() override {return {70, 28}; };
    
    ComponentBoundsConstrainer get_constrainer()  override {
        
    };
    
    void mouseDrag(const MouseEvent & e) override {
        input.mouseDrag(e);
        int dist = -e.getDistanceFromDragStartY();
        if(abs(dist) > 2) {
            float newval = input.getText().getFloatValue() + ((float)dist / 100.);
            input.setText(String(newval));
        }
        //onMouseDrag();
    }
    
    void timerCallback() override {
       // bang_button.setToggleState(false, dontSendNotification);
    }
    
    void resized() override;
    
    void register_object() override;

};

struct SliderComponent : public GUIComponent, public Timer
{

    bool v_slider;
    
    Slider slider;
    
    SliderComponent(bool vertical, Box* parent);
    
    std::pair<int, int> get_best_size() override {
        if(v_slider) return {35, 130};
        
        return {130, 35}; };
    
    ComponentBoundsConstrainer get_constrainer()  override {
        
    };
    
    void timerCallback() override {
       // bang_button.setToggleState(false, dontSendNotification);
    }
    
    void resized() override;
    
    void register_object() override;

};


struct RadioComponent : public GUIComponent, public Timer
{
    
    bool v_radio;
    RadioComponent(bool vertical, Box* parent);
    
    std::array<TextButton, 8> radio_buttons;
    
    std::pair<int, int> get_best_size() override {
        if(v_radio) return {24, 164};
        
        return {161, 24};
    };
    
    ComponentBoundsConstrainer get_constrainer()  override {
        
    };
    
    void timerCallback() override {
       // bang_button.setToggleState(false, dontSendNotification);
    }
    
    void resized() override;
    
    void register_object() override;
    
    
    
    
};
