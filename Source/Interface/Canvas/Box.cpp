#include "Box.h"
#include "../../MainComponent.h"
#include "Connection.h"
#include "CodeEditor.h"
#include "../ComponentDictionary.h"

Box::Box(Canvas* parent, ValueTree node) : CanvasComponent(node, this)
{
	canvas = parent;
	
	parameterNode = state.getOrCreateChildWithName(Identifier("Parameters"), &canvas->undoManager);
	edgeNode = state.getOrCreateChildWithName(Identifier("Edges"), &canvas->undoManager);

    setColour(PopupMenu::backgroundColourId, Colours::darkgrey.darker());
    popupmenu = std::make_unique<PopupMenu>();
    
    info = ComponentDictionary::undefined;
	if(node.hasProperty("Ground"))
		isGround = bool(node.getProperty("Ground"));

	setOpaque(true);
	edgeManager = std::make_unique<boxEdgeManager>(edgeNode, canvas, this);
	edgeManager->rebuildObjects();
	setSize(85, 23);
	setConnectedEdges(12); // No rounded corners
	update();
    
    
}

Box::~Box()
{
	canvas->repaint();
	setLookAndFeel(nullptr);
}

void Box::resized()
{
	if(GraphicalComponent != nullptr)
	{
		GraphicalComponent->setTopLeftPosition(getX(), getY() + getHeight());
		setSize(GraphicalComponent->getWidth(), getHeight());
	}

	for(auto edge : edgeManager->objects)
		edge->componentMovedOrResized(*this, true, true);
    
}

void  Box::mouseDown (const MouseEvent& e)
{
	int clicks = e.getNumberOfClicks();

	if(ModifierKeys::getCurrentModifiers().isRightButtonDown())
	{
        popupmenu->clear();
        popupmenu->addItem(1, "Edit", true);
        popupmenu->addSeparator();
        popupmenu->addItem(3,"Cut", true);
        popupmenu->addItem(4,"Copy", true);
        popupmenu->addItem(5,"Duplicate", true);
        popupmenu->addItem(6,"Delete", true);
        popupmenu->setLookAndFeel(&getLookAndFeel());
        
        int result = popupmenu->show();
        
        if(result == 1)
            canvas->openCode(this);
        else if(result == 3) {
            Array<Box*> boxarray = canvas->getSelectedBoxes();
            Array<LooseEdge*> letarray = canvas->getSelectedEdges();

            for(int i = 0; i < boxarray.size(); i++)
                canvas->boxNode.removeChild(boxarray[i]->state, &canvas->undoManager);

            for(int i = 0; i < letarray.size(); i++)
            {
                letarray[i]->clearConnections();
                canvas->edgeNode.removeChild(letarray[i]->state, &canvas->undoManager);
            }
        }
        
        else if(result == 4) {
            copyTextToClipboard(canvas->getState(canvas->getSelectedBoxes(), canvas->getSelectedEdges()));
        }
        else if(result == 5) {
            canvas->addState(canvas->getState(canvas->getSelectedBoxes(), canvas->getSelectedEdges()));
        }
        else if(result == 6) {
            canvas->removeBox(this);
        }
		return;
	}

	if (clicks > 1 && ModifierKeys::getCurrentModifiers().isLeftButtonDown())
		doubleClicked();

	canvas->updateSelection();
	canvas->handleMouseDown(this, e);
}
void  Box::mouseDrag (const MouseEvent& e)
{
	
	canvas->handleMouseDrag(e);
}

void Box::mouseUp (const MouseEvent& e)
{
	
	canvas->updateSelection();
	canvas->handleMouseUp(this, e);
}

void  Box::doubleClicked (bool ShouldSelect)
{
	if(GraphicalComponent != nullptr)
		GraphicalComponent->setVisible(false);

	
	canvas->namesbox.createCalloutBox(this);
}

void Box::changeType()
{
    String previousType = type;
    name = state.getProperty("Name").toString();
	StringArray tokens;
	tokens.addTokens(name, " ", "\"");
	
	isGround = tokens[0].contains("ground");
	state.setProperty("Ground", isGround, &canvas->undoManager);
	info = ComponentDictionary::getInfo(name);
    
	std::vector<String> arguments;

	for(int t = 1; t < tokens.size(); t++)
	{
		String argString = tokens[t];

		if(argString.substring(0, argString.length() - 2).containsOnly(".0123456789") && argString.getLastCharacters(1).containsOnly("pnumkM"))
		{
			argString = argString.replace("p", "e-12").replace("n", "e-9").replace("u", "e-6").replace("m", "e-3").replace("k", "e3").replace("M", "e6");
		}
		else if(FSManager::home.getChildFile("Media").getChildFile(argString).exists())
			argString = FSManager::home.getChildFile("Media").getChildFile(argString).getFullPathName();

		arguments.push_back(argString);
	}
    
    type = tokens[0];
    
    if(previousType.containsNonWhitespaceChars() && type != previousType)
        parameterNode.removeAllChildren(&canvas->undoManager);
        
    if(arguments.size() >= info.numArguments )
		ComponentDictionary::getParameters(tokens[0], parameterNode, arguments);

    info.fillEdgeTree(edgeNode, &canvas->undoManager);

    GraphicalComponent.reset(ComponentDictionary::getComponent(state, this));

	if(GraphicalComponent)
	{
		canvas->addAndMakeVisible(GraphicalComponent.get());
		GraphicalComponent->addMouseListener(&canvas->mouseListener, true);
		GraphicalComponent->setBounds(getX(), getY() + getHeight(), GraphicalComponent->getBestSize().x, GraphicalComponent->getBestSize().y);
		setSize(GraphicalComponent->getWidth(), getHeight());
	}
    
	changeWidthToFitText();
	resized();
}

void Box::clearEdges()
{
	
	edgeNode.removeAllChildren(&canvas->undoManager);
};

Array<Edge*> Box::getEdges()
{
	return edgeManager->objects;
}


int Box::shouldSnap()
{
	Array<Edge*> concat = getEdges();
	bool snappingX = false;
	bool snappingY = false;

	for(int i = 0; i < concat.size(); i++)
	{
		for(int j = 0; j < concat[i]->connections.size(); j++)
		{
			Connection* conn = concat[i]->connections[j];
			conn->snapX = fabs(conn->start->getBounds().getCentre().x - conn->end->getBounds().getCentre().x) == 0;
			conn->snapY = fabs(conn->start->getBounds().getCentre().y - conn->end->getBounds().getCentre().y) == 0;
			snappingX |= conn->snapX;
			snappingY |= conn->snapY;
		}
	}

	return snappingX + 2 * snappingY;
}

void Box::addToCanvas()
{
	
	canvas->addAndMakeVisible(this);
	addMouseListener(&canvas->mouseListener, true);
    
	if(state.hasProperty("Name")) state.sendPropertyChangeMessage("Name");

	if(state.hasProperty("X") && state.hasProperty("Y"))
		setTopLeftPosition(int(state.getProperty("X")), int(state.getProperty("Y")));

	if(GraphicalComponent != nullptr)
	{
		canvas->addAndMakeVisible(GraphicalComponent.get());
        GraphicalComponent->setBounds(getX(), getY() + getHeight(), GraphicalComponent->getBestSize().x, GraphicalComponent->getBestSize().y);
	}

	canvas->repaint();
	resized();

};


void Box::removeFromCanvas()
{
	clearEdges();
	canvas->selectedComponents.deselect(this);
	state.setProperty("X", getX(), &canvas->undoManager);
	state.setProperty("Y", getY(), &canvas->undoManager);
	canvas->removeChildComponent(this);

	if(GraphicalComponent != nullptr)
		canvas->removeChildComponent(GraphicalComponent.get());

	canvas->repaint();
};


void Box::stateChanged(ValueTree &treeWhosePropertyHasChanged, const Identifier &property, const var &value)
{
	if(property == Identifier("Name"))
	{
		if(value.toString().isNotEmpty())
		{
			setButtonText(value.toString());
			changeType();
		}
		else
		{
			setSize(80, 23);
			doubleClicked();
		}
	}
	else if(property == Identifier("Orientation"))
	{
		bool state = bool(value);

		if(getButtonText().isEmpty()) stateChanged(treeWhosePropertyHasChanged, Identifier("Name"), treeWhosePropertyHasChanged.getProperty("Name"));

		if(state == true)
		{
            int height = std::max(info.getNumInputs(), info.getNumOutputs()) - 1;
            setSize(getWidth(), 23 + (height * 15));
		}
		else
		{
			setSize(getWidth(), 23);
		}

		if(GraphicalComponent == nullptr)
			changeWidthToFitText();

		resized();

		for(auto edge : getEdges())
		{
			edge->componentMovedOrResized(*this, true, true);

			for(auto con : edge->connections)
			{
				con->resized();
			}
		}
	}
}
