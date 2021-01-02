#pragma once

#include <JuceHeader.h>
#include "Sidebar/EngineSettings.h"
#include "Looks.h"


class ExportComponent : public Component, public Thread, public Timer
{
    
    double progress = 0;
    int numBlocks;
    int formatidx;
    
    int samplerate;
    CeriteLookAndFeel clook;
    AudioAppComponent* main;
    
    OwnedArray<AudioFormat> formats = {new WavAudioFormat, new AiffAudioFormat, new FlacAudioFormat, nullptr, new OggVorbisAudioFormat};
    
    
    Array<String> samplerates = {"22050", "44100", "48000", "96000", "192000"};
    Array<String> extensions = {"*.wav", "*.aif", "*.flac", "*.mp3", "*.ogg"};
    std::unique_ptr<FileChooser> saveChooser;
    
    File destination = File("~/test23432bhb.wav");
    
    std::atomic<bool> halt = false;
    std::vector<double> settings;
        
    std::unique_ptr<dsp::Oversampling<float>> oversampler;
    
    EngineSettings exportSettings;
    TextButton exportButton = TextButton("Export", "Export");
    
    TextEditor lengthEditor;
    
    ComboBox samplerateSel;
    ComboBox formatSel;
    ToggleButton startPlaying;
    ProgressBar progressbar = ProgressBar(progress);
    
    Label donelabel = Label("Done", "Done");
    
public:
    ExportComponent(AudioAppComponent* m);
    
    ~ExportComponent() {
        setLookAndFeel(nullptr);
        halt = true;
        stopThread(-1);
    };
    
    void resized() override;
    
    void paint(Graphics & g) override;
    
    void exportAudio();
    
    void timerCallback() override {
        progressbar.setVisible(false);
        donelabel.setVisible(true);
        main->setAudioChannels(2, 2);
        stopTimer();
    }
    
    void run() override;
        
};


class ExportDialog : public DialogWindow::LaunchOptions
{
public:
    
    ExportDialog(AudioAppComponent* m);

    ~ExportDialog();
    


};

