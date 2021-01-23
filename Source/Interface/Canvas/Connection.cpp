#include "Connection.h"
#include "../../MainComponent.h"
#include "Edge.h"
#include "../ComponentDictionary.h"


Connection::Connection(Canvas* parent, ValueTree node, std::string domain) : CanvasComponent(node, this), canvas(parent)
{
    type = ComponentDictionary::domainInformation[domain];
    reconnectbuttons.push_back(std::make_unique<ReconnectButton>());
    reconnectbuttons.push_back(std::make_unique<ReconnectButton>());
    
    for(int i = 0; i < 2; i++)
    {
        addChildComponent(reconnectbuttons[i].get());
        reconnectbuttons[i]->setColour(TextButton::buttonColourId, Colours::lightgrey);
        reconnectbuttons[i]->setAlpha(0.7);
        reconnectbuttons[i]->onClick = [this, i]() mutable
        {
            canvas->removeConnection(this);
            canvas->connectingEdge = i == 0 ? end : start;
        };
        reconnectbuttons[i]->setSize(6, 6);
    }
    
    setInterceptsMouseClicks(false, dontSendNotification);
}

Connection::~Connection()
{
    
};

void Connection::paint(Graphics & g)
{
    Point<float> pstart = end->getBounds().getCentre().toFloat() - getPosition().toFloat();
    Point<float> pend = start->getBounds().getCentre().toFloat() - getPosition().toFloat();
    bool isMoving = canvas->isSelected(static_cast<TextButton*>(start->box)) || canvas->isSelected(static_cast<TextButton*>(start)) || static_cast<TextButton*>(end->box) || canvas->isSelected(static_cast<TextButton*>(end));
    bool shouldSnap = path.getLength() > 16 && isMoving && canvas->didStartDragging && canvas->snapToGrid;
    
    if(snapX && shouldSnap)
    {
        Line<float> angleLine = Line<float>(pstart.translated(9, 0), pend.translated(9, 0));
        g.setColour(Colours::red);
        g.drawLine(angleLine.withShortenedStart(9).withShortenedEnd(9));
    }
    
    if(snapY && shouldSnap)
    {
        Line<float> angleLine = Line<float>(pstart.translated(0, 9), pend.translated(0, 9));
        g.setColour(Colours::red);
        g.drawLine(angleLine.withShortenedStart(9).withShortenedEnd(9));
    }
    
    g.setColour(Colours::grey);
    g.strokePath(path, PathStrokeType(3.0f));
    // Fix digital colours!!
    g.setColour(getColour(hoverstate));
    g.strokePath(path, PathStrokeType(1.5f));
    moved = false;
}


void Connection::resized()
{
    reconnectbuttons[0]->setCentrePosition(path.getPointAlongPath(16 - (reconnectbuttons[0]->showHoverEffect * 9)).toInt());
    reconnectbuttons[1]->setCentrePosition(path.getPointAlongPath(path.getLength() - (16 - (reconnectbuttons[1]->showHoverEffect * 9))).toInt());
    pstart = start->getBounds().getCentre().toFloat() - getPosition().toFloat();
    pend = end->getBounds().getCentre().toFloat() - getPosition().toFloat();
    path.clear();
    path.startNewSubPath(pstart.x, pstart.y);
    int curvetype = 0;
    
    if(start->position >= 4 && end->position >= 4)
        curvetype = 2;
    else if((start->position == end->position && start->position != 3) || (start->position < 3 || end->position < 3))
        curvetype = 1;
    else
        curvetype = fabs(pstart.x - pend.x) < fabs(pstart.y - pend.y) ? 1 : 2;
    
    curvetype *= !(fabs(pstart.x - pend.x) < 2 || fabs(pstart.y - pend.y) < 2);
    
    if (curvetype == 1) // smooth vertical lines
        path.cubicTo(pstart.x, fabs(pstart.y - pend.y) * 0.5f, pend.x, fabs(pstart.y - pend.y) * 0.5f, pend.x, pend.y);
    else if (curvetype == 2) // smooth horizontal lines
        path.cubicTo(fabs(pstart.x - pend.x) * 0.5f, pstart.y, fabs(pstart.x - pend.x) * 0.5f, pend.y, pend.x, pend.y);
    else // Dont smooth when almost straight
        path.lineTo(pend.x, pend.y);
    
    repaint();
}

void Connection::showHandles(bool shouldShowHandles)
{
    for(auto& but : reconnectbuttons) but->setVisible(shouldShowHandles);
}

void Connection::mouseDown(const MouseEvent& e)
{
    int clicks = e.getNumberOfClicks();
    
    for(int i = 0; i < 2; i++)
        if(reconnectbuttons[i]->getBounds().contains(e.getPosition()) && isSelected()) reconnectbuttons[i]->triggerClick();
    
    if(!ModifierKeys::getCurrentModifiers().isAnyModifierKeyDown())
        canvas->deselectAll();
    
    if(clicks > 1)
        splitConnection(e.getEventRelativeTo(canvas).getPosition());
    else if (clicks == 1 && canvas->connectingEdge != nullptr && type.position == 0)
        canvas->finishConnecting(splitConnection(e.getEventRelativeTo(canvas).getPosition()));
    else if (clicks == 1)
    {
        setSelected(true);
    }
    
    if(ModifierKeys::getCurrentModifiers().isRightButtonDown())
    {
        setHovered(false);
        canvas->removeConnection(this);
    }
}


bool Connection::hitTest(int x, int y)
{
    auto position = Point<float>(x, y);
    Point<float> nearest;
    path.getNearestPoint(Point<float>(x, y), nearest);
    auto distance = position.getDistanceFrom(nearest);
    return distance < 5;
}

void Connection::mouseMove(const MouseEvent& e)
{
    for(int i = 0; i < 2; i++)
    {
        bool hittest = reconnectbuttons[i]->getBounds().contains(e.getPosition()) && isSelected();
        bool needsUpdate = reconnectbuttons[i]->showHoverEffect != hittest;
        reconnectbuttons[i]->showHoverEffect = hittest;
        
        if(needsUpdate)
            moved = true;
        
        repaint();
    }
}

void Connection::mouseEnter(const MouseEvent& e)
{
    setHovered(true);
    repaint();
    //canvas->tipWindow.displayTip(e.getScreenPosition(), getTooltip()); // necessary???
}

void Connection::mouseExit(const MouseEvent& e)
{
    setHovered(false);
    repaint();
}

void Connection::addToCanvas()
{
    canvas->addAndMakeVisible(this);
    addMouseListener(&canvas->mouseListener, true);
    
    start = canvas->getEdgeByID(state.getProperty("Inlet").toString());
    end = canvas->getEdgeByID(state.getProperty("Outlet").toString());
    
    if((start == nullptr || end == nullptr) && state.hasProperty("StartBox")) {
        
        Box* startbox = canvas->getBox(int(state.getProperty("StartBox")));
        Box* endbox = canvas->getBox(int(state.getProperty("EndBox")));
        
        int startidx = int(state.getProperty("StartIdx"));
        int endidx = int(state.getProperty("EndIdx"));
        
        if(startidx >= 0 && startidx < startbox->getEdges().size())
            start = startbox->getEdges()[startidx];
        else if(startidx < -1) {
            start = canvas->getEdge(int(state.getProperty("StartBox")));
        }
        
        if(endidx >= 0 && endidx < endbox->getEdges().size())
            end = endbox->getEdges()[endidx];
        else if(endidx < -1) {
            end = canvas->getEdge(int(state.getProperty("EndBox")));
        }
        
        if(start != nullptr && end != nullptr) {
            state.setProperty("Inlet", start->state.getProperty("ID"), nullptr);
            state.setProperty("Outlet", end->state.getProperty("ID"), nullptr);
        }
        else {
            canvas->removeConnection(this);
            std::cout << "Err!" << std::endl;
        }
    }
    
    if(start == nullptr || end == nullptr) {
        canvas->removeConnection(this);
        std::cout << "err: Invalid Connection!" << std::endl;
        return;
    }
    
    update();
    toBack();
}

void Connection::removeFromCanvas()
{
    canvas->removeChildComponent(this);
    
    if(canvas->nodes[node].contains(this))
        canvas->nodes[node].removeAllInstancesOf(this);
    
    if(start != nullptr) {
        start->removeComponentListener(this);
        start->connections.removeAllInstancesOf(this);
    }
    
    if(end != nullptr) {
        end->removeComponentListener(this);
        end->connections.removeAllInstancesOf(this);
    }
    
    canvas->repaint();
    
    
}

void Connection::stateChanged(ValueTree &treeWhosePropertyHasChanged, const Identifier &property, const var &value)
{
    if(property == Identifier("Inlet"))
    {
        String id = value.toString();
        start = canvas->getEdgeByID(value.toString());
        
        if(start == nullptr) {
            canvas->removeConnection(this);
            std::cout << "err: Invalid Connection!" << std::endl;
            return;
        }
        
        start->connections.addIfNotAlreadyThere(this);
        start->addComponentListener(this);
    }
    else if(property == Identifier("Outlet"))
    {
        end = canvas->getEdgeByID(value);
        
        if(end == nullptr) {
            canvas->removeConnection(this);
            std::cout << "err: Invalid Connection!" << std::endl;
            return;
        }
        
        end->connections.addIfNotAlreadyThere(this);
        end->addComponentListener(this);
    }
    
    if(start != nullptr && end != nullptr && property != Identifier("Node")) componentMovedOrResized(*this, false, false);
}


void Connection::componentMovedOrResized (Component &component, bool wasMoved, bool wasResized)
{
    int left = std::min(start->getBounds().getCentreX(), end->getBounds().getCentreX()) - 10;
    int top = std::min(start->getBounds().getCentreY(), end->getBounds().getCentreY()) - 10;
    int right = std::max(start->getBounds().getCentreX(), end->getBounds().getCentreX()) + 10;
    int bottom = std::max(start->getBounds().getCentreY(), end->getBounds().getCentreY()) + 10;
    moved = true;
    setBounds(left, top, right - left, bottom - top);
}

void Connection::setSelected(bool shouldBeSelected)
{
    hoverstate = shouldBeSelected ? SELECTED : OFF;
    oldstate = hoverstate; // maybe only for analog?
    showHandles(shouldBeSelected);
    repaint();
}

void Connection::setHighlighted(bool shouldBeHighlighted)
{
    oldstate = shouldBeHighlighted ? hoverstate : oldstate;
    hoverstate = shouldBeHighlighted ? HOVERED : oldstate;
}

void Connection::setHovered(bool shouldBeHovered)
{
    if(type.position == 0) {
        for(int i = 0; i < canvas->nodes[node].size(); i++)
        {
            if(canvas->nodes[node][i].getComponent() == this || canvas->nodes[node][i].getComponent() == nullptr) continue;

            //canvas->nodes[node][i]->setHighlighted(shouldBeHovered);
            //canvas->nodes[node][i]->repaint();
        }

        if (!isSelected()) hoverstate = shouldBeHovered ? HOVERED : oldstate;

        repaint();
        
    }
    else if(type.position == 1) {
        if (isSelected()) return;

        hoverstate = shouldBeHovered ? HOVERED : OFF;
        repaint();
    }
    else if(type.position == 2) {
        
    }
}




bool Connection::isSelected()
{
    return hoverstate == SELECTED;
};

bool Connection::isHighlighted()
{
    return hoverstate == HIGHLIGHTED;
};

Edge* Connection::splitConnection(Point<int> position)
{
    if(type.position != 0) return nullptr;
    
    canvas->startNewAction();
    ValueTree edgeTree = canvas->addEdge(position);
    LooseEdge* newedge = canvas->edgemanager->getObject(edgeTree);
    canvas->addConnection(start, newedge);
    canvas->addConnection(newedge, end);
    canvas->removeConnection(this);
    return newedge;
}

String Connection::getTooltip()
{
    /*
    MNASystem* m = static_cast<MainComponent*>(canvas->main)->msystem;

    if(m != nullptr && node < m->xPlot.size())
    {
        return String("        Node " + std::to_string(node) + "\n Voltage:  " + std::to_string(m->xPlot[node]).substr(0, 8) + "  \n                  ");
    }
     */

    return String("");
}

Colour Connection::getColour(int hoverstate)
{
    if (hoverstate == OFF)          return Colours::lightgrey;
    if (hoverstate == HIGHLIGHTED)  return type.colour.brighter();
    if (hoverstate == HOVERED)      return type.colour;
    if (hoverstate == SELECTED)     return type.colour.darker();
  
};
