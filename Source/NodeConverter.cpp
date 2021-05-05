//
//  NodeConverter.cpp
//  Cerite_Light - ConsoleApp
//
//  Created by Timothy Schoen on 28/04/2021.
//

#include "NodeConverter.hpp"
#include "Library.hpp"

void NodeConverter::initialise(File object_dir, ObjectMap contexts) {
    split_object = Engine::parse_object(object_dir.getChildFile("Glue/split.obj").loadFileAsString(), "split", contexts);
    
    splitsig_object = Engine::parse_object(object_dir.getChildFile("Signal/split~.obj").loadFileAsString(), "splitsig", contexts);
}
void NodeConverter::deallocate() {
    split_object = Object();
    splitsig_object = Object();
}

std::map<String, int> NodeConverter::count_nodes(NodeList& nodes, ObjectMap ctx_map) {
    
    std::map<String, int> num_nodes;
    
    auto ctx_iter = ctx_map.begin();
    for(int c = 0; c < ctx_map.size(); c++, ctx_iter++) {
        Array<int> used_nodes;
        auto& [ctx_name, ctx] = *ctx_iter;
        
        for(int i = 0; i < nodes.size(); i++) {
            auto& [object, node_list, x, y] = nodes[i];
            
            if(node_list.count(ctx_name)) {
                for(int j = 0; j < node_list[ctx_name].size(); j++) {
                    for(int k = 0; k < node_list[ctx_name][j].size(); k++) {
                        used_nodes.addIfNotAlreadyThere(node_list[ctx_name][j][k]);
                    }
                }
            }
        }
        num_nodes[ctx_name] = used_nodes.size();
    }
    return num_nodes;
}

SimplifiedNodes NodeConverter::format_nodes(NodeList& nodes, ObjectMap ctx_map) {
    
    auto num_nodes = count_nodes(nodes, ctx_map);
    
    bool has_dsp = false;
    bool has_data = false;
    
    for(int i = 0; i < nodes.size(); i++) {
        auto& [object, node_list, x, y] = nodes[i];
        
        //if(node_list.count("mna"))
        
        if(node_list.count("dsp")) {
            has_dsp = true;
            apply_splits(nodes, num_nodes["dsp"], i, "dsp", splitsig_object);
        }
        
        if(node_list.count("data")) {
            has_data = true;
            apply_splits(nodes, num_nodes["data"], i, "data", split_object);
            
        }
    }
    
    if(has_data) {
        format_data(nodes);
    }
    
    auto flat_nodes = flatten_nodes(nodes);
    
    if(has_dsp) {
        format_dsp(flat_nodes);
    }
    
    
    return flat_nodes;
}


void NodeConverter::apply_splits(NodeList& list, int& num_nodes, int current_idx, String type, Object split_obj) {
    auto& [object, node_map, x, y] = list[current_idx];
    
    auto& nodes = node_map[type];
    
    Ports ports = std::get<5>(object);
    int num_in = ports[type].first.getIntValue();
    
    for(int i = num_in; i < nodes.size(); i++) {
        if(nodes[i].size() > 1) {
            std::vector<std::vector<int>> split_nodes;
            split_nodes.resize(nodes[i].size() + 1);
            
            split_nodes[0] = {num_nodes};
            
            for(int n = 0; n < nodes[i].size(); n++) {
                split_nodes[n + 1] = {nodes[i][n]};
            }
            
            // Kind of messy but we need to actually find the object that this node points to...
            std::vector<int> target_indices(nodes[i].size());
            for(int n = 0; n < nodes[i].size(); n++) {
                for(int j = 0; j < list.size(); j++) {
                    auto& [object_j, node_map_j, x_j, y_j] = list[j];
                    
                    for(int k = 0; k < node_map_j["data"].size(); k++) {
                        if(std::count(node_map_j["data"][k].begin(), node_map_j["data"][k].end(), nodes[i][n])) {
                            target_indices[n] = j;
                        }
                    }
                }
            }
            
            // Checks if the split will call the objects in right-to-left order
            for (int x = 0; x < target_indices.size(); x++)
            for(int j = 0; j < target_indices.size(); j++) {
                auto& [object_j, node_map_j, x_j, y_j] = list[target_indices[j]];
                
                for(int k = j + 1; k < target_indices.size(); k++) {
                    auto& [object_k, node_map_k, x_k, y_k] = list[target_indices[k]];
                    
                    if(x_j < x_k) {
                        std::swap(target_indices[j], target_indices[k]);
                        std::swap(split_nodes[j + 1], split_nodes[k + 1]);
                    }
                }
            }
            
            ObjectSpec split = {split_obj, {{String(type), split_nodes}}, x, y};
            
            Engine::set_arguments(std::get<0>(split), String(nodes[i].size()));
            
            nodes[i] = {num_nodes++};
            
            list.push_back(split);
        }
    }
}

SimplifiedNodes NodeConverter::flatten_nodes(NodeList& list) {
    SimplifiedNodes flat_nodes;
    
    for(int i = 0; i < list.size(); i++) {
        auto& [object, node_map, x, y] = list[i];
        std::map<String, std::vector<int>> new_nodes;
        
        for(auto& [ctx_name, nodes] : node_map) {
            new_nodes[ctx_name].resize(nodes.size());
            
            for(int n = 0; n < nodes.size(); n++) {
                if(nodes[n].size() > 1)
                    std::cout << "Error while flattening nodes!" << std::endl;
                if(nodes[n].size() == 0) continue;
                
                new_nodes[ctx_name][n] = nodes[n][0];
            }
        }
        flat_nodes.push_back({object, new_nodes});
    }
    return flat_nodes;
}

void NodeConverter::format_dsp(SimplifiedNodes& list) {
    
    // Sorting algorithm
    // Sorts based on whether the input of an earlier component is dependent on the output of a later component
    // This will place the dsp objects in correct signal graph order
    // Currently the order will be random in case of feedback: I think this is not an issue, there will be a single sample of delay either way
    for (int x = 0; x < list.size(); x++)
    for (int i = 0; i < list.size(); i++) {
        
        auto& [object_i, node_map_i] = list[i];
        
        if(node_map_i.count("dsp") == 0) continue;
        
        for (int j = i + 1; j < list.size(); j++) {
            
            auto& [object_j, node_map_j] = list[j];
            
            if(node_map_j.count("dsp") == 0) continue;
            
            int istart = std::get<5>(object_i)["dsp"].first.getIntValue();
            int jstart = std::get<5>(object_j)["dsp"].first.getIntValue();
            
            if(checkOrder(node_map_i["dsp"], node_map_j["dsp"], istart, jstart)) {
                std::swap(list[j], list[i]);
            }
        }
    }
}
bool NodeConverter::checkOrder(std::vector<int> lnodes, std::vector<int> rnodes, int lstart, int rstart) {
    for(int k = rstart; k < rnodes.size(); k++) {
        if(rnodes[k] == 0) continue;
        
        for(int j = 0; j < lstart; j++) {
            if(lnodes[j] == rnodes[k] && lnodes[j] != 0) {
                return true;
            }
        }
    }
    return false;
}


void NodeConverter::format_data(NodeList& list) {
    for (int x = 0; x < list.size(); x++)
    for(int i = 0; i < list.size(); i++) {
        auto& [object_i, node_map_i, x_i, y_i] = list[i];
        
        if(node_map_i.count("data") == 0) continue;
        for(int j = i + 1; j < list.size(); j++) {
            
            auto& [object_j, node_map_j, x_j, y_j] = list[j];
            
            if(node_map_j.count("data") == 0) continue;
            
            if(x_i <= x_j) {
                std::swap(list[j], list[i]);
            }
        }
    }
}

NodeList NodeConverter::create_objects(Patch& list) {
    NodeList result;
    
    for(auto& [name, id, x, y, nodes] : list) {
        
        auto tokens = StringArray::fromTokens(name, " ", "");
        auto object = Library::objects[tokens[0]];
        
        tokens.remove(0);
        
        Engine::set_arguments(object, tokens.joinIntoString(" "));
        
        
        result.push_back({object, nodes, x, y});
    }
    
    return result;
    
}
