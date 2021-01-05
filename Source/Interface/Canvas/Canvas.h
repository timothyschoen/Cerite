#pragma once

#include <JuceHeader.h>
#include <mutex>

#include "MultiDragger.h"
#include "ProgramState.h"
#include "NameSuggestions.h"
#include "CodeEditor.h"

#include "../Looks.h"


class MainComponent;
class Canvas : public Component, public ProgramState, public MultiComponentDragger
{
public:


	class cnvCompListener : public MouseListener
	{
	public:
		Canvas* cnv;

		cnvCompListener(Canvas* canvas)
		{
			cnv = canvas;
		};

		void mouseMove(const MouseEvent& e) override
		{
			if(cnv->connectingEdge != nullptr)
				cnv->repaint();
		}

	};

    std::unique_ptr<CodeEditorWindow> codeEditor;
    
	File projectFile;

	cnvCompListener mouseListener = cnvCompListener(this);
	nameSuggestions namesbox;

	Edge* connectingEdge = nullptr;

    TooltipWindow tipWindow;

	Canvas(MainComponent* parent);

	~Canvas()
	{
		setLookAndFeel(nullptr);
	};

	void mouseMove(const MouseEvent& e) override;
	void moveAll(int moveX, int moveY) override;

	void updateSelection();

    void setUndoState(bool setUndo, bool canUndo, bool setRedo, bool canRedo) override;
    
	void updateUndoState() override;
	void updateSystemState() override;
	void update() override;
	void reset() override;

	Point<int> getMousePosition()
	{
		return Desktop::getMousePosition() - getScreenPosition();
	};

	// Keeps track of which analog connections are directly connected to eachother
	// These connections will be part of a single node
	Array<Array<SafePointer<Connection>>> nodes;

	int assignNodes() override; // Assigns a node number to connections
    
	void followNode(Connection* connection, Array<SafePointer<Connection>>& node, Array<SafePointer<Connection>>& connections);

	void startConnecting(Edge* init); // Functions to initiate, finish or abort connecting
	Connection* finishConnecting(Edge* init);

	void stopConnecting();

	void addToHistory(File openedfile);
    
    void openCode(Box* caller);
    void closeCode();
private:

	LassoComponent<TextButton*> lasso;

	CeriteLookAndFeel tiplook;
	std::vector<int> tooltipContent = {0, 0, 0, 0}; // could be nicer??
	String tooltipString = " ";

	bool keyPressed(const KeyPress & key) override;

	void paint(Graphics& g) override;

	void mouseDown(const MouseEvent& e) override;
	void mouseDrag(const MouseEvent& e) override;
	void mouseUp(const MouseEvent& e) override;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Canvas)
};
