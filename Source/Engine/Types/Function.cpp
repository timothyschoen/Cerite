#include "Function.h"
namespace Cerite {


void Function::fixIndices(const std::vector<std::pair<int, int>> indices, const std::string& name) {
    
    if(indices.size()) {
        for(auto token : body[name]) {
            for(auto& arg : token->args) {
                if(token->rvalue && indices[std::stoi(arg.symbol)].first == 0 && token->args.size() == 1) {
                    token->args.clear();
                    token->symbol = "0";
                }
                    
                else {
                    arg.symbol = std::to_string(indices[std::stoi(arg.symbol)].first);
                }
                
                    
            }
        }
    }
}
}
