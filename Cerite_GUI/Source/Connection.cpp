#include "Connection.h"
#include "Edge.h"
#include "Box.h"
#include "Canvas.h"
#include "../../Source/Library.hpp"
//==============================================================================
Connection::Connection(ValueTree tree) : ValueTreeObject(tree)
{
    rebuildObjects();
    
    start = Edge::all_edges[tree.getProperty("StartID")];
    end = Edge::all_edges[tree.getProperty("EndID")];
    
    start->addComponentListener(this);
    end->addComponentListener(this);

    setInterceptsMouseClicks(false, false);
    
    setSize (600, 400);
    
    componentMovedOrResized(*start, true, true);
    componentMovedOrResized(*end, true, true);
    
    resized();
    repaint();
}

Connection::~Connection()
{
    delete_listeners();
}

void Connection::delete_listeners() {
    if(start) {
        start->removeComponentListener(this);
    }
    if(end) {
        end->removeComponentListener(this);
    }
}
//==============================================================================
void Connection::paint (Graphics& g)
{
    g.setColour(Colours::grey);
    g.strokePath(path, PathStrokeType(3.0f));

    auto base_colour = Colours::white;

    if(is_selected) {
        base_colour = Library::colours[(String)start->ValueTreeObject::getState().getProperty("Context")];
    }
    
    g.setColour(base_colour);
    g.strokePath(path, PathStrokeType(1.5f));
}

void Connection::mouseDown(const MouseEvent& e)  {
    
    if(path.contains(e.getPosition().toFloat())) {
        is_selected = !is_selected;
    }
}

void Connection::mouseDrag(const MouseEvent& e)  {
    Point<int> down_pos = e.getMouseDownPosition();
    Point<int> current_pos = e.getPosition();
    
    
    
    Point<int> pstart = start->getBounds().getCentre();
    Point<int> pend = end->getBounds().getCentre();
    
    pstart = getLocalPoint(start, pstart);
    pend = getLocalPoint(end, pend);
    
    Rectangle<int> r(down_pos, current_pos);
    
    Line<int> line = {pstart, pend};
    
    bool intersects = line.intersects(Line<int>({r.getX(), r.getY()}, {r.getX() + r.getWidth(), r.getY()})) ||
    line.intersects(Line<int>({r.getX() + r.getWidth(), r.getY()}, {r.getX() + r.getWidth(), r.getY() + r.getHeight()})) ||
    line.intersects(Line<int>({r.getX() + r.getWidth(), r.getY() + r.getHeight()}, {r.getX(), r.getY() + r.getHeight()})) ||
    line.intersects(Line<int>({r.getX(), r.getY() + r.getHeight()}, {r.getX(), r.getY()})) || (r.contains(pstart) && r.contains(pend));
    
    if(!is_selected && intersects) {
        is_selected = true;
        repaint();
    }
    else if(is_selected && !intersects) {
        is_selected = false;
        repaint();
    }
}


void Connection::componentMovedOrResized (Component &component, bool wasMoved, bool wasResized) {
    int left = std::min(start->get_canvas_bounds().getCentreX(), end->get_canvas_bounds().getCentreX()) - 10;
    int top = std::min(start->get_canvas_bounds().getCentreY(), end->get_canvas_bounds().getCentreY()) - 10;
    int right = std::max(start->get_canvas_bounds().getCentreX(), end->get_canvas_bounds().getCentreX()) + 10;
    int bottom = std::max(start->get_canvas_bounds().getCentreY(), end->get_canvas_bounds().getCentreY()) + 10;
    
    setBounds(left, top, right - left, bottom - top);
    resized();
    repaint();
}

void Connection::resized()
{
    Point<float> pstart = start->get_canvas_bounds().getCentre().toFloat() - getPosition().toFloat();
    Point<float> pend = end->get_canvas_bounds().getCentre().toFloat() - getPosition().toFloat();
    path.clear();
    path.startNewSubPath(pstart.x, pstart.y);

    int curvetype = fabs(pstart.x - pend.x) < (fabs(pstart.y - pend.y) * 5.0f) ? 1 : 2;
    
    curvetype *= !(fabs(pstart.x - pend.x) < 2 || fabs(pstart.y - pend.y) < 2);
    
    if (curvetype == 1) // smooth vertical lines
        path.cubicTo(pstart.x, fabs(pstart.y - pend.y) * 0.5f, pend.x, fabs(pstart.y - pend.y) * 0.5f, pend.x, pend.y);
    else if (curvetype == 2) // smooth horizontal lines
        path.cubicTo(fabs(pstart.x - pend.x) * 0.5f, pstart.y, fabs(pstart.x - pend.x) * 0.5f, pend.y, pend.x, pend.y);
    else // Dont smooth when almost straight
        path.lineTo(pend.x, pend.y);
}


void Connection::componentBeingDeleted(Component& component) {
    delete_listeners();
    getState().getParent().removeChild(getState(), nullptr);
}
