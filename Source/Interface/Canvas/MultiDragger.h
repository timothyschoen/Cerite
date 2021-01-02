/*
  ==============================================================================

    jcf_multi_selection.h
    Created: 25 Feb 2016 9:00:28am
    Author:  Jim Credland

  ==============================================================================
*/

#pragma once


#include <JuceHeader.h>
#include "Box.h"
#include "LooseEdge.h"
/*
 * MultiComponentDragger allows the user to select objects and drag them around the
 * screen.  Multiple objects can be selected and dragged at once.  The behaviour
 * is similar to Microsoft PowerPoint and probably lots of other applications.
 *
 * Holding down Command(Control) or Shift allows multiple selection.  Holding down
 * shift can optionally also constrain the objects movement to only the left or
 * right axis.
 *
 * The movement can be constrained to be within the bounds of the parent component.
 *
 * Objects directly attached to the desktop are not supported.
 *
 * Using: see handleMouseUp, handleMouseDown and handleMouseDrag
 *
 * You will probably also want to check isSelected() in your objects paint(Graphics &)
 * routine and ensure selected objects are highlighted.  Repaints are triggered
 * automatically if the selection status changes.
 *
 * @TODO: Add 'grid' support.
 */



class MultiComponentDragger : public LassoSource<TextButton*>
{
public:

	enum snapTypes {None, X, Y};

	virtual ~MultiComponentDragger() {}


	virtual void moveAll(int moveX, int moveY) {};

	void setConstrainBoundsToParent(bool shouldConstrainToParentSize,
	                                BorderSize<int> amountPermittedOffscreen_);

	/**
	 If this flag is set then the dragging behaviour when shift
	 is held down will be constrained to the vertical or horizontal
	 direction.  This the the behaviour of Microsoft PowerPoint.
	 */
	void setShiftConstrainsDirection(bool constrainDirection);

	/**
	 * Adds a specified component as being selected.
	 */
	void setSelected(TextButton * component, bool shouldNowBeSelected);

	/** Toggles the selected status of a particular component. */
	void toggleSelection(TextButton * component);

	/**
	 You should call this when the user clicks on the background of the
	 parent component.
	 */
	void deselectAll();

	/**
	 Find out if a component is marked as selected.
	 */
	bool isSelected(TextButton * component) const;

	/**
	 Call this from your components mouseDown event.
	 */
	void handleMouseDown (TextButton* component, const MouseEvent & e);

	/**
	 Call this from your components mouseUp event.
	 */
	void handleMouseUp (TextButton* component, const MouseEvent & e);
	/**
	 Call this from your components mouseDrag event.
	 */
	void handleMouseDrag (const MouseEvent& e);

	SelectedItemSet<TextButton*> selectedComponents;
	SelectedItemSet<Component*> selectedConnections;

	Array<Box*> getSelectedBoxes();
	Array<LooseEdge*> getSelectedEdges();
	Array<Connection*> getSelectedConns();

	bool didStartDragging {false};
	bool snapToGrid {true};

	Rectangle<int> getAreaOfAllComponents();
	Rectangle<int> getAreaOfSelectedComponents();


private:

	void applyDirectionConstraints(const MouseEvent &e, Point<int> &delta);


	void removeSelectedComponent(TextButton * component);
	enum
	{
		noConstraint,
		xAxisOnly,
		yAxisOnly
	} constrainedDirection;

	const int minimumMovementToStartDrag = 10;

	bool isSnapping {false};

	bool autoResizeParent {true};
	bool shiftConstrainsDirection {false};

	bool didJustSelect {false};

	Point<int> mouseDownWithinTarget;
	Point<int> totalDragDelta;
	Point<int> snapPosition;

	TextButton * componentBeingDragged { nullptr };

	BorderSize<int> amountPermittedOffscreen;

	SelectedItemSet<TextButton*>& getLassoSelection() override;

	void findLassoItemsInArea (Array<TextButton*>& itemsFound, const Rectangle<int>& area) override;
};

