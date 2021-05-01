#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/


class Edge  : public TextButton, public gin::ValueTreeObject
{
    
public:
    Edge(ValueTree tree);
    ~Edge() override;

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;
    
    void mouseMove(const MouseEvent& e) override;
    
    Rectangle<int> get_canvas_bounds();

    inline static Array<Colour> ctx_colours = {Colour (0xff42a2c8), Colour (0xfff6c82e), Colour(0xff735d9b)};
    
    
    static inline SafePointer<Edge> connecting_edge = nullptr;
    static inline std::map<String, Edge*> all_edges;
private:

    //==============================================================================
    // Your private member variables go here...


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Edge)
};
