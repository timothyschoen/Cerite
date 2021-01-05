#include <iostream>
#include <unistd.h>

#include "MainComponent.h"
#include "Interface/Sidebar/EngineSettings.h"
#include "Interface/Sidebar/Console.h"
#include "Utility/FSManager.h"
#include "Interface/SaveDialog.h"
#include "Interface/Components/AudioPlayerContainer.h"
#include "Engine/Worker/Source/Message.h"


//==============================================================================

MainComponent::MainComponent() : canvas(this), sidebar(this), topmenu(&canvas, this), statusbar(this), appcmds(&canvas, this), toolbar(this)
{
    setSize (1200, 720);
    setLookAndFeel(&clook);
    addAndMakeVisible(statusbar);
    viewport.setViewedComponent(&canvas, false);
    addAndMakeVisible(viewport);
    addAndMakeVisible(toolbar);
    addAndMakeVisible(sidebar);
    addAndMakeVisible(topmenu);
    
    resized();
    ((Console*)sidebar.content[2])->clear();
    commandManager.registerAllCommandsForTarget(&appcmds);
    commandManager.setFirstCommandTarget(&appcmds);
    
    launchSlaveProcess(FSManager::exec.getChildFile("CeriteWorker"), "CeritePort", 0);
    
    //pd.reset(new CeriteAudioProcessor);
    //setAudioChannels (2, 2);
}

MainComponent::~MainComponent()
{
    // This shuts down the audio device and clears the audio source.
    //shutdownAudio();
    /*
     toolbar.setLookAndFeel(nullptr);
     sidebar.setLookAndFeel(nullptr);
     canvas.setLookAndFeel(nullptr); */
    setLookAndFeel(nullptr);
    /*
     viewport.removeChildComponent(&canvas);
     removeChildComponent(&statusbar);
     removeChildComponent(&sidebar);
     removeChildComponent(&canvas);
     removeChildComponent(&viewport);
     removeChildComponent(&toolbar); */
    
    killSlaveProcess();
}


void MainComponent::updateSystem()
{
    if(bool(canvas.programState.getProperty("Power")))
    {
        
       /* Cerite::Patch* patch = canvas.createPatch().compile<Cerite::Patch>([](const char* str) {
            logMessage(str);
        }); */
        
    }
}
//==============================================================================

void MainComponent::setVolume(float level) {
    
    MemoryOutputStream memstream;
    memstream.writeInt(MessageID::Volume);
    memstream.writeFloat(level);
    
    sendMessageToSlave(memstream.getMemoryBlock());
}

void MainComponent::startAudio (double sampleRate, std::vector<double> settings)
{
   
    setVolume(Decibels::decibelsToGain ((float) statusbar.volumeSlider.getValue()));
    
    MemoryOutputStream memstream;
    memstream.writeInt(MessageID::Start);
    memstream.writeString(Compiler::exportCode(canvas.createPatch()));
    sendMessageToSlave(memstream.getMemoryBlock());
    
    
    canvas.programState.setProperty("Power", true, nullptr);
    
}

void MainComponent::stopAudio()
{
    MemoryOutputStream memstream;
    memstream.writeInt(MessageID::Stop);
    sendMessageToSlave(memstream.getMemoryBlock());
    
    statusbar.lvlmeter->setLevel(0);
    
    canvas.programState.setProperty("Power", false, nullptr);
}

//==============================================================================
void MainComponent::paint (Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
    
    if (somethingIsBeingDraggedOver)
    {
        g.setColour(findColour(ScrollBar::ColourIds::thumbColourId));
        g.drawRect(viewport.getBounds(), 3);
    }
}

void MainComponent::resized()
{
    sidebar.setBounds (getLocalBounds().removeFromRight (sidebar.getWidth()));
    viewport.setBounds (0, 40, getWidth() - sidebar.getWidth(), getHeight() - 70);
    statusbar.setBounds(0, getHeight() - 30, getWidth() - sidebar.getWidth(), 30);
    // Only make space for a topmenu on windows and linux
#if !JUCE_MAC
    toolbar.setBounds(0, 20, sidebar.getX(), 40);
    topmenu.setBounds(0, 0, getWidth(), 20);
#else
    toolbar.setBounds(0, 0, sidebar.getX(), 40);
#endif
    
}

bool MainComponent::saveAudioFile()
{
    //ExportDialog exporter(this);
    
    return true;
};


void MainComponent::setTitle(std::string newTitle)
{
    DocumentWindow* window = findParentComponentOfClass<DocumentWindow>();
    
    if(window != nullptr)
        window->setName(newTitle);
}

void MainComponent::fileDragEnter (const StringArray& files, int /*x*/, int /*y*/)
{
    somethingIsBeingDraggedOver = true;
    repaint();
}

void MainComponent::fileDragExit (const StringArray& /*files*/)
{
    somethingIsBeingDraggedOver = false;
    repaint();
}

void MainComponent::filesDropped (const StringArray& files, int x, int y)
{
    File droppedfile(files[0]);
    
    if(droppedfile.getFileExtension().equalsIgnoreCase(".clab"))
    {
        //if (canvas.askToSave())
        //canvas.openProject(files[0].toStdString()); TODO: reimplement this
    }
    else if(droppedfile.getFileExtension().equalsIgnoreCase(".wav"))
    {
        canvas.addBox(String("input " + files[0].toStdString() + " 0.3"), Point<int>(x, y));
    }
    else
    {
        std::cout << "No action for this file type" << std::endl;
    }
    
    somethingIsBeingDraggedOver = false;
    repaint();
}

bool MainComponent::askToSave()
{
    if(canvas.boxNode.getNumChildren() > 0 && canvas.changedSinceSave)
    {
        std::unique_ptr<SaveDialog> sdialog = std::make_unique<SaveDialog>();
        sdialog->setBounds(viewport.getScreenX()  + (viewport.getWidth() / 2.) - 200., viewport.getScreenY(), 400, 130);
        int action = sdialog->runModalLoop(); // appearantly this is a bad method to use but it does what i want...
        sdialog->setVisible(false);
        
        if(action == 1)
        {
            appcmds.invokeDirectly(AppCommands::keyCmd::Save, true);
            return true;
        }
        
        if(action == 2)
            return true;
        
        return false;
    }
    else
        return true;
}

void MainComponent::saveCode() {
    
    //saveChooser.reset(new FileChooser("Export Audio", FSManager::home.getChildFile("Saves"), extensions[formatidx]));
    
    // if (saveChooser->browseForFileToSave(true))
    //     destination = saveChooser->getResult();
    //else return;
    
    std::unique_ptr<Cerite::Patch> patch;
    std::string ccode = Compiler::exportCode(canvas.createPatch());
    //return ccode;
}

void MainComponent::logMessage(const char* str) {

    MainComponent::getInstance()->sidebar.console->logMessage(str);
}

void MainComponent::handleConnectionLost () {
    
    
    crashed = true;
    //jassertfalse;
    
    startTimer(100);
    //launchSlaveProcess(FSManager::exec.getChildFile("CeriteWorker"), "CeritePort", 0);
}


void MainComponent::timerCallback() {
    launchSlaveProcess(FSManager::exec.getChildFile("CeriteWorker"), "CeritePort", 0);
    stopTimer();
}

void MainComponent::handleMessageFromSlave (const MemoryBlock& m) {
    MemoryInputStream memstream(m, false);
    
    MessageID type = (MessageID)memstream.readInt();
    
    switch (type) {
        case Stop: {
            MessageManager::callAsync(
                     [=] () {
                statusbar.powerButton.setToggleState(false, sendNotification);
            });
            break;
        }
        case Ready:
            attachParameters();
            break;
        case Volume:
            statusbar.lvlmeter->setLevel(memstream.readFloat());
            break;
        case Log:
            logMessage(memstream.readString().toRawUTF8());
            break;
        case SetParam: {
            int idx = memstream.readInt();
            Data d = DataStream::readFromStream(memstream);
            if(idx < guiComponents.size())
            {
                guiComponents[idx]->receive(d);
            }
            break;
        }
        case SendProcessor: {
            int idx = memstream.readInt();
            if(memstream.readInt() == ProcessorType::AudioPlayerType) {
                int messageType = memstream.readInt();
                if(messageType == 2) {
                    static_cast<AudioPlayerContainer*>(guiComponents[idx])->setPosition(memstream.readInt());
                }
            }
            break;
        }
            
    }
}

void MainComponent::attachParameters() {
    
    Array<String> used;
    guiComponents.clear();
    
    int total = 0;
    int processorcount = 0;
    
    for(auto& box : canvas.boxmanager->objects) {
        if(box->GraphicalComponent) {
            
            guiComponents.add(box->GraphicalComponent.get());
            int count = 1;
            
            String paramname = box->type.substring(0, 3) + String(count);
            while(used.contains(paramname)) {
                paramname = box->type.substring(0, 3) + String(++count);
            }
            used.add(paramname);
            
            if(box->GraphicalComponent->type != ProcessorType::None) {
               
                MemoryOutputStream memstream;
                memstream.writeInt(MessageID::AddProcessor);
                memstream.writeInt(box->GraphicalComponent->type);
                memstream.writeInt(processorcount);
                memstream.writeInt(total);
                memstream.writeString(paramname);
                sendMessageToSlave(memstream.getMemoryBlock());
                box->GraphicalComponent->setID(processorcount);
                processorcount++;
                total++;
                continue;
            }

            // Send registration message for parameter
            MemoryOutputStream memstream;
            memstream.writeInt(MessageID::LoadParam);
            memstream.writeString(paramname);
            memstream.writeInt(total);
            sendMessageToSlave(memstream.getMemoryBlock());
            
            // Register ID with component
            box->GraphicalComponent->setID(total);
            
            total++;
        }
    }
    
}

JUCE_IMPLEMENT_SINGLETON(MainComponent);
