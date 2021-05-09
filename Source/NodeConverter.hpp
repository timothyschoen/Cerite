//
//  NodeConverter.hpp
//  Cerite_Light - ConsoleApp
//
//  Created by Timothy Schoen on 28/04/2021.
//

#pragma once
#include <JuceHeader.h>
#include "Engine.hpp"

using ObjectSpec = std::tuple<Object, std::map<String, std::vector<std::vector<int>>>, int, int>;
using NodeList = std::vector<ObjectSpec>;

using Patch = std::vector<std::tuple<String, int, int, std::map<String, std::vector<std::vector<int>>>>>;


struct NodeConverter
{
    
    inline static Object split_object;
    inline static Object splitsig_object;
    
    
    static void initialise(File object_dir, ObjectMap contexts);
    
    static void deallocate();
    
    static std::map<String, int> count_nodes(NodeList& nodes);
    
    static SimplifiedNodes format_nodes(NodeList& nodes, ObjectMap ctx_map);
    
    
    static void apply_splits(NodeList& list, int& num_nodes, int current_idx, String type, Object split_obj);
    
    static SimplifiedNodes flatten_nodes(NodeList& list);
    
    static void format_dsp(SimplifiedNodes& list);
    
    static bool checkOrder(std::vector<int> lnodes, std::vector<int> rnodes, int lstart, int rstart);
    
    static void format_data(NodeList& list);
    
    static NodeList create_objects(Patch& list);
    
};
