/*
  ==============================================================================

    Synth.h
    Created: 10 Nov 2025 6:45:11pm
    Author:  Jhonatan

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Voice.h"
#include "NoiseGenerator.h"


class Synth
{
public:
    Synth();

    void allocateResources(double sampleRate, int samplesPerblock);
    void deallocateResources();
    void reset();
    void render(float** outputBuffers, int sampleCount);
    void midiMessage(uint8_t data0, uint8_t data1, uint8_t data2);

private:
    NoiseGenerator noiseGen;

    void noteOn(int note, int velocity);
    void noteOff(int note);
    float sampleRate;
    Voice voice;

};