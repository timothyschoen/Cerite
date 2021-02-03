#include "Function.h"

namespace Cerite {


std::string Function::toString(const Document& d) const {
    std::string funcstr;
    
    funcstr += body.toC(d) + "\n";
    // fix indentation
    boost::replace_all(funcstr, ";", ";\n\t");
    return writeInitialiser() + " {\n\t" + funcstr + "\n}";
}

std::string Function::writeInitialiser() const {
    
    std::vector<std::string> arg_list;
    boost::split(arg_list, args, boost::is_any_of(":"));
    
    for(int i = 0; i < arg_list.size(); i++) {
        if(arg_list[i].empty()) continue;
        arg_list[i] += " f_arg" + std::to_string(i);
    }
     
    
    return "void " + name + "( " + boost::join(arg_list, ",") + ")";
}
    


void Function::fixIndices(const std::vector<std::pair<int, int>> indices, const std::string& name, int type) {
    
    std::vector<int> nodes;
    
    for(auto& idx : indices) {
        if(idx.second == type)
            nodes.push_back(idx.first);
    }
    
    if(nodes.size()) {
        for(auto token : body[name]) {
            for(auto& arg : token->args) {
                // in case of variable index
                if(!arg.symbol.empty() && !std::all_of(arg.symbol.begin(), arg.symbol.end(), ::isdigit)) {
                    
                    std::string newsymbol = "((int[" + std::to_string(nodes.size()) + "]){";
                    
                    for(auto& node : nodes) {
                        newsymbol += std::to_string(node) + ", ";
                    }
                    
                    newsymbol.erase(newsymbol.end() - 2, newsymbol.end());
                    
                    newsymbol += "})[" + arg.symbol + "]";
                    
                    arg.symbol = newsymbol;
                }
                else if(token->rvalue && nodes[std::stoi(arg.symbol)] == 0 && token->args.size() == 1 ) {
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




void Function::fixIndices(const std::vector<int> nodes, const std::string& name, int type) {

    if(nodes.size()) {
        for(auto token : body[name]) {
            for(auto& arg : token->args) {
                // in case of variable index
                if(!arg.symbol.empty() && !std::all_of(arg.symbol.begin(), arg.symbol.end(), ::isdigit)) {
                    
                    std::string newsymbol = "((int[" + std::to_string(nodes.size()) + "]){";
                    
                    for(auto& node : nodes) {
                        newsymbol += std::to_string(node) + ", ";
                    }
                    
                    newsymbol.erase(newsymbol.end() - 2, newsymbol.end());
                    
                    newsymbol += "})[" + arg.symbol + "]";
                    
                    arg.symbol = newsymbol;
                }
                else if(token->rvalue && nodes[std::stoi(arg.symbol)] == 0 && token->args.size() == 1 ) {
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
