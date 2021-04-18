#pragma once

#include <JuceHeader.h>
#include "Edge.h"
#include "../DomainInformation.h"


class ReconnectButton : public TextButton
{

public:

	bool showHoverEffect = false;

	ReconnectButton()
	{
		setInterceptsMouseClicks(false, false);
	}

};

class Canvas;
class Connection : public CanvasComponent, public Component, private ComponentListener, public TooltipClient
{
public:
    enum hoverStates {OFF, HIGHLIGHTED, HOVERED, SELECTED};

	Edge* start = nullptr; // Pointers to the start and end points of the connection
	Edge* end = nullptr;

	Point<float> pstart;

	Point<float> pend;

	std::vector<std::unique_ptr<ReconnectButton> > reconnectbuttons;

	bool snapX = false;
	bool snapY = false;

	bool deleting = false;

	int node = 0;   // Variable for assigning node numbers
	int hoverstate = 0;  // Hover/highlight state
	int oldstate = 0;

	Path path;      // Position of the connection

	Connection(Canvas* parent, ValueTree node, std::string type);
	~Connection();

    DomainInformation type;
	bool moved = true;

	void addToCanvas() override;
	void removeFromCanvas() override;

	bool hitTest(int x, int y) override;

    String getTooltip() override;

	void resized() override;

    virtual bool isSelected();
    virtual bool isHighlighted();

    virtual void setSelected(bool shouldBeSelected);
    virtual void setHovered(bool shouldBeHovered);
	virtual void setHighlighted(bool shouldBeHovered);

	virtual Edge* splitConnection(Point<int> position);

	virtual Colour getColour(int hoverstate);

	void showHandles(bool shouldShowHandles);

    Canvas* canvas;

private:

	Line<float> line;

	void paint(Graphics & g) override;

	void stateChanged(ValueTree &treeWhosePropertyHasChanged, const Identifier &property, const var &value) override;
	void componentMovedOrResized (Component &component, bool wasMoved, bool wasResized) override;

	void mouseEnter(const MouseEvent& e) override;
	void mouseExit(const MouseEvent& e) override;
    void mouseDown(const MouseEvent& e) override;
	void mouseMove(const MouseEvent& e) override;

};
