#pragma once

#include <JuceHeader.h>
#include "GUIContainer.h"
#include "../Canvas/Box.h"
/*
class LEDContainer : public GUIContainer, public ValueTree::Listener//, private Timer
{

public:

	float brightness = 0.5;
	int systempos = 0;

	ValueTree boxTree;

	~LEDContainer()
	{
		//stopTimer();
	}

	LEDContainer(ValueTree boxtree, Box* box)
	{
		boxTree = boxtree;
		boxTree.addListener(this);
		//startTimerHz(20);
	}

	void paint(Graphics & g) override;

	void resized() override;

	Point<int> getBestSize() override;

	void timerCallback();

	void valueTreePropertyChanged(ValueTree &treeWhosePropertyHasChanged, const Identifier &property) override;

};
*/
