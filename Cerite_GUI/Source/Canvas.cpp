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
    
    viewport.setViewedComponent(this, false);
    
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
    if(!ModifierKeys::getCurrentModifiers().isRightButtonDown()) {
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
    }
    else
    {
        auto& lasso_selection = dragger.getLassoSelection();
        bool has_selection = lasso_selection.getNumSelected();
        bool multiple = lasso_selection.getNumSelected() > 1;
        
        popupmenu.clear();
        popupmenu.addItem(1, "Open", has_selection && !multiple && !lasso_selection.getSelectedItem(0)->findChildrenOfClass<Canvas>().isEmpty());
        popupmenu.addSeparator();
        popupmenu.addItem(2, "Encapsulate to new object", has_selection && multiple);
        popupmenu.addItem(3, "Encapsulate to subpatcher", has_selection && multiple);
        popupmenu.addSeparator();
        popupmenu.addItem(4,"Cut", has_selection);
        popupmenu.addItem(5,"Copy", has_selection);
        popupmenu.addItem(6,"Duplicate", has_selection);
        popupmenu.addItem(7,"Delete", has_selection);
        popupmenu.setLookAndFeel(&getLookAndFeel());
        
        int result = popupmenu.show();
        
        if(result == 1) {
            auto* new_cnv = lasso_selection.getSelectedItem(0)->findChildrenOfClass<Canvas>()[0];
            MainComponent::current_main->add_tab(new_cnv);
        }
        else if(result == 4) {
            SystemClipboard::copyTextToClipboard(copy_selection());
            remove_selection();
        }
        else if(result == 5) {
            SystemClipboard::copyTextToClipboard(copy_selection());
        }
        else if(result == 6) {
            auto copy = copy_selection();
            paste_selection(copy);
        }
        else if(result == 7) {
            remove_selection();
        }
        
        else if(result == 3) {
        std::function<String(ValueTree)> encapsulate_callback = [this](ValueTree state) {
            auto temp_cnv = Canvas(state);
            auto patch = temp_cnv.create_patch();
            auto objects = NodeConverter::create_objects(patch);
            auto formatted = NodeConverter::format_nodes(objects, Library::contexts);

            String result_path;
                
            auto returned_true = patch_save_chooser.browseForFileToSave(true);
                
            auto file = patch_save_chooser.getResult();

            if(!returned_true) return String();
            
            FileOutputStream ostream(file);
            temp_cnv.getState().writeToStream(ostream);
        
            result_path = file.getRelativePathFrom(MainComponent::home_dir.getChildFile("Saves"));
            
            return "p " + result_path.upToLastOccurrenceOf(".", false, false);
        };
            
            encapsulate(encapsulate_callback);
            
        }
        else if(result == 2) {
            std::function<String(ValueTree)> encapsulate_callback = [this](ValueTree state) {
            auto temp_cnv = Canvas(state);
            auto patch = temp_cnv.create_patch();
            auto objects = NodeConverter::create_objects(patch);
            auto formatted = NodeConverter::format_nodes(objects, Library::contexts);

            String result_path;
                
            auto returned_true = obj_save_chooser.browseForFileToSave(true);
                
            auto file = obj_save_chooser.getResult();

            if(!returned_true) return String();
            
            Uuid id;
            String name = file.getFileNameWithoutExtension() + id.toString().substring(0, 6);
            auto subpatcher = Engine::create_subpatcher(name, formatted, Library::contexts);
            String to_save = Engine::reconstruct_object(subpatcher, Library::contexts);
            file.replaceWithText(to_save);
        
                Library::refresh();
                
                return file.getRelativePathFrom(MainComponent::home_dir.getChildFile("Objects")).upToLastOccurrenceOf(".", false, false);
            };
            
            encapsulate(encapsulate_callback);
        }
    }
}

Edge* Canvas::find_edge_by_id(String ID) {
    for(auto& box : findChildrenOfClass<Box>()) {
        auto tree = box->getState().getChildWithProperty(Identifiers::edge_id, ID);
        if(auto* result = box->findObjectForTree<Edge>(tree))
            return result;
    };
    
    return nullptr;
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
    
    if(connecting_with_drag) repaint();
}

void Canvas::mouseUp(const MouseEvent& e)
{
    
    if(connecting_with_drag) {
        auto pos = e.getEventRelativeTo(this).getPosition();
        auto all_edges = get_all_edges();
        
        Edge* nearest_edge = nullptr;
        
        for(auto& edge : all_edges) {
            
            auto bounds = edge->get_canvas_bounds().expanded(150, 150);
            if(bounds.contains(pos)) {
                if(!nearest_edge) nearest_edge = edge;
                
                auto old_pos = nearest_edge->get_canvas_bounds().getCentre();
                auto new_pos = bounds.getCentre();
                nearest_edge = new_pos.getDistanceFrom(pos) < old_pos.getDistanceFrom(pos) ? edge : nearest_edge;
            }
        }

        if(nearest_edge) nearest_edge->create_connection();
        
        Edge::connecting_edge = nullptr;
        connecting_with_drag = false;
    }
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
    ValueTree copy_tree = ValueTree("Canvas");
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


Patch Canvas::create_patch(int gui_offset) {
    
    std::map<SafePointer<Edge>, Array<SafePointer<Connection>>> edge_connections;
    std::map<SafePointer<Edge>, std::map<String, std::vector<int>>> edge_nodes;
    
    std::map<String, int> ctx_sizes;
    
    ctx_sizes["gui"] = gui_offset;
    
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
    
    std::map<Box*, std::map<String, std::vector<std::vector<int>>>> box_list;
    
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
        
        String box_type = box->text_label.getText();
        
        // Store the GUI communication points in the object,
        // Add current state as argument to gui object
        if(box_nodes.count("gui") && box->graphics) {
            box->gui_nodes = gui_nodes;
            box_type = box_type.upToFirstOccurrenceOf(" ", false, false) + " " + box->graphics->get_state();
        }

        box_list[box] = box_nodes;
        patch.push_back({box_type, box->getX(), box->getY(), box_nodes});
    }
    
    
    // Linearize subpatchers
    for(auto& box : findChildrenOfClass<Box>()) {
        String box_name = box->getState().getProperty(Identifiers::box_name).toString().upToFirstOccurrenceOf(" ", false, false);
        
        if(box_name == "p") {
            auto subpatcher = encapsulate_subpatcher(box, box_list[box], box->ports, ctx_sizes);
            
            for(auto& obj : subpatcher) {
                patch.push_back(obj);
            }
            continue;
        }
    }
    
    return patch;
};

void Canvas::encapsulate(std::function<String(ValueTree)> encapsulate_func) {
    
    ValueTree copy_tree = ValueTree("Canvas");
    Array<std::pair<Connection*, ValueTree>> found_connections;
    
    int total_in_found = 0;
    int total_out_found = 0;
    
    std::vector<String> edge_ids;
    
    for(auto& con : findChildrenOfClass<Connection>()) {
        int num_points_found = 0;
        bool at_start = false;
        for(auto& box : findChildrenOfClass<Box>()) {
            if(!dragger.isSelected(box)) continue;
            for(auto& edge : box->findChildrenOfClass<Edge>()) {
                if(con->start == edge || con->end == edge) {
                    num_points_found++;
                    at_start = con->start == edge;
                }
            }
        }
        
        if(num_points_found > 1) {
            ValueTree connection_tree("Connection");
            connection_tree.copyPropertiesFrom(con->getState(), nullptr);
            found_connections.add({con, connection_tree});
        }
        else if(num_points_found == 1) {
            auto box = ValueTree(Identifiers::box);
    
            auto edge_in = at_start ? con->start : con->end;  // edge that is on our new object
            auto edge_out = at_start ? con->end : con->start; // edge that is outside our object
            
            bool is_input = edge_in->ValueTreeObject::getState().getProperty(Identifiers::edge_in);
            
            total_in_found += is_input;
            total_out_found += !is_input;
            auto ctx = edge_in->ValueTreeObject::getState().getProperty(Identifiers::edge_ctx);
            
            auto original_position = edge_out->findParentOfType<Box>()->getPosition();
            String index = " " + String(is_input ? total_in_found : total_out_found);
            box.setProperty(Identifiers::box_name, Library::subpatchers_connections[ctx][!is_input] + index, nullptr);
            box.setProperty(Identifiers::box_x, original_position.getX(), nullptr);  // Fix this!
            box.setProperty(Identifiers::box_y, original_position.getY(), nullptr);
            
            edge_ids.push_back(edge_out->ValueTreeObject::getState().getProperty(Identifiers::edge_id));
            
            ValueTree edge_tree("Edge");
            edge_tree.copyPropertiesFrom(edge_out->ValueTreeObject::getState(), nullptr);
            
            box.appendChild(edge_tree, nullptr);
            copy_tree.appendChild(box, nullptr);
            
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
    
  
    String destination = encapsulate_func(copy_tree);

    auto box = ValueTree(Identifiers::box);
    box.setProperty(Identifiers::box_name, destination, nullptr);
    
    auto new_position = dragger.getAreaOfSelectedComponents().getCentre();
    box.setProperty(Identifiers::box_x, new_position.getX(), nullptr);
    box.setProperty(Identifiers::box_y, new_position.getY(), nullptr);
    getState().appendChild(box, &undo_manager);
    
    auto* box_object = findObjectForTree<Box>(box);
    auto edges = box_object->findChildrenOfClass<Edge>();
    
    remove_selection();
    
    for(int i = 0; i < std::min<int>(edges.size(), edge_ids.size()); i++) {
        
        
        ValueTree new_connection = ValueTree(Identifiers::connection);
        new_connection.setProperty(Identifiers::start_id, edges[i]->ValueTreeObject::getState().getProperty(Identifiers::edge_id), nullptr);
        new_connection.setProperty(Identifiers::end_id, edge_ids[i], nullptr);
        getState().appendChild(new_connection, &undo_manager);
    }
    
    resized();
}

void Canvas::timerCallback() {
    if(!undo_manager.isPerformingUndoRedo())
        undo_manager.beginNewTransaction();
    
    if(auto* box = findParentOfType<Box>()) {
        auto new_ports = box->update_subpatch(getState());
        box->update_ports(new_ports);
    }

    
    MainComponent::current_main->triggerChange();

    stopTimer();
}

Array<Edge*> Canvas::get_all_edges() {
    Array<Edge*> all_edges;
    for(auto* box : findChildrenOfClass<Box>()) {
       for(auto* edge : box->findChildrenOfClass<Edge>())
           all_edges.add(edge);
    };
    
    return all_edges;
}

Patch Canvas::encapsulate_subpatcher(Box* subpatcher, std::map<String, std::vector<std::vector<int>>> box_nodes, std::map<String, std::pair<int, int>> box_ports, std::map<String, int> offset)
{

    auto* sub_cnv = subpatcher->findChildOfClass<Canvas>(0);
    auto tree = sub_cnv->getState();
    Patch new_patch = sub_cnv->create_patch(offset["gui"]);
    
    // First add an offset to all the nodes
    for(const auto& [ctx_name, ctx] : Library::contexts) {
        if(ctx_name == "gui") continue;
        for(auto& [obj_name, x, y, nodes] : new_patch) {
            for(int i = 0; i < nodes[ctx_name].size(); i++) {
                for(int j = 0; j < nodes[ctx_name][i].size(); j++) {
                    nodes[ctx_name][i][j] += offset[ctx_name];
                }
            }
        }
    }

    // Find all the in and out objects and forward their connections
    for(const auto& [ctx_name, ctx] : Library::contexts) {
        for(auto& [obj_name, x, y, nodes] : new_patch) {
            String obj_type = obj_name.upToFirstOccurrenceOf(" ", false, false);
            int idx_arg = obj_name.fromFirstOccurrenceOf(" ", false, false).upToFirstOccurrenceOf(" ", false, false).getIntValue();
            // Fix nodes for in and out objects
            if(Library::subpatchers_connections[ctx_name].contains(obj_type)) {
     
                bool input = Library::subpatchers_connections[ctx_name][0] == obj_type;
                int final_idx = (input ? idx_arg : idx_arg + box_ports[ctx_name].first) - 1;
                
                for(auto& [obj_2_name, x_2, y_2, nodes_2] : new_patch) {
                if(!nodes_2.count(ctx_name)) continue;
                for(int i = 0; i < nodes_2[ctx_name].size(); i++) {
                    for(int j = 0; j < nodes_2[ctx_name][i].size(); j++) {
                        // bad, it could change it multiple times...
                        if(nodes_2[ctx_name][i][j] == nodes[ctx_name][0][0] && obj_2_name != obj_name) {
                            nodes_2[ctx_name][i].erase(nodes_2[ctx_name][i].begin() + j);
                            nodes_2[ctx_name][i].insert(nodes_2[ctx_name][i].end(), box_nodes[ctx_name][final_idx].begin(), box_nodes[ctx_name][final_idx].end());
                        }
                    }
                }
                }
            }

        }

    }
    
    
    
    return new_patch;
    
}
