#include <JuceHeader.h>
#include <mutex>
#include "ProgramState.h"
#include "Canvas.h"
#include "../../MainComponent.h"
#include "../../Engine/Worker/Source/Message.h"
ProgramState::ProgramState(Component* parent)
{
    cnv = static_cast<Canvas*>(parent);
    
    //infoDictionary = ComponentDictionary::createInfoMap();
    
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
        for(int j = 0; j < edges[i]->connections.size(); j++)
        {
            if(edges[i]->connections[j]->start == edges[1-i] || edges[i]->connections[j]->end == edges[1-i]) {
                std::cout << "Connection already exists" << std::endl;
                return ValueTree();
            }
        }
        
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
    MainComponent::getInstance()->sidebar.inspector->deselect();
    boxmanager->getObject(boxNode.getChild(index))->clearEdges();
    boxNode.removeChild(index, &undoManager);
};

void ProgramState::removeBox(Box* box)
{
    startNewAction();
    MainComponent::getInstance()->sidebar.inspector->deselect();
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
        MainComponent::getInstance()->updateSystem();
    
    state->undoManager.beginNewTransaction();
    stopTimer();
    state->updateUndoState();
}

void ProgramState::logMessage(const char* msg) {
    MainComponent::getInstance()->logMessage(msg);
}

void ProgramState::loadPdPatch(String pdPatch)
{
    StringArray tokens;
    tokens.addTokens (pdPatch, ";\n", "");
    
    clearState();
    
    for(auto& line : tokens)
    {
        if(line.startsWith("#X"))
        {
            StringArray lineTokens;
            lineTokens.addTokens (line.substring(3), " ", "");
            if(lineTokens[0] == "obj")
            {
               
                Point<int> position(lineTokens[1].getIntValue(), lineTokens[2].getIntValue());
                lineTokens.removeRange(0, 3);
                
                if(lineTokens[0] == "bng")
                {
                    lineTokens.removeRange(1, lineTokens.size()-1);
                }
                addBox(lineTokens.joinIntoString(" "), position);
            }
            if(lineTokens[0] == "text")
            {
                Point<int> position(lineTokens[1].getIntValue(), lineTokens[2].getIntValue());
                lineTokens.removeRange(0, 3);
                addBox(lineTokens.joinIntoString(" "), position);
            }
            if(lineTokens[0] == "graph")
            {
                Point<int> position(lineTokens[1].getIntValue(), lineTokens[2].getIntValue());
                lineTokens.removeRange(0, 2);
                addBox("graph", position);
            }
            if(lineTokens[0] == "array")
            {
                Point<int> position(lineTokens[1].getIntValue(), lineTokens[2].getIntValue());
                lineTokens.removeRange(0, 2);
                addBox("array", position);
            }
            if(lineTokens[0] == "msg")
            {
                Point<int> position(lineTokens[1].getIntValue(), lineTokens[2].getIntValue());
                lineTokens.removeRange(0, 2);
                addBox("msg", position);
            }
            
            if(lineTokens[0] == "floatatom")
            {
                Point<int> position(lineTokens[1].getIntValue(), lineTokens[2].getIntValue());
                lineTokens.removeRange(0, 2);
                addBox("floatatom", position);
            }
            
            else if (lineTokens[0] == "connect"){
                
                int startIdx = lineTokens[1].getIntValue();
                int endIdx = lineTokens[3].getIntValue();
                
                Box* startBox = getBox(startIdx);
                Box* endBox = getBox(endIdx);
                
                int edgeStartIdx = startBox->info.getNumInputs() + lineTokens[2].getIntValue();
                int edgeEndIdx = lineTokens[4].getIntValue();

                Edge* start = nullptr;
                Edge* end = nullptr;
                
                if(startBox != nullptr && edgeStartIdx < startBox->info.edges.size()) {
                    start = startBox->getEdges()[edgeStartIdx];
                }
                if(endBox != nullptr && edgeEndIdx < endBox->info.edges.size()) {
                    end = endBox->getEdges()[edgeEndIdx];
                }
                    
                if(start && end) {
                    addConnection(start, end);
                }
            }
            
            
        }
        else if(line.startsWith("#N"))
        {
            
        }
        
    }
    
}
