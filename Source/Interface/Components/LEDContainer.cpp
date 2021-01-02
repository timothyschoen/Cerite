#include <JuceHeader.h>
#include "LEDContainer.h"
#include "../../MainComponent.h"


/*
void LEDContainer::paint(Graphics & g)
{
	g.setColour(Colours::darkgrey);
	g.fillRect(0, 0, 35, 35);
	g.setColour(Colours::grey);
	g.drawEllipse(9, 9, 17, 17, 1);
	g.setColour((Colours::green).withBrightness(brightness));
	g.fillEllipse(9, 9, 17, 17);
}

void LEDContainer::resized()
{
}

Point<int> LEDContainer::getBestSize()
{
	return Point<int>(35, 35);
}

void LEDContainer::valueTreePropertyChanged(ValueTree &treeWhosePropertyHasChanged, const Identifier &property)
{
	if(treeWhosePropertyHasChanged.hasType("BrightnessPosition") && property == Identifier("Value"))
	{
		systempos = int(treeWhosePropertyHasChanged.getProperty("Value"));
	}
};

void LEDContainer::timerCallback()
{
	MainComponent* main = findParentComponentOfClass<MainComponent>();

	if(main == nullptr || main->msystem == nullptr)
	{
		return;
	}

	brightness = fabs(main->msystem->xPlot[systempos]) * 1e14 + 0.3;
	repaint();
}
*/
