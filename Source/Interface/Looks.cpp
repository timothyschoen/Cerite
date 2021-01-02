#include "Looks.h"
#include "Canvas/Canvas.h"
#include <JuceHeader.h>


#include <JuceHeader.h>

static void drawTriangle (Graphics& g, float x1, float y1, float x2, float y2, float x3, float y3, Colour fill, Colour outline)
{
    Path p;
    p.addTriangle (x1, y1, x2, y2, x3, y3);
    g.setColour (fill);
    g.fillPath (p);

    g.setColour (outline);
    g.strokePath (p, PathStrokeType (0.3f));
}


CeriteLookAndFeel::CeriteLookAndFeel()
{
	float alpha = 0.8;
	setColour (ResizableWindow::backgroundColourId, Colour(50, 50, 50));
	setColour (TextButton::buttonColourId, Colour(41, 41, 41));
	setColour (TextButton::buttonOnColourId, Colour(25, 25, 25));
	setColour (juce::TextEditor::backgroundColourId, Colour(68, 68, 68));
	setColour (Toolbar::backgroundColourId, Colour(41, 41, 41).withAlpha(alpha));
	setColour (SidePanel::backgroundColour, Colour(50, 50, 50));
	setColour (ComboBox::backgroundColourId, Colour(50, 50, 50));
	setColour (ListBox::backgroundColourId, Colour(50, 50, 50));
	setColour (Slider::backgroundColourId, Colour(60, 60, 60));
	setColour (Slider::trackColourId, Colour(50, 50, 50));
	setColour(CodeEditorComponent::backgroundColourId, Colour(50, 50, 50));
	setColour(CodeEditorComponent::defaultTextColourId, Colours::white);
	setColour(TextEditor::textColourId, Colours::white);
	setColour(ColourScheme::widgetBackground, Colour(50, 50, 50));
	setColour(TooltipWindow::backgroundColourId, Colour(25, 25, 25).withAlpha(float(0.8)));
    setColour (PopupMenu::backgroundColourId, Colour(50, 50, 50));
    setColour (PopupMenu::highlightedBackgroundColourId, Colour(41, 41, 41));
   
	//setColour (Toolbar::buttonMouseOverBackgroundColourId, ));
	//setColour (Toolbar::buttonMouseDownBackgroundColourId, ));
}

void CeriteLookAndFeel::drawButtonBackground (Graphics& g,
                                           Button& button,
                                           const Colour& backgroundColour,
                                           bool shouldDrawButtonAsHighlighted,
                                           bool shouldDrawButtonAsDown)
{

    auto bounds = button.getLocalBounds().toFloat();

    auto baseColour = backgroundColour.withMultipliedSaturation (button.hasKeyboardFocus (true) ? 1.3f : 0.9f)
                                      .withMultipliedAlpha (button.isEnabled() ? 1.0f : 0.5f);

    if (shouldDrawButtonAsDown || shouldDrawButtonAsHighlighted)
        baseColour = baseColour.contrasting (shouldDrawButtonAsDown ? 0.2f : 0.05f);

    
    Path path;
    path.addRectangle (bounds);

    g.setColour (baseColour);
    g.fillPath (path);
    g.setColour (button.findColour (ComboBox::outlineColourId));
    g.strokePath (path, PathStrokeType (2.f));
    
}

void CeriteLookAndFeel::drawFileBrowserRow (Graphics& g, int width, int height,
        const File&, const String& filename, Image* icon,
        const String& fileSizeDescription,
        const String& fileTimeDescription,
        bool isDirectory, bool isItemSelected,
        int /*itemIndex*/, DirectoryContentsDisplayComponent& dcc)
{
	auto fileListComp = dynamic_cast<Component*> (&dcc);

	if (isItemSelected)
		g.fillAll (fileListComp != nullptr ? fileListComp->findColour (DirectoryContentsDisplayComponent::highlightColourId)
		           : findColour (DirectoryContentsDisplayComponent::highlightColourId));

	const int x = 32;
	g.setColour (Colours::black);

	if (icon != nullptr && icon->isValid())
	{
		g.drawImageWithin (*icon, 2, 2, x - 4, height - 4,
		                   RectanglePlacement::centred | RectanglePlacement::onlyReduceInSize,
		                   false);
	}
	else
	{
		if (auto* d = isDirectory ? getDefaultFolderImage()
		              : getDefaultDocumentFileImage())
			d->drawWithin (g, Rectangle<float> (2.0f, 2.0f, x - 4.0f, height - 4.0f),
			               RectanglePlacement::centred | RectanglePlacement::onlyReduceInSize, 1.0f);
	}

	if (isItemSelected)
		g.setColour (fileListComp != nullptr ? fileListComp->findColour (DirectoryContentsDisplayComponent::highlightedTextColourId)
		             : findColour (DirectoryContentsDisplayComponent::highlightedTextColourId));
	else
		g.setColour (fileListComp != nullptr ? fileListComp->findColour (DirectoryContentsDisplayComponent::textColourId)
		             : findColour (DirectoryContentsDisplayComponent::textColourId));

	g.setFont (height * 0.7f);
	auto sizeX = roundToInt (width * 0.7f);
	auto dateX = roundToInt (width * 0.8f);
	g.drawFittedText (filename,
	                  x, 0, sizeX - x, height,
	                  Justification::centredLeft, 1);
	g.setFont (height * 0.5f);
	g.setColour (Colours::darkgrey);

	if (! isDirectory)
	{
		g.drawFittedText (fileSizeDescription,
		                  sizeX, 0, dateX - sizeX - 8, height,
		                  Justification::centredRight, 1);
		g.drawFittedText (fileTimeDescription,
		                  dateX, 0, width - 8 - dateX, height,
		                  Justification::centredRight, 1);
	}
}


void CeriteLookAndFeel::drawDocumentWindowTitleBar (DocumentWindow& window, Graphics& g,
        int w, int h, int titleSpaceX, int titleSpaceW,
        const Image* icon, bool drawTitleTextOnLeft)
{
	if (w * h == 0)
		return;

	auto isActive = window.isActiveWindow();
	g.setColour (findColour(ResizableWindow::backgroundColourId));
	g.fillAll();
	Font font (h * 0.65f, Font::plain);
	g.setFont (font);
	auto textW = font.getStringWidth (window.getName());
	auto iconW = 0;
	auto iconH = 0;

	if (icon != nullptr)
	{
		iconH = static_cast<int> (font.getHeight());
		iconW = icon->getWidth() * iconH / icon->getHeight() + 4;
	}

	textW = jmin (titleSpaceW, textW + iconW);
	auto textX = drawTitleTextOnLeft ? titleSpaceX
	             : jmax (titleSpaceX, (w - textW) / 2);

	if (textX + textW > titleSpaceX + titleSpaceW)
		textX = titleSpaceX + titleSpaceW - textW;

	if (icon != nullptr)
	{
		g.setOpacity (isActive ? 1.0f : 0.6f);
		g.drawImageWithin (*icon, textX, (h - iconH) / 2, iconW, iconH,
		                   RectanglePlacement::centred, false);
		textX += iconW;
		textW -= iconW;
	}

	if (window.isColourSpecified (DocumentWindow::textColourId) || isColourSpecified (DocumentWindow::textColourId))
		g.setColour (window.findColour (DocumentWindow::textColourId));
	else
		g.setColour (getCurrentColourScheme().getUIColour (ColourScheme::defaultText));

	g.drawText (window.getName(), textX, 0, textW, h, Justification::centredLeft, true);
}
// Toolbar buttons
void CeriteLookAndFeel::paintToolbarButtonLabel (Graphics & g, int x, int y, int width, int height, const String & text, ToolbarItemComponent & button)
{
	g.setFont (Entypo);
	g.setFont (13);
	g.setColour (button.findColour (button.getToggleState() ? TextButton::textColourOnId
	                                : TextButton::textColourOffId)
	             .withMultipliedAlpha (button.isEnabled() ? 1.0f : 0.5f));
	auto yIndent = jmin (4, button.proportionOfHeight (0.3f));
	auto cornerSize = jmin (button.getHeight(), button.getWidth()) / 2;
	auto fontHeight = roundToInt (button.getHeight() * 0.9f);
	auto leftIndent  = jmin (fontHeight, 2 + cornerSize / (button.isConnectedOnLeft()  ? 4 : 2));
	auto rightIndent = jmin (fontHeight, 2 + cornerSize / (button.isConnectedOnRight() ? 4 : 2));
	auto textWidth = button.getWidth() - leftIndent - rightIndent;

	if (textWidth > 0)
		g.drawFittedText (button.getButtonText(),
		                  leftIndent, yIndent, textWidth, button.getHeight() - yIndent * 2,
		                  Justification::centred, 2);
}




// For tooltips
TextLayout CeriteLookAndFeel::layoutTooltipText (const String& text, Colour colour) noexcept
{
	const float tooltipFontSize = 13.0f;
	const int maxToolTipWidth = 400;
	AttributedString s;
	s.setJustification (Justification::centred);
	s.append (text, Font (tooltipFontSize, Font::bold), colour);
	TextLayout tl;
	tl.createLayoutWithBalancedLineLengths (s, (float) maxToolTipWidth);
	return tl;
}


//==============================================================================
void CeriteLookAndFeel::drawComboBox (Graphics& g, int width, int height, bool,
                                   int, int, int, int, ComboBox& box)
{
    Rectangle<int> boxBounds (0, 0, width, height);

    g.setColour (box.findColour (ComboBox::backgroundColourId));
    g.drawRect (boxBounds.toFloat());

    g.setColour (box.findColour (ComboBox::outlineColourId));
    g.drawRect (boxBounds.toFloat().reduced (0.5f, 0.5f));

    Rectangle<int> arrowZone (width - 20, 5, 12, height - 10);
    
    Colour triangleColour = box.findColour (ComboBox::arrowColourId).withAlpha ((box.isEnabled() ? 0.9f : 0.2f));
    drawTriangle(g, (float) arrowZone.getX() + 3.0f, (float) arrowZone.getCentreY() - 2.0f, (float) arrowZone.getCentreX(), (float) arrowZone.getCentreY() + 3.0f, (float) arrowZone.getRight() - 3.0f, (float) arrowZone.getCentreY() - 2.0f, triangleColour, triangleColour);
   
    
    /*
    Path path;
    path.startNewSubPath ();
    path.lineTo ();
    path.lineTo ();

    g.setColour (box.findColour (ComboBox::arrowColourId).withAlpha ((box.isEnabled() ? 0.9f : 0.2f)));
    g.strokePath (path, PathStrokeType (2.0f)); */
}

void CeriteLookAndFeel::drawTooltip(Graphics&g, const String& text, int width, int height)
{
	Rectangle<int> bounds (width, height);
	auto cornerSize = 5.0f;
	g.setColour (findColour(SidePanel::ColourIds::backgroundColour).darker().withAlpha((float)0.9));
	g.fillRoundedRectangle (bounds.toFloat(), cornerSize);
	g.setColour (findColour (TooltipWindow::outlineColourId));
	g.drawRoundedRectangle (bounds.toFloat().reduced (0.5f, 0.5f), cornerSize, 1.0f);
	layoutTooltipText (text, findColour (TooltipWindow::textColourId)).draw (g, { static_cast<float> (width), static_cast<float> (height) });
}

roundButton::roundButton()
{
	setColour (TextButton::buttonColourId, Colour(41, 41, 41));
	setColour (TextButton::buttonOnColourId, Colour(41, 41, 41).darker());
}

Font roundButton::getTextButtonFont (TextButton&, int buttonHeight)
{
	Entypo.setHeight(buttonHeight / 2.2);
	return Entypo;
}

void roundButton::drawButtonBackground(Graphics & g, Button & button, const Colour & backgroundColour, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat();

    auto baseColour = backgroundColour.withMultipliedSaturation (button.hasKeyboardFocus (true) ? 1.3f : 0.9f)
                                      .withMultipliedAlpha (button.isEnabled() ? 1.0f : 0.5f);

    if (shouldDrawButtonAsDown || shouldDrawButtonAsHighlighted)
        baseColour = baseColour.contrasting (shouldDrawButtonAsDown ? 0.2f : 0.05f);

    
    Path path;
    path.addRectangle (bounds);

    g.setColour (baseColour);
    g.fillPath (path);
    g.setColour (button.findColour (ComboBox::outlineColourId));
    g.strokePath (path, PathStrokeType (2.f));
}

graphicsButton::graphicsButton()
{
	setColour (TextButton::buttonColourId, Colour(41, 41, 41));
	setColour (TextButton::buttonOnColourId, Colour(41, 41, 41).darker());
}

Font graphicsButton::getTextButtonFont (TextButton&, int buttonHeight)
{
	Entypo.setHeight(13);
	return Entypo;
}

void graphicsButton::drawButtonBackground(Graphics & g, Button & button, const Colour & backgroundColour, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
	auto buttonArea = button.getLocalBounds();

	if(shouldDrawButtonAsDown)
	{
		g.setColour (backgroundColour.darker());
	}
	else if (shouldDrawButtonAsHighlighted)
	{
		g.setColour (backgroundColour.brighter());
	}
	else
	{
		g.setColour (backgroundColour);
	}

	g.fillRect (buttonArea.toFloat());
}

SidebarLookAndFeel::SidebarLookAndFeel()
{
	/*
	 tabOutlineColourId
	 tabTextColourId
	 frontOutlineColourId
	 frontTextColourId
	 */
}

Font SidebarLookAndFeel::getTabButtonFont (TabBarButton& b, float height)
{
	Entypo.setHeight(10);
	return Entypo;
}

void SidebarLookAndFeel::drawTabButtonText (TabBarButton & button, Graphics & g, bool isMouseOver, bool isMouseDown)
{
	auto area = button.getTextArea().toFloat();
	auto length = area.getWidth();
	auto depth  = area.getHeight();

	if (button.getTabbedButtonBar().isVertical())
		std::swap (length, depth);

	Font font (getTabButtonFont (button, depth));
	font.setUnderline (button.hasKeyboardFocus (false));
	Colour col;

	if (button.isFrontTab() && (button.isColourSpecified (TabbedButtonBar::frontTextColourId)
	                            || isColourSpecified (TabbedButtonBar::frontTextColourId)))
		col = findColour (TabbedButtonBar::frontTextColourId);
	else if (button.isColourSpecified (TabbedButtonBar::tabTextColourId)
	         || isColourSpecified (TabbedButtonBar::tabTextColourId))
		col = findColour (TabbedButtonBar::tabTextColourId);
	else
		col = button.getTabBackgroundColour().contrasting();

	auto alpha = button.isEnabled() ? ((isMouseOver || isMouseDown) ? 1.0f : 0.8f) : 0.3f;
	g.setColour (col.withMultipliedAlpha (alpha));
	g.setFont (font);
	//g.addTransform (t);
	g.drawFittedText (button.getButtonText().trim(),
	                  8, 13, (int) length, (int) depth,
	                  Justification::centred,
	                  jmax (1, ((int) depth) / 12));
}

void SidebarLookAndFeel::drawTabButton (TabBarButton& button, Graphics& g, bool isMouseOver, bool isMouseDown)
{
	Path tabShape;
	createTabButtonShape (button, tabShape, isMouseOver, isMouseDown);
	auto activeArea = button.getActiveArea();
	tabShape.applyTransform (AffineTransform::translation ((float) activeArea.getX(),
	                         (float) activeArea.getY()));
	fillTabButtonShape (button, g, tabShape, isMouseOver, isMouseDown);
	drawTabButtonText (button, g, isMouseOver, isMouseDown);
}

void SidebarLookAndFeel::drawTabAreaBehindFrontButton (TabbedButtonBar& bar, Graphics& g, const int w, const int h)
{
	auto shadowSize = 0.2f;
	Rectangle<int> line;
	ColourGradient gradient (Colours::black.withAlpha (bar.isEnabled() ? 0.25f : 0.15f), 0, 0,
	                         Colours::transparentBlack, 0, 0, false);

	switch (bar.getOrientation())
	{
	case TabbedButtonBar::TabsAtLeft:
		gradient.point1.x = (float) w;
		gradient.point2.x = w * (1.0f - shadowSize);
		//shadowRect.setBounds ((int) gradient.point2.x, 0, w - (int) gradient.point2.x, h);
		line.setBounds (w - 1, 0, 1, h);
		break;

	case TabbedButtonBar::TabsAtRight:
		gradient.point2.x = w * shadowSize;
		//shadowRect.setBounds (0, 0, (int) gradient.point2.x, h);
		line.setBounds (0, 0, 1, h);
		break;

	case TabbedButtonBar::TabsAtTop:
		gradient.point1.y = (float) h;
		gradient.point2.y = h * (1.0f - shadowSize);
		//shadowRect.setBounds (0, (int) gradient.point2.y, w, h - (int) gradient.point2.y);
		line.setBounds (0, h - 1, w, 1);
		break;

	case TabbedButtonBar::TabsAtBottom:
		gradient.point2.y = h * shadowSize;
		//shadowRect.setBounds (0, 0, w, (int) gradient.point2.y);
		line.setBounds (0, 0, w, 1);
		break;

	default:
		break;
	}

	g.setGradientFill (gradient);
	//g.fillRect (shadowRect.expanded (2, 2));
	g.setColour (Colour (0x80000000));
	g.fillRect (line);
}




nameSuggestionsButtons::nameSuggestionsButtons()
{
	setColour (TextButton::buttonColourId, Colour::fromString("EE121318"));
	setColour (TextButton::buttonOnColourId, Colour::fromString("FF121318").darker());
}

void nameSuggestionsButtons::drawButtonText (Graphics& g, TextButton& button, bool isMouseOverButton, bool isButtonDown)
{
	auto font = getTextButtonFont (button, button.getHeight());
	g.setFont (font);
	g.setColour (button.findColour (button.getToggleState() ? TextButton::textColourOnId
	                                : TextButton::textColourOffId)
	             .withMultipliedAlpha (button.isEnabled() ? 1.0f : 0.5f));
	auto yIndent = jmin (4, button.proportionOfHeight (0.3f));
	auto cornerSize = jmin (button.getHeight(), button.getWidth()) / 2;
	auto fontHeight = roundToInt (font.getHeight() * 0.6f);
	auto leftIndent  = 25;
	auto rightIndent = jmin (fontHeight, 2 + cornerSize / (button.isConnectedOnRight() ? 4 : 2));
	auto textWidth = button.getWidth() - leftIndent - rightIndent;

	if (textWidth > 0)
		g.drawFittedText (button.getButtonText(),
		                  leftIndent, yIndent, textWidth, button.getHeight() - yIndent * 2,
		                  Justification::left, 2);
}


void nameSuggestionsButtons::drawButtonBackground(Graphics & g, Button & button, const Colour & backgroundColour, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
	auto buttonArea = button.getLocalBounds();

	if(shouldDrawButtonAsDown)
	{
		g.setColour (backgroundColour.darker());
	}
	else if (shouldDrawButtonAsHighlighted)
	{
		g.setColour (backgroundColour.brighter());
	}
	else
	{
		g.setColour (backgroundColour);
	}

	g.fillRect (buttonArea.toFloat());
}


void audioSliderLook::drawLinearSlider (Graphics& g, int x, int y, int width, int height,
                                        float sliderPos,
                                        float minSliderPos,
                                        float maxSliderPos,
                                        const Slider::SliderStyle style, Slider& slider)
{
	float trackWidth = 2.;
	Point<float> startPoint (slider.isHorizontal() ? x : x + width * 0.5f,
	                         slider.isHorizontal() ? y + height * 0.5f : height + y);
	Point<float> endPoint (slider.isHorizontal() ? width + x : startPoint.x,
	                       slider.isHorizontal() ? startPoint.y : y);
	Path backgroundTrack;
	backgroundTrack.startNewSubPath (startPoint);
	backgroundTrack.lineTo (endPoint);
	g.setColour (slider.findColour (Slider::backgroundColourId));
	g.strokePath (backgroundTrack, { trackWidth, PathStrokeType::curved, PathStrokeType::rounded });
	Path valueTrack;
	Point<float> minPoint, maxPoint;
	auto kx = slider.isHorizontal() ? sliderPos : (x + width * 0.5f);
	auto ky = slider.isHorizontal() ? (y + height * 0.5f) : sliderPos;
	minPoint = startPoint;
	maxPoint = { kx, ky };
	auto thumbWidth = getSliderThumbRadius (slider);
	valueTrack.startNewSubPath (minPoint);
	valueTrack.lineTo (maxPoint);
	g.setColour (slider.findColour (Slider::trackColourId));
	g.strokePath (valueTrack, { trackWidth, PathStrokeType::curved, PathStrokeType::rounded });
	g.setColour (slider.findColour (Slider::thumbColourId));
	g.fillEllipse (Rectangle<float> (static_cast<float> (thumbWidth), static_cast<float> (thumbWidth)).withCentre (maxPoint));
}

int audioSliderLook::getSliderThumbRadius(Slider &)
{
	return 6;
}



void volumeSliderLook::drawLinearSlider (Graphics& g, int x, int y, int width, int height,
        float sliderPos,
        float minSliderPos,
        float maxSliderPos,
        const Slider::SliderStyle style, Slider& slider)
{
	float trackWidth = 6.;
	Point<float> startPoint (slider.isHorizontal() ? x : x + width * 0.5f,
	                         slider.isHorizontal() ? y + height * 0.5f : height + y);
	Point<float> endPoint (slider.isHorizontal() ? width + x : startPoint.x,
	                       slider.isHorizontal() ? startPoint.y : y);
	Path backgroundTrack;
	backgroundTrack.startNewSubPath (startPoint);
	backgroundTrack.lineTo (endPoint);
	g.setColour (slider.findColour (Slider::backgroundColourId));
	g.strokePath (backgroundTrack, { trackWidth, PathStrokeType::mitered});
	Path valueTrack;
	Point<float> minPoint, maxPoint;
	auto kx = slider.isHorizontal() ? sliderPos : (x + width * 0.5f);
	auto ky = slider.isHorizontal() ? (y + height * 0.5f) : sliderPos;
	minPoint = startPoint;
	maxPoint = { kx, ky };
	auto thumbWidth = getSliderThumbRadius (slider);
	valueTrack.startNewSubPath (minPoint);
	valueTrack.lineTo (maxPoint);
	g.setColour (slider.findColour (Slider::trackColourId));
	g.strokePath (valueTrack, { trackWidth, PathStrokeType::mitered});
	g.setColour (slider.findColour (Slider::thumbColourId));
	g.fillRect (Rectangle<float> (static_cast<float> (thumbWidth), static_cast<float> (24)).withCentre (maxPoint));
}

int volumeSliderLook::getSliderThumbRadius(Slider &)
{
	return 6;
}
