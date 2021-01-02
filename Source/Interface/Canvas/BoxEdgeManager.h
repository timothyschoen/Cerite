/*
  ==============================================================================

    Utilities.h
    Created: 2 Oct 2017 12:14:45pm
    Author:  David Rowland

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "Edge.h"
#include "../../Utility/ValueTreeObjectList.h"

class boxEdgeManager : public ValueTreeObjectList<Edge>
{

	Canvas* cnv;
	Box* box;

public:

	boxEdgeManager(ValueTree edgeTree, Canvas* canvas, Box* parent) : ValueTreeObjectList(edgeTree)
	{
		cnv = canvas;
		box = parent;
	}

	~boxEdgeManager()
	{
		freeObjects();
	};

	bool isSuitableType (const juce::ValueTree& tree) const
	{
		return tree.getType().toString().contains("let") || tree.getType().toString().contains("Edge");
	}

	Edge* createNewObject (const juce::ValueTree& tree)
	{
		if(int(tree.getProperty("Type")) == 0)
			return new Edge(cnv, box, tree, "mna");
		else if (int(tree.getProperty("Type")) == 1)
			return new Edge(cnv, box, tree, "dsp");
        else
            return new Edge(cnv, box, tree, "data");
	}
	void deleteObject (Edge* edge)
	{
		delete edge;
	}

	void newObjectAdded (Edge* edge)
	{
		edge->addToCanvas();
	}
	void objectRemoved (Edge* edge)
	{
		edge->removeFromCanvas();
	}
	void objectOrderChanged()
	{
	};

};
