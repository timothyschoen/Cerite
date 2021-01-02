#include "EngineSettings.h"
#include "../../MainComponent.h"



EngineSettings::EngineSettings(bool showAdvanced)
{
	for(int i = 0; i < 3; i++)
	{
		TextEditor* textbox = new TextEditor();
        
		settingsBoxes.add(textbox);
		textbox->onFocusLost = [this, textbox, i]() mutable
		{
			if(textbox->getText().isEmpty())
			{
				textbox->setText("1");
			}
			settings[i] = std::stof(textbox->getText().toStdString());
		};
		addAndMakeVisible(textbox);
      }
	
    setLookAndFeel(&clook);

	settingsBoxes[0]->setInputRestrictions(10, "0123456789"); //int
	settingsBoxes[0]->setText("8");
	settings[0] = 8;
	settingsBoxes[1]->setInputRestrictions(10, "-e0123456789."); //double
	settingsBoxes[1]->setText("5e-3");
	settings[1] = 5e-3;
	settingsBoxes[2]->setInputRestrictions(10, "-e0123456789."); //double
	settingsBoxes[2]->setText("1e-6");
    settings[2] = 1e-6;
	resized();
    
    reducefactor.onStateChange = [this]() {
        settings[3] = reducefactor.getToggleState();
    };
    
    oversampSel.addItemList({"1x", "2x", "4x", "8x", "16x"}, 1);
    
    oversampSel.onChange = [this]() {
        settings[5] = oversampSel.getSelectedItemIndex();
    };
    
    disablechecking.onStateChange = [this]() {
        settings[4] = disablechecking.getToggleState();
    };
    
    oversampSel.setSelectedItemIndex(0);
    addAndMakeVisible(oversampSel);
    addAndMakeVisible(reducefactor);
    addAndMakeVisible(disablechecking);
    
    if(!showAdvanced) {
        labels.removeLast(2);
        reducefactor.setVisible(false);
        disablechecking.setVisible(false);
    }
        
}

void EngineSettings::paint(Graphics & g)
{
	g.fillAll (findColour(SidePanel::ColourIds::backgroundColour));
	g.setColour(Colours::white);

	for(int i = 0; i < labels.size(); i++)
		g.drawFittedText(labels[i], 10, (i * 30) + 10, (getWidth() - 30) / 2, 20, Justification::left, 1);
    
    
}

void EngineSettings::resized()
{
	for(int i = 0; i < settingsBoxes.size(); i++)
		settingsBoxes[i]->setBounds(getWidth() / 2, (i * 30) + 10, (getWidth() - 10) / 2, 20);
    
    oversampSel.setBounds(getWidth() / 2, 100, (getWidth() - 10) / 2, 20);
    reducefactor.setBounds(getWidth() / 2, 130, (getWidth() - 10) / 2, 20);
    disablechecking.setBounds(getWidth() / 2, 160, (getWidth() - 10) / 2, 20);
}
