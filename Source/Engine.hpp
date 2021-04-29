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
                          StringArray   // Default values
>;

using Ports = std::map<String, std::pair<String, String>>;


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
    static T& find_by_name(Array<T>& list, String name) {
        for(int i = 0; i < list.size(); i++) {
            if(std::get<0>(list[i]) == name) {
                return list.getReference(i);
            }
        }
        
        assert(false);
    }
    
    static String empty_brackets(String str);

    static String find_section(String text, String identifier, bool remove_spaces = false);

    static std::pair<int, int> match_bracket(const String& str, std::pair<char, char> selectors);

    static void make_local(String& target, const String& to_find);

    static Object parse_object(const String& file, const StringRef name, ObjectMap& contexts, bool is_context = false);

    static void set_arguments(Object& target, const String& arguments);
    
    static String combine_objects(SimplifiedNodes& node_list, ObjectMap contexts);

    static std::pair<String, String> write_code(Object& object, const String& unique_name, WriteType write_type);
};
