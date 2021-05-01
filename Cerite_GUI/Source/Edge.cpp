#include "Edge.h"
#include "Canvas.h"
#include "../../Source/Library.hpp"

//==============================================================================
Edge::Edge(ValueTree tree) : ValueTreeObject(tree)
{
    setSize (8, 8);
    
    
    all_edges[tree.getProperty("ID")] = this;
    
    factory = [this](const Identifier& id, const juce::ValueTree& tree) {
            return static_cast<ValueTreeObject*>(nullptr);
    };
    
    onClick = [this](){
        
        bool connection_allowed = Edge::connecting_edge && Edge::connecting_edge->ValueTreeObject::getState().getProperty("Context") == ValueTreeObject::getState().getProperty("Context") && Edge::connecting_edge->ValueTreeObject::getState().getProperty("Input") != ValueTreeObject::getState().getProperty("Input");
        
        if(Edge::connecting_edge == this) {
            Edge::connecting_edge = nullptr;
        }
        else if(Edge::connecting_edge && !connection_allowed) {
            Edge::connecting_edge = nullptr;
        }
        else if(connection_allowed) {
            ValueTree new_connection = ValueTree("Connection");
            new_connection.setProperty("StartID", Edge::connecting_edge->ValueTreeObject::getState().getProperty("ID"), nullptr);
            new_connection.setProperty("EndID", ValueTreeObject::getState().getProperty("ID"), nullptr);
            
            findParentComponentOfClass<Canvas>()->getState().appendChild(new_connection, nullptr);
            Edge::connecting_edge = nullptr;
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
    
    
    auto ctx_iter = Library::contexts.find((String)ValueTreeObject::getState().getProperty("Context"));
    int ctx_idx = std::distance(Library::contexts.begin(), ctx_iter);
    
    auto background_colour = ctx_colours[ctx_idx];

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
