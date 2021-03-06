#include "Canvas.h"
#include "../../MainComponent.h"
#include "../Sidebar/Inspector/Inspector.h"


Canvas::Canvas(MainComponent* parent) : ProgramState(this), tipWindow(this, 200)
{
	setBounds (0, 40, getParentWidth(), getParentHeight());
	setWantsKeyboardFocus (true);
	addAndMakeVisible(lasso);
	lasso.setAlwaysOnTop(true);
	lasso.setColour(LassoComponent<TextButton>::lassoFillColourId, findColour(ScrollBar::ColourIds::thumbColourId).withAlpha((float)0.3));
	setOpaque(true);
	addAndMakeVisible(namesbox);
}

void Canvas::paint (Graphics& g)
{
	g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));

	if(connectingEdge != nullptr)
	{
        
		g.setColour(Colours::lightgrey);
		g.drawLine(Line<float>(lastMousePosition, connectingEdge->getBounds().getCentre().toFloat() ), 2.0f);
	}
}

void Canvas::mouseDown(const MouseEvent& e)
{
	namesbox.deleteCalloutBox(this);

	if(connectingEdge != nullptr) stopConnecting();

    if(!ModifierKeys::getCurrentModifiers().isAnyModifierKeyDown()) {
		deselectAll();
        MainComponent::getInstance()->sidebar.inspector->deselect();
    }

	lasso.beginLasso(e, this);
}

void Canvas::mouseDrag(const MouseEvent& e)
{
	lasso.dragLasso(e);
}

void Canvas::mouseUp(const MouseEvent& e)
{
	lasso.endLasso();
	updateSelection();
}


// Since negative positions in a viewport are difficult to handle when things are dragged out of bounds,
// i just move everything in the opposite direction which has the same effect
void Canvas::moveAll(int moveX, int moveY)
{
	for(auto element : boxmanager->objects)
	{
		element->setTopLeftPosition(element->getX() - moveX, element->getY() - moveY);
		element->resized();
	}

	for(auto element : edgemanager->objects)
	{
		element->setTopLeftPosition(element->getX() - moveX, element->getY() - moveY);
		element->resized();
	}

	setSize(getWidth() - moveX, getHeight() - moveY);
}

void Canvas::mouseMove(const MouseEvent& e)
{
    if(connectingEdge) {
        lastMousePosition = e.getPosition().toFloat();
        repaint();
    }
}

bool Canvas::keyPressed(const KeyPress& key)
{
	KeyPressMappingSet* keycommands = MainComponent::getInstance()->commandManager.getKeyMappings();
    
    
	if(keycommands->keyPressed(key, this)) return true;

	return false;
}

void Canvas::updateSelection()
{
	MainComponent* m = MainComponent::getInstance();
	Array<Box*> boxarray = getSelectedBoxes();

	if(boxarray.size() == 1)
		m->sidebar.inspector->select(boxarray[0]->state);
};

void Canvas::update()
{
	auto targetArea = getAreaOfAllComponents();
	auto limit = getBounds();

	if (targetArea.getX() < 0)  moveAll(targetArea.getX(), 0);

	if (targetArea.getY() < 0)  moveAll(0, targetArea.getY());

	if (targetArea.getBottom() > limit.getBottom()) setSize(getWidth(), targetArea.getBottom());

	if (targetArea.getRight() > limit.getRight())   setSize(targetArea.getRight(), getHeight());

	resized();
	repaint();
}

void Canvas::startConnecting(Edge* init)
{
	connectingEdge = init;
	updateUndoState();
};

Connection* Canvas::finishConnecting(Edge* init)
{
	ValueTree contree = addConnection(init, connectingEdge);
	connectingEdge = nullptr;
	updateUndoState();
	repaint();
	return conmanager->getObject(contree);
};

void Canvas::stopConnecting()
{
	connectingEdge = nullptr;
	repaint();
	updateUndoState();
};

void Canvas::updateUndoState()
{
	MainComponent::getInstance()->updateUndoState();
}

void Canvas::setUndoState(bool setUndo, bool canUndo, bool setRedo, bool canRedo) {
    
    MainComponent::getInstance()->setUndoState(setUndo, canUndo, setRedo, canRedo);
    
}

void Canvas::updateSystemState()
{
	MainComponent::getInstance()->updateSystem();
}


void Canvas::reset()
{
	nodes.clear();
	update();
	undoManager.clearUndoHistory();
	
	changedSinceSave = false;
	MainComponent* m = MainComponent::getInstance();
    updateUndoState();
	m->stopAudio();
}


void Canvas::addToHistory(File openedfile)
{
	XmlElement* recent = FSManager::meta->getChildByName("recentlyOpened");

	// Remove any unwanted entries
	for(int i = 0; i < recent->getNumChildElements(); i++)
	{
		if(recent->getChildElement(i)->getStringAttribute("Path").equalsIgnoreCase(openedfile.getFullPathName()) || openedfile.getFileName().equalsIgnoreCase("autosave.clab")) recent->removeChildElement(recent->getChildElement(i), true);
	}

	XmlElement* newelement = new XmlElement(openedfile.getFileName());
	newelement->setAttribute("Path", openedfile.getFullPathName());
	recent->insertChildElement(newelement, 0);

	if(recent->getNumChildElements() >= 6)
	{
		for(int i = 6; i < recent->getNumChildElements(); i++)
			recent->removeChildElement(recent->getChildElement(i), true);
	}

	FSManager::meta->writeTo(FSManager::metaData);
	MainComponent::getInstance()->topmenu.menuItemsChanged();
};

void Canvas::openCode(Box* caller) {
    
    codeEditor.reset(new CodeEditorWindow(this, caller));
    
    MainComponent* m = MainComponent::getInstance();
    
    // Unregister app commands to get proper key commands on the text editor
    m->commandManager.clearCommands();
    m->commandManager.setFirstCommandTarget(nullptr);
    m->topmenu.menuItemsChanged();
    
}

void Canvas::closeCode() {
    
    codeEditor.reset(nullptr);
    
    MainComponent* m = MainComponent::getInstance();
    
    // reregister app commands
    m->commandManager.registerAllCommandsForTarget(&m->appcmds);
    m->commandManager.setFirstCommandTarget(&m->appcmds);
    m->topmenu.menuItemsChanged();
    
}
