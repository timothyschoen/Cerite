#include "Canvas.h"
#include "Box.h"
#include "Connection.h"
#include "../../Source/Library.hpp"

//==============================================================================
Canvas::Canvas(ValueTree tree) : ValueTreeObject(tree)
{
    setSize (600, 400);
    
    factory = [this](const Identifier& id, const juce::ValueTree& tree) {
        if(id == Identifier("Box")){
            auto* box = new Box(tree, dragger);
            addAndMakeVisible(box);
            box->addMouseListener(this, true);
            return static_cast<ValueTreeObject*>(box);
        }
        if(id == Identifier("Connection")){
            auto* connection = new Connection(tree);
            addAndMakeVisible(connection);
            connection->addMouseListener(this, true);
            return static_cast<ValueTreeObject*>(connection);
        }
        return static_cast<ValueTreeObject*>(nullptr);
    };
    
    addAndMakeVisible(&lasso);
    lasso.setAlwaysOnTop(true);
    lasso.setColour(LassoComponent<Box>::lassoFillColourId, findColour(ScrollBar::ColourIds::thumbColourId).withAlpha((float)0.3));
    
    addKeyListener(this);
    
    setWantsKeyboardFocus(true);
}

Canvas::~Canvas()
{
    ValueTreeObject::getState().removeAllChildren(nullptr);
}

void Canvas::mouseDown(const MouseEvent& e)
{
    auto* source = e.originalComponent;

    if(dynamic_cast<Connection*>(source)) {
        lasso.beginLasso(e.getEventRelativeTo(this), &dragger);
    }
    else if(source == this){
        Edge::connecting_edge = nullptr;
        lasso.beginLasso(e, &dragger);
        
        for(auto& con : findChildrenOfClass<Connection>()) {
            con->is_selected = false;
            con->repaint();
        }
    }
}

void Canvas::mouseDrag(const MouseEvent& e)
{
    auto* source = e.originalComponent;
    
    if(dynamic_cast<Connection*>(source)) {
        lasso.dragLasso(e.getEventRelativeTo(this));
    }
    else if(source == this){
        Edge::connecting_edge = nullptr;
        lasso.dragLasso(e);
        
        for(auto& con : findChildrenOfClass<Connection>()) {
            con->mouseDrag(e.getEventRelativeTo(con));
        }
    }
}

void Canvas::mouseUp(const MouseEvent& e)
{
    lasso.endLasso();
}

//==============================================================================
void Canvas::paintOverChildren (Graphics& g)
{
    if(Edge::connecting_edge) {
        Point<float> mouse_pos = getMouseXYRelative().toFloat();
        Point<int> edge_pos =  Edge::connecting_edge->get_canvas_bounds().getPosition();
        
        edge_pos += Point<int>(4, 4);
        
        Path path;
        path.startNewSubPath(edge_pos.toFloat());
        path.lineTo(mouse_pos);
        
        g.setColour(Colours::grey);
        g.strokePath(path, PathStrokeType(3.0f));
    }

}

void Canvas::mouseMove(const MouseEvent& e) {
    repaint();
}
void Canvas::resized()
{

}
bool Canvas::keyPressed(const KeyPress &key, Component *originatingComponent) {

    if(key.getTextCharacter() == 'n') {
        auto box = ValueTree("Box");
        getState().appendChild(box, nullptr);
        return true;
    }
    
    if(key.getKeyCode() == KeyPress::backspaceKey) {
        for(auto& sel : dragger.getLassoSelection()) {
             removeMouseListener(sel);
            getState().removeChild(dynamic_cast<ValueTreeObject*>(sel)->getState(), nullptr);
        }
        
        for(auto& con : findChildrenOfClass<Connection>()) {
            if(con->is_selected) {
                removeMouseListener(con);
                getState().removeChild(con->getState(), nullptr);
            }
        }
        
        dragger.deselectAll();
        return true;
    }
    
    return false;
}


Patch Canvas::create_patch() {
  
    std::map<Edge*, Array<Connection*>> edge_connections;
    std::map<Edge*, std::map<String, std::vector<int>>> edge_nodes;
    
    std::map<String, int> ctx_sizes;
    
    Patch result;
    
    
    auto contexts = Library::contexts;
    
    
    for(auto& [key, val] : contexts) {
        ctx_sizes[key] = 0;
    }
    
    for(auto& con : findChildrenOfClass<Connection>()) {
        edge_connections[con->start].add(con);
        edge_connections[con->end].add(con);
    }
    
    for(auto& [edge, connections] : edge_connections) {
        if(edge->ValueTreeObject::getState().getProperty("Input")) {
            String ctx = edge->ValueTreeObject::getState().getProperty("Context");
            
            ctx_sizes[ctx]++;
            
            edge_nodes[edge][ctx].push_back(ctx_sizes[ctx]);
            
            
            for(auto& con : connections) {
                
                Edge* output_edge = edge == con->start ? con->end : con->start;
                
                edge_nodes[output_edge][ctx].push_back(ctx_sizes[ctx]);
            }
            
        }
    }
    
    for(auto& box : findChildrenOfClass<Box>()) {
        std::map<String, std::vector<std::vector<int>>> box_nodes;
        
        
        for(auto& [name, ctx] : contexts) {
            int num_edges = box->ports[name].first + box->ports[name].second ;
            if(num_edges == 0) continue;
            box_nodes[name].resize(num_edges);
        }
        
        for(auto& edge : box->findChildrenOfClass<Edge>()) {
            String ctx = edge->ValueTreeObject::getState().getProperty("Context");
 
            int pos = edge->ValueTreeObject::getState().getProperty("Position");
            std::vector<int> node = edge_nodes.count(edge) ? edge_nodes[edge][ctx] : std::vector<int>{0};
            
            
            box_nodes[ctx][pos] = node;
            
        }
        
        result.push_back({box->text_label.getText(), box->getX(), box->getY(), box_nodes});
        
    }
    
    return result;
    
    
};
