/*
  ==============================================================================

    Utilities.h
    Created: 2 Oct 2017 12:14:45pm
    Author:  David Rowland

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

#include "../../Utility/ValueTreeObjectList.h"
#include "LooseEdge.h"

class Canvas;
class EdgeManager   : public ValueTreeObjectList<LooseEdge>
{

    Canvas* cnv;

public:

	EdgeManager(ValueTree edgeTree, Canvas* canvas) : ValueTreeObjectList(edgeTree)
	{
		cnv = canvas;
	}

	~EdgeManager()
	{
		freeObjects();
	};

	bool isSuitableType (const juce::ValueTree& tree) const
	{
		return tree.getType().toString().contains("let") || tree.getType().toString().contains("Edge");
	}

	LooseEdge* createNewObject (const juce::ValueTree& tree)
	{
		return new LooseEdge(cnv, tree);
	}
	void deleteObject (LooseEdge* edge)
	{
		delete edge;
	}

	void newObjectAdded (LooseEdge* edge)
	{
		edge->addToCanvas();
	}
	void objectRemoved (LooseEdge* edge)
	{
		edge->removeFromCanvas();
	}
	void objectOrderChanged()
	{
	};

};
