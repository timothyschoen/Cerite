#include "Edge.h"
#include "Canvas.h"
#include "../../Source/Library.hpp"

//==============================================================================
Edge::Edge(ValueTree tree) : ValueTreeObject(tree)
{
    setSize (8, 8);
    
    all_edges[tree.getProperty("ID")] = this;

    onClick = [this](){
        if(Edge::connecting_edge) {
            
            bool is_input = ValueTreeObject::getState().getProperty("Input");
            String ctx1 = is_input ? Edge::connecting_edge->ValueTreeObject::getState().getProperty("Context") : ValueTreeObject::getState().getProperty("Context");
            String ctx2 = !is_input ? Edge::connecting_edge->ValueTreeObject::getState().getProperty("Context") : ValueTreeObject::getState().getProperty("Context");
            
            
            bool connection_allowed = (ctx1 == ctx2 || Library::can_convert(ctx1, ctx2)) && (bool)Edge::connecting_edge->ValueTreeObject::getState().getProperty("Input") != is_input && connecting_edge->getParentComponent() != getParentComponent();
        
            if(Edge::connecting_edge == this) {
                Edge::connecting_edge = nullptr;
            }
            else if(!connection_allowed) {
                Edge::connecting_edge = nullptr;
            }
            else if(connection_allowed) {
                ValueTree new_connection = ValueTree(Identifiers::connection);
                new_connection.setProperty("StartID", Edge::connecting_edge->ValueTreeObject::getState().getProperty("ID"), nullptr);
                new_connection.setProperty("EndID", ValueTreeObject::getState().getProperty("ID"), nullptr);
                Canvas* cnv = findParentComponentOfClass<Canvas>();
                cnv->getState().appendChild(new_connection, &cnv->undo_manager);
                Edge::connecting_edge = nullptr;
            }
        }
        else {
            Edge::connecting_edge = this;
        }
    };
}

Edge::~Edge()
{
    all_edges.erase(ValueTreeObject::getState().getProperty("ID"));
}

Rectangle<int> Edge::get_canvas_bounds()
{
    auto bounds = getBounds();
    if(auto* box = dynamic_cast<Box*>(getParentComponent())) {
        bounds += box->getPosition();
    }
    
    return bounds;
}

//==============================================================================
void Edge::paint (Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    auto background_colour = Library::colours[(String)ValueTreeObject::getState().getProperty("Context")];

    auto base_colour = background_colour.withMultipliedSaturation (hasKeyboardFocus (true) ? 1.3f : 0.9f)
                                      .withMultipliedAlpha (isEnabled() ? 1.0f : 0.5f);

    
    if (isDown() || isOver())
        base_colour = base_colour.contrasting (isDown() ? 0.2f : 0.05f);

    
    Path path;
    path.addEllipse(bounds);
    
    g.setColour (base_colour);
    g.fillPath (path);
    g.setColour (findColour (ComboBox::outlineColourId));
    g.strokePath (path, PathStrokeType (1.f));
}

void Edge::resized()
{
    // This is called when the MainComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
}

void Edge::mouseMove(const MouseEvent& e) {
    findParentComponentOfClass<Canvas>()->repaint();
}
