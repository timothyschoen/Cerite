/*
 ==============================================================================

 This file was auto-generated!

 It contains the basic startup code for a JUCE application.

 ==============================================================================
 */
#include <JuceHeader.h>
#include "MainComponent.h"
#include "Utility/FSManager.h"

//==============================================================================
class CeriteGUI  : public JUCEApplication
{
public:
	//==============================================================================
    CeriteGUI() {}

	const String getApplicationName() override
	{
		return ProjectInfo::projectName;
	}
	const String getApplicationVersion() override
	{
		return ProjectInfo::versionString;
	}
	bool moreThanOneInstanceAllowed() override
	{
		return true;
	}

	//==============================================================================
	void initialise (const String& commandLine) override
	{
        
		// This method is where you should put your application's initialisation code..
		FSManager::createFilesystem();
		mainWindow.reset (new MainWindow (getApplicationName()));
        mainWindow->setSize(1100, 900);
		mainWindow->setResizeLimits(800, 600, 5000, 5000);
		// Create a new project!
		MainComponent* mainPtr = static_cast<MainComponent*>(mainWindow->getChildComponent(0));
		XmlElement* recent = FSManager::meta->getChildByName("recentlyOpened");
		XmlElement* file = recent->getChildElement(0);
		File lastproj = File(file->getStringAttribute("Path"));
        
        loadPresets();
        
		if (lastproj.exists())
		{
			mainPtr->canvas.clearState();

			try
			{
				mainPtr->setTitle(lastproj.getFileName().toStdString() + " | Cerite");
				//mainPtr->canvas.setState(lastproj.loadFileAsString());
				//mainPtr->canvas.projectFile = lastproj;
			}
			catch(...)
			{
				mainPtr->canvas.clearState();
			}
		}
		else
         
		{
			mainPtr->appcmds.invokeDirectly(AppCommands::keyCmd::New, true);
		}
         
        /*
        PresetManager::presetDictionary = ValueTree::fromXml(presetDir.getChildFile("library.xml").loadFileAsString());
        int count = 0;
        //CODE FOR MODIFYING PRESETS
        for(auto component : PresetManager::presetDictionary) {
            String name = component.getType().toString();
            ValueTree referenceTree(name);
            
            ComponentDictionary::getParameters(name, referenceTree, {"0", "0", "0", "0"});
            for(auto preset : component) {
                for(auto param : preset) {
                    
                    if(double(param.getProperty("Value")) < double(referenceTree.getChildWithName(param.getType()).getProperty("Minimum")) || double(param.getProperty("Value")) > double(referenceTree.getChildWithName(param.getType()).getProperty("Maximum")))
                        std::cout << "ERR: Obj:" << component.getType().toString() << "param:" << param.getType().toString() << "val: " << double(param.getProperty("Value")) << std::endl;
                    param.setProperty("Skew",    referenceTree.getChildWithName(param.getType()).getProperty("Skew"), nullptr);
                    param.setProperty("Minimum", referenceTree.getChildWithName(param.getType()).getProperty("Minimum"), nullptr);
                    param.setProperty("Maximum", referenceTree.getChildWithName(param.getType()).getProperty("Maximum"), nullptr);
                    param.setProperty("Dynamic", referenceTree.getChildWithName(param.getType()).getProperty("Dynamic"), nullptr);
                
                    String value = param.getProperty("Value").toString();
                    
                    if(value.contains("=")) {
                        //std::cout << param.getProperty("Value").toString() << std::endl;
                        int idx = value.indexOf("=");
                        String fixedval = value.substring(idx + 1);
                        if(!fixedval.startsWith("0") && fixedval.getDoubleValue() == 0)
                            std::cout << "err:" << fixedval << std::endl;
                        param.setProperty("Value", fixedval, nullptr);
                        count++;
                    }
                }

            }
            
        }
        std::cout << "Count:" << count << std::endl;
        
        FileOutputStream foutstr(presetDir.getChildFile("newlib.lib"));
        
        PresetManager::presetDictionary.writeToStream(foutstr);
        
        presetDir.getChildFile("newlib.xml").replaceWithText(PresetManager::presetDictionary.toXmlString());
         */
	}

    
    void loadPresets ()
    {
        
        File presetDir = (File::getSpecialLocation(File::userDocumentsDirectory)).getChildFile("Cerite").getChildFile("Presets");
        
        FileOutputStream ffg(presetDir.getChildFile("user.lib"));
        ValueTree("User").writeToStream(ffg);
        
        FileInputStream factorystream(presetDir.getChildFile("factory.lib"));
        //PresetManager::factoryPresets = ValueTree::readFromStream(factorystream);
        
        FileInputStream userstream(presetDir.getChildFile("user.lib"));
        //PresetManager::userPresets = ValueTree::readFromStream(userstream);
        
        
    }
	void shutdown() override
	{
		// Add your application's shutdown code here..

        MainComponent* mainPtr = static_cast<MainComponent*>(mainWindow->getChildComponent(0));
        if(mainPtr->askToSave()) {
            mainWindow = nullptr; // (deletes our window)
        };
        
		
	}

	//==============================================================================
	void systemRequestedQuit() override
	{
		// This is called when the app is being asked to quit: you can ignore this
		// request and let the app carry on running, or call quit() to allow the app to close.
		quit();
	}

	void anotherInstanceStarted (const String& commandLine) override
	{
		// This is called when we use "open with" on this app
		MainComponent* mainPtr = static_cast<MainComponent*>(mainWindow->getChildComponent(0));
		StringArray tokens;
		tokens.addTokens (commandLine, " ", "\"");

		for(int i = 0; i < tokens.size(); i++)
		{
			File pFile = File(tokens[i]);

			if(pFile.exists() && pFile.getFileExtension().equalsIgnoreCase(".clab"))
			{
				mainPtr->canvas.projectFile = pFile;
				mainPtr->setTitle(pFile.getFileName().toStdString() + " | Cerite");
				mainPtr->canvas.setState(pFile.loadFileAsString());
				mainPtr->canvas.addToHistory(pFile); // maybe do this in the toolbar when redesigning FSManager
				return;
			}
		}

		std::cout << "Could not open project" << std::endl;
	}

	//==============================================================================
	/*
	 This class implements the desktop window that contains an instance of
	 our MainComponent class.
	 */
	class MainWindow    : public DocumentWindow
	{

	public:
		MainWindow (String name)  : DocumentWindow (name,
			        Desktop::getInstance().getDefaultLookAndFeel()
			        .findColour (ResizableWindow::backgroundColourId),
			        DocumentWindow::allButtons)
		{
			setUsingNativeTitleBar (true);
			setContentOwned (MainComponent::getInstance(), false);
#if JUCE_IOS || JUCE_ANDROID
			setFullScreen (true);
#else
			setResizable (true, true);
			centreWithSize (getWidth(), getHeight());
#endif
			setVisible (true);
		}

		void closeButtonPressed() override
		{
			// This is called when the user tries to close this window. Here, we'll just
			// ask the app to quit when this happens, but you can change this to do
			// whatever you need.
			JUCEApplication::getInstance()->systemRequestedQuit();
		}

		/* Note: Be careful if you override any DocumentWindow methods - the base
		 class uses a lot of them, so by overriding you might break its functionality.
		 It's best to do all your work in your content component instead, but if
		 you really have to override any DocumentWindow methods, make sure your
		 subclass also calls the superclass's method.
		 */

	private:
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
	};

	std::unique_ptr<MainWindow> mainWindow;
};

//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION (CeriteGUI)
