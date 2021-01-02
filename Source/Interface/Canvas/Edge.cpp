#include "Edge.h"
#include "../../MainComponent.h"
#include "Connection.h"
#include "../ComponentDictionary.h"

Edge::Edge(Canvas* parent, Box* attached, ValueTree node, std::string domain) : CanvasComponent(node, this), canvas(parent)
{
    type = ComponentDictionary::domainInformation[domain];
    state.setProperty("Type", type.position, &canvas->undoManager);
    setColour(buttonColourId, type.colour);
    
    setLookAndFeel(&rlook);

	if(!state.hasProperty("ID"))
		state.setProperty("ID", Uuid().toString(), &canvas->undoManager);

	box = attached;

	if(box != nullptr)
		isGround = static_cast<Box*>(box)->isGround;

	setSize(8, 8);
	onClick = [this]()
	{
		if(!ModifierKeys::getCurrentModifiers().isRightButtonDown())
		{
			if(canvas->connectingEdge == nullptr) canvas->startConnecting(this); // Check if we're already connecting, if not start connecting
			else canvas->finishConnecting(this); // Else, finish the connection that was being made
		}
	};
	setAlwaysOnTop(true);
	update();
}

Edge::~Edge()
{
    setLookAndFeel(nullptr);
}

void Edge::clearConnections()
{
	for (int i = connections.size(); i -- > 0; )
	{
		if(connections[i] != nullptr)
			canvas->removeConnection(connections[i]);
		else
			connections.remove(i);
	}

	connections.clear();
}

void Edge::addToCanvas()
{
	addMouseListener(&canvas->mouseListener, true);
	box->addComponentListener(this);

	//if(static_cast<Box*>(box)->GraphicalComponent != nullptr)
	//	static_cast<Box*>(box)->GraphicalComponent->addComponentListener(this);

	canvas->addAndMakeVisible(this);
	update();
	componentMovedOrResized(*box, true, true);
};


void Edge::removeFromCanvas()
{
	clearConnections();
	canvas->removeChildComponent(this);
	box->removeComponentListener(this);

	//if(static_cast<Box*>(box)->GraphicalComponent != nullptr)
	//	static_cast<Box*>(box)->GraphicalComponent->removeComponentListener(this);
};

void Edge::resized()
{
};


void Edge::stateChanged(ValueTree &treeWhosePropertyHasChanged, const Identifier &property, const var &value)
{
	if(property == Identifier("Position"))
		side = (value.toString() == "In");

	//if(property == Identifier("Type"))
	//	isSignal = value.toString() != "Analog";

	if(property == Identifier("Index"))
		index = int(value);
}


void Edge::componentMovedOrResized (Component &component, bool wasMoved, bool wasResized)
{
	Box* parent = static_cast<Box*>(box);
	Rectangle<int> parentbounds = parent->getBounds();
    
    int numEdges = side ? parent->info.getNumInputs() : parent->info.getNumOutputs();
    
	bool sideways = bool(parent->state.getProperty(Identifier("Orientation")));
	int newY = 0;
	int newX = 0;

	if(sideways)
	{
        newY = (side ? index : index - parent->info.getNumInputs()) * (parent->getHeight() - 24 / (numEdges - 1 + (numEdges == 1))) + 12 + parentbounds.getY();
		newX = side ? parentbounds.getTopLeft().x : parentbounds.getRight();
	}
	else
	{
		newY = side ? parentbounds.getTopLeft().y : parentbounds.getBottom();
        newX = (side ? index : index - parent->info.getNumInputs()) * ((parent->getWidth() - 24) / (numEdges - 1 + (numEdges == 1))) + 12 + parentbounds.getX();
	}

	if(!sideways && parent->GraphicalComponent != nullptr && !side)
		newY += parent->GraphicalComponent->getHeight();

	setCentrePosition(newX, newY);
}

String Edge::getTooltip()
{
	return state.getProperty("Description").toString();
}
