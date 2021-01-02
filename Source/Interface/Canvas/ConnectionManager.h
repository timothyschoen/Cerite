/*
  ==============================================================================

    Utilities.h
    Created: 2 Oct 2017 12:14:45pm
    Author:  David Rowland

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "Connection.h"
#include "../../Utility/ValueTreeObjectList.h"

class Canvas;
class ConnManager   : public ValueTreeObjectList<Connection>
{

    Canvas* cnv;

public:

	ConnManager(ValueTree connTree, Canvas* canvas) : ValueTreeObjectList(connTree)
	{
		cnv = canvas;
	}

	~ConnManager()
	{
		freeObjects();
	};

	bool isSuitableType (const juce::ValueTree& tree) const
	{
		return tree.getType().toString().contains("Connection");
	}

	Connection* createNewObject (const juce::ValueTree& tree)
	{
         
        if(int(tree.getProperty("Type")) == 0)
            return new Connection(cnv, tree, "mna");
		else if(int(tree.getProperty("Type")) == 1)
			return new Connection(cnv, tree, "dsp");
        else if(int(tree.getProperty("Type")) == 2)
            return new Connection(cnv, tree, "data");
        
        return new Connection(cnv, tree, "mna");
	}

	void deleteObject (Connection* conn)
	{
		delete conn;
	}

	void newObjectAdded (Connection* conn)
	{
		conn->addToCanvas();
	}
	void objectRemoved (Connection* conn)
	{
		conn->removeFromCanvas();
	}
	void objectOrderChanged()
	{
	};

};
