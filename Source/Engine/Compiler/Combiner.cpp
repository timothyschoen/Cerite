#include "../Interface/Document.h"
#include "Combiner.h"
#include "../Interface/Library.h"


namespace Cerite {
// Combine multiple Cerite documents into one using a node list
Object Combiner::combineDocuments(const std::string& name, std::vector<Object>& documents, NodeList& nodes, std::vector<int> ports) {
    
    parseArguments(documents);
    Object result = Object::combineObjects(name, documents, nodes, ports);

    result.name = name;
    
    return result;
    
}

void Combiner::parseArguments(std::vector<Object>& documents)
{
    for(auto& doc : documents) {        
        doc.replaceVarName("argc", std::to_string(doc.nArgs));
        
        Vector argvec("argv", doc.nArgs, 1, true);
        argvec.origin = doc.name;
        argvec.ctype = "Data";
        argvec.update = true;
        argvec.definition.resize(doc.nArgs);
        
        for(int i = 0; i < doc.nArgs; i++) {
            const std::string& str = doc.varArgs[i];
            if(str.empty()) continue;
            if(std::all_of(str.begin(), str.end(), ::isdigit)) {
                argvec.definition[i] = "(Data){tNumber, " + str + ", \"\", 0, 0}";
            }
            else {
                argvec.definition[i] = "(Data){tString, 0, \""+ str +"\", 0, 0}";
            }
        }
        
        doc.addVector(argvec);
    }
}

}
