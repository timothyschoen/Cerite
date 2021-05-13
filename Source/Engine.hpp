//
//  Engine.hpp
//  Cerite_Light - ConsoleApp
//
//  Created by Timothy Schoen on 27/04/2021.
//

#pragma once
#include <JuceHeader.h>

using Variable = std::tuple<String, // Name
                            String, // Type
                            String  // Default value
>;

using Function = std::tuple<String,      // Name
                            StringArray, // Args
                            String    // Function body
>;

using Vector = std::tuple<String,       // Name
                          String,       // Element Type
                          int,          // Size
                          int,          // Dimensions (always squared!)
                          StringArray   // Default values
>;

using Ports = std::map<String, std::tuple<String, String, String>>;


enum WriteType {
    Context,
    ObjectFirst,
    ObjectSecond
};

using Object = std::tuple<String,           // Name
                          Array<String>,    // Imports
                          Array<Variable>,  // Variables
                          Array<Vector>,    // Vectors
                          Array<Function>,  // Functions
                          Ports,            // In-out specification
                          int               // Number of arguments
>;

using ObjectMap = std::map<String, Object>;
using ObjectList = Array<Object>;

using SimplifiedObject = std::pair<Object, std::map<String, std::vector<int>>>;
using SimplifiedNodes = std::vector<SimplifiedObject>;

struct Engine
{
    template <typename T>
    static T* find_by_name(Array<T>& list, String name) {
        for(int i = 0; i < list.size(); i++) {
            if(std::get<0>(list[i]) == name) {
                return list.getRawDataPointer() + i;
            }
        }
        return nullptr;
    }
    
    template <typename T>
    static void prefix_names_in_functions(Array<Function>& functions, Array<T> variables, String prefix, bool replace = false) {
        for(int i = 0; i < variables.size(); i++) {
            prefix_name_in_functions(functions, variables[i], prefix, replace);
        }
    }
    
    template <typename T>
    static void prefix_name_in_functions(Array<Function>& functions, T variable, String prefix, bool replace = false) {
            for(auto& [f_name, f_args, f_body] : functions) {
                prefix_whole_word(f_body, std::get<0>(variable), prefix, replace);
            }
    }
    
    
    static String empty_brackets(String str);
    
    static StringArray parse_tokens (StringRef target, StringRef breakCharacters);

    static String find_section(String text, String identifier, bool remove_spaces = false);

    static std::pair<int, int> match_bracket(const String& str, std::pair<char, char> selectors);

    static Object parse_object(const String& file, const StringRef name, ObjectMap& contexts, bool is_context = false);

    static void set_arguments(Object& target, const String& arguments);
    
    static String combine_objects(SimplifiedNodes& node_list, ObjectMap contexts);

    static std::pair<String, String> write_code(Object& object, const String& unique_name, WriteType write_type);
    
    static Object create_subpatcher(String new_name, SimplifiedNodes node_list, ObjectMap contexts);
    
    static String reconstruct_object(Object to_reconstruct, ObjectMap contexts);
    
    static void prefix_whole_word(String& to_process, String& to_find, String replacement, bool replace = false);
    
    static int indexOfWholeWord (StringRef string_to_search, StringRef word) noexcept;
    

    static StringArray c_preprocess(StringArray sections);
};
