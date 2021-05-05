#pragma once

#include <JuceHeader.h>

struct MainLook : public LookAndFeel_V4
{
    MainLook() {
        setColour (ResizableWindow::backgroundColourId, Colour(50, 50, 50));
        setColour (TextButton::buttonColourId, Colour(41, 41, 41));
        setColour (TextButton::buttonOnColourId, Colour(25, 25, 25));
        setColour (juce::TextEditor::backgroundColourId, Colour(68, 68, 68));
        setColour (SidePanel::backgroundColour, Colour(50, 50, 50));
        setColour (ComboBox::backgroundColourId, Colour(50, 50, 50));
        setColour (ListBox::backgroundColourId, Colour(50, 50, 50));
        setColour (Slider::backgroundColourId, Colour(60, 60, 60));
        setColour (Slider::trackColourId, Colour(50, 50, 50));
        setColour(CodeEditorComponent::backgroundColourId, Colour(50, 50, 50));
        setColour(CodeEditorComponent::defaultTextColourId, Colours::white);
        setColour(TextEditor::textColourId, Colours::white);
        setColour(TooltipWindow::backgroundColourId, Colour(25, 25, 25).withAlpha(float(0.8)));
        setColour (PopupMenu::backgroundColourId, Colour(50, 50, 50));
        setColour (PopupMenu::highlightedBackgroundColourId, Colour(41, 41, 41));
    }
};

struct ToolbarLook : public MainLook
{
    
    inline static Font icon_font = Font(Typeface::createSystemTypefaceFor (BinaryData::cerite_font_ttf, BinaryData::cerite_font_ttfSize));
    
    bool icons;
    
    ToolbarLook(bool use_icons = true) {
        icons = use_icons;
    }
    
    void drawButtonBackground (Graphics& g, Button& button, const Colour& backgroundColour,  bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) {
        
        auto rect = button.getLocalBounds();
        
        auto base_colour = Colour(41, 41, 41);
        
        auto highlight_colour = Colour (0xff42a2c8);
        
        if(shouldDrawButtonAsHighlighted || button.getToggleState())
            highlight_colour = highlight_colour.brighter (0.4f);
        
        if(shouldDrawButtonAsDown)
            highlight_colour = highlight_colour.brighter (0.1f);
        else
            highlight_colour = highlight_colour.darker(0.2f);
        
        
        g.setColour(base_colour);
        g.fillRect(rect);
        
        auto highlight_rect = Rectangle<float>(rect.getX(), rect.getY() + rect.getHeight() - 8, rect.getWidth(), 4);
        
        g.setColour(highlight_colour);
        g.fillRect(highlight_rect);
        
    }
    
    
    Font getTextButtonFont (TextButton&, int buttonHeight)
    {
        icon_font.setHeight(buttonHeight / 3.6);
        return icons ? icon_font : Font (jmin (15.0f, (float) buttonHeight * 0.6f));
    }
};

struct SidebarLook : public MainLook
{
    float scalar;
    
    SidebarLook(float button_scalar = 1.0f) {
        scalar = button_scalar;
        setColour(ComboBox::outlineColourId, findColour(TextButton::buttonColourId));
    }
    
    Font getTextButtonFont (TextButton&, int buttonHeight)
    {
        ToolbarLook::icon_font.setHeight(buttonHeight / (3.8 / scalar));
        return ToolbarLook::icon_font;
    }
};
