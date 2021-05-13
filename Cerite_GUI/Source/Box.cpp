#include "Box.h"
#include "Canvas.h"
#include "Edge.h"
#include "MainComponent.h"
#include "../../Source/Library.hpp"


void ClickLabel::mouseDown(const MouseEvent & e)
{
    is_down = true;
    dragger.handleMouseDown(box, e);
}

void ClickLabel::mouseUp(const MouseEvent & e)
{
    is_down = false;
    dragger.handleMouseUp(box, e);
    
    if(e.getDistanceFromDragStart() > 10 || e.getLengthOfMousePress() > 600) {
        Edge::connecting_edge = nullptr;
    }
    
    Viewport* viewport = findParentComponentOfClass<Viewport>();
    Canvas* canvas = findParentComponentOfClass<Canvas>();
    
    if(canvas) {
        
        auto pos = e.getEventRelativeTo(canvas).getPosition() - e.getPosition();
        auto bounds = Rectangle<int>(pos, pos + Point<int>(getParentWidth(), getParentHeight()));
        if(!canvas->getLocalBounds().contains(bounds)) {
            if(bounds.getRight() > canvas->getWidth())
                canvas->setSize(bounds.getRight() + 10, canvas->getHeight());
            
            if(bounds.getBottom() > canvas->getHeight())
                canvas->setSize(canvas->getWidth(), bounds.getBottom() + 10);
            
            
            if(bounds.getX() < 0.0f) {
                for(auto& box : canvas->findChildrenOfClass<Box>()) {
                    if(&box->text_label != this)
                        box->setTopLeftPosition(box->getX() - bounds.getX(), box->getY());
                    else
                        box->setTopLeftPosition(0, box->getY());
                }
                
            }
            if(bounds.getY() < 0) {
                for(auto& box : canvas->findChildrenOfClass<Box>()) {
                    if(&box->text_label != this)
                        box->setTopLeftPosition(box->getX(), box->getY() - bounds.getY());
                    else
                        box->setTopLeftPosition(box->getX(), 0);
                }
            }
            
            
        }
    }
    
    if(auto* box = dynamic_cast<Box*>(getParentComponent())) {
        box->update_position();
    }
    
    
}

void ClickLabel::mouseDrag(const MouseEvent & e)
{
    dragger.handleMouseDrag(e);
}


//==============================================================================
Box::Box(Canvas* parent, ValueTree tree, MultiComponentDragger<Box>& multi_dragger) : ValueTreeObject(tree), dragger(multi_dragger), text_label(this, multi_dragger)
{
    cnv = parent;
    
    if(!tree.hasProperty(Identifiers::box_x)) {
        auto pos = cnv->getMouseXYRelative();
        tree.setProperty(Identifiers::box_x, pos.getX(), &cnv->undo_manager);
        tree.setProperty(Identifiers::box_y, pos.getY(), &cnv->undo_manager);
    }
    
    
    setSize (100, 32);
    
    text_label.setEditable(false, true);
    text_label.setJustificationType(Justification::centred);
    
    addAndMakeVisible(&text_label);
    
    text_label.toBack();
    
    text_label.onTextChange = [this]() {
        String new_text = text_label.getText();
        getState().setProperty(Identifiers::box_name, new_text, &cnv->undo_manager);
    };
    
    
    
    rebuildObjects();
    
    auto edges = findChildrenOfClass<Edge>();
    int total = edges.size();
    int num_in = 0;
    for(auto& edge : edges) {
        auto edge_tree = edge->ValueTreeObject::getState();
        num_in += (int)edge_tree.getProperty("Input");
    }
    total_in = num_in;
    total_out = total - num_in;
    
    getState().sendPropertyChangeMessage(Identifiers::box_name);
    getState().sendPropertyChangeMessage(Identifiers::box_x);
    
    
}

Box::~Box()
{
}

ValueTreeObject* Box::factory (const juce::Identifier& id, const juce::ValueTree& tree)
{
    if(getState().indexOf(tree) < 0) return nullptr;
    
    if(id == Identifiers::edge) {
        auto* new_edge = new Edge(tree);
        addAndMakeVisible(new_edge);
        return static_cast<ValueTreeObject*>(new_edge);
    }
    if(id == Identifiers::canvas) {
        auto* canvas = new Canvas(tree);
        return static_cast<ValueTreeObject*>(canvas);
    }
    
    return nullptr;
}

void Box::mouseMove(const MouseEvent& e) {
    findParentComponentOfClass<Canvas>()->repaint();
}


void Box::set_type (String new_type)
{
    String arguments = new_type.fromFirstOccurrenceOf(" ", false, false);
    String type = new_type.upToFirstOccurrenceOf(" ", false, false);
    
    graphics.reset(GUIComponent::create_gui(type, this));
    
    if(graphics) {
        addAndMakeVisible(graphics.get());
        auto [w, h] = graphics->get_best_size();
        setSize(std::max(getWidth(), w + 8), h + 28);
        graphics->toBack();
    }
    else
    {
        setSize(getWidth(), 32);
    }

    
    if(type.isEmpty()) {
        setSize (100, 32);
    }
    
    // Handle subpatchers
    if(auto* subcnv = findChildOfClass<Canvas>(0)) {
        getState().removeChild(subcnv->getState(), &cnv->undo_manager);
    }
    
    Ports sub_ports;
    if(type == "p") {
        sub_ports = load_subpatch(arguments.upToFirstOccurrenceOf(" ", false, false));
    }
    
    if(!Library::objects.count(type) && type != "p") {
        resized();
        getState().removeAllChildren(&cnv->undo_manager);
        return;
    }
    
    if(type != "edge" && is_edge) {
        is_edge = false;
        text_label.setVisible(true);
        findChildOfClass<Edge>(0)->removeMouseListener(&text_label);
    }
    
    auto object = Library::objects[type];
    Engine::set_arguments(object, arguments);
    
    auto obj_ports = type != "p" ? std::get<5>(object) : sub_ports;
    update_ports(obj_ports);
    
    if(type == "edge") {
        is_edge = true;
        setSize(20, 20);
        text_label.setVisible(false);
        findChildOfClass<Edge>(0)->addMouseListener(&text_label, false);
    }
    
    
    
    resized();
}

//==============================================================================
void Box::paint (Graphics& g)
{
    if(is_edge) return;
    
    auto rect = getLocalBounds().reduced(4);
    
    auto base_colour = findColour(TextButton::buttonColourId);
    
    bool is_over = getLocalBounds().contains(getMouseXYRelative());
    bool is_down = text_label.is_down;
    
    if (is_down || is_over || dragger.isSelected(this))
        base_colour = base_colour.contrasting (is_down ? 0.2f : 0.05f);
    
    
    g.setColour(base_colour);
    g.fillRect(rect);
    
    g.setColour(findColour(ComboBox::outlineColourId));
    g.drawRect(rect);
    
}

void Box::moved()
{
    for(auto& edge : findChildrenOfClass<Edge>()) {
        edge->triggerResizedOrMoved(true, true);
    }
    
}

void Box::update_position()
{
    if(!cnv->undo_manager.isPerformingUndoRedo()) {
        getState().setPropertyExcludingListener(this, Identifiers::box_x, getX(), &cnv->undo_manager);
        getState().setPropertyExcludingListener(this, Identifiers::box_y, getY(), &cnv->undo_manager);
    }
}

void Box::resized()
{
    text_label.setBounds(4, 4, getWidth() - 8, 24);
    
    if(graphics) {
        graphics->setBounds(4, 28, getWidth() - 8, getHeight() - 32);
    }
    
    
    int index = 0;
    for(auto& edge : findChildrenOfClass<Edge>()) {
        
        auto& state = edge->ValueTreeObject::getState();
        bool is_input = state.getProperty(Identifiers::edge_in);
        
        int position = index < total_in ? index : index - total_in;
        
        bool sideways = false;
        
        int total = is_input ? total_in - hidden_in : total_out - hidden_out;
        
        float newX, newY;
        if(sideways)
        {
            //newY = (is_input ? index : index - total_in) * (getHeight() - 24 / (total - 1 + (total == 1))) + 12;
            //newX = is_input ? getLocalBounds().getTopLeft().x : getLocalBounds().getRight();
        }
        else
        {
            
            newY = is_input ? 4 : getHeight() - 4;
            newX = position * ((getWidth() - 32) / (total - 1 + (total == 1))) + 16;
        }
        
        edge->setCentrePosition(newX, newY);
        edge->setSize(8, 8);
        
        index++;
    }
}

void Box::valueTreePropertyChanged (ValueTree &treeWhosePropertyHasChanged, const Identifier &property)
{
    if(treeWhosePropertyHasChanged != getState()) return;
    
    if(property == Identifiers::box_x || property == Identifiers::box_y) {
        setTopLeftPosition(getState().getProperty(Identifiers::box_x), getState().getProperty(Identifiers::box_y));
        update_position();
    }
    if(property == Identifiers::box_name) {
        String new_name = treeWhosePropertyHasChanged.getProperty(Identifiers::box_name);
        int new_size = text_label.getFont().getStringWidth(new_name) + (text_label.getHeight() * 1.35f);
        setSize(std::max(50, new_size), getHeight());
        text_label.setText(new_name, NotificationType::dontSendNotification);
        set_type(new_name);
    }
}

Ports Box::load_subpatch(String title) {
    File subpatcher = MainComponent::home_dir.getChildFile("Saves").getChildFile(title);
    subpatcher = subpatcher.existsAsFile() ? subpatcher : subpatcher.withFileExtension(".crpat");
    
    ValueTree new_cnv;
    
    if(subpatcher.existsAsFile()) {
        auto stream = subpatcher.createInputStream();
        new_cnv = ValueTree::readFromStream(*stream);
    }
    else {
        new_cnv = ValueTree(Identifiers::canvas);
    }
    
    new_cnv.setProperty("Title", title, nullptr);
    getState().appendChild(new_cnv, nullptr);

    if(auto* canvas = findObjectForTree<Canvas>(new_cnv)) {
        canvas->setBounds(cnv->getBounds());
    }
    return update_subpatch(new_cnv);
}

Ports Box::update_subpatch(ValueTree tree) {
    
    Ports result;
    
    for(const auto& [ctx_name, ctx] : Library::contexts) {
        int num_in = 0, num_out = 0;
        for(auto obj : tree) {
            if(obj.getType() != Identifiers::box) continue;
            String obj_name = obj.getProperty(Identifiers::box_name).toString();
            String obj_type = obj_name.upToFirstOccurrenceOf(" ", false, false);
            if(Library::subpatchers_connections[ctx_name].contains(obj_type)) {
                if(Library::subpatchers_connections[ctx_name][0] == obj_type) {
                    num_in++;
                }
                else {
                    num_out++;
                }
            }
        }
        if(num_in != 0 || num_out != 0) {
            result[ctx_name] = {String(num_in), String(num_out), String()};
        }
    }
    
    return result;
}

void Box::update_ports(Ports obj_ports) {
    
    int old_total_in = 0;
    int old_total_out = 0;
    
    for(auto& edge : findChildrenOfClass<Edge>()) {
        (bool)edge->ValueTreeObject::getState().getProperty("Input") ? old_total_in++ : old_total_out++;
    }
    
    total_in = 0;
    total_out = 0;
    
    ports.clear();
    
    for(auto& [ctx_name, port] : obj_ports) {
        int num_in = std::get<0>(port).getIntValue();
        int num_out = std::get<1>(port).getIntValue();
        
        total_in += num_in;
        total_out += num_out;
        ports[ctx_name] = {num_in, num_out};
    }
    
    while(total_in < old_total_in) {
        int idx = old_total_in-1;
        getState().removeChild(idx, nullptr);
        old_total_in--;
    }
    while(total_in > old_total_in) {
        Uuid id;
        ValueTree edge = ValueTree(Identifiers::edge);
        edge.setProperty(Identifiers::edge_in, true, nullptr);
        edge.setProperty(Identifiers::edge_id, id.toString(), nullptr);
        
        getState().addChild(edge, old_total_in, nullptr);
        old_total_in++;
    }
    
    while(total_out < old_total_out) {
        int idx = old_total_out-1;
        getState().removeChild(total_in + idx, nullptr);
        old_total_out--;
    }
    while(total_out > old_total_out) {
        Uuid id;
        ValueTree edge = ValueTree(Identifiers::edge);
        edge.setProperty(Identifiers::edge_in, false, nullptr);
        edge.setProperty(Identifiers::edge_id, id.toString(), nullptr);
        
        getState().addChild(edge, total_in + old_total_out, nullptr);
        old_total_out++;
    }
    
    hidden_in = 0;
    hidden_out = 0;
    
    for(int i = 0; i < total_in + total_out; i++) {
        auto edge = getState().getChild(i);
        if(!edge.isValid()) continue;
        bool input = edge.getProperty(Identifiers::edge_in);
        
        int ctx_ports = input ? 0 : total_in;
        for(auto& [ctx, ports] : ports) {
            ctx_ports += input ? ports.first : ports.second;
            if(ctx_ports > i) {
                
                String old_ctx = edge.getProperty(Identifiers::edge_ctx);
                if(ctx != old_ctx && old_ctx.isNotEmpty()) {
                    getState().removeChild(edge, nullptr);
                    getState().addChild(edge, i, nullptr);
                }
                if(ctx == "gui") {
                    input ? hidden_in++ : hidden_out++;
                    findChildOfClass<Edge>(i)->setVisible(false);
                }
                edge.setProperty(Identifiers::edge_ctx, ctx, nullptr);
                break;
            }
        }
    }
    resized();
}
