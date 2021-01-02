#include "Library.h"
#include <fstream>


namespace Cerite {

Document Library::get(std::string name, std::vector<double> args) {

    Document result = components[name];

    for(int i = 0; i < args.size(); i++) {
        result.setArgument(i, args[i], true);
    }

    return result;
}



std::string Library::readFile(std::string path) {
    std::ifstream t(path);
    std::string file;

    t.seekg(0, std::ios::end);
    file.reserve(t.tellg());
    t.seekg(0, std::ios::beg);

    file.assign((std::istreambuf_iterator<char>(t)),
                    std::istreambuf_iterator<char>());
    
    return file;
}


}
