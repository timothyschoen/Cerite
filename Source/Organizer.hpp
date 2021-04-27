//
//  Organizer.hpp
//  Cerite_Light - ConsoleApp
//
//  Created by Timothy Schoen on 28/04/2021.
//

#pragma once
#include <JuceHeader.h>
#include "Engine.hpp"

using ObjectSpec = std::tuple<Object, std::map<String, std::vector<std::vector<int>>>, int, int>;
using NodeList = std::vector<ObjectSpec>;


using FormattedNodes = std::vector<std::vector<std::vector<int>>>;


struct Organizer
{
    
    inline static Object split_object;
    inline static Object splitsig_object;
    
    
    static std::map<String, int> count_nodes(NodeList& nodes, ContextMap ctx_map) {
        
        std::map<String, int> num_nodes;
        
        auto ctx_iter = ctx_map.begin();
        for(int c = 0; c < ctx_map.size(); c++, ctx_iter++) {
            
            for(int i = 0; i < nodes.size(); i++) {
                auto& [object, node_list, x, y] = nodes[i];
                
                Array<int> used_nodes;
                
                auto& [ctx_name, ctx] = *ctx_iter;
                if(node_list.count(ctx_name)) {
                    for(int j = 0; j < node_list[ctx_name].size(); j++) {
                        for(int k = 0; k < node_list[ctx_name][j].size(); k++) {
                            used_nodes.addIfNotAlreadyThere(node_list[ctx_name][j][k]);
                        }
                    }
                }
                num_nodes[ctx_name] = used_nodes.size();
            }
            
        }
        return num_nodes;
    }
    
    static void format_nodes(NodeList& nodes, ContextMap ctx_map) {
        
        auto num_nodes = count_nodes(nodes, ctx_map);
        
        
        for(int i = 0; i < nodes.size(); i++) {
            auto& [object, node_list, x, y] = nodes[i];
            
            
            for(auto& [ctx_name, ctx] : node_list) {
                if(ctx_name == "mna") continue;
                if(ctx_name == "dsp") {
                    format_dsp(nodes, ctx, x, y, num_nodes[ctx_name]);
                }
                else if(ctx_name == "data") {
                    format_data(nodes, ctx, x, y, num_nodes[ctx_name]);
                }
            }
            auto ctx_iter = ctx_map.begin();
            for(int c = 0; c < node_list.size(); c++, ctx_iter++) {
                
                
                
                
            }
        }
    }
    
    static void format_dsp(NodeList& list, std::vector<std::vector<int>>& nodes, int x, int y, int& num_nodes) {
        
        // TODO: make sure we only do this for outputs! log num ports on object tuple?
        for(int i = 0; i < nodes.size(); i++) {
            if(nodes[i].size() > 1) {
                std::vector<std::vector<int>> split_nodes;
                split_nodes.resize(nodes[i].size());
                
                // this won't work without arguments!!
                ObjectSpec split = {splitsig_object, {{String("dsp"), split_nodes}}, x, y};
                list.push_back(split);
                
                num_nodes++;
            }
            
        }
        
    }
    
    static void format_data(NodeList& list, std::vector<std::vector<int>> nodes, int x, int y, int& num_nodes) {
        
    }
    
};
