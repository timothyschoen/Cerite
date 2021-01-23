#pragma once

#include <JuceHeader.h>
#include <mutex>
#include "Box.h"
#include "Connection.h"
#include "Edge.h"
#include "LooseEdge.h"
#include "BoxManager.h"
#include "EdgeManager.h"
#include "../ComponentDictionary.h"
#include "ConnectionManager.h"

class Canvas;
class ProgramState
{
public:

	class UndoTimer : public Timer
	{
		ProgramState* state;

		bool rebuildSystem = true;

		void timerCallback();

	public:

		void shouldRebuild(bool rebuild)
		{
			rebuildSystem = rebuild;
		};

		UndoTimer(ProgramState* parent)
		{
			state = parent;
		}
	};

    Canvas* cnv;
    
    std::unordered_map<String, Document> classDictionary;
    std::unordered_map<String, componentInformation> infoDictionary;

	UndoTimer undoTimer = UndoTimer(this);

	Identifier mainNodeID = Identifier("Main");
	Identifier boxNodeID = Identifier("Boxes");
	Identifier edgeNodeID = Identifier("Inlets");
	Identifier connectionNodeID = Identifier("Connections");

	ValueTree programState = ValueTree(mainNodeID);
	ValueTree edgeNode = ValueTree(edgeNodeID);
	ValueTree connectionNode = ValueTree(connectionNodeID);
	ValueTree boxNode = ValueTree(boxNodeID);

	UndoManager undoManager;

	std::unique_ptr<ConnManager> conmanager;
	std::unique_ptr<EdgeManager> edgemanager;
    std::unique_ptr<BoxManager> boxmanager;

	bool changedSinceSave = false;

	ProgramState(Component* parent);

	virtual ~ProgramState();

	Box* getBox(int index);
	Edge* getEdge(int index);
	Connection* getConnection(int index);


	ValueTree addBox(String name, Point<int> position);
	ValueTree addEdge(Point<int> position);
	ValueTree addConnection(Edge* start, Edge* end);

	Edge* getEdgeByID(String ID);

	void removeBox(int index);
	void removeBox(Box* box);

	void removeEdge(int index);
	void removeEdge(LooseEdge* edge);

	void removeConnection(int index);
	void removeConnection(Connection* con);


	// For parsing pasted text
	// This adds another state to our current state
	// We need to do a few calculations to keep the connections intact!
	void addState(String stateToAdd);

	void setState(String newState);

	void clearState();

	void startNewAction(bool shouldRebuildSystem = true);
        
    virtual void setUndoState(bool setUndo, bool canUndo, bool setRedo, bool canRedo) = 0;
	virtual void updateUndoState() = 0;
	virtual void update() = 0;
	virtual void updateSystemState() = 0;
	virtual void reset() = 0;

	String getState(Array<Box*> boxlist, Array<LooseEdge*> edgelist);

	CanvasComponent* castCanvasComponent(Component* componentToBeCast)
	{
		return dynamic_cast<CanvasComponent*>(componentToBeCast);
	}
    
    class FileListener : public gin::FileSystemWatcher::Listener
       {
       public:
           FileListener() {};
           // Since all other calls to the library class are made from the message thread,
           // this should secure it from data races
           void folderChanged (const File folder) override {
               MessageManager::callAsync([](){
               ComponentDictionary::refresh();
               });
           }
           void fileChanged (const File file, gin::FileSystemWatcher::FileSystemEvent fsEvent) override {
               MessageManager::callAsync([](){
               ComponentDictionary::refresh();
               });
           }
       };

       gin::FileSystemWatcher fileWatcher;
       FileListener fileListener;

    static void logMessage(const char* msg);
};
