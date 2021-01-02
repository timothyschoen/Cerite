#pragma once

#include <JuceHeader.h>
#include <libtcc.h>
#include "GUIContainer.h"
#include "../Canvas/Box.h"
#include "../../Engine/Cerite/src/Types/Data.h"

struct DataProcessor : public ExternalProcessor
{
    
    Cerite::Object* destination;
    void(*target)(Data);
    
    
    std::atomic<bool> changed;
    double value;
    
    DataProcessor(std::string paramname, ValueTree tree) : ExternalProcessor(paramname, tree) {
        changed = false;
    };
    
    void init(Cerite::Object* obj) override {
        destination = obj;
        
        std::string functarget;
        
        
        //target = destination->getVariablePtr(param);
        target = (void(*)(Data))destination->getFunctionPtr(name + "_attach");
        
        //tcc_add_symbol(obj->state,  callback.c_str(), (void*)(void(*)(Data))[](Data d) {
        //    std::cout << "hey" << std::endl;
        //});
        
        target({tBang, 0, "bang", 0, 0});
        
    }
    
    void tick(int channel = 0) override {
        if(changed) {
            //*target
        }
    }
    // TODO: make it work with strings by making value a class of data!
    void setValue(double newval) {
        value = newval;
    }
    
};

class HSliderContainer : public GUIContainer
{
    
public:
    
    DataProcessor* processor;
    ValueTree boxTree;
    
    Slider slider;
    
    ~HSliderContainer();
    
    HSliderContainer(ValueTree boxTree, Box* box);
    
    void resized() override;
    
    Point<int> getBestSize() override;
    
    void update();
    
    ExternalProcessor* createProcessor(std::string paramname) override {
        processor = new DataProcessor(paramname, boxTree);
        return processor;
    }
    
    
};


class VSliderContainer : public GUIContainer
{
    
public:
    
    DataProcessor* processor;
    ValueTree boxTree;
    
    Slider slider;
    
    ~VSliderContainer();
    
    VSliderContainer(ValueTree boxTree, Box* box);
    
    void resized() override;
    
    Point<int> getBestSize() override;
    
    void update();
    
    ExternalProcessor* createProcessor(std::string paramname) override {
        processor = new DataProcessor(paramname, boxTree);
        return processor;
    }
};


struct BangButton : public TextButton
{
    LookAndFeel_V4 dlook;
    
    std::function<void()> onMouseDown;
    
    void mouseDown (const MouseEvent &) override;
    
    
};

class BangContainer : public GUIContainer
{
    
public:
    
    DataProcessor* processor;
    ValueTree boxTree;
    
    LookAndFeel_V4 dlook;
    
    BangButton bangbutton;
    
    ~BangContainer();
    
    BangContainer(ValueTree boxTree, Box* box);
    
    void resized() override;
    
    Point<int> getBestSize() override;
    
    void update();
    
    ExternalProcessor* createProcessor(std::string paramname) override {
        processor = new DataProcessor(paramname, boxTree);
        return processor;
    }
    
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
    DataProcessor* processor;
    ValueTree boxTree;
    
    TglButton togglebutton;
    
    ~ToggleContainer();
    
    ToggleContainer(ValueTree boxTree, Box* box);
    
    void resized() override;
    
    Point<int> getBestSize() override;
    
    void update();
    
    ExternalProcessor* createProcessor(std::string paramname) override {
        processor = new DataProcessor(paramname, boxTree);
        return processor;
    }
    
};

class NumComponent : public TextEditor
{
    
public:
    std::function<void()> onFocusGained;
    std::function<void()> onMouseDrag;
    
private:
    float oldval;
    
    void mouseDown(const MouseEvent & e) override;
    
    void focusGained(FocusChangeType type) override;
    
    void mouseDrag(const MouseEvent & e) override;
    
};

class NumboxContainer : public GUIContainer
{
    
public:
    DataProcessor* processor;
    ValueTree boxTree;
    
    NumComponent input;
    
    ~NumboxContainer();
    
    NumboxContainer(ValueTree boxTree, Box* box);
    
    
    void resized() override;
    
    Point<int> getBestSize() override;
    
    void update();
    
    ExternalProcessor* createProcessor(std::string paramname) override {
        processor = new DataProcessor(paramname, boxTree);
        return processor;
    }
    
};


class MessageContainer : public GUIContainer
{
    
public:
    DataProcessor* processor;
    ValueTree boxTree;
    
    TextEditor input;
    BangButton bangbutton;
    
    std::string last;
    
    ~MessageContainer();
    
    MessageContainer(ValueTree boxTree, Box* box);
    
    
    void resized() override;
    
    Point<int> getBestSize() override;
    
    void update();
    
    void updateValue();
    
    ExternalProcessor* createProcessor(std::string paramname) override {
        processor = new DataProcessor(paramname, boxTree);
        return processor;
    }
};



class HRadioGroup : public GUIContainer
{
    
public:
    
    DataProcessor* processor;
    ValueTree boxTree;
    
    OwnedArray<TextButton> radiobuttons;
    
    ~HRadioGroup();
    
    HRadioGroup(ValueTree boxTree, Box* box);
    
    void resized() override;
    
    Point<int> getBestSize() override;
    
    void update();
    
    ExternalProcessor* createProcessor(std::string paramname) override {
        processor = new DataProcessor(paramname, boxTree);
        return processor;
    }
};


class VRadioGroup : public GUIContainer
{
    
public:
    
    DataProcessor* processor;
    ValueTree boxTree;
    
    OwnedArray<TextButton> radiobuttons;
    
    ~VRadioGroup();
    
    VRadioGroup(ValueTree boxTree, Box* box);
    
    void resized() override;
    
    Point<int> getBestSize() override;
    
    void update();
    
    ExternalProcessor* createProcessor(std::string paramname) override {
        processor = new DataProcessor(paramname, boxTree);
        return processor;
    }
    
};
