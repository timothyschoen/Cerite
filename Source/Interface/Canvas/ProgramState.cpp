#include <JuceHeader.h>
#include <mutex>
#include "ProgramState.h"
#include "Canvas.h"
#include "../../MainComponent.h"

ProgramState::ProgramState(Component* parent)
{
	cnv = static_cast<Canvas*>(parent);
    
    infoDictionary = ComponentDictionary::createInfoMap();
    
    fileWatcher.addFolder(FSManager::home.getChildFile("Objects"));
    fileWatcher.addListener(&fileListener);
    
    
	programState.appendChild(boxNode, &undoManager);
	programState.appendChild(edgeNode, &undoManager);
	programState.appendChild(connectionNode, &undoManager);
	boxmanager = std::make_unique<BoxManager>(boxNode, cnv);
	edgemanager = std::make_unique<EdgeManager>(edgeNode, cnv);
	conmanager = std::make_unique<ConnManager>(connectionNode, cnv);
	boxmanager->rebuildObjects();
	edgemanager->rebuildObjects();
	conmanager->rebuildObjects();
};

ProgramState::~ProgramState()
{
};

Box* ProgramState::getBox(int index)
{
	return boxmanager->objects[index];
};

Edge* ProgramState::getEdge(int index)
{
	return edgemanager->objects[index];
};

Edge* ProgramState::getEdgeByID(String ID)
{
	for(auto box : boxmanager->objects)
		for(auto edge : box->getEdges())
			if(!edge->state.getProperty("ID").toString().compare(ID)) return edge;

	for(auto edge : edgemanager->objects)
		if(!edge->state.getProperty("ID").toString().compare(ID)) return edge;

	return nullptr;
}


Connection* ProgramState::getConnection(int index)
{
	return conmanager->objects[index];
};


ValueTree ProgramState::addBox(String name, Point<int> position)
{
	startNewAction();
	ValueTree boxTree = ValueTree("Box");
	boxTree.setProperty("X", position.getX(), &undoManager);
	boxTree.setProperty("Y", position.getY(), &undoManager);
	boxTree.setProperty("Name", name, &undoManager);
	boxNode.appendChild(boxTree, &undoManager);
	return boxTree;
};

ValueTree ProgramState::addEdge(Point<int> position)
{
	startNewAction();
	ValueTree edgeTree = ValueTree(Identifier("Edge"));
	edgeTree.setProperty("X", position.getX(), &undoManager);
	edgeTree.setProperty("Y", position.getY(), &undoManager);
	edgeNode.appendChild(edgeTree, &undoManager);
	return edgeTree;
};

ValueTree ProgramState::addConnection(Edge* start, Edge* end)
{
    std::vector<Edge*> edges = {start, end};
    
    // Check if the connection is legit
    for(int i = 0; i < 2; i++) {
        if(edges[i]->type.position == 0 && edges[1-i]->type.position > 0)
        {
            std::cout << "Can't connect analog and digital inlets" << std::endl;
            return ValueTree();
        }
        
        else if(edges[i]->type.position > 0 && edges[1-i]->type.position > 0 && edges[i]->side == edges[1-i]->side)
        {
            std::cout << "Wrong data/signal path" << std::endl;
            return ValueTree();
        }
        
        else if(edges[i]->type.position == 1 && edges[i]->side == 0 && edges[1-i]->type.position == 2)
        {
            std::cout << "Can't connect signal to data inlets" << std::endl;
            return ValueTree();
        }

        else if(((start->box != nullptr) && (start->box == end->box)) || start == end)
        {
            std::cout << "Can't connect object to itself" << std::endl;
            return ValueTree();
        }
        
    }

	startNewAction();
	ValueTree connectionTree = ValueTree(Identifier("Connection" + Uuid().toString().toStdString()));
	connectionTree.setProperty("Outlet", start->state.getProperty("ID"), &undoManager);
	connectionTree.setProperty("Inlet", end->state.getProperty("ID"), &undoManager);
	connectionTree.setProperty("Type", start->type.position, &undoManager);
	connectionNode.appendChild(connectionTree, &undoManager);
	return connectionTree;
}



void ProgramState::removeBox(int index)
{
	startNewAction();
    static_cast<MainComponent*>(static_cast<Canvas*>(this)->main)->sidebar.inspector->deselect();
	boxmanager->getObject(boxNode.getChild(index))->clearEdges();
	boxNode.removeChild(index, &undoManager);
};

void ProgramState::removeBox(Box* box)
{
	startNewAction();
    static_cast<MainComponent*>(static_cast<Canvas*>(this)->main)->sidebar.inspector->deselect();
	box->clearEdges();
	boxNode.removeChild(box->state, &undoManager);
};

void ProgramState::removeEdge(int index)
{
	startNewAction();
	edgeNode.removeChild(index, &undoManager);
};

void ProgramState::removeEdge(LooseEdge* edge)
{
	startNewAction();
	edgeNode.removeChild(edge->state, &undoManager);
};

void ProgramState::removeConnection(int index)
{
	startNewAction();
	connectionNode.removeChild(index, &undoManager);
}

void ProgramState::removeConnection(Connection* con)
{
	startNewAction();
	connectionNode.removeChild(con->state, &undoManager);
}

// For parsing pasted text
// This adds another state to our current state
// We need to do a few calculations to keep the connections intact!
void ProgramState::addState(String stateToAdd)
{
	MemoryOutputStream xmlStream;
	Base64::convertFromBase64 (xmlStream, stateToAdd);
	ValueTree newState = ValueTree::fromXml(xmlStream.toString());
	Canvas* canvas = static_cast<Canvas*>(cnv);
	canvas->deselectAll();
	startNewAction();

	for(auto box : newState.getChildWithName("Boxes"))
	{
		box.setProperty("X", int(box.getProperty("X")) + 25, nullptr);
		box.setProperty("Y", int(box.getProperty("Y")) + 25, nullptr);
		ValueTree newbox = box.createCopy();
		boxNode.appendChild(newbox, &undoManager);
		canvas->setSelected(boxmanager->getObject(newbox), true);
	}

	for(auto edge : newState.getChildWithName("Edges"))
	{
		edge.setProperty("X", int(edge.getProperty("X")) + 25, nullptr);
		edge.setProperty("Y", int(edge.getProperty("Y")) + 25, nullptr);
		ValueTree newedge = edge.createCopy();
		edgeNode.appendChild(newedge, &undoManager);
		canvas->setSelected(edgemanager->getObject(newedge), true);
	}

	for(auto con : newState.getChildWithName("Connections"))
	{
		ValueTree newcon = con.createCopy();
		connectionNode.appendChild(newcon, &undoManager);
		conmanager->getObject(newcon)->setSelected(true);
	}

	update();
};

void ProgramState::setState(String newState)
{
    if(!newState.containsNonWhitespaceChars())
        return;
    
	MemoryOutputStream xmlStream;
	Base64::convertFromBase64 (xmlStream, newState);
	programState = ValueTree::fromXml(xmlStream.toString());
	boxNode = programState.getOrCreateChildWithName("Boxes", &undoManager);
	edgeNode = programState.getOrCreateChildWithName("Edges", &undoManager);
	connectionNode = programState.getOrCreateChildWithName("Connections", &undoManager);
	boxmanager->freeObjects();
	edgemanager->freeObjects();
	conmanager->freeObjects();
	boxmanager = std::make_unique<BoxManager>(boxNode, cnv);
	edgemanager = std::make_unique<EdgeManager>(edgeNode, cnv);
	conmanager = std::make_unique<ConnManager>(connectionNode, cnv);
	boxmanager->rebuildObjects();
	edgemanager->rebuildObjects();
	conmanager->rebuildObjects();
	update();
    programState.setProperty("Power", false, &undoManager); // In case we saved it powered on
	updateSystemState();
	reset();
};

String ProgramState::getState(Array<Box*> boxlist, Array<LooseEdge*> edgelist)
{
	Array<Connection*> connlist;
	ValueTree selectedState("Main");
	ValueTree selectedBoxes = selectedState.getOrCreateChildWithName("Boxes", nullptr);
	ValueTree selectedEdges = selectedState.getOrCreateChildWithName("Edges", nullptr);
	ValueTree selectedConns = selectedState.getOrCreateChildWithName("Connections", nullptr);
	std::map<String, String> uuidMap;

	for(auto box : boxlist)
	{
		ValueTree boxTree = box->state.createCopy();

		for(auto edge : box->getEdges())
		{
			for(auto con : edge->connections)
				connlist.addIfNotAlreadyThere(con);

			String newUuid = Uuid().toString();
			uuidMap[edge->state.getProperty("ID").toString()] = newUuid;
			boxTree.getChildWithName("Edges").getChildWithProperty("ID", edge->state.getProperty("ID").toString()).setProperty("ID", newUuid, nullptr);
		}

		selectedBoxes.appendChild(boxTree, nullptr);
	}

	for(auto edge : edgelist)
	{
		for(auto con : edge->connections)
			connlist.addIfNotAlreadyThere(con);

		String newUuid = Uuid().toString();
		uuidMap[edge->state.getProperty("ID").toString()] = newUuid;
		ValueTree edgeTree = edge->state.createCopy();
		edgeTree.setProperty("ID", newUuid, nullptr);
		selectedEdges.appendChild(edgeTree, nullptr);
	}

	for(auto con : connlist)
	{
		ValueTree connelt = con->state.createCopy();
		String inID = connelt.getProperty("Inlet").toString();
		String outID = connelt.getProperty("Outlet").toString();

		if(uuidMap.count(inID) + uuidMap.count(outID) == 2)
		{
			connelt.setProperty("Inlet", uuidMap[inID], nullptr);
			connelt.setProperty("Outlet", uuidMap[outID], nullptr);
			selectedConns.appendChild(connelt, nullptr);
		}
	}

	return Base64::toBase64(selectedState.toXmlString());
}

void ProgramState::clearState()
{
	connectionNode.removeAllChildren(&undoManager);
	boxNode.removeAllChildren(&undoManager);
	edgeNode.removeAllChildren(&undoManager);
	reset();
}

void ProgramState::startNewAction(bool shouldRebuildSystem)
{
    setUndoState(true, true, true, false);
	undoTimer.shouldRebuild(shouldRebuildSystem);
    changedSinceSave = true;
	undoTimer.startTimerHz(4);
}

void ProgramState::UndoTimer::timerCallback()
{
	if(rebuildSystem)
		static_cast<MainComponent*>(static_cast<Canvas*>(state)->main)->updateSystem();

	state->undoManager.beginNewTransaction();
	stopTimer();
	state->updateUndoState();
}


Cerite::Document ProgramState::createPatch()
{
	int nNodes = assignNodes();
    NodeList allnodes;
    std::vector<Document> documents;
    
	for(auto box : boxNode)
	{
        Box* boxobj = boxmanager->getObject(box);
        if(boxobj->info.pdObject) continue;
		StringArray tokens;
		std::vector<std::pair<int, int>> nodes;
        
		std::vector<String> args;
		tokens.addTokens (box.getProperty("Name").toString(), " ", "\"");
        

		for(int t = 1; t < tokens.size(); t++)
		{
			String argString = tokens[t];

			if(argString.substring(0, argString.length() - 2).containsOnly(".0123456789") && argString.getLastCharacters(1).containsOnly("pnumkM"))
			{
				argString = argString.replace("p", "e-12").replace("n", "e-9").replace("u", "e-6").replace("m", "e-3").replace("k", "e3").replace("M", "e6");
			}
			else if(FSManager::home.getChildFile("Media").getChildFile(argString).exists())
				argString = FSManager::home.getChildFile("Media").getChildFile(argString).getFullPathName();

			args.push_back(argString.toStdString());
		}

		for(auto& edge : boxobj->getEdges())
		{
            
			if (edge->connections.size() > 0)
                nodes.push_back({edge->node, edge->type.position});
            else
                nodes.push_back({edge->type.position > 0, edge->type.position});
		}
        

        MainComponent* main = static_cast<MainComponent*>(static_cast<Canvas*>(this)->main);
        
        documents.push_back(ComponentDictionary::getObject(box, tokens[0], args));
        
        if(boxobj->GraphicalComponent) {
        int count = 0;
        for(auto& doc : documents)
            if(doc.name == tokens[0])
                count++;
        
            std::string paramname = documents.back().name.substr(0, 3) + std::to_string(count);
            
            
           //std::string returnname = "callback_" + documents.back().name.substr(0, 3) + std::to_string(count);
            
            main->external.add(boxobj->GraphicalComponent->createProcessor(paramname));
        
        }
        
        allnodes.push_back(nodes);
    }
    
    
    // The system only allows a single connection from an input
    // We need to add a virtual split object to resolve this
    std::vector<Box*> boxes(boxmanager->objects.begin(), boxmanager->objects.end());
    
    for(int i = 0; i < boxes.size(); i++) {
        Array<Edge*> edges = boxes[i]->getEdges();
        for(int j = 0; j < edges.size(); j++) {
            Edge* edge = edges[j];
            
            if(edge->type.position > 0 && edge->side == 0 && edge->connections.size() > 1) {
                
                std::vector<std::pair<int, int>> splitout = {{nNodes, edge->type.position}};
                Document splitobject = ComponentDictionary::library.get(std::string("split") + (edge->type.position == 1 ? "~" : ""));
                
                for(int c = 0; c < edge->connections.size(); c++) {
                    Connection* con = edge->connections[c];
                    
                    if(c >= 2) splitobject.addPort(c + 1, edge->type.name.toStdString(), true);
                    
                    splitout.push_back({(con->start->side == 1 ? con->start->node : con->end->node), edge->type.position});
                }
                
                splitobject.setArgument(1, edge->connections.size());
                
                allnodes[i][j] = {nNodes, edge->type.position};
                allnodes.push_back(splitout);
                documents.push_back(splitobject);
                nNodes++;
                
            }
        }
    }
     
    // Apply correct signal graph order
    // The compiler is dumb in this regard and just adds all the functions together in the supplied order
    // So we make sure we supply the objects in the correct order
    // Incorrect ordering will introduce unwanted delay between objects
    constructGraph(allnodes, documents);
    
    
    return Document::concat("project", documents, allnodes);
}

void ProgramState::constructGraph(NodeList& nodes, std::vector<Document>& docs) {
    // sorting algorithm
    // sorts based on whether the input of an earlier component is dependent on the output of a later component
    
    // can we reduce the num iterations of x?
    for (int x = 0; x < nodes.size(); x++)
    for (int i = 0; i < nodes.size(); i++)
    for (int j = i + 1; j < nodes.size(); j++) {
        
        if(checkOrder(nodes[i], nodes[j], docs[i].outstart["dsp"], docs[j].outstart["dsp"])) {
            std::swap(nodes[j], nodes[i]);
            std::swap(docs[j], docs[i]);
        }
      }
}

bool ProgramState::checkOrder(std::vector<std::pair<int, int>> lnodes, std::vector<std::pair<int, int>> rnodes, int lstart, int rstart) {
   
    for(int k = rstart; k < rnodes.size(); k++) {
        if(std::find(lnodes.begin(), lnodes.begin() + lstart, rnodes[k]) != lnodes.begin() + lstart)
            return true;
        
    }

    return false;
    

}

void ProgramState::logMessage(const char* msg) {
    Canvas::mainCanvas->main->logMessage(msg);
}
