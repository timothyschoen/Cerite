#pragma once

#include <JuceHeader.h>
#include <libtcc.h>
#include "GUIContainer.h"
#include "../Canvas/Box.h"
#include "../../Engine/Types/Data.h"


class HSliderContainer : public GUIContainer, public Timer
{
    
public:
    
    double value;
    bool dragging = false;
    
    ValueTree boxTree;
    
    Slider slider;
    
    ~HSliderContainer();
    
    HSliderContainer(ValueTree boxTree, Box* box);
    
    void resized() override;
    
    Point<int> getBestSize() override;
    
    void receive(Data d) override;
    
    void timerCallback() override;
    
};


class VSliderContainer : public GUIContainer, public Timer
{
    
public:
    
    double value;
    bool dragging = false;
    
    ValueTree boxTree;
    
    Slider slider;
    
    ~VSliderContainer();
    
    VSliderContainer(ValueTree boxTree, Box* box);
    
    void resized() override;
    
    Point<int> getBestSize() override;
    
    void receive(Data d) override;
    
    void timerCallback() override;
   
};


struct BangButton : public TextButton
{
    LookAndFeel_V4 dlook;
    
    std::function<void()> onMouseDown;
    
    void mouseDown (const MouseEvent &) override;
    

};

class BangContainer : public GUIContainer, private Timer
{
    
public:
    
    
    ValueTree boxTree;
    
    LookAndFeel_V4 dlook;
    
    BangButton bangbutton;
    
    ~BangContainer();
    
    BangContainer(ValueTree boxTree, Box* box);
    
    void resized() override;
    
    Point<int> getBestSize() override;
    
    
    void timerCallback() override;
    
    void receive(Data d) override;

};


struct TglButton : public TextButton
{
    
    std::function<void()> onMouseDown;
    
    
    void mouseDown (const MouseEvent &) override;
    
    
    
    
};

class ToggleContainer : public GUIContainer
{
    
    LookAndFeel_V4 dlook;
    
public:
    
    ValueTree boxTree;
    
    TglButton togglebutton;
    
    ~ToggleContainer();
    
    ToggleContainer(ValueTree boxTree, Box* box);
    
    void resized() override;
    
    Point<int> getBestSize() override;
    
    void receive(Data d) override;
    
};

class NumComponent : public TextEditor
{
    
public:
    std::function<void()> onFocusGained;
    std::function<void()> onMouseDrag;
    
    std::function<void()> onDragStart;
    std::function<void()> onDragEnd;
    
private:
    float oldval;
    
    void mouseDown(const MouseEvent & e) override;
    
    void mouseUp(const MouseEvent & e) override;
    
    void focusGained(FocusChangeType type) override;
    
    void mouseDrag(const MouseEvent & e) override;
    
};

class NumboxContainer : public GUIContainer, private Timer
{
    
public:
    
    std::atomic<bool> dragging;
    std::atomic<double> value;
    
    ValueTree boxTree;
    
    NumComponent input;
    
    ~NumboxContainer();
    
    NumboxContainer(ValueTree boxTree, Box* box);
    
    
    void resized() override;
    
    Point<int> getBestSize() override;
    
    
    void timerCallback() override;
    
    void receive(Data d) override;
    
};


class MessageContainer : public GUIContainer
{
    
public:
    
    ValueTree boxTree;
    
    TextEditor input;
    BangButton bangbutton;
    
    std::string last;
    
    ~MessageContainer();
    
    MessageContainer(ValueTree boxTree, Box* box);
    
    
    void resized() override;
    
    Point<int> getBestSize() override;
    
    
    void updateValue();
    
    void receive(Data d) override;

};



class HRadioGroup : public GUIContainer
{
    
public:
    
    
    ValueTree boxTree;
    
    OwnedArray<TextButton> radiobuttons;
    
    ~HRadioGroup();
    
    HRadioGroup(ValueTree boxTree, Box* box);
    
    void resized() override;
    
    Point<int> getBestSize() override;
    
    void receive(Data d) override;

};


class VRadioGroup : public GUIContainer
{
    
public:
    
    
    ValueTree boxTree;
    
    OwnedArray<TextButton> radiobuttons;
    
    ~VRadioGroup();
    
    VRadioGroup(ValueTree boxTree, Box* box);
    
    void resized() override;
    
    Point<int> getBestSize() override;
    
    void receive(Data d) override;
    
};
