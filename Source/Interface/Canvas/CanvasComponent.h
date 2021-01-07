#pragma once

#include <JuceHeader.h>


class Canvas;
class Connection;
class Box;
class Edge;
struct CanvasComponent : ValueTree::Listener
{
	CanvasComponent(ValueTree stateTree, Component* thisComponent)
	{
		object = thisComponent;
		jassert(stateTree.isValid());
		state = stateTree;
		state.addListener(this);
	}
	virtual ~CanvasComponent() {};

	ValueTree state;

	void update()
	{
        
		for(int i = 0; i < state.getNumProperties(); i++)
			state.sendPropertyChangeMessage(state.getPropertyName(i));

		object->resized();
		object->repaint();
	}

	virtual int shouldSnap()
	{
		return 0;
	};

	virtual void stateChanged(ValueTree &treeWhosePropertyHasChanged, const Identifier &property, const var &value) {};

	void valueTreePropertyChanged (ValueTree &treeWhosePropertyHasChanged, const Identifier &property) override
	{
		if((property == Identifier("Y") && treeWhosePropertyHasChanged.hasProperty("X")) || (property == Identifier("X") && treeWhosePropertyHasChanged.hasProperty("Y")))
		{
			object->setTopLeftPosition(int(treeWhosePropertyHasChanged.getProperty("X")), int(treeWhosePropertyHasChanged.getProperty("Y")));
			object->resized();
		}

		stateChanged(treeWhosePropertyHasChanged, property, treeWhosePropertyHasChanged.getProperty(property));
	}
    
    virtual void addToCanvas() = 0;
    virtual void removeFromCanvas() = 0;

protected:
	Component* object;
};
