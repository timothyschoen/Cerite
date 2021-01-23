#include "StateConverter.h"
#include "../../MainComponent.h"
#include "../../Engine/Compiler/Combiner.h"

Array<int> StateConverter::assignNodes()
{
    
    Array<Array<SafePointer<Connection>>> newnodes;
    Array<SafePointer<Connection>> connectionList;
    Array<int> sizes = {0, 2, 2};
    

    // Seperate analog and digital
    for(auto conNode : cnv->connectionNode)
    {
        Connection* con = cnv->conmanager->getObject(conNode);

        if(con) {
            connectionList.add(con);
        }
    }

    // Assign analog nodes
    while (connectionList.size() > 0)
    {
        Array<SafePointer<Connection>> node;
        followNode(connectionList.getFirst(), node, connectionList);
        newnodes.add(node);
    }

    sizes.getReference(0) = newnodes.size();
    // Find ground node, move it to position 0
    bool done = false;

    for(int i = 0; i < newnodes.size(); i++)
    {
        for(int j = 0; j < newnodes[i].size(); j++)
        {
            if(bool(newnodes[i][j]->start->isGround) || bool(newnodes[i][j]->end->isGround))
            {
                newnodes.move(i, 0);
                done = true;
                break;
            }
        }

        if(done) break;
    }

    // Assign numbers
    for(int i = 0; i < newnodes.size(); i++)
    {
        for(int j = 0; j < newnodes[i].size(); j++)
        {
            newnodes[i][j]->node = i;
            newnodes[i][j]->state.setProperty("Node", i, &cnv->undoManager);
            newnodes[i][j]->start->node = i;
            newnodes[i][j]->end->node = i;
        }
    }

    //int numDigiInputs = 2;
    
    for(auto& box : cnv->boxmanager->objects) {
        
        for(auto& input : box->edgeManager->objects) {
            bool typeCheck = input->side == 1 && input->type.position != 0;
            if(typeCheck && input->connections.size() > 0) {
                int t = input->type.position;
                input->node = sizes[t];
                
                for(auto& conn : input->connections) {
                    if(conn->start == input) {
                        conn->end->node = sizes[t];
                    }
                    else {
                        conn->start->node = sizes[t];
                    }
                }
                sizes.getReference(t)++;
            }
            
            else if (typeCheck) {
                input->node = 0;
            }
        }
    }

    //cnv->programState.setProperty("nNodes", newnodes.size(), &cnv->undoManager);
    //cnv->programState.setProperty("nDigiNodes", digiconns.size() + 1, &cnv->undoManager);
    cnv->nodes = newnodes;
    
    return sizes;
}

Cerite::Object StateConverter::createPatch(Canvas* cnv)
{
    StateConverter converter;
    MainComponent* main = MainComponent::getInstance();
    converter.cnv = cnv;
    
    Array<int> nNodes = converter.assignNodes();
    NodeList allnodes;
    std::vector<Object> documents;
    
    
    for(auto box : cnv->boxNode)
    {
        Box* boxobj = cnv->boxmanager->getObject(box);
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
        
        documents.push_back(ComponentDictionary::getObject(box, tokens[0], args));
        
        allnodes.push_back(nodes);
    }
    
    std::vector<Box*> boxes(cnv->boxmanager->objects.begin(), cnv->boxmanager->objects.end());
    
    
    // Apply right-to-left order for data processing
    for(int i = 0; i < boxes.size(); i++) {
        Array<Edge*> edgesI = boxes[i]->getEdges();
        
        if(std::find_if(edgesI.begin(), edgesI.end(),
                        [](const Edge* x) { return x->type.position == 2;}) == edgesI.end())
        continue;
        
        for(int k = i; k < boxes.size(); k++) {
            Array<Edge*> edgesK = boxes[k]->getEdges();
            if(std::find_if(edgesK.begin(), edgesK.end(),
                            [](const Edge* x) { return x->type.position == 2;}) == edgesK.end())
            continue;
            
            if(boxes[i]->getX() <= boxes[k]->getX()) {
                //boxmanager->objects.swap(i, k);
                std::swap(allnodes[i], allnodes[k]);
                std::swap(boxes[i], boxes[k]);
                std::swap(documents[i], documents[k]);
            }
        }
    }
    // pre-apply rtl ordering on splits
    for(int i = 0; i < boxes.size(); i++) {
        Array<Edge*> edges = boxes[i]->getEdges();
        for(int j = 0; j < edges.size(); j++) {
            Edge* edge = edges[j];
            if(edge->position == 1) continue;
        for(int c = 0; c < edge->connections.size(); c++) {
            
            std::sort( edge->connections.begin(), edge->connections.end(), [&edge]( const Connection* lhs, const Connection* rhs )
            {
                Edge* inletToCompare = lhs->start == edge ? lhs->end : lhs->start;
                Edge* outletToCompare =  rhs->start == edge ? rhs->end : rhs->start;
                
                if(inletToCompare->box == nullptr || outletToCompare->box == nullptr) {
                    return false;
                }
                
                // TODO: This is not finished yet
                return inletToCompare->box->getX() > outletToCompare->box->getX();
            });
            
        }
    }
        
    }
    
    for(int i = 0; i < boxes.size(); i++) {
        Array<Edge*> edges = boxes[i]->getEdges();
        for(int j = 0; j < edges.size(); j++) {
            Edge* edge = edges[j];
            int type = edge->type.position;
            if(type > 0 && edge->side == 0 && edge->connections.size() > 1) {
                
                std::vector<std::pair<int, int>> splitout = {{nNodes[type], type}};
                Object splitobject = ComponentDictionary::library.components[std::string("split") + (type == 1 ? "~" : "")];
                
                
                for(int c = 0; c < edge->connections.size(); c++) {
                    Connection* con = edge->connections[c];
                    
                    splitout.push_back({(con->start->side == 1 ? con->start->node : con->end->node), edge->type.position});
                }
                
                splitobject.setArgument(1, edge->connections.size());
                
                allnodes[i][j] = {nNodes[type], type};
                allnodes.push_back(splitout);
                documents.push_back(splitobject);
                nNodes.getReference(type)++;
                
            }
        }
    }
    
    // Apply correct signal graph order
    // The compiler is dumb in this regard and just adds all the functions together in the supplied order
    // So we make sure we supply the objects in the correct order
    // Incorrect ordering will introduce unwanted delay between objects
    converter.constructGraph(allnodes, documents);
    
    Array<String> used;
    main->guiComponents.clear();
    
    int total = 0;
    int processorcount = 0;
    
    for(auto& box : boxes) {
        if(box->GraphicalComponent) {
            
            main->guiComponents.add(box->GraphicalComponent.get());
            int count = 1;
            
            String paramname = box->type.substring(0, 3) + String(count);
            while(used.contains(paramname)) {
                paramname = box->type.substring(0, 3) + String(++count);
            }
            used.add(paramname);
            box->GraphicalComponent->parameterName = paramname;
            
            if(box->GraphicalComponent->type != ProcessorType::None) {
                box->GraphicalComponent->setID(total, processorcount);
                processorcount++;
            }
            else {
                box->GraphicalComponent->setID(total);
            }
            total++;
        }
    }
    
    return Combiner::combineDocuments("project", documents, allnodes, {});
}


void StateConverter::constructGraph(NodeList& nodes, std::vector<Object>& docs) {
    // sorting algorithm
    // sorts based on whether the input of an earlier component is dependent on the output of a later component
    
    // can we reduce the num iterations of x?
    for (int x = 0; x < nodes.size(); x++)
    for (int i = 0; i < nodes.size(); i++) {
    if(docs[i].imports.count("dsp") == 0) continue;
    for (int j = i + 1; j < nodes.size(); j++) {
        std::vector<int> digiNodes;
        
        if(docs[j].imports.count("dsp") == 0) continue;
        
        int istart = docs[i].imports["dsp"].getPorts(&docs[i]).first;
        int jstart = docs[j].imports["dsp"].getPorts(&docs[j]).first;
        
        if(checkOrder(nodes[i], nodes[j], istart, jstart)) {
            //boxmanager->objects.swap(j, i);
            std::swap(nodes[j], nodes[i]);
            std::swap(docs[j], docs[i]);
        }
    }
  }
}

bool StateConverter::checkOrder(std::vector<std::pair<int, int>> lnodes, std::vector<std::pair<int, int>> rnodes, int lstart, int rstart) {
    int rend = rnodes.size();
    
    for(int k = rstart; k < rend; k++) {
        if(rnodes[k].second != 1)  {
            rend++;
            continue;
        }
        for(int j = 0; j < lstart; j++) {
            if(lnodes[j].second != 1)  {
                lstart++;
                continue;
            }
            
            if(lnodes[j].first == rnodes[k].first) {
                return true;
            }
        }
        /*
         if(std::find(lnodes.begin(), lnodes.begin() + lstart, rnodes[k]) != lnodes.begin() + lstart)
         return true;
         */
    }
    
    return false;
    
    
}


// Recursive function to assign node numbers to connections
void StateConverter::followNode(Connection* connection, Array<SafePointer<Connection>>& node, Array<SafePointer<Connection>>& connections)
{
    node.addIfNotAlreadyThere(connection);
    connections.removeAllInstancesOf(connection);

    for(int i = 0; i < connection->start->connections.size(); i++)
    {
        Connection* evalconnection = connection->start->connections[i];

        if(!node.contains(evalconnection))
        {
            followNode(evalconnection, node, connections);
        }
    }

    for(int i = 0; i < connection->end->connections.size(); i++)
    {
        Connection* evalconnection = connection->end->connections[i];

        if(!node.contains(evalconnection))
        {
            followNode(evalconnection, node, connections);
        }
    }
}
