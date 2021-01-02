#pragma once
#include <unistd.h>
#include "RtAudio.h"
#include "AudioFile.h"

namespace Cerite_Utils {

unsigned int bufferFrames, sr = 44100, offset = 0;
unsigned int channs;
RtAudio::StreamOptions options;
unsigned int frameCounter = 0;
bool checkCount = false;
unsigned int nFrames = 0;
unsigned int bufferBytes;

std::function<double(double)> audioCallback;

// Audio loop for RtAudio
int inout( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
           double /*streamTime*/, unsigned int status, void *data )
{
        unsigned int i;
        extern unsigned int channs;
        double *buffer = (double *) outputBuffer; // output buffer that we pass back to rtaudio

        for ( i=0; i<nBufferFrames; i++ )
        {
            
            double val = audioCallback(((double*)inputBuffer)[i]);
            *buffer++ = val;
            *buffer++ = val;

        }


        frameCounter += nBufferFrames;
        if ( checkCount && ( frameCounter >= nFrames ) ) return 1;
        return 0;
}

struct AudioClass
{

    // Rtaudio error callback
    void errorCallback( RtAudioError::Type type, const std::string &errorText )
    {
            std::cout << "in errorCallback" << std::endl;
            if ( type == RtAudioError::WARNING )
                    std::cerr << '\n' << errorText << "\n\n";
            else if ( type != RtAudioError::WARNING )
                    throw( RtAudioError( errorText, type ) );
    }

    void start() {
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
        bufferFrames = 512;
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
                dac.openStream( &oParams, &iParams, RTAUDIO_FLOAT64, 44100, &bufferFrames, &inout, (void *)&bufferBytes, &options );
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
        if ( dac.isStreamOpen() ) dac.closeStream();
        free( data );
    }

    
};

}
