#pragma once
#include <JuceHeader.h>

struct edgeInformation
{
	int type;
	int side;
	int index;
	String description;

	// Generate a ValueTree for this edge
	void addEdgeToTree(ValueTree parentTree, UndoManager* undoManager)
	{
		ValueTree edgeTree("Edge");
		edgeTree.setProperty(Identifier("Type"), type, nullptr);
		edgeTree.setProperty(Identifier("Position"), side ? "Out" : "In", nullptr);
		edgeTree.setProperty(Identifier("Index"), index, nullptr);
		edgeTree.setProperty(Identifier("Description"), description, nullptr);
		parentTree.addChild(edgeTree, index, undoManager);
	}
};
