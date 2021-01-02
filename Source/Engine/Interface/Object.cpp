#include "Object.h"
#include "Document.h"

namespace Cerite {

Object::Object(const Document& doc) : document(new Document(doc)) {
    //nets = doc.size;
    //ports = (int)doc.ports.size();
    //name = doc.name;
}


Object::~Object() {
    if(state)
        tcc_delete(state);
}


}
