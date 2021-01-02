#include "../../MainComponent.h"
#include "LooseEdge.h"
#include "Connection.h"


LooseEdge::LooseEdge(Canvas* parent, ValueTree node) : Edge(parent, nullptr, node, "mna")
{
	setColour(buttonColourId, findColour(ScrollBar::thumbColourId));
	position = 3;
	setSize(9, 9);

	if(state.hasProperty("X") && state.hasProperty("Y"))
	{
		setTopLeftPosition(int(state.getProperty("X")), int(state.getProperty("Y")));
	}
}

// Function for being dragged
void LooseEdge::mouseDrag(const MouseEvent& e)
{
	
	canvas->handleMouseDrag(e);
}

void LooseEdge::mouseDown(const MouseEvent& e)
{
	

	if(ModifierKeys::getCurrentModifiers().isRightButtonDown())
	{
		clearConnections();
		canvas->edgeNode.removeChild(state, &canvas->undoManager);
	}
	else
	{
		canvas->handleMouseDown(this, e);
	}
}

void LooseEdge::mouseUp(const MouseEvent& e)
{
	
	canvas->handleMouseUp(this, e);

	if(e.getDistanceFromDragStart() < 2 && e.getLengthOfMousePress() < 500 && isShowing())
		onClick();

	resized();
}

void LooseEdge::addToCanvas()
{
	
	canvas->addAndMakeVisible(this);
	addMouseListener(&canvas->mouseListener, true);

	if(state.hasProperty("X") && state.hasProperty("Y"))
	{
		setTopLeftPosition(int(state.getProperty("X")), int(state.getProperty("Y")));
	}
};


void LooseEdge::removeFromCanvas()
{
	
	clearConnections();
	canvas->selectedComponents.deselect(this);
	state.setProperty("X", getX(), &canvas->undoManager);
	state.setProperty("Y", getY(), &canvas->undoManager);
	canvas->removeChildComponent(this);
	canvas->repaint();
};

int LooseEdge::shouldSnap()
{
	bool snappingX = false;
	bool snappingY = false;

	for(int j = 0; j < connections.size(); j++)
	{
		Connection* conn = connections[j];
		conn->snapX = fabs(conn->start->getBounds().getCentre().x - conn->end->getBounds().getCentre().x) == 0;
		conn->snapY = fabs(conn->start->getBounds().getCentre().y - conn->end->getBounds().getCentre().y) == 0;
		snappingX |= conn->snapX;
		snappingY |= conn->snapY;
	}

	return snappingX + snappingY * 2;
}
