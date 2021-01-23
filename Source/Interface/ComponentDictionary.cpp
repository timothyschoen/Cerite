#include <utility>
#include "ComponentDictionary.h"
#include "Components/AudioPlayerContainer.h"

Component* ComponentDictionary::cnv = nullptr; // We'll overwrite this with Canvas
// do we ever use this though???

componentInformation ComponentDictionary::undefined = {"undefined", 0, {}};

std::unordered_map<String, GUIContainer* (*)(ValueTree, Box*)> ComponentDictionary::graphicalDictionary = ComponentDictionary::createGraphicsMap();


std::unordered_map<String, void (*)(ValueTree, std::vector<String>)> ComponentDictionary::parameterDictionary = ComponentDictionary::createParameterMap();

std::vector<String> ComponentDictionary::keys = ComponentDictionary::getKeys();

std::unordered_map<String, GUIContainer* (*)(ValueTree, Box*)> ComponentDictionary::createGraphicsMap ()
{
    std::unordered_map<String, GUIContainer* (*)(ValueTree, Box*)> graphicsMap;
    //graphicsMap["led"] = &createGraphics<LEDContainer>;
    graphicsMap["input"] = &createGraphics<AudioPlayerContainer>;
    graphicsMap["input~"] = &createGraphics<AudioPlayerContainer>;
    
    graphicsMap["bng"] = &createGraphics<BangContainer>;
    graphicsMap["tgl"] = &createGraphics<ToggleContainer>;
    graphicsMap["nbx"] = &createGraphics<NumboxContainer>;
    graphicsMap["msg"] = &createGraphics<MessageContainer>;
    graphicsMap["hsl"] = &createGraphics<HSliderContainer>;
    graphicsMap["vsl"] = &createGraphics<VSliderContainer>;
    graphicsMap["hradio"] = &createGraphics<HRadioGroup>;
    graphicsMap["vradio"] = &createGraphics<VRadioGroup>;
    
    
    
    
    //graphicsMap["stinput"] = &createGraphics<AudioPlayerContainer>;
    
    return graphicsMap;
}


std::unordered_map<String, DomainInformation> ComponentDictionary::createDomainMap()
{
    std::unordered_map<String, DomainInformation> domains;
    
    domains["mna"] = {"mna", 0, Colour (0xff42a2c8), NoOrdering, NoSplit};
    domains["dsp"] = {"dsp", 1, Colour (0xfff6c82e), StartToEnd, OutToIn};
    domains["data"] = {"data", 2, Colour(0xff735d9b), RightToLeft, NoSplit};
    
    return domains;
}



std::unordered_map<String, void (*)(ValueTree, std::vector<String>)> ComponentDictionary::createParameterMap()
{
    std::unordered_map<String, void (*)(ValueTree, std::vector<String>)> parameterMap;
    
    return parameterMap;
}
componentInformation ComponentDictionary::getInfo(String name)
{
    // Check if name is empty
    if(!name.containsNonWhitespaceChars())
        return undefined;
    
    StringArray tokens;
    tokens.addTokens(name, " ", "");
    
    String type = tokens[0];
    tokens.remove(0);
    
    // move this elsewhere
    domainInformation = createDomainMap();
    
    auto documents = library.components;
    
    Object doc = Library::components[type.toStdString()];
    
    for(int i = 0; i < tokens.size(); i++) {
        doc.setArgument(i + 1, tokens[i].toStdString());
    }
    
    std::vector<edgeInformation> edges;
    
    int numIn = 0;
    int numOut = 0;

    for(auto& [key, ctx] : doc.imports) {
        for(int i = 0; i < ctx.getPorts(&doc).first; i++) {
            edges.push_back({domainInformation[key].position, 0, numIn, ""});
            numIn++;
        }
    }
    
    for(auto& [key, ctx] : doc.imports) {
        for(int i = 0; i < ctx.getPorts(&doc).second; i++) {
            edges.push_back({domainInformation[key].position, 1,  numIn + numOut, ""});
            numOut++;
        }
    }

    
    return {doc.name, 1, edges};
}


std::vector<String> ComponentDictionary::getKeys()
{
    std::vector<String> allkeys;
    
    for(auto& pair : Library::components) {
        allkeys.push_back(pair.first);
        
        // Insert the words in the Ternary Search Tree
        insert(&tree, pair.first);
    }
    
    return allkeys;
}

Array<String> ComponentDictionary::finishName (String text)
{
    Array<String> result;
    
    if(!text.containsNonWhitespaceChars() || text.contains(" "))
        return result;
    
    // Run auto-completion algorithm
    autocomplete(&tree, text, result);
    return result;
}

GUIContainer* ComponentDictionary::getComponent(ValueTree boxTree, Box* box)
{
    if(graphicalDictionary.count(box->type) > 0)
        return graphicalDictionary[box->type](boxTree, box);
    else return nullptr;
    
}

void ComponentDictionary::getParameters(String name, ValueTree params, std::vector<String> args)
{
    if(parameterDictionary.count(name) > 0)
        parameterDictionary[name](params, args);
}


Object ComponentDictionary::getObject (ValueTree boxTree, String name, std::vector<String> arguments)
{
    
    Object newcomponent = library.components[name.toStdString()];
    
    for(int i = 0; i < arguments.size(); i++) {
        newcomponent.setArgument(i + 1, arguments[i].toStdString());
    }
    
    return newcomponent;
}

void ComponentDictionary::refresh() {
    Library::refresh();    
}


ValueTree PresetManager::factoryPresets = ValueTree();

ValueTree PresetManager::userPresets = ValueTree();

void PresetManager::loadPreset() {
    
    
}

void PresetManager::savePreset() {
    
    File presetDir = (File::getSpecialLocation(File::userDocumentsDirectory)).getChildFile("Cerite").getChildFile("Presets");
    File userpresets = presetDir.getChildFile("user.lib");
    
    userpresets.deleteFile();
    
    FileOutputStream foutstr(userpresets);
    PresetManager::userPresets.writeToStream(foutstr);
    
}

void PresetManager::exportPreset() {
    
    
}

