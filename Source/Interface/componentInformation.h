#pragma once
#include <JuceHeader.h>
#include "edgeInformation.h"

struct componentInformation
{
    
	String name;
	int numArguments;
	std::vector<edgeInformation> edges;
    bool pdObject = false;
	enum {TYPE_ANALOG, TYPE_SIGNAL, TYPE_DATA};
	enum {POSITION_IN, POSITION_OUT};


	void fillEdgeTree(ValueTree edgeTree, UndoManager* undomanager)
	{
		Array<edgeInformation*> newIns;
		Array<edgeInformation*> newOuts;
		// Count number of inputs and outputs on the previous configuration
		int oldIns = 0;

		for(auto edge : edgeTree)
			if(edge.getProperty("Position").toString() == "In")
				oldIns++;

		int oldOuts = 0;

		for(auto edge : edgeTree)
			if(edge.getProperty("Position").toString() == "Out")
				oldOuts++;

		// Seperate inputs and outputs from current configuration
		for (auto& edge : edges)
			if(edge.side == POSITION_IN)
				newIns.add(&edge);

		for (auto& edge : edges)
			if(edge.side == POSITION_OUT)
				newOuts.add(&edge);

		// Evaluate which inputs need to be replaced and which can stay in place
		for(int i = 0; i < newIns.size(); i++)
		{
			ValueTree edge = getEdgeFromTree(edgeTree, i, POSITION_IN);
            
            if(edge.isValid() && int(edge.getProperty("Type")) == newIns[i]->type)
			{
				edge.setProperty("Index", newIns[i]->index, undomanager);
				edge.setProperty(Identifier("Description"), newIns[i]->description, undomanager);
			}
			else
			{
				if(edge.isValid()) edgeTree.removeChild(edge, undomanager);
				newIns[i]->addEdgeToTree(edgeTree, undomanager);
			}
		}

		for(int i = oldIns - 1; i >= newIns.size(); i--)
		{
			edgeTree.removeChild(getEdgeFromTree(edgeTree, i, POSITION_IN), undomanager);
		}

		// Evaluate which outputs need to be replaced and which can stay in place
		for(int i = 0; i < newOuts.size(); i++)
		{
			ValueTree edge = getEdgeFromTree(edgeTree, i, POSITION_OUT);
			if(edge.isValid() && int(edge.getProperty("Type")) == newOuts[i]->type)
			{
				edge.setProperty("Index", newOuts[i]->index, undomanager);
				edge.setProperty(Identifier("Description"), newOuts[i]->description, undomanager);
			}
			else
			{
				if(edge.isValid()) edgeTree.removeChild(edge, undomanager);
				newOuts[i]->addEdgeToTree(edgeTree, undomanager);
			}
		}

        for(int i = oldOuts - 1; i >= newOuts.size(); i--)
		{
			edgeTree.removeChild(getEdgeFromTree(edgeTree, i, POSITION_OUT), undomanager);
		}
	}

	ValueTree getEdgeFromTree(ValueTree tree, int idx, bool position)
	{
		int count = 0;

		for(auto edge : tree)
		{
			if(edge.getProperty("Position").toString() == (position ? "Out" : "In"))
			{
				if(count >= idx)
					return edge;

				count++;
			}
		}

		return ValueTree();
	}

	int getNumInputs()
	{
		int numIns = 0;

		for(auto edge : edges)
			if(edge.side == POSITION_IN)
				numIns++;

		return numIns;
	}

	int getNumOutputs()
	{
		int numOuts = 0;

		for(auto edge : edges)
			if(edge.side == POSITION_OUT)
				numOuts++;

		return numOuts;
	}
};
