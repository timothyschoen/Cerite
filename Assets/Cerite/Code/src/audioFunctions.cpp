// OSC libraries
#include <lo/lo.h>
#include <lo/lo_cpp.h>

#include <unistd.h>
#include "../include/RtAudio.h"
#include "../include/AudioFile.h"
#include "../include/audioFunctions.h"
#include "../include/inputFunctions.h"
#include "../patch.h"


int samplerate = 44100;
int length = 8 * samplerate;

Patch patch;

void exportFile(std::string path) {

        // Set up audio file for non-realtime mode

        AudioFile<double> audioOutFile;

        audioOutFile.setAudioBufferSize (1, length);

        // Tick the system and put the output in a file

        auto t1 = std::chrono::high_resolution_clock::now();
        for (int i = 0; i<length; i++)
        {

                if(i%20000 == 0)
                {
                        std::cout << int(i * (100./float(length))) << "%" << std::endl;
                }


                audioOutFile.samples[0][i] = patch.process(0)*0.6;


        }


        auto t2 = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( t2 - t1 ).count();


        // Export
        audioOutFile.setBitDepth (24);
        audioOutFile.setSampleRate (44100);

        audioOutFile.save(path);
        std::cout << "Saved as: " << path << std::endl;
        std::cout << "Export time: " << duration/1000. << "s" << std::endl;
        patch.destroy();

}

unsigned int bufferFrames, fs, offset = 0;
unsigned int channs;
RtAudio::StreamOptions options;
const unsigned int callbackReturnValue = 1;
int buffersize;
unsigned int frameCounter = 0;
bool checkCount = false;
unsigned int nFrames = 0;
unsigned int bufferBytes;

void audio_init(){

        // Set buffer size and sample rate
        buffersize = 512;
        fs = 44100;

        // Init OSC for sliders

        lo::ServerThread st(9004);

        if (!st.is_valid()) {
                std::cout << "Could not open OSC Server. Make sure channel 9000 is free" << std::endl;
        }

        st.start();
        /*
           for (size_t i = 0; i < 16; i++) {
                int idx = i;
                std::string address = "/slider" + std::to_string(i);
                st.add_method(address, "f",
                              [&, idx](lo_arg **argv, float){
                        process.setSlider(idx, argv[0]->f);
                });
           } */

        st.add_method("/slider0", "f",
                      [&](lo_arg **argv, float){
                patch.setSlider(0, argv[0]->f);
        });

        // Init the patch

        patch.init(); // Initialze the circuit


}

std::vector<unsigned char> message;


int audioloop( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
               double /*streamTime*/, RtAudioStreamStatus status, void *data )
{
        unsigned int i;
        double samp;
        extern unsigned int channs;
        double* inbuf = (double *)inputBuffer;
        double *buffer = (double *) outputBuffer; // output buffer that we pass back to rtaudio


        for ( i=0; i<nBufferFrames; i++ )
        {
                samp = patch.process(inbuf[i])*0.6;
                *buffer++ = samp; // get left output from the netlist
                *buffer++ = samp; // get right output from the netlist

        }


        frameCounter += nBufferFrames;
        if ( checkCount && ( frameCounter >= nFrames ) ) return callbackReturnValue;
        return 0;
}


void realtime_audio() {
        RtAudio dac;

        if ( dac.getDeviceCount() < 1 )
        {
                std::cout << "\nNo audio devices found!\n";
                exit( 1 );
        }

        channs = 2;


        double *data = (double *) calloc( channs, sizeof( double ) );

        // Let RtAudio print messages to stderr.
        dac.showWarnings( true );

        // Set our stream parameters
        bufferFrames = buffersize;
        RtAudio::StreamParameters oParams;
        oParams.deviceId = dac.getDefaultOutputDevice();
        oParams.nChannels = channs;
        oParams.firstChannel = offset;

        RtAudio::StreamParameters iParams;
        iParams.deviceId = dac.getDefaultInputDevice();
        iParams.nChannels = 1;
        iParams.firstChannel = offset;

        options.flags = RTAUDIO_HOG_DEVICE;
        options.flags = RTAUDIO_SCHEDULE_REALTIME;

        try
        {
                // Open the stream, passes the audio processing function
                dac.openStream( &oParams, &iParams, RTAUDIO_FLOAT64, samplerate, &bufferFrames, &audioloop, (void *)&bufferBytes, &options );
                dac.startStream();
        }
        catch ( RtAudioError& e )
        {
                e.printMessage();
                goto cleanup;
        }

        if ( checkCount )
        {
                while ( dac.isStreamRunning() == true ) usleep( 100 );
        }
        else
        {
                std::cout << "Stream latency = " << dac.getStreamLatency() << "\n" << std::endl;

                while(true) {
                        usleep(10000);
                }

                try
                {
                        // Stop the stream
                        dac.stopStream();
                }
                catch ( RtAudioError& e )
                {
                        e.printMessage();
                }
        }

cleanup:
        patch.destroy();
        if ( dac.isStreamOpen() ) dac.closeStream();
        free( data );

}
