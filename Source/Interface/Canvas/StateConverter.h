#pragma once

#include <JuceHeader.h>
#include <mutex>
#include "Connection.h"
#include "../../Engine/Interface/Object.h"

#define SafePointer Component::SafePointer

class Canvas;
class StateConverter
{
public:
    
	static Cerite::Object createPatch(Canvas* cnv);
    
private:
    
    Canvas* cnv;
    
    Array<int> assignNodes(); // Assigns a node number to connections
    
    void constructGraph(Cerite::NodeList& nodes, std::vector<Cerite::Object>& docs);
    
    bool checkOrder(std::vector<std::pair<int, int>> lnodes, std::vector<std::pair<int, int>> rnodes, int lstart, int rstart);

    void followNode(Connection* connection, Array<SafePointer<Connection>>& node, Array<SafePointer<Connection>>& connections);
    
};
