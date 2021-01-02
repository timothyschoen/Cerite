#include <utility>
#include "ComponentDictionary.h"
#include "Components/AudioPlayerContainer.h"

Component* ComponentDictionary::cnv = nullptr; // We'll overwrite this with Canvas
// do we ever use this though???

componentInformation ComponentDictionary::undefined = {"undefined", 0, {}};

std::unordered_map<String, GUIContainer* (*)(ValueTree, Box*)> ComponentDictionary::graphicalDictionary = ComponentDictionary::createGraphicsMap();

std::unordered_map<String, componentInformation> ComponentDictionary::infoDictionary = ComponentDictionary::createInfoMap();

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

std::unordered_map<String, componentInformation> ComponentDictionary::createInfoMap()
{
    
    domainInformation = createDomainMap();
    
    auto documents = library.components;
    
    std::unordered_map<String, componentInformation> infoMap;
    for(auto& doc : documents) {
        Document& value = doc.second;
        int inIdx = 0;
        std::vector<edgeInformation> edges;
        int outIdx = 0;
        
        for(auto& context : value.contexts)
            outIdx += value.outstart[context];
        
        for(auto& context : value.contexts) {
            
            int numIn = value.outstart[context];
            
            for(int i = 0; i < value.ports[context].size(); i++) {
                //std::cout << i % numIn+1 << std::endl;
                bool side = i >= value.outstart[context];
                int type = domainInformation[context].position;
                edges.push_back({type, side, side ? outIdx : inIdx, ""});
                
                
                side ? outIdx++ : inIdx++;
            }
        }

        
        infoMap[doc.first] = {value.name, 1, edges};
    }

    return infoMap;
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
    
    // Try to find a cerite object
    if(infoDictionary.count(type) > 0)
        return infoDictionary[type];
    
    
    componentInformation boxInfo = undefined;
    
    //boxInfo.pdObject = libpd_type_exists(type.toRawUTF8());

    return boxInfo;
}

std::vector<String> ComponentDictionary::getKeys()
{
    std::vector<String> allkeys; /* = {"!=", "%", "&", "&&", "~", "+", "+~", "-", "-~", "/", "/~", "<", "<", ">=", ">>", "abs", "abs~", "atan", "atan2", "b", "bang", "bendin", "bendout", "biquad~", "bng", "bp~", "catch~", "change", "clip", "clip~", "cos", "cos~", "cpole~", "ctlin", "ctlout", "czero_rev~", "czero~", "dbtopow", "dbtopow~", "dbtorms", "dbtorms~", "declare", "del", "delay", "delread~", "delwrite~", "div", "env~", "exp", "exp~", "f", "float", "floatatom", "ftom", "ftom~", "hip~", "hradio", "hsl", "i", "int", "line", "line~", "loadbang", "log", "lop~", "makenote", "max", "max~", "metro", "min", "min~", "mod", "moses", "mtof", "mtof~", "nbx", "noise~", "notein", "noteout", "osc~", "pack", "pgmin", "pgmout", "phasor~", "pipe", "poly", "pow", "powtodb", "powtodb~", "pow~", "print", "q8_rsqrt~", "q8_sqrt~", "r", "random", "receive", "receive~", "rmstodb", "rmstodb~", "route", "rpole~", "rsqrt~", "rzero_rev~", "rzero~", "r~", "s", "samphold~", "samplerate~", "sel", "select", "send", "send~", "sig~", "sin", "snapshot~", "spigot", "sqrt", "sqrt~", "swap", "symbol", "symbolatom", "s~", "t", "table", "tabosc4~", "tabplay~", "tabread", "tabread4~", "tabread~", "tabwrite", "tabwrite~", "tan", "tgl", "throw~", "timer", "touchin", "touchout", "trigger", "unpack", "until", "vcf~", "vd~", "vradio", "vsl", "wrap", "wrap~", "|", "||"}; */
   
    for(auto& pair : infoDictionary)
        allkeys.push_back(pair.first);
    // Insert the words in the
    // Ternary Search Tree
    for (String str : allkeys)
        insert(&tree, str);
    
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


Document ComponentDictionary::getObject (ValueTree boxTree, String name, std::vector<String> arguments)
{

    Document newcomponent = library.get(name.toStdString());
    
    for(int i = 0; i < arguments.size(); i++) {
        newcomponent.setArgument(i + 1, arguments[i].toStdString());
    }
    
    return newcomponent;
}

void ComponentDictionary::refresh() {
    Library::refresh();
    infoDictionary = createInfoMap();
    
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

