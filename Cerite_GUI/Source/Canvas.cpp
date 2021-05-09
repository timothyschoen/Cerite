#include "Canvas.h"
#include "Box.h"
#include "Connection.h"
#include "MainComponent.h"
#include "../../Source/Library.hpp"

//==============================================================================
Canvas::Canvas(ValueTree tree) : ValueTreeObject(tree)
{
    setSize (600, 400);
    
    addAndMakeVisible(&lasso);
    lasso.setAlwaysOnTop(true);
    lasso.setColour(LassoComponent<Box>::lassoFillColourId, findColour(ScrollBar::ColourIds::thumbColourId).withAlpha((float)0.3));
    
    addKeyListener(this);
    
    setWantsKeyboardFocus(true);
    
    rebuildObjects();
}

Canvas::~Canvas()
{
    removeAllChildren();
    removeKeyListener(this);
    ValueTreeObject::getState().removeAllChildren(&undo_manager);
}
ValueTreeObject* Canvas::factory(const juce::Identifier & id, const juce::ValueTree & tree)
{
    if(id == Identifier(Identifiers::box)){
        auto* box = new Box(this, tree, dragger);
        addAndMakeVisible(box);
        box->addMouseListener(this, true);
        return static_cast<ValueTreeObject*>(box);
    }
    if(id == Identifier(Identifiers::connection)){
        auto* connection = new Connection(this, tree);
        addAndMakeVisible(connection);
        connection->addMouseListener(this, true);
        return static_cast<ValueTreeObject*>(connection);
    }
    return static_cast<ValueTreeObject*>(nullptr);
}

void Canvas::mouseDown(const MouseEvent& e)
{
    auto* source = e.originalComponent;
    
    drag_start_position = e.getMouseDownPosition();
    
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
    
    if(ModifierKeys::getCurrentModifiers().isRightButtonDown())
    {
        bool has_selection = dragger.getLassoSelection().getNumSelected();
        bool multiple = dragger.getLassoSelection().getNumSelected() > 1;
        
        popupmenu.clear();
        popupmenu.addItem(1, "Edit", has_selection && !multiple);
        popupmenu.addSeparator();
        popupmenu.addItem(2, "Encapsulate", has_selection && multiple);
        popupmenu.addSeparator();
        popupmenu.addItem(3,"Cut", has_selection);
        popupmenu.addItem(4,"Copy", has_selection);
        popupmenu.addItem(5,"Duplicate", has_selection);
        popupmenu.addItem(6,"Delete", has_selection);
        popupmenu.setLookAndFeel(&getLookAndFeel());
        
        int result = popupmenu.show();
        
        if(result == 1) {
            // not implemented
        }
        else if(result == 3) {
            SystemClipboard::copyTextToClipboard(copy_selection());
            remove_selection();
        }
        else if(result == 4) {
            SystemClipboard::copyTextToClipboard(copy_selection());
        }
        else if(result == 5) {
            auto copy = copy_selection();
            paste_selection(copy);
        }
        else if(result == 6) {
            remove_selection();
        }
        else if(result == 2) {
            // TODO: create patch from selection!
            auto patch = create_patch();
            auto objects = NodeConverter::create_objects(patch);
            auto formatted = NodeConverter::format_nodes(objects, Library::contexts);

            save_chooser.launchAsync(FileBrowserComponent::saveMode | FileBrowserComponent::warnAboutOverwriting, [this, formatted](const FileChooser &f) mutable {
                auto file = f.getResult();
                Uuid id;
                String name = file.getFileNameWithoutExtension() + id.toString().substring(0, 6);
                auto test_obj = Engine::create_subpatcher(name, formatted, Library::contexts);
                String to_save = Engine::reconstruct_object(test_obj, Library::contexts);
                file.replaceWithText(to_save);
                
            });
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
            
            Line<int> path(con->start->get_canvas_bounds().getCentre(), con->end->get_canvas_bounds().getCentre());
            
            bool intersect = false;
            for(float i = 0; i < 1; i += 0.005) {
                if(lasso.getBounds().contains(path.getPointAlongLineProportionally(i))) {
                    intersect = true;
                }
            }
            
            if(!con->is_selected && intersect) {
                con->is_selected = true;
                con->repaint();
            }
            else if(con->is_selected && !intersect) {
                con->is_selected = false;
                con->repaint();
            }
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
        auto box = ValueTree(Identifiers::box);
        getState().appendChild(box, &undo_manager);
        return true;
    }
    
    if(key.getKeyCode() == KeyPress::backspaceKey) {
        remove_selection();
        return true;
    }
    // cmd-c
    if(key.getModifiers().isCommandDown() && key.isKeyCode(67)) {
        
        SystemClipboard::copyTextToClipboard(copy_selection());
        
        return true;
    }
    // cmd-v
    if(key.getModifiers().isCommandDown() && key.isKeyCode(86)) {
        paste_selection(SystemClipboard::getTextFromClipboard());
        return true;
    }
    // cmd-x
    if(key.getModifiers().isCommandDown() && key.isKeyCode(88)) {
        
        SystemClipboard::copyTextToClipboard(copy_selection());
        remove_selection();
        
        return true;
    }
    // cmd-d
    if(key.getModifiers().isCommandDown() && key.isKeyCode(68)) {
        
        String copied = copy_selection();
        paste_selection(copied);
        
        return true;
    }
    
    // cmd-shift-z
    if(key.getModifiers().isCommandDown() && key.getModifiers().isShiftDown() && key.isKeyCode(90)) {
    
        redo();
        return true;
    }
    // cmd-z
    if(key.getModifiers().isCommandDown() && key.isKeyCode(90)) {
    
        undo();
        return true;
    }

    return false;
}

String Canvas::copy_selection() {
    ValueTree copy_tree = ValueTree("Copy");
    
    
    Array<std::pair<Connection*, ValueTree>> found_connections;

    for(auto& con : findChildrenOfClass<Connection>()) {
        int num_points_found = 0;
        for(auto& box : findChildrenOfClass<Box>()) {
            if(!dragger.isSelected(box)) continue;
            for(auto& edge : box->findChildrenOfClass<Edge>()) {
                if(con->start == edge || con->end == edge) {
                    num_points_found++;
                }
                
            }
        }
        
        if(num_points_found > 1) {
            ValueTree connection_tree("Connection");
            connection_tree.copyPropertiesFrom(con->getState(), nullptr);
            found_connections.add({con, connection_tree});
        }
    }
        
        
    Array<std::pair<Box*, ValueTree&>> found_boxes;
    for(auto& box : findChildrenOfClass<Box>()) {
        if(dragger.isSelected(box)) {
            ValueTree box_tree(Identifiers::box);
            box_tree.copyPropertiesFrom(box->getState(), nullptr);
            
            
            found_boxes.add({box, box_tree});
            
            
            for(auto& edge : box->findChildrenOfClass<Edge>()) {
                
                Uuid new_id;
                auto edge_copy = edge->ValueTreeObject::getState().createCopy();

                for(auto& [con, con_tree] : found_connections) {

                    if(con_tree.getProperty(Identifiers::start_id) == edge_copy.getProperty(Identifiers::edge_id) || con_tree.getProperty(Identifiers::end_id) == edge_copy.getProperty(Identifiers::edge_id))
                    {
                        
                        bool start_id = con_tree.getProperty(Identifiers::start_id) == edge_copy.getProperty(Identifiers::edge_id);
                        con_tree.setProperty(start_id ? Identifiers::start_id : Identifiers::end_id , new_id.toString(), nullptr);
                    }
                }
                
                edge_copy.setProperty(Identifiers::edge_id, new_id.toString(), nullptr);
                box_tree.appendChild(edge_copy, nullptr);
                
            }
            
            copy_tree.appendChild(box_tree, nullptr);
        }
    }
    
    for(auto& [con, con_tree] : found_connections) {
        copy_tree.appendChild(con_tree, nullptr);
    }
    
    // Count which connections have both start and end point within the selection

    
    
    return copy_tree.toXmlString();
}

void Canvas::remove_selection() {
    for(auto& sel : dragger.getLassoSelection()) {
        removeMouseListener(sel);
        getState().removeChild(dynamic_cast<ValueTreeObject*>(sel)->getState(), &undo_manager);
    }
    
    for(auto& con : findChildrenOfClass<Connection>()) {
        if(con->is_selected) {
            removeMouseListener(con);
            getState().removeChild(con->getState(), &undo_manager);
        }
    }
    
    dragger.deselectAll();
}

void Canvas::paste_selection(String to_paste) {
    String text = to_paste;
    
    auto tree = ValueTree::fromXml(text);
    for(auto child : tree) {
        if(child.hasProperty(Identifiers::box_x)) {
            int old_x = child.getProperty(Identifiers::box_x);
            int old_y = child.getProperty(Identifiers::box_y);
            child.setProperty(Identifiers::box_x, old_x + 30, nullptr);
            child.setProperty(Identifiers::box_y, old_y + 30, nullptr);
        }
        
        getState().appendChild(child.createCopy(), &undo_manager);
    }
}


Patch Canvas::create_patch() {
    
    std::map<SafePointer<Edge>, Array<SafePointer<Connection>>> edge_connections;
    std::map<SafePointer<Edge>, std::map<String, std::vector<int>>> edge_nodes;
    
    std::map<String, int> ctx_sizes;
    
    Patch patch;
    
    auto contexts = Library::contexts;

    
    for(auto& con : findChildrenOfClass<Connection>()) {
        edge_connections[con->start].add(con);
        edge_connections[con->end].add(con);
    }
    
    for(auto& [edge, connections] : edge_connections) {
        if(edge && edge->ValueTreeObject::getState().getProperty("Input")) {
            String ctx = edge->ValueTreeObject::getState().getProperty("Context");
        
            ctx_sizes[ctx]++;
            
            edge_nodes[edge][ctx].push_back(ctx_sizes[ctx]);
            
            String conversion_ctx = ctx;
            for(auto& con : connections) {
                
                SafePointer<Edge> output_edge = edge == con->start ? con->end : con->start;
                
                if(output_edge && edge->ValueTreeObject::getState().getProperty("Context") !=
                   output_edge->ValueTreeObject::getState().getProperty("Context")) {
                    
                    conversion_ctx = output_edge->ValueTreeObject::getState().getProperty("Context").toString() + "->" + edge->ValueTreeObject::getState().getProperty("Context").toString();
                }
                
                edge_nodes[output_edge][conversion_ctx].push_back(ctx_sizes[ctx]);
            }
        }
    }
    
    
    for(auto& box : findChildrenOfClass<Box>()) {
        std::map<String, std::vector<std::vector<int>>> box_nodes;
        
        std::vector<int> gui_nodes;
        
        for(auto& [name, ctx] : contexts) {
            int num_edges = box->ports[name].first + box->ports[name].second ;
            if(num_edges == 0) continue;
            box_nodes[name].resize(num_edges);
        }
        
        std::map<String, int> num_edges;
        
        for(auto& edge : box->findChildrenOfClass<Edge>()) {
            String ctx = edge->ValueTreeObject::getState().getProperty("Context");
            //if(!num_edges.count(ctx)) num_edges[ctx] = 0;
            
            if(edge_nodes[edge].size() > 1 || (edge_nodes[edge].count(ctx) == 0 && edge_nodes[edge].size() != 0)) {
                for(auto& [ctx_name, n_list] : edge_nodes[edge]) {
                    if(ctx_name.contains("->")) {
                        String out = ctx_name.upToFirstOccurrenceOf("->", false, false);
                        String in = ctx_name.fromFirstOccurrenceOf("->", false, false);
                        auto converter = Library::conversions[{out, in}];
                        
                        ctx_sizes[out]++;
                        edge_nodes[edge][out].push_back(ctx_sizes[out]);
                        
                        std::map<String, std::vector<std::vector<int>>> converter_nodes;
                        
                        converter_nodes[out].resize(1);
                        converter_nodes[in].resize(1);
                        
                        converter_nodes[out][0].push_back(ctx_sizes[out]);
                        converter_nodes[in][0] = n_list; // forward connections from the conversion object
                        
                        std::tuple<String, int, int, std::map<String, std::vector<std::vector<int>>>> convert_obj = {converter, box->getX(), box->getY(), converter_nodes};
                        
                        patch.push_back(convert_obj);
                        edge_nodes[edge].erase(ctx_name);
                    }
                }
            }
            
            std::vector<int> node = edge_nodes.count(edge) ? edge_nodes[edge][ctx] : std::vector<int>(1, 0);
            
            if(ctx == "gui") {
                ctx_sizes[ctx]++;
                node = std::vector<int>(1, ctx_sizes[ctx]);
                gui_nodes.push_back(ctx_sizes[ctx]);
            }
            
            std::sort(node.begin(), node.end());
            node.erase(std::unique(node.begin(), node.end()), node.end());
            box_nodes[ctx][num_edges[ctx]] = node;
            num_edges[ctx]++;
        }
        
        if(box_nodes.count("gui")) {
            box->gui_nodes = gui_nodes;
        }
        
        String id = box->getState().getProperty("ID");
        patch.push_back({box->text_label.getText(), box->getX(), box->getY(), box_nodes});
        
    }
    
    return patch;
};

void Canvas::timerCallback() {
    if(!undo_manager.isPerformingUndoRedo())
        undo_manager.beginNewTransaction();
    
    MainComponent::current_main->triggerChange();

    stopTimer();
}
