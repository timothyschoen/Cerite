//
//  Engine.cpp
//  Cerite_Light - ConsoleApp
//
//  Created by Timothy Schoen on 27/04/2021.
//

#include "Engine.hpp"
#include "Library.hpp"


// One-time action: parse the object to intermediate representation in a simple tuple
// Any work we can already do here is fantastic because that will limit the amount of stuff
// we need to do when we construct out patch
Object Engine::parse_object(const String& file, const StringRef name, std::map<String, Object>& contexts, bool is_context) {
    Object object;
    auto& [obj_name, imports, variables, vectors, functions, ports, num_args] = object;
    
    obj_name = name;
    num_args = 0;
    
    auto arg_defs = parse_tokens(find_section(file, "args", true), ",");
    auto variable_defs = parse_tokens(find_section(file, "var", true), ",");
    auto import_defs = parse_tokens(find_section(file, "import", true), ",");
    
    
    if(is_context) {
        num_args = find_section(file, "thread", true).removeCharacters(" ").getIntValue();
    }
    
    for(auto& import : import_defs) {
        import = import.removeCharacters(" ");
        if(contexts.count(import)) {
            imports.add(import);
        }
        
        String port_initialiser = find_section(file, import + ".ports", true);
        auto port_defs = StringArray::fromTokens(port_initialiser, "|", "");
        
        while(port_defs.size() < 3) {
            port_defs.add("0");
        }
        ports[import] = {port_defs[0].removeCharacters(" "), port_defs[1].removeCharacters(" "), port_defs[2].removeCharacters(" ")};
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
            
            // Pre-calculate math expressions for default values
            if(!default_value.containsOnly("0123456789.e-") && default_value.containsOnly("0123456789.e-()+-/*"))   {
                String parse_error;
                String backup_value = default_value;
                Expression expr(default_value, parse_error);
                default_value = String(expr.evaluate());
                
                if(parse_error.isNotEmpty()) {
                    std::cout << parse_error << std::endl;
                    default_value = backup_value;
                }
            }

            arg = arg.substring(0, eq_pos);
        }
        variables.add({arg, type, default_value});
        num_args++;
    }
    
    if(!is_context) {
        vectors.add({"argv", "Data", 0, 1, {}});
        variables.add({"argc", "int", "0"});
        for(auto& [port_ctx, port_list] : ports) {
            variables.add({port_ctx + "_inputs", "int", std::get<0>(port_list)});
        }
    }
    
    for(auto& variable : variable_defs) {
        String type = "double";
        String default_value = "0";
        
        StringArray definition;
        
        variable = variable.removeCharacters("\\");
        // Handle vectors
        if(variable.contains("[")) {
            // Handle vector types
            if(variable.contains("(")) {
                auto [paren_start, paren_end] = match_bracket(variable, {'(', ')'});
                type = variable.substring(paren_start + 1, paren_end);
                variable = variable.substring(paren_end + 1);
            }
            // Parse inline definitions
            if(variable.contains("{")) {
                auto [brack_start, brack_end] = match_bracket(variable, {'{', '}'});
                definition = parse_tokens(variable.substring(brack_start + 1, brack_end), ",");
                for(auto& item : definition) item.removeCharacters(" ");
            }
            auto [brack_start, brack_end] = match_bracket(variable, {'[', ']'});
            
            int size = -1;
            if(brack_start < brack_end - 1) {
                size = variable.substring(brack_start + 1, brack_end).getIntValue();
            }
            
            // Only 2d for now, might support more dimensions later
            int dims = variable.contains("][") + 1;
            
            vectors.add({variable.substring(0, brack_start).removeCharacters(" "), type, size, dims, definition});
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
                auto args = parse_tokens(variable.substring(paren_start + 1, paren_end), ",");
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
        
        if(!is_context) {
            for(auto& [v_name, v_type, v_init] : variables) prefix_whole_word(body, v_name, "obj->");
            for(auto& [v_name, v_type, v_size, v_dims, v_def] : vectors) prefix_whole_word(body, v_name, "obj->");
        }
    }
    
    if(!is_context) {
        for(int i = functions.size()-1; i >= 0; i--) {
            auto [func_name, args, body] = functions[i];
            if(body.isEmpty()) {
                functions.remove(i);
            }
        }
    }

    return object;
    
}

void Engine::set_arguments(Object& target, const String& arguments) {
   auto tokens = StringArray::fromTokens(arguments, " ", "\'\"\\");

    auto& [name, imports, variables, vectors, functions, ports, num_args] = target;
    
    for(int i = 0; i < std::min(num_args, tokens.size()); i++) {
        auto& [v_name, v_type, v_default] = variables.getReference(i);
        v_default = tokens[i];
        
        // Replace any arguments used to decide the number of in-out ports
        for(auto& [ctx, io] : ports) {
            std::get<0>(io) = std::get<0>(io).replace(v_name, tokens[i]);
            std::get<1>(io) = std::get<1>(io).replace(v_name, tokens[i]);
            std::get<2>(io) = std::get<2>(io).replace(v_name, tokens[i]);
        }
        
    }
    
    if(num_args < tokens.size()) {
        StringArray definition;
        
        for(int i = num_args; i < tokens.size(); i++) {
            if(tokens[i].containsOnly("0123456789.,e-")) {
                definition.add("(Data){tNumber, " + tokens[i] + ", \"\", 0, 0}");
            }
            else {
                definition.add("(Data){tString, 0, strdup(\""+ tokens[i] +"\"), 0, 0}");
            }
        }
        int argc = tokens.size() - num_args;
    
        
        if(auto* found = find_by_name(vectors, "argv")) {
            auto& [vec_name, vec_type, vec_size, vec_dims, vec_def] = *found;
            vec_size = argc;
            vec_def = definition;
        }
           
       if(auto* found = find_by_name(variables, "argc")) {
           auto& [var_name, var_type, var_def] = *found;
           var_def = String(argc);
       }
        
        
        for(auto& [ctx, io] : ports) {
            std::get<0>(io) = std::get<0>(io).replace("argc", String(argc));
            std::get<1>(io)= std::get<1>(io).replace("argc", String(argc));
            std::get<2>(io)= std::get<2>(io).replace("argc", String(argc));
        }
    }
    // Parse math expressions in ports
    for(auto& [ctx, io] : ports) {
        bool parse_first = !std::get<0>(io).containsOnly("0123456789");
        bool parse_second = !std::get<1>(io).containsOnly("0123456789");
        //bool parse_third = !std::get<2>(io).containsOnly("0123456789");
        
        String parse_error_1, parse_error_2;
        
        if(parse_first)   {
            Expression expr(std::get<0>(io), parse_error_1);
            std::get<0>(io) = String((int)expr.evaluate());
            
            if(parse_error_1.isNotEmpty()) std::cout << parse_error_1 << std::endl;
        }
        if(parse_second)   {
            Expression expr(std::get<1>(io), parse_error_2);
            std::get<1>(io) = String((int)expr.evaluate());
            
            if(parse_error_2.isNotEmpty()) std::cout << parse_error_2 << std::endl;
        }
        
        if(auto* found = find_by_name(variables, ctx + "_inputs")) {
            std::get<2>(*found) =  std::get<0>(io);
        }
    }
    
   
    
}



String Engine::combine_objects(SimplifiedNodes& node_list, std::map<String, Object> contexts)
{
    
    String object_code;
    String context_functions;
    String context_structs;
    
    String reset_code = "void reset() {\n";
    
    StringArray names;
    StringArray used_contexts;
    
    std::map<String, int> num_nodes;
   
    // Count the size of each context, defined by the number of inputs and output that are within that context
    auto ctx_iter = contexts.begin();
    for(int c = 0; c < contexts.size(); c++, ctx_iter++) {
        int internal_nodes = 0;
        auto& [ctx_name, ctx] = *ctx_iter;
        
        Array<int> used_nodes;
        for(int i = 0; i < node_list.size(); i++) {
            auto&[obj, nodes] = node_list[i];
            
            int num_internal = std::get<2>(std::get<5>(obj)[ctx_name]).getIntValue();
            internal_nodes += num_internal;
            

            if(!std::get<1>(obj).contains(ctx_name)) continue;
            
            for(int j = 0; j < nodes[ctx_name].size(); j++) {
                used_nodes.addIfNotAlreadyThere(nodes[ctx_name][j]);
            }
        }
        
        if(used_nodes.size()) {
            num_nodes[ctx_name] = *std::max_element(used_nodes.begin(),  used_nodes.end());
        }
        else {
            num_nodes[ctx_name] = 0;
        }
        
        for(auto& [v_name, v_type, v_size, v_dims, v_def] : std::get<3>(ctx)) {
            if(v_size < 1) v_size = num_nodes[ctx_name] + internal_nodes + 1;
        }
    }
    
    for(int i = 0; i < node_list.size(); i++) {
        auto&[obj, nodes] = node_list[i];
        
        auto& [name, imports, variables, vectors, functions, ports, num_args] = obj;
        
        for(auto& import : imports) {
            // Check which contexts are used
            used_contexts.addIfNotAlreadyThere(import);
            
            int num_internal = std::get<2>(ports[import]).getIntValue();

            // Add node vector for each context to object
            Array<String> stringified;
            for(auto& num : nodes[import]) {
                stringified.add(String(num));
            }
            // Add node numbers for internal nodes
            for(int n = 0; n < num_internal; n++) {
                stringified.add(String(++num_nodes[import]));
            }
            
            vectors.add({import + "_nodes", "int", stringified.size(), 1, stringified});
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
            // Check for context member functions that this object wants to contribute to
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
        
        auto [struct_code, function_code] = write_code(obj, unique_name, (WriteType)((idx != 1)+1));
        object_code += struct_code + function_code;
        
        reset_code += unique_name + "_reset();\n";
        
    }

    
    int max_threads = 0;
    for(auto& ctx : used_contexts) {
        max_threads = std::max(std::get<6>(contexts[ctx]), max_threads);
    }
    max_threads++;
    
    
    String calc_code;
    for(int t = 0; t < max_threads; t++) {
        calc_code += "void thread_" + String(t) + "_calc() {\n";
        
        for(auto& ctx : used_contexts) {
            if(std::get<6>(contexts[ctx]) == t) {
                calc_code += ctx + "_obj_calc();\n";
            }
        }
        calc_code += "}\n\n";
    }
    
    String prepare_code = "void prepare() {\n";
    String free_code = "void free() {\n";
    for(auto& context : used_contexts) {
        auto& [ctx_name, ctx_imports, ctx_variables, ctx_vectors, ctx_functions, ctx_ports, ctx_num_args] = contexts[context];
        
        ctx_name += "_obj";
        
        String ctx_struct, ctx_func;
        std::tie(ctx_struct, ctx_func) = write_code(contexts[context], context, Context);
        
        context_functions += ctx_func;
        context_structs += ctx_struct;
        

        reset_code += context + "_reset();\n";
        
        if(find_by_name(ctx_functions, "free")) {
            free_code += context + "_obj_free();\n";
        }
        if(find_by_name(ctx_functions, "prepare")) {
            prepare_code += context + "_obj_prepare();\n";
        }
    }
    free_code += "}\n\n";
    reset_code += "}\n\n";

    prepare_code += "}\n\n";
    
    
    String audio_func;
    
    
    if(used_contexts.contains("dsp")) {
        audio_func += "double* get_audio_output() { "
                      "  return dsp.audio_output;   "
                      "}                            ";
    }
    
    String gui_funcs;
    if(used_contexts.contains("gui")) {
        gui_funcs +=    "void gui_external_register(int f_arg0, void(*f_arg1)(void*, Data, int)) {\n"
                        "  gui.send[f_arg0] = f_arg1;\n"
                        "}\n\n"
                        "void gui_external_send(int f_arg0, Data data) {\n"
                        "gui.send[f_arg0](gui.objects[f_arg0], data, f_arg0);\n"
                        "}\n";
    }
    
    return context_structs + object_code + context_functions + reset_code + calc_code + prepare_code + free_code + audio_func + gui_funcs;
}


std::pair<String, String> Engine::write_code(Object& object, const String& unique_name, WriteType write_type)
{
    String struct_code;
    String function_code;
    
    auto& [name, imports, variables, vectors, functions, ports, num_args] = object;
        
    prefix_names_in_functions(functions, functions, name + "_");
    
    if(write_type == Context) {
        // Will this be okay in subpatchers?
        String real_name = name.upToFirstOccurrenceOf("_", false, false);
        prefix_names_in_functions(functions, variables, real_name + ".");
        prefix_names_in_functions(functions, vectors, real_name + ".");
    }
    
    // Create struct and function to initialize structs to default value
    struct_code += "\n\ntypedef struct {\n";
    String set_defaults = "void " + unique_name + "_reset() {\n";
    
    // Add variables to struct
    for(auto& [var_name, type, init_value] : variables) {
        struct_code += type + " " + var_name + ";\n";
        
        if(type == "int" || type == "double" || type == "float")
        set_defaults += unique_name + "." + var_name + " = " + String(init_value) + ";\n";
    }
    
    // Add vectors to struct
    for(auto& [vec_name, type, size, vec_dims, vec_def] : vectors) {
        // Special case: vector of function pointers
        if(type.contains("(*)")) {
            String fixed_type = type.replace("(*)", "(*" + vec_name + "[" + String(size) + "])");
            struct_code += fixed_type + ";\n";
            continue;
        }
        
        struct_code += type + " " + vec_name + "[" + String(size) + "]";
        for(int i = 1; i < vec_dims; i++) struct_code += "[" + String(size) + "]";
        
        struct_code += ";\n";
        
        
        // FIX FOR MULTI-DIM VECTORS!
        for(int n = 0; n < pow(size, vec_dims); n++) {
            
            String value = vec_def.size() <= n || vec_def[n].isEmpty() ? "0" : vec_def[n];
            
            // Not for more than 2d for now...
            int dim = n / size;
            int idx = n % size;
            
            String subscript = vec_dims == 1 ? "[" + String(n) + "]" : "[" + String(dim) + "]" + "[" + String(idx) + "]";
            
            
            set_defaults += unique_name + "." + vec_name + subscript + " = " + value + ";\n";
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


// Attempt to allow subpatches
Object Engine::create_subpatcher(String new_name, SimplifiedNodes node_list, ObjectMap contexts)
{
    
    
    StringArray used_contexts;
    StringArray used_names;
    Object new_object;
    
    std::map<std::pair<String, StringArray>, String> function_map;
    
    auto& [name, imports, variables, vectors, functions, ports, num_args] = new_object;
    
    name = new_name;
    
    std::map<String, int> num_nodes;
    // Count the size of each context, defined by the number of inputs and output that are within that context
    auto ctx_iter = contexts.begin();
    for(int c = 0; c < contexts.size(); c++, ctx_iter++) {
        int internal_nodes = 0;
        auto& [ctx_name, ctx] = *ctx_iter;
        
        Array<int> used_nodes;
        for(int i = 0; i < node_list.size(); i++) {
            auto&[obj, nodes] = node_list[i];
            
            if(!std::get<1>(obj).contains(ctx_name)) continue;

            internal_nodes += std::get<2>(std::get<5>(obj)[ctx_name]).getIntValue();
            
            for(int j = 0; j < nodes[ctx_name].size(); j++) {
                used_nodes.addIfNotAlreadyThere(nodes[ctx_name][j]);
            }
        }
        
        int external_nodes = used_nodes.isEmpty() ? 0 : *std::max_element(used_nodes.begin(),  used_nodes.end());
        num_nodes[ctx_name] = external_nodes + internal_nodes;
        if(num_nodes[ctx_name] == 0) num_nodes.erase(ctx_name);
    }
    
    for(auto& [ctx, size] : num_nodes) {
        ports[ctx] = {"0", "0", String(size)};
    }
    
    
    // Run preprocessor to expand macros
    {
        StringArray all_functions;
        for(int i = 0; i < node_list.size(); i++) {
            auto&[obj, nodes] = node_list[i];
            auto& [obj_name, obj_imports, obj_variables, obj_vectors, obj_functions, obj_ports, obj_num_args] = obj;
            for(auto& [f_name, f_args, f_body] : obj_functions) {
                
                all_functions.add(f_body);
            }
        }
        
        all_functions = c_preprocess(all_functions);
        
        int num_funcs = 0;
        for(int i = 0; i < node_list.size(); i++) {
            auto&[obj, nodes] = node_list[i];
            auto& [obj_name, obj_imports, obj_variables, obj_vectors, obj_functions, obj_ports, obj_num_args] = obj;
            for(auto& [f_name, f_args, f_body] : obj_functions) {
                f_body = all_functions[num_funcs];
                num_funcs++;
            }
        }
    }
    
    for(int i = 0; i < node_list.size(); i++) {
        auto&[obj, nodes] = node_list[i];
        
        auto& [obj_name, obj_imports, obj_variables, obj_vectors, obj_functions, obj_ports, obj_num_args] = obj;
        
        String obj_type = obj_name.upToFirstOccurrenceOf("_", false, false);
        
        if(Library::subpatchers_connections[obj_imports[0]].contains(obj_type)) {
            std::get<0>(ports[obj_imports[0]]) = String(std::get<0>(ports[obj_imports[0]]).getIntValue() + (Library::subpatchers_connections[obj_imports[0]][0] == obj_type));
            std::get<1>(ports[obj_imports[0]]) = String(std::get<1>(ports[obj_imports[0]]).getIntValue() + (Library::subpatchers_connections[obj_imports[0]][1] == obj_type));
        }
        
        // Give unique names to objects
        int suffix = 1;
        while(used_names.contains(obj_name + String(suffix))) {
            suffix++;
        }
              
        obj_name += String(suffix);
        used_names.add(obj_name);
        
        for(auto& import : obj_imports) {
            // Check which contexts are used
            imports.addIfNotAlreadyThere(import);
            
            // Add node vector for each context to object
            Array<String> stringified;
            for(auto& num : nodes[import]) {
                stringified.add(String(num));
            }
            obj_vectors.add({import + "_nodes", "int", stringified.size(), 1, stringified});
        }
        
        
        for(auto& variable : obj_variables) {
            auto& [var_name, var_type, var_default] = variable;
            
            prefix_name_in_functions(obj_functions, variable, new_name + "_" + obj_name + "_");
            
            var_name = new_name + "_" + obj_name  + "_" + var_name;
            variables.add(variable);
        }
    
        
        for(auto& vector : obj_vectors) {
            auto&[vec_name, vec_type, vec_size, vec_dims, vec_def] = vector;
            
            prefix_name_in_functions(obj_functions, vector, new_name + "_" + obj_name + "_");
            
            vec_name = new_name + "_" + obj_name + "_" +  vec_name;
            vectors.add(vector);
        }
        
        for(auto& function : obj_functions) {
            auto& [f_name, f_args, f_body] = function;
            for(auto& import : obj_imports) {
                String to_find = import + ".";
                prefix_whole_word(f_body, import, new_name + "_");
            }
            
            prefix_name_in_functions(obj_functions, function, new_name + "_" + obj_name + "_");

            if(f_name.contains(".")) {
                function_map[{f_name, f_args}] += f_body;
            }
            else {
                f_name = new_name + "_" + obj_name + "_" + f_name;
                functions.add(function);
            }
        }
    }
    
    for(auto& import : imports) {
        variables.add({new_name + "_" + import, import + "_obj", ""});
    }
    
    for(auto& [f_key, f_body] : function_map) {
        auto& [f_name, f_args] = f_key;
       
        if(f_name.contains("prepare")) {
            String ctx = f_name.upToFirstOccurrenceOf(".", false, false);
            for(auto& [v_name, v_type, v_value] : std::get<2>(contexts[ctx])) {
                if(v_type == "int" || v_type == "double" || v_type == "float")
                f_body += new_name + "_" + ctx + "." + v_name + " = " + String(v_value) + ";\n";
            }
        }
        
        functions.add({f_name, f_args, f_body});
    }

    return new_object;
}

// Recreate object format from parsed object
String Engine::reconstruct_object(Object to_reconstruct, ObjectMap contexts)
{
    String include_initializers = "import: ";
    String variable_initialize = "var: ";
    
    String function_bodies;
    
    String ports_initializers;
    
    auto& [name, imports, variables, vectors, functions, ports, num_args] = to_reconstruct;
    
    for(auto& import : imports) {
        include_initializers += import + ", ";
    }
    
    for(auto& [ctx, port] : ports) {
        ports_initializers += ctx + ".ports: ";
        ports_initializers += std::get<0>(port) + " | ";
        ports_initializers += std::get<1>(port) + " | ";
        ports_initializers += std::get<2>(port) + "\n\n";
    }
    
    for(auto& [var_name, var_type, var_default] : variables) {
        String type = var_type == "double" ? "" : "(" + var_type + ")";
        String definition = var_default.isEmpty() ? "" : " = " + var_default;
        variable_initialize += type + var_name + definition + ", ";
    }
    
    for(auto& [vec_name, vec_type, vec_size, vec_dims, vec_def] : vectors) {
        
        if(vec_size == 0) continue;
        // does this work for funcptr vectors???
        String type = vec_type == "double" ? "" : "(" + vec_type + ")";
        String size = "[" + String(vec_size) +"]";
        
        String definition = vec_def.size() == 0 ? "" : " = {";
        definition += vec_def.joinIntoString(", ");
        definition += vec_def.size() == 0 ? "" : "}";
        
        variable_initialize += type + vec_name + size + definition + ", ";
    }
    
    for(auto& [func_name, func_args, func_body] : functions) {
        
        func_body = func_body.replace("obj->", "");
        if(!func_name.contains(".")) {
            variable_initialize += func_name + "(" + func_args.joinIntoString(", ") + "), ";
        }
        
        if(func_body.isNotEmpty()) {
            function_bodies += func_name + ":\n\t" + func_body + "\n";
        }
    }
    
    include_initializers = include_initializers.upToLastOccurrenceOf(",", false, false) + "\n\n";
    variable_initialize = variable_initialize.upToLastOccurrenceOf(",", false, false) + "\n\n";

    String final_format = include_initializers + variable_initialize + ports_initializers + function_bodies;
    
    File("/Users/timschoen/Documents/obj_export_test.obj").replaceWithText(final_format);
    
    return final_format;
}




// String utility functions

// Copy of the indexOfWholeWord method of juce::String, but with "_" recognized as part of the word.
int Engine::indexOfWholeWord (StringRef string_to_search, StringRef word) noexcept
{
    std::function<bool(char)> is_letter = [](char in){
        return String("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_\0").containsChar(in);
    };
    
    if (word.isNotEmpty())
    {
        auto t = string_to_search.text;
        auto wordLen = word.length();
        auto end = (int) t.length() - wordLen;

        for (int i = 0; i <= end; ++i)
        {
            if (t.compareUpTo (word.text, wordLen) == 0
                  && (i == 0 || !is_letter(*(t - 1)))
                && (!is_letter(*(t + wordLen))))
                return i;

            ++t;
        }
    }

    return -1;
}

void Engine::prefix_whole_word(String& to_process, String& to_find, String replacement, bool replace)
{
    int i = 0;
    int offset = 0;
    while ((i = indexOfWholeWord(to_process.substring(offset), to_find)) >= 0)
    {
        i += offset;
        if(i >= to_process.length()) break;
        to_process = to_process.replaceSection (i, replace ? replacement.length() : 0, replacement);
        offset = i + replacement.length() + to_find.length();
    }
}

// Function to clear out any code in brackets
// By replacing anything within brackets, parentheses or quotes with spaces,
// We effectively escape all that content without changing the position of everything else
// Anything inbetween parentheses or quotes is definitely not meant to be interpreted by our parser in most cases
String Engine::empty_brackets(String str) {
    
    std::vector<std::pair<char, char>> selectors = {{'[', ']'}, {'(', ')'}, {'{', '}'}};
    
    std::string input = str.toStdString();
    std::string output = input;
    for(auto& selector : selectors) {
        int nest_in = 0;
        int nest_out = 0;
        
        for(int end = 1; end < input.length() - 1; end++) {
            if(input[end] == selector.first)
                nest_in++;
            if(input[end + 1] == selector.second)
                nest_out++;
            
  
            if(nest_in > nest_out)
                output[end + 1] = ' ';
        }
    }
    
    String result(output);
    
    for(int i = 0; i < result.length(); i++) {
        int comment_start = result.indexOf(i, "//");
        if(comment_start == -1) break;
        
        int comment_end = result.indexOf(comment_start, "\n") + 1;
        String whitespace = String::repeatedString(" ", comment_end - comment_start);
        result.replaceSection(comment_start, comment_end, whitespace);
        
        i = comment_end - 1;
    }
    
    for(int i = 0; i < result.length(); i++) {
        int comment_start = result.indexOf("/*");
        if(comment_start == -1) break;
        
        int comment_end = result.indexOf(comment_start, "*/") + 2;
        String whitespace = String::repeatedString(" ", comment_end - comment_start);
        result.replaceSection(comment_start, comment_end, whitespace);
        
        i = comment_end - 1;
    }
    
    return result;
}


StringArray Engine::parse_tokens (StringRef target, StringRef breakWord)
{
    
    int num = 0;
    
    StringArray strings;
    

    if (target.isNotEmpty())
    {
        String blanked_string = Engine::empty_brackets(target);
        StringRef blanked(blanked_string);
        
        for (auto t = blanked.text;;)
        {
            
            auto tokenEnd = CharacterFunctions::find(t, breakWord.text);
            
            long offset = (t - blanked.text);
            long length = (tokenEnd - t);
            
            strings.add (String (target.text.getAddress() + offset, length));
            ++num;

            if (tokenEnd.isEmpty())
                break;

            t = tokenEnd.getAddress() + breakWord.length();
        }
    }

    return strings;
}


// Find section of code by identifier
String Engine::find_section(String text, String identifier, bool remove_spaces)
{
    int start = text.indexOf(identifier + ":");
    if(start == -1) return String();
    
    start += identifier.length() + 1;
    int end = empty_brackets(text).indexOf(start, ":");
    
   
    if(end == -1)  {
        return remove_spaces ? text.substring(start).removeCharacters("\n\r") : text.substring(start);
    }
    
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

StringArray Engine::c_preprocess(StringArray sections) {
    
    String libcerite = "/Users/timschoen/Documents/Cerite/.exec/libcerite.h";
    
    ChildProcess preprocessor;
    
    // Add include
    sections.insert(0, "#define SUBPATCHER \n #include \"" + libcerite + "\"\n");
    
    auto input = sections.joinIntoString("\n@ENDOFINCLUDE@\n");
    
    
    auto c_file = File::createTempFile(".c");
    c_file.replaceWithText(input);
    preprocessor.start("gcc -E " + c_file.getFullPathName());
    
    String preprocessed = preprocessor.readAllProcessOutput();
    auto output = preprocessed.fromFirstOccurrenceOf("@ENDOFINCLUDE@", false, false);
    
    auto lines = StringArray::fromLines(output);
    
    for(int i = lines.size() - 1; i >= 0; i--) {
        if(lines[i].trimStart().startsWithChar('#'))
            lines.remove(i);
    }
    
    
    
    String in = lines.joinIntoString("\n");
    String to_find = "obj->";
    prefix_whole_word(in, to_find, "", true);
   
    auto result = parse_tokens(in, "@ENDOFINCLUDE@");
    

    
    return result;
}
