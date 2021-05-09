#include "Box.h"
#include "Canvas.h"
#include "Edge.h"
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
    
    Viewport* viewport = findParentComponentOfClass<Viewport>();
    Canvas* canvas = findParentComponentOfClass<Canvas>();
    
    if(canvas) {
        
        auto pos = e.getEventRelativeTo(canvas).getPosition() - e.getPosition();
        auto bounds = Rectangle<int>(pos, pos + Point<int>(getWidth(), getHeight()));
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
    if(id == Identifiers::edge) {
        auto* new_edge = new Edge(tree);
        addAndMakeVisible(new_edge);
        return static_cast<ValueTreeObject*>(new_edge);
    }
    
    return nullptr;
}

void Box::mouseMove(const MouseEvent& e) {
    findParentComponentOfClass<Canvas>()->repaint();
}


void Box::set_type (String new_type)
{
    StringArray tokens = StringArray::fromTokens(new_type, " ", "");
    
    String type = tokens[0];
    tokens.remove(1);
    
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
    
    int old_total_in = 0;
    int old_total_out = 0;
    
    for(auto& edge : findChildrenOfClass<Edge>()) {
        (bool)edge->ValueTreeObject::getState().getProperty("Input") ? old_total_in++ : old_total_out++;
    }
    
    total_in = 0;
    total_out = 0;
    
    ports.clear();
    
    if(type.isEmpty()) {
        setSize (100, 32);
    }
    
    if(!Library::objects.count(type)) {
        resized();
        getState().removeAllChildren(nullptr);
        return;
    }
    

    auto object = Library::objects[type];
    
    Engine::set_arguments(object, tokens.joinIntoString(" "));
    
    auto obj_ports = std::get<5>(object);

    for(auto& [ctx_name, port] : obj_ports) {
        
        int num_in = std::get<0>(port).getIntValue();
        int num_out = std::get<1>(port).getIntValue();
        
        total_in += num_in;
        total_out += num_out;
        ports[ctx_name] = {num_in, num_out};
    }
    
    while(total_in < old_total_in) {
        int idx = old_total_in-1;
        getState().removeChild(idx, &cnv->undo_manager);
        old_total_in--;
    }
    while(total_in > old_total_in) {
        Uuid id;
        ValueTree edge = ValueTree(Identifiers::edge);
        edge.setProperty(Identifiers::edge_in, true, &cnv->undo_manager);
        edge.setProperty(Identifiers::edge_id, id.toString(), &cnv->undo_manager);
        
        getState().addChild(edge, old_total_in, &cnv->undo_manager);
        old_total_in++;
    }
    
    while(total_out < old_total_out) {
        int idx = old_total_out-1;
        getState().removeChild(total_in + idx, &cnv->undo_manager);
        old_total_out--;
    }
    while(total_out > old_total_out) {
        Uuid id;
        ValueTree edge = ValueTree(Identifiers::edge);
        edge.setProperty(Identifiers::edge_in, false, &cnv->undo_manager);
        edge.setProperty(Identifiers::edge_id, id.toString(), &cnv->undo_manager);
        
        getState().addChild(edge, total_in + old_total_out, &cnv->undo_manager);
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
                    getState().removeChild(edge, &cnv->undo_manager);
                    getState().addChild(edge, i, &cnv->undo_manager);
                }
                if(ctx == "gui") {
                    input ? hidden_in++ : hidden_out++;
                    findChildOfClass<Edge>(i)->setVisible(false);
                }
                edge.setProperty(Identifiers::edge_ctx, ctx, &cnv->undo_manager);
                break;
            }
        }
    }
    
    resized();
}

//==============================================================================
void Box::paint (Graphics& g)
{
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
    if(property == Identifiers::box_x || property == Identifiers::box_y) {
        setTopLeftPosition(getState().getProperty(Identifiers::box_x), getState().getProperty(Identifiers::box_y));
    }
    if(property == Identifiers::box_name) {
        String new_name = treeWhosePropertyHasChanged.getProperty(Identifiers::box_name);
        int new_size = text_label.getFont().getStringWidth(new_name) + (text_label.getHeight() * 1.35f);
        setSize(std::max(50, new_size), getHeight());
        text_label.setText(new_name, NotificationType::dontSendNotification);
        set_type(new_name);
    }
}
