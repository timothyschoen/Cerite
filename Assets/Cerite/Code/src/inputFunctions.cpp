#include <unistd.h>
#include <iosfwd>
#include <vector>
#include "../include/inputFunctions.h"

int midiMessage[3];
int midiin[3];

int* getMidiNote() {
        for (size_t i = 0; i < 3; i++) {
                midiin[i] = 0;
        }

        if (midiMessage[0] == 144)
        {
                for (size_t i = 0; i < 3; i++) {
                        midiin[i] = midiMessage[i];
                }
        }
        else if (midiMessage[0] == 128)
        {
                for (size_t i = 0; i < 2; i++) {
                        midiin[i] = 0;
                }
                midiin[2] = midiMessage[2];
        }
        return midiin;
}


int* getMidiCC()  {
        for (size_t i = 0; i < 3; i++) {
                midiin[i] = 0;
        }
        if (midiMessage[0] == 176) {
                {
                        for (size_t i = 0; i < 3; i++) {
                                midiin[i] = midiMessage[i];
                        }
                }
        }
        return midiin;
}


void setMidiInput(std::vector<unsigned char> mess)  {

        if(mess.size() > 0)  {
                for (size_t i = 0; i < 3; i++) {
                        midiMessage[i] = (int)mess[i];
                }
        }
}


void getOSC(std::vector<unsigned char> mess)  {

}



void setOSC(std::vector<unsigned char> mess)  {


}
