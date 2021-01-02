/*
  ==============================================================================

    Utilities.h
    Created: 2 Oct 2017 12:14:45pm
    Author:  David Rowland

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "Box.h"
#include "../../Utility/ValueTreeObjectList.h"

class BoxManager   : public ValueTreeObjectList<Box>
{

	Canvas* cnv;

public:

	BoxManager(ValueTree boxTree, Canvas* canvas) : ValueTreeObjectList(boxTree)
	{
		cnv = canvas;
	}

	~BoxManager()
	{
		freeObjects();
	};

	bool isSuitableType (const juce::ValueTree& tree) const
	{
		return tree.getType().toString().contains("Box");
	}

	Box* createNewObject (const juce::ValueTree& tree)
	{
		return new Box(cnv, tree);
	}
	void deleteObject (Box* box)
	{
		delete box;
	}

	void newObjectAdded (Box* box)
	{
		box->addToCanvas();
	}
	void objectRemoved (Box* box)
	{
		box->removeFromCanvas();
	}
	void objectOrderChanged()
	{
	};

};
