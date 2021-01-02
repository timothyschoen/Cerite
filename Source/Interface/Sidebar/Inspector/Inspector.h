#pragma once

#include <JuceHeader.h>
#include "ParameterComponent.h"
#include "PresetComponent.h"

class Inspector : public Component, public ValueTree::Listener
{
public:
    
    //PresetManager pmanager;
    
    PresetComponent pcomp;
    
    Component parameterView;
    Viewport parameterViewport;

	std::vector<int> isArgument;

    OwnedArray<CollapseablePanel> panels;

	ValueTree parameters;
	ValueTree currentBox;
	// this only works on heap for some reason...
	Array<String> titles = {"Arguments", "Advanced Parameters"};

    Array<String> keys;
    Array<String> values;

    Array<Colour> colours = {Colour(42, 42, 42), Colour(50, 50, 50)};
	Array<String> restrictions = {"", "-e0123456789", "-e01234567890.", "01"};

	Box* lastViewed = nullptr;

	Inspector(Canvas* cnv);


	Canvas* canvas;

	void select(ValueTree boxTree);
    
    void deselect();
    
    void update();
    
    void resized() override;

private:

	void paint(Graphics & g) override;
    
    void refreshParameters();

    void valueTreePropertyChanged (ValueTree &treeWhosePropertyHasChanged, const Identifier &property) override;
};



