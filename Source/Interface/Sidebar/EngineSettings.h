#pragma once

#include "../Looks.h"
class EngineSettings : public Component
{
public:

	std::vector<double> settings = std::vector<double>(6);
	Array<String> labels = {"Iterations ", "Newton Tol. ", "Solver Tol. ", "Oversample", "Reduce refactor (experimental)", "Disable Checking"};

    CeriteLookAndFeel clook;
    
    ToggleButton reducefactor;
    ToggleButton disablechecking;
    
    ComboBox oversampSel;
    
	EngineSettings(bool showAdvanced = true);

    ~EngineSettings(){
        setLookAndFeel(nullptr);
    }
private:

	OwnedArray<TextEditor> settingsBoxes;

	void paint(Graphics & g) override;

	void resized() override;

};



