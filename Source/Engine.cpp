//
//  Engine.cpp
//  Cerite_Light - ConsoleApp
//
//  Created by Timothy Schoen on 27/04/2021.
//

#include "Engine.hpp"


// Function to clear out any code in brackets
// By replacing anything within brackets, parentheses or quotes with spaces,
// We effectively escape all that content without changing the position of everything else
// Anything inbetween parentheses or quotes is definitely not meant to be interpreted by our parser in most cases
String Engine::empty_brackets(String str) {
    
    std::vector<std::pair<char, char>> selectors = {{'[', ']'}, {'(', ')'}, {'\"', '\"'}};
    
    for(auto& selector : selectors) {
        int start = str.indexOfChar(selector.first);
        int nest = 0;
        
        if(start == -1) continue;
        start -= 1;
        
        for(int end = start + 1; end < str.length(); end++) {
            if(selector.second != selector.first) {
                if(str[end] == selector.second)     nest--;
                else if(str[end] == selector.first) nest++;
            }
            else if (str[end] == selector.first) {
                nest = 1 - nest;
            }
            
            if(nest > 0) {
                str = str.replaceSection(end, 1, " ");
            }
        }
    }
    return str;
}

// Find section of code by identifier
String Engine::find_section(String text, String identifier, bool remove_spaces)
{
    
    int start = text.indexOf(identifier + ":");
    if(start == -1) return String();
    
    start += identifier.length() + 1;
    int end = empty_brackets(text).indexOf(start, ":");
    
   
    if(end == -1) end = text.length();
    
    String result = text.substring(start, end).upToLastOccurrenceOf("\n", false, false);
    return remove_spaces ? result.removeCharacters("\n\r") : result;
}

// Find outer matching pairs of brackets in a string
std::pair<int, int> Engine::match_bracket(const String& str, std::pair<char, char> selectors) {
    int start = str.indexOfChar(selectors.first);
    int end = start + 1;
    int nest = 0;
    
    while(true) {
        if(end >= str.length()) {
            std::cerr << "Error: non-matching brackets!" << std::endl;
            break;
        }
        
        if(str[end] == selectors.second)     nest--;
        else if(str[end] == selectors.first) nest++;
        
        if(nest == -1) break;
        else           end++;
        
    }
    return {start, end};
}

// Make sure that all the passed variables and vectors will be prefixed by the struct that they are stored in
// All functions get a pointer named obj as argument which points to the object that the call belongs to
void Engine::make_local(String& target, const String& to_find) {
    int idx = 0;
    while(true) {
        int pos = idx;
        idx = target.substring(idx).indexOfWholeWord(to_find);
        if(idx == -1) break;
        idx += pos;
        
        target = target.replaceSection(idx, 0, "obj->");
        
        idx += 6;
        if(idx >= target.length()) break;
    }
}

// One-time action: parse the object to intermediate representation in a simple tuple
// Any work we can already do here is fantastic because that will limit the amount of stuff
// we need to do when we construct out patch
Object Engine::parse_object(const String& file, const StringRef name, std::map<String, Object>& contexts) {
    Object object;
    auto& [obj_name, imports, variables, vectors, functions, ports, num_args] = object;
    
    obj_name = name;
    num_args = 0;
    
    String arg_initialiser = find_section(file, "args", true);
    String variable_initialiser = find_section(file, "var", true);
    String import_initialiser = find_section(file, "import", true);
    
    auto arg_defs = StringArray::fromTokens(arg_initialiser, ",", "\\");
    auto variable_defs = StringArray::fromTokens(variable_initialiser, ",", "\\");
    auto import_defs = StringArray::fromTokens(import_initialiser, ",", "\\");
    
    for(auto& import : import_defs) {
        import = import.removeCharacters(" ");
        if(contexts.count(import)) {
            imports.add(import);
        }
        
        String port_initialiser = find_section(file, import + ".ports", true);
        auto port_defs = StringArray::fromTokens(port_initialiser, "|", "");
        
        while(port_defs.size() < 2) {
            port_defs.add("0");
        }
        ports[import] = {port_defs[0], port_defs[1]};
    }
    
    // Add arguments before variables
    for(auto& arg : arg_defs) {
        
        String type = "double";
        if(arg.contains("(")) {
            auto [paren_start, paren_end] = match_bracket(arg, {'(', ')'});
            type = arg.substring(paren_start + 1, paren_end);
            arg = arg.substring(paren_end + 1).removeCharacters(" ");
        }
        
        arg = arg.removeCharacters(" ");
        
        String default_value = "0";
        // Allow setting a default value
        if(arg.contains("=")) {
            int eq_pos = arg.indexOf("=");
            default_value = arg.substring(eq_pos + 1);
            arg = arg.substring(0, eq_pos);
        }
        variables.add({arg, type, default_value});
        num_args++;
    }
    
    
    for(auto& variable : variable_defs) {
        String type = "double";
        String default_value = "0.0";
        
        variable = variable.removeCharacters("\\");
        // Handle vectors
        if(variable.contains("[")) {
            // Handle vector types
            if(variable.contains("(")) {
                auto [paren_start, paren_end] = match_bracket(variable, {'(', ')'});
                type = variable.substring(paren_start + 1, paren_end);
                variable = variable.substring(paren_end + 1);
            }
            
            auto [brack_start, brack_end] = match_bracket(variable, {'[', ']'});
            
            int size = -1;
            if(brack_start < brack_end - 1) {
                size = variable.substring(brack_start, brack_end).getIntValue();
            }
            
            vectors.add({variable.substring(0, brack_end-1).removeCharacters(" "), type, size, {}});
            continue;
        }
        
        // Handle casting types and functions
        if(variable.contains("(")) {
            auto [paren_start, paren_end] = match_bracket(variable, {'(', ')'});
            
            // Case of setting type
            if(paren_end <= variable.length() - 3) {
                type = variable.substring(paren_start + 1, paren_end);
                variable = variable.substring(paren_end + 1).removeCharacters(" ");
            }
            // Case function
            else {
                auto args = StringArray::fromTokens(variable.substring(paren_start + 1, paren_end), ",", "");
                functions.add({variable.substring(0, paren_start).removeCharacters(" "), args, ""});
                continue;
            }
        }
        
        variable = variable.removeCharacters(" ");
        
        // Allow setting a default value
        if(variable.contains("=")) {
            int eq_pos = variable.indexOf("=");
            default_value = variable.substring(eq_pos + 1);
            variable = variable.substring(0, eq_pos);
        }
        
        variables.add({variable, type, default_value});
    }
    
    // Add function definitions from imports
    for(auto& import : imports) {
        for(auto& [f_name, f_args, f_body] : std::get<4>(contexts[import])) {
            functions.add({import + "." + f_name, f_args, ""});
        }
    }
    
    // Find function bodies
    for(auto& [func_name, args, body] : functions) {
        
        if(!file.contains(func_name + ":")) continue;
        
        body = find_section(file, func_name);
        
        for(auto& [v_name, v_type, v_init] : variables) make_local(body, v_name);
        for(auto& [v_name, v_type, v_size, v_def] : vectors) make_local(body, v_name);
        
        for(auto& import : imports) {
            auto& [ctx_name, ctx_imp, variables, vectors, functions, ports, num_args] = contexts[import];
            
            // Make sure all vector indexes of vectors taken from imports point to the relative position
            for(auto& [v_name, v_type, v_size, v_def] : vectors) {
                
                int idx = 0;
                while(true) {
                    int pos = body.substring(idx).indexOfWholeWord(ctx_name + "." + v_name);
                    if(pos == -1) break;
                    pos += idx;
                    
                    auto [brack_start, brack_end] = match_bracket(body.substring(pos), {'[', ']'});
                    brack_start += pos + 1;
                    brack_end += pos;
                    
                    String sub = body.substring(brack_start, brack_end);
                    int vector_idx = sub.getIntValue();
                    
                    String replacement = "obj->" + import + "_nodes[" + String(vector_idx) + "]";
                    
                    body = body.replaceSection(brack_start, sub.length(), replacement);
                    
                    idx = brack_start + replacement.length() + 2;
                    if(idx >= body.length()) break;
                }
            }
        }
    }
    
    return object;
    
}

void Engine::set_arguments(Object& target, const String& arguments) {
   auto tokens = StringArray::fromTokens(arguments, " ", "\'\"\\");

    auto& [name, imports, variables, vectors, functions, ports, num_args] = target;
    
    if(tokens.size() > num_args) {
        std::cout << "Warning: Too many arguments for object " + name + ", additional arguments will be ignored. \n";
    }
    
    for(int i = 0; i < std::min(num_args, tokens.size()); i++) {
        auto& [v_name, v_type, v_default] = variables.getReference(i);
        v_default = tokens[i];
        
        // Replace any arguments used to decide the number of in-out ports
        for(auto& [ctx, io] : ports) {
            io.first = io.first.replace(v_name, tokens[i]);
            io.second = io.second.replace(v_name, tokens[i]);
        }
        
    }
    
}



String Engine::combine_objects(Array<Object>& objects, std::map<String, Object> contexts, Array<Array<int>>& nodes)
{
    String object_code;
    String context_functions;
    String context_structs;
    
    String reset_code = "void reset() {\n";
    
    StringArray names;
    StringArray used_contexts;
    
    std::map<String, int> num_nodes;
    
    auto ctx_iter = contexts.begin();
    for(int c = 0; c < contexts.size(); c++, ctx_iter++) {
        
        auto& [ctx_name, ctx] = *ctx_iter;
        
        Array<int> used_nodes;
        for(int i = 0; i < nodes.size(); i++) {
            if(!std::get<1>(objects[i]).contains(ctx_name)) continue;
            
            for(int j = 0; j < nodes.size(); j++) {
                used_nodes.addIfNotAlreadyThere(nodes[i][j]);
            }
        }
        
        num_nodes[ctx_name] = used_nodes.size();
        
        for(auto& [v_name, v_type, v_size, v_def] : std::get<3>(ctx)) {
            v_size = used_nodes.size();
        }
    }
    
    for(int i = 0; i < objects.size(); i++) {
        auto& [name, imports, variables, vectors, functions, ports, num_args] = objects.getReference(i);
        
        for(auto& import : imports) {
            // Check which contexts are used
            used_contexts.addIfNotAlreadyThere(import);
            
            // Add node vector for each context to object
            Array<String> stringified;
            for(auto& num : nodes[i]) {
                stringified.add(String(num));
            }
            vectors.add({import + "_nodes", "int", stringified.size(), stringified});
        }
        
        // Create unique name
        int idx = 1;
        String unique_name = name;
        while(true) {
            unique_name = name + "_" + String(idx);
            
            if(!names.contains(unique_name)) break;
            idx++;
        }
        
        names.add(unique_name);
        
        for(auto& [f_name, f_args, f_body] : functions) {
            if(f_name.contains(".")) {
                
                auto ctx_name = f_name.substring(0, f_name.indexOf("."));
                auto func_name = f_name.substring(f_name.indexOf(".") + 1);
                
                f_name = f_name.replaceCharacter('.', '_');
                
                auto& [ctxname, ctx_imports, ctx_variables, ctx_vectors, ctx_functions, ctx_ports, ctx_args] = contexts[ctx_name];
                
                auto pos = std::find_if(
                                        ctx_functions.begin(), ctx_functions.end(),
                                        [&func_name](const Function& x) { return std::get<0>(x) == func_name;});
                
                auto& [n, m, body] = (*pos);
                body += name + "_" + ctx_name + "_" + func_name + "(&" + unique_name + ");\n";
                
            }
        }
        
        auto [struct_code, function_code] = write_code(objects.getReference(i), unique_name, (WriteType)((idx != 1)+1));
        object_code += struct_code + function_code;
        
        reset_code += unique_name + "_reset();\n";
        
    }

    String calc_code = "void calc() {\n";
    String prepare_code = "void prepare() {\n";
    
    for(auto& context : used_contexts) {
        std::get<0>(contexts[context]) += "_obj";
        
        std::tie(context_structs, context_functions) = write_code(contexts[context], context, Context);
        
        reset_code += context + "_reset();\n";
        
        calc_code += context + "_obj_calc();\n";
        prepare_code += context + "_obj_prepare();\n";
    }
    
    reset_code += "}\n\n";
    calc_code += "}\n\n";
    prepare_code += "}\n\n";
    
    return context_structs + object_code + context_functions + reset_code + calc_code + prepare_code;
}

std::pair<String, String> Engine::write_code(Object& object, const String& unique_name, WriteType write_type)
{
    String struct_code;
    String function_code;
    
    auto& [name, imports, variables, vectors, functions, ports, num_args] = object;
    
    // Fix function call names
    if(write_type != Context)
        for(auto& [f1_name, f1_args, f1_body] : functions) {
            for(auto& [f2_name, f2_args, f2_body] : functions) {
                int idx = f1_body.indexOfWholeWord(f2_name);
                if(idx < 0) continue;
                f1_body = f1_body.replaceSection(idx, 0, name + "_");
            }
        }
    
    // Create struct and function to initialize structs to default value
    struct_code += "\n\ntypedef struct {\n";
    String set_defaults = "void " + unique_name + "_reset() {\n";
    
    // Add variables to struct
    for(auto& [var_name, type, init_value] : variables) {
        struct_code += type + " " + var_name + ";\n";
        set_defaults += unique_name + "." + var_name + " = " + String(init_value) + ";\n";
    }
    
    // Add vectors to struct
    for(auto& [vec_name, type, size, vec_def] : vectors) {
        // Special case: vector of function pointers
        if(type.contains("(*)")) {
            String fixed_type = type.replace("(*)", "(*" + vec_name + "[" + String(size) + "])");
            struct_code += fixed_type + ";\n";
            continue;
        }
        
        struct_code += type + " " + vec_name + "[" + String(size) + "];\n";
        
        for(int n = 0; n < size; n++) {
            String value = vec_def.size() < size || vec_def[n].isEmpty() ? "0" : vec_def[n];
            set_defaults += unique_name + "." + vec_name + "[" + String(n) + "] = " + value + ";\n";
        }
    }
    
    struct_code += "} " + name + ";\n";
    
    set_defaults += "}\n\n";
    
    // If this object type has already been processed before, only add the initializer function
    // The same struct can be reused between objects of the same type
    if(write_type != WriteType::ObjectSecond) {
        struct_code += name + " " + unique_name + ";\n\n";
        struct_code += set_defaults;
    }
    else {
        struct_code = name + " " + unique_name + ";\n\n";
        struct_code += set_defaults;
    }
    
    for(auto& [func_name, args, body] : functions) {
        // Don't write functions for objects after the first time, they can also be reused
        if(write_type == WriteType::ObjectSecond) break;
        
        // Write function arguments
        String arg_code = "(" + (write_type != Context ? name + "* obj" : "");
        
        for(int a = 0; a < args.size(); a++) {
            arg_code += ", " + args[a].removeCharacters(" ") + " f_arg" + String(a);
        }
        arg_code += ")";
        
        // Write function
        function_code += "void " + name + "_" + func_name + arg_code + "{\n" + body + "\n}\n\n";
    }
    
    return {struct_code, function_code};
}
