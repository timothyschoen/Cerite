#pragma once

#include <JuceHeader.h>

#include "Edge.h"
#include "BoxEdgeManager.h"

#include "../componentInformation.h"
#include "../Components/GUIContainer.h"

// This class represents a single Box
// When double clicked, it will open a text editor on top of it so you can change the type

class Box : public CanvasComponent, public TextButton, public SystemClipboard
{
public:

	Box(Canvas* parent, ValueTree node);

	~Box();
    
    int numIn = 0;
    int numOut = 0;
    componentInformation info;
    
    //t_pd* pdobject = nullptr;
    
    Array<String> intypes = {"pot", "varres", "dac"};
    Array<String> outtypes = {"adc"};
    
	ValueTree parameterNode;
	ValueTree edgeNode;

	std::unique_ptr<boxEdgeManager> edgeManager;

	std::unique_ptr<GUIContainer> GraphicalComponent = nullptr;

	// If a component is ground, we need to know!
	bool isGround = false;

	// Information about the object
	String name;
	String fullName;
    String type;

    std::unique_ptr<PopupMenu> popupmenu;
	std::vector<String> arguments;
	std::vector<std::pair<int, int>> nodes;
	std::vector<std::vector<int> > digiNodes;

	void resized() override;

	void setSelected (bool selected);
	void doubleClicked (bool ShouldSelect = true);

	// Functions for the name suggestor box
	void updateText();
	void clearEdges();
	void changeType();
	Array<Edge*> getEdges();

	void addToCanvas() override;
	void removeFromCanvas() override;

private:

	// For calls to parent component
    Canvas* canvas;

	void mouseDown (const MouseEvent& e) override;
	void mouseDrag (const MouseEvent& e) override;
	void mouseUp (const MouseEvent& e) override;

	int shouldSnap() override;

	void stateChanged(ValueTree &treeWhosePropertyHasChanged, const Identifier &property, const var &value) override;

};
