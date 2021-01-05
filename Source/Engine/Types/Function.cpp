#include "Function.h"
namespace Cerite {


void Function::fixIndices(const std::vector<std::pair<int, int>> indices, const std::string& name, int type) {
    
    std::vector<int> nodes;
    
    for(auto& idx : indices) {
        if(idx.second == type)
            nodes.push_back(idx.first);
    }
    
    if(nodes.size()) {
        for(auto token : body[name]) {
            for(auto& arg : token->args) {
                if(token->rvalue && nodes[std::stoi(arg.symbol)] == 0 && token->args.size() == 1 ) {
                    token->args.clear();
                    if(type < 2) {
                        token->symbol = "0";
                    }
                    else {
                        token->symbol = "do_nothing";
                    }
                }
                    
                else {
                    arg.symbol = std::to_string(nodes[std::stoi(arg.symbol)]);
                }
                
                    
            }
        }
    }
}
}
