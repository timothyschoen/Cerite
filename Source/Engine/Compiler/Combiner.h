#pragma once

#include <boost/algorithm/string.hpp>
#include <libtcc.h>

namespace Cerite {

class Document;
struct Combiner
{

    // Combine multiple Cerite documents into one using a node specification
    static Object combineDocuments(const std::string& name, std::vector<Object>& documents, NodeList& nodes, std::vector<int> ports);
    
    static void parseArguments(std::vector<Object>& documents);


};
}
