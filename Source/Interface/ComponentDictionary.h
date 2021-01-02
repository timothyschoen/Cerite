#pragma once

#include "Canvas/Box.h"
#include "Components/GUIContainer.h"
#include "Components/DataGUI.h"
#include "../Utility/FSManager.h"
#include "componentInformation.h"
#include "DomainInformation.h"
#include "../Engine/Cerite/src/Interface/Document.h"
#include "../Engine/Cerite/src/Interface/Library.h"

#include <utility>
#include <JuceHeader.h>
/*
template<typename Boxtype>  IComponent* createInstance(ValueTree boxTree, std::vector<String> arguments, std::vector<int> nodes, std::vector<int> pdNodes)
{
	return new Boxtype(boxTree, arguments, nodes, pdNodes);
} */


template<typename GraphicsType>  GUIContainer* createGraphics(ValueTree boxTree, Box* box)
{
	return new GraphicsType(boxTree, box);
}

using namespace Cerite;

struct ComponentDictionary
{
    
	static Component* cnv;
    
    inline static Library library = Library("/Users/timothy/Documents/Cerite/Objects");

	// Dictionaries
    static std::unordered_map<String, componentInformation> infoDictionary;
    inline static std::unordered_map<String, DomainInformation> domainInformation = {};
	static std::unordered_map<String, void (*)(ValueTree, std::vector<String>)> parameterDictionary;
	static std::unordered_map<String, GUIContainer* (*)(ValueTree, Box*)> graphicalDictionary;
	//static;
	static std::vector<String> keys;
    
    static componentInformation undefined;

	// Functions to get info about a component
	static GUIContainer* getComponent(ValueTree boxTree, Box* box);
	static Document getObject (ValueTree boxTree, String name, std::vector<String> arguments);
	static componentInformation getInfo (String name);
	static void getParameters (String name, ValueTree params, std::vector<String> args);


	// Functions to fill the static dictionary objects
	static std::unordered_map<String, GUIContainer* (*)(ValueTree, Box*)> createGraphicsMap();
	static std::unordered_map<String, componentInformation> createInfoMap();
	static std::unordered_map<String, void (*)(ValueTree, std::vector<String>)> createParameterMap();
    static std::unordered_map<String, DomainInformation> createDomainMap();

	static std::vector<String> getKeys();
    
    static void refresh();

   //static componentInformation getPdInfo(t_pd* obj, String name);

	static Array<String> finishName (String text);
    
};



struct PresetManager
{
    
    File presetDir;
    
    static ValueTree factoryPresets; // gets overwritten in Main.cpp to prevent static init order problems...
    
    static ValueTree userPresets;
    
    static void savePreset();
    
    static void exportPreset();
    
    static void loadPreset();
};

// ASCII size
#define ALPHABET_SIZE 128
#define CHAR_TO_INDEX(c) ((int)c - (int)'\0')
    
// trie node
struct TrieNode
{
    struct TrieNode *children[ALPHABET_SIZE];
  
    // isWordEnd is true if the node represents
    // end of a word
    bool isWordEnd;
};

static TrieNode tree;
// Returns new trie node (initialized to NULLs)
static TrieNode *getNode(void)
{
    struct TrieNode *pNode = new TrieNode;
    pNode->isWordEnd = false;
  
    for (int i = 0; i < ALPHABET_SIZE; i++)
        pNode->children[i] = NULL;
  
    return pNode;
}
  
// If not present, inserts key into trie.  If the
// key is prefix of trie node, just marks leaf node
static void insert(struct TrieNode *root,  const String key)
{
    struct TrieNode *pCrawl = root;
  
    for (int level = 0; level < key.length(); level++)
    {
        int index = CHAR_TO_INDEX(key[level]);
        if (!pCrawl->children[index])
            pCrawl->children[index] = getNode();
  
        pCrawl = pCrawl->children[index];
    }
  
    // mark last node as leaf
    pCrawl->isWordEnd = true;
}
  
// Returns true if key presents in trie, else false
static bool search(struct TrieNode *root, const String key)
{
    int length = key.length();
    struct TrieNode *pCrawl = root;
    for (int level = 0; level < length; level++)
    {
        int index = CHAR_TO_INDEX(key[level]);
  
        if (!pCrawl->children[index])
            return false;
  
        pCrawl = pCrawl->children[index];
    }
  
    return (pCrawl != NULL && pCrawl->isWordEnd);
}
  
// Returns 0 if current node has a child
// If all children are NULL, return 1.
static bool isLastNode(struct TrieNode* root)
{
    for (int i = 0; i < ALPHABET_SIZE; i++)
        if (root->children[i])
            return 0;
    return 1;
}
  
// Recursive function to print auto-suggestions for given
// node.
static void suggestionsRec(struct TrieNode* root, std::string currPrefix, Array<String>& result)
{
    // found aString in Trie with the given prefix
    if (root->isWordEnd)
    {
        result.add(currPrefix);
    }
  
    // All children struct node pointers are NULL
    if (isLastNode(root))
        return;
  
    for (int i = 0; i < ALPHABET_SIZE; i++)
    {
        if (root->children[i])
        {
            // append current character to currPrefixString
            currPrefix.push_back(i);
  
            // recur over the rest
            suggestionsRec(root->children[i], currPrefix, result);
            // remove last character
            currPrefix.pop_back();
        }
    }
}


  
// print suggestions for given query prefix.
static int autocomplete(TrieNode* root, String query, Array<String>& result)
{
    struct TrieNode* pCrawl = root;
  
    // Check if prefix is present and find the
    // the node (of last level) with last character
    // of givenString.
    int level;
    int n = query.length();
    for (level = 0; level < n; level++)
    {
        int index = CHAR_TO_INDEX(query[level]);
  
        // noString in the Trie has this prefix
        if (!pCrawl->children[index])
            return 0;
  
        pCrawl = pCrawl->children[index];
    }
  
    // If prefix is present as a word.
    bool isWord = (pCrawl->isWordEnd == true);
  
    // If prefix is last node of tree (has no
    // children)
    bool isLast = isLastNode(pCrawl);
  
    // If prefix is present as a word, but
    // there is no subtree below the last
    // matching node.
    if (isWord && isLast)
    {
        result.add(query);
        return -1;
    }
  
    // If there are are nodes below last
    // matching character.
    if (!isLast)
    {
        String prefix = query;
        suggestionsRec(pCrawl, prefix.toStdString(), result);
        return 1;
    }
    return 0;
}

