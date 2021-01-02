#include "MultiDragger.h"
#include "Canvas.h"


void MultiComponentDragger::setConstrainBoundsToParent(bool shouldResizeParent,
        BorderSize<int> amountPermittedOffscreen_)
{
	autoResizeParent = shouldResizeParent;
	amountPermittedOffscreen = amountPermittedOffscreen_;
}

/**
 If this flag is set then the dragging behaviour when shift
 is held down will be constrained to the vertical or horizontal
 direction.  This the the behaviour of Microsoft PowerPoint.
 */
void MultiComponentDragger::setShiftConstrainsDirection(bool constrainDirection)
{
	shiftConstrainsDirection = constrainDirection;
}

/**
 * Adds a specified component as being selected.
 */
void MultiComponentDragger::setSelected(TextButton* component, bool shouldNowBeSelected)
{
	/* Asserts here? This class is only designed to work for components that have a common parent. */
	jassert(selectedComponents.getNumSelected() == 0 || component->getParentComponent() == selectedComponents.getSelectedItem(0)->getParentComponent());
	bool isAlreadySelected = isSelected(component);
	component->setToggleState(shouldNowBeSelected, dontSendNotification);

	if (! isAlreadySelected && shouldNowBeSelected)
	{
		selectedComponents.addToSelection(component);
	}

	if (isAlreadySelected && ! shouldNowBeSelected)
		removeSelectedComponent(component);
}

/** Toggles the selected status of a particular component. */
void MultiComponentDragger::toggleSelection(TextButton* component)
{
	setSelected(component, ! isSelected(component));
}

/**
 You should call this when the user clicks on the background of the
 parent component.
 */
void MultiComponentDragger::deselectAll()
{
	for (auto c : selectedComponents)
		if (c)
			c->setToggleState(false, dontSendNotification);

	for (auto c : static_cast<Canvas*>(this)->conmanager->objects)
		if (c)
			c->setSelected(false);

	selectedComponents.deselectAll();
	static_cast<Canvas*>(this)->repaint();
}

/**
 Find out if a component is marked as selected.
 */
bool MultiComponentDragger::isSelected(TextButton* component) const
{
	return std::find(selectedComponents.begin(),
	                 selectedComponents.end(),
	                 component) != selectedComponents.end();
}

/**
 Call this from your components mouseDown event.
 */
void MultiComponentDragger::handleMouseDown (TextButton* component, const MouseEvent & e)
{
	jassert (component != nullptr);

	if (! isSelected(component))
	{
		if (! (e.mods.isShiftDown() || e.mods.isCommandDown()))
			deselectAll();

		setSelected(component, true);
		didJustSelect = true;
	}

	if (component != nullptr)
		mouseDownWithinTarget = e.getEventRelativeTo (component).getMouseDownPosition();

	componentBeingDragged = component;
	Component::beginDragAutoRepeat(100);
	totalDragDelta = {0, 0};
	constrainedDirection = noConstraint;
	component->repaint();
}

/**
 Call this from your components mouseUp event.
 */
void MultiComponentDragger::handleMouseUp (TextButton* component, const MouseEvent & e)
{
	Canvas* cnv = static_cast<Canvas*>(this);

	if(!didStartDragging && (e.mods.isShiftDown() || e.mods.isCommandDown()))
	{
		setSelected(component, false);
	}
	else if (didStartDragging)
		didStartDragging = false;

	didJustSelect = false;

	if(e.getDistanceFromDragStart() > 30) cnv->startNewAction(false);

	for (auto comp : selectedComponents)
	{
		CanvasComponent* cnvcomp = cnv->castCanvasComponent(comp);
		cnvcomp->state.setPropertyExcludingListener(static_cast<ValueTree::Listener*>(cnvcomp), "X", comp->getX(), &cnv->undoManager);
		cnvcomp->state.setProperty("Y", comp->getY(), &cnv->undoManager);
		comp->resized();
		comp->repaint();
	}

	if (autoResizeParent)
	{
		auto targetArea = getAreaOfAllComponents();
		auto limit = cnv->getBounds();

		if (targetArea.getX() < 0)
			moveAll(targetArea.getX(), 0);

		if (targetArea.getY() < 0)
			moveAll(0, targetArea.getY());

		if (targetArea.getBottom() > limit.getHeight())
			cnv->setSize(cnv->getWidth(), targetArea.getBottom());

		if (targetArea.getRight() > limit.getWidth())
			cnv->setSize(targetArea.getRight(), cnv->getHeight());
	}

	cnv->repaint();
}

/**
 Call this from your components mouseDrag event.
 */
void MultiComponentDragger::handleMouseDrag (const MouseEvent& e)
{
	Canvas* cnv = static_cast<Canvas*>(this);
	jassert (e.mods.isAnyMouseButtonDown()); // The event has to be a drag event!

	/** Ensure tiny movements don't start a drag. */
	if (!didStartDragging && e.getDistanceFromDragStart() < minimumMovementToStartDrag)
		return;

	didStartDragging = true;
	Point<int> delta = e.getEventRelativeTo (componentBeingDragged).getPosition() - mouseDownWithinTarget;

	if (snapToGrid && selectedComponents.getNumSelected() == 1)
	{
		CanvasComponent* snapComp = cnv->castCanvasComponent(componentBeingDragged);
		int shouldBeSnapping = snapComp->shouldSnap();

		if (!isSnapping && shouldBeSnapping)   // start snap
		{
			snapPosition = e.getPosition();
			isSnapping = true;
		}

		if (shouldBeSnapping == 0)   // don't snap
		{
			isSnapping = false;
		}

		if (isSnapping && snapPosition.getDistanceFrom(e.getPosition()) > 8)   // unsnap
		{
			isSnapping = false;
		}
		else if (shouldBeSnapping == 1)    // Snap X-axis
		{
			delta.x = 0;
			isSnapping = true;
		}
		else if (shouldBeSnapping == 2)   // Snap Y-axis
		{
			delta.y = 0;
			isSnapping = true;
		}
		else if (shouldBeSnapping == 3)   // Snap both axes
		{
			delta.x = 0;
			delta.y = 0;
			isSnapping = true;
		}
	}

	for (auto comp : selectedComponents)
	{
		if (comp != nullptr)
		{
			Rectangle<int> bounds (comp->getBounds());
			bounds += delta;
			comp->setBounds (bounds);
			comp->resized();
		}
	}

	totalDragDelta += delta;
}



Rectangle<int> MultiComponentDragger::getAreaOfSelectedComponents()
{
	if (selectedComponents.getNumSelected() == 0)
		return Rectangle<int>(0, 0, 0, 0);

	Rectangle<int> a = selectedComponents.getSelectedItem(0)->getBounds();

	for (auto comp : selectedComponents)
		if (comp)
			a = a.getUnion(comp->getBounds());

	return a;
}


void MultiComponentDragger::applyDirectionConstraints(const MouseEvent &e, Point<int> &delta)
{
	if (shiftConstrainsDirection && e.mods.isShiftDown())
	{
		/* xy > 0 == movement mainly X direction, xy < 0 == movement mainly Y direction. */
		int xy = abs(totalDragDelta.x + delta.x) - abs(totalDragDelta.y + delta.y);

		/* big movements remove the lock to a particular axis */

		if (xy > minimumMovementToStartDrag)
			constrainedDirection = xAxisOnly;

		if (xy < -minimumMovementToStartDrag)
			constrainedDirection = yAxisOnly;

		if ((xy > 0 && constrainedDirection != yAxisOnly) || (constrainedDirection == xAxisOnly))
		{
			delta.y = -totalDragDelta.y; /* move X direction only. */
			constrainedDirection = xAxisOnly;
		}
		else if ((xy <= 0 && constrainedDirection != xAxisOnly) || constrainedDirection == yAxisOnly)
		{
			delta.x = -totalDragDelta.x; /* move Y direction only. */
			constrainedDirection = yAxisOnly;
		}
		else
		{
			delta = {0, 0};
		}
	}
	else
	{
		constrainedDirection = noConstraint;
	}
}

void MultiComponentDragger::removeSelectedComponent(TextButton* component)
{
	selectedComponents.deselect(component);
}


SelectedItemSet<TextButton*>& MultiComponentDragger::getLassoSelection()
{
	return selectedComponents;
}


void MultiComponentDragger::findLassoItemsInArea (Array<TextButton*> & itemsFound, const Rectangle<int>& area)
{
	Canvas* cnv = static_cast<Canvas*>(this);

	for(auto element : cnv->boxmanager->objects)
	{
		if (area.intersects(element->getBounds()))
		{
			itemsFound.add(element);
			setSelected(element, true);
		}
		else if (!ModifierKeys::getCurrentModifiers().isAnyModifierKeyDown())
		{
			setSelected(element, false);
		}
	}

	for(auto element : cnv->edgemanager->objects)
	{
		if (area.intersects(element->getBounds()))
		{
			itemsFound.add (element);
			setSelected(element, true);
		}
		else if (!ModifierKeys::getCurrentModifiers().isAnyModifierKeyDown())
		{
			setSelected(element, false);
		}
	}

	for(auto element : cnv->conmanager->objects)
	{
		// not ideal but reliable
		int numPoints = element->path.getLength() / 10.;

		for(int i = 0; i < numPoints; i++)
		{
			Point<float> testpoint = element->path.getPointAlongPath(i * (element->path.getLength() / numPoints)) + element->getPosition().toFloat();

			if(area.contains(testpoint.toInt()))
			{
				element->setSelected(true);
				element->repaint();
				break;
			}
			else if (element->isSelected() && !ModifierKeys::getCurrentModifiers().isAnyModifierKeyDown())
			{
				element->setSelected(false);
				element->repaint();
			}
		}
	}
}

Array<Box*> MultiComponentDragger::getSelectedBoxes()
{
	Canvas* cnv = static_cast<Canvas*>(this);
	Array<Box*> selectedBoxes;

	for (int i = 0; i < selectedComponents.getNumSelected(); i++)
	{
		Box* boxitem = static_cast<Box*>(selectedComponents.getSelectedItem(i));

		if (cnv->boxmanager->objects.contains(boxitem)) selectedBoxes.add(boxitem);
	}

	return selectedBoxes;
}

Array<LooseEdge*> MultiComponentDragger::getSelectedEdges ()
{
	Canvas* cnv = static_cast<Canvas*>(this);
	Array<LooseEdge*> selectedEdges;

	for (int i = 0; i < selectedComponents.getNumSelected(); i++)
	{
		LooseEdge* edge = static_cast<LooseEdge*>(selectedComponents.getSelectedItem(i));

		if(cnv->edgemanager->objects.contains(edge)) selectedEdges.add(edge);
	}

	return selectedEdges;
}

Array<Connection*> MultiComponentDragger::getSelectedConns ()
{
	Array<Connection*> selectedConns;

	for (auto con : static_cast<Canvas*>(this)->conmanager->objects)
		if(con->isSelected())
			selectedConns.add(con);

	return selectedConns;
}

Rectangle<int> MultiComponentDragger::getAreaOfAllComponents()
{
	Canvas* cnv = static_cast<Canvas*>(this);

	if (cnv->boxNode.getNumChildren() + cnv->edgeNode.getNumChildren() == 0)
		return Rectangle<int>(0, 0, 0, 0);

	Rectangle<int> a;

	for(auto element : cnv->boxmanager->objects)
	{
		if(element)
		{
			a = a.getUnion(element->getBounds());

			//if(element->GraphicalComponent != nullptr)
			//	a = a.getUnion(element->GraphicalComponent->getBounds());
		}
	}

	for(auto element : cnv->edgemanager->objects)
	{
		if(element)
		{
			a = a.getUnion(element->getBounds());
		}
	}

	return a;
}
