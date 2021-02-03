#pragma once

#include <JuceHeader.h>
#include "CanvasComponent.h"
#include "../Looks.h"
#include "../DomainInformation.h"

class Edge : public TextButton, public CanvasComponent, private ComponentListener
{
public:

	int position = 0; // for smart line curving decisions

	int index;
	int side;
    
    LookAndFeel_V4 rlook;
    
	Canvas* canvas;

	String name;

	ValueTree connectionNode;

	Array<Connection*> connections;

    DomainInformation type;
	bool dragging = false;
	bool isGround = false;

	int node = 0;

    Box* box = nullptr;

	Edge(Canvas* parent, Box* attached, ValueTree node, std::string domain);

	~Edge();

	void resized() override;

	void clearConnections();

    void addToCanvas() override;
    void removeFromCanvas() override;
    
    void mouseMove(const MouseEvent& e) override;

	void stateChanged(ValueTree &treeWhosePropertyHasChanged, const Identifier &property, const var &value) override;

	void componentMovedOrResized (Component &component, bool wasMoved, bool wasResized) override;

	String getTooltip() override;

};
