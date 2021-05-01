#include "Box.h"
#include "Canvas.h"
#include "Edge.h"
#include "../../Source/Library.hpp"


//==============================================================================
Box::Box(ValueTree tree, MultiComponentDragger<Box>& multi_dragger) : ValueTreeObject(tree), dragger(multi_dragger), text_label(this, multi_dragger)
{
    
    text_label.setEditable(false, true);
    text_label.setJustificationType(Justification::centred);
    
    addAndMakeVisible(&text_label);
    
    text_label.onTextChange = [this]() {
        String new_text = text_label.getText();
        
        int new_size = text_label.getFont().getStringWidth(new_text) + (text_label.getHeight() * 1.35f);
        setSize(std::max(50, new_size), getHeight());
        set_type(new_text);
    };
    
    factory = [this](const Identifier& id, const juce::ValueTree& tree) {
        if(id == Identifier("Edge")) {
            auto* new_edge = new Edge(tree);
            addAndMakeVisible(new_edge);
            return static_cast<ValueTreeObject*>(new_edge);
        }
    };
    
    
    
    setSize (110, 32);
}

Box::~Box()
{
}

void Box::mouseMove(const MouseEvent& e) {
    findParentComponentOfClass<Canvas>()->repaint();
}


void Box::set_type (String new_type)
{
    StringArray tokens = StringArray::fromTokens(new_type, " ", "");
    
    String type = tokens[0];
    tokens.remove(1);
    
    int old_total_in = total_in;
    int old_total_out = total_out;
    
    total_in = 0;
    total_out = 0;
    
    //getState().removeAllChildren(nullptr);
    
    if(!Library::objects.count(type)) {
        resized();
        return;
    }
    
    
    ports.clear();
    
    auto object = Library::objects[type];
    
    Engine::set_arguments(object, tokens.joinIntoString(" "));
    
    auto obj_ports = std::get<5>(object);

    for(auto& [ctx_name, port] : obj_ports) {
        int num_in = port.first.getIntValue();
        int num_out = port.second.getIntValue();
        
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
        ValueTree edge = ValueTree("Edge");
        edge.setProperty("Input", true, nullptr);
        edge.setProperty("ID", id.toString(), nullptr);
        
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
        ValueTree edge = ValueTree("Edge");
        edge.setProperty("Input", false, nullptr);
        edge.setProperty("ID", id.toString(), nullptr);
        
        getState().addChild(edge, total_in + old_total_out, nullptr);
        old_total_out++;
    }

    for(int i = 0; i < total_in + total_out; i++) {
        auto edge = getState().getChild(i);
        edge.setProperty("Position", i , nullptr);
        
        bool input = edge.getProperty("Input");
        
        int ctx_ports = input ? 0 : total_in;
        for(auto& [ctx, ports] : ports) {
            ctx_ports += input ? ports.first : ports.second;
            if(ctx_ports >= i) {
                String old_ctx = edge.getProperty("Context");
                if(ctx != old_ctx && old_ctx.isNotEmpty()) {
                    getState().removeChild(edge, nullptr);
                    getState().addChild(edge, i, nullptr);
                }
                edge.setProperty("Context", ctx, nullptr);
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

void Box::resized()
{
    auto rect = getLocalBounds().reduced(4);
    text_label.setBounds(rect);
    
    int index = 0;
    for(auto& edge : findChildrenOfClass<Edge>()) {
        
        auto& state = edge->ValueTreeObject::getState();
        bool is_input = state.getProperty("Input");
        
        
        int position = state.getProperty("Position");
        position = position < total_in ? position : position - total_in;
        
        bool sideways = false;
        
        int total = is_input ? total_in : total_out;
        
        float newX, newY;
        if(sideways)
        {
            newY = (is_input ? index : index - total_in) * (getHeight() - 24 / (total - 1 + (total == 1))) + 12;
            newX = is_input ? getLocalBounds().getTopLeft().x : getLocalBounds().getRight();
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
