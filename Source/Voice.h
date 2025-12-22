/*
  ==============================================================================

    Voice.h
    Created: 10 Nov 2025 6:45:29pm
    Author:  Jhonatan

  ==============================================================================
*/

#pragma once

//#include "Oscillator.h"
#include "OscillatorPolyBLEP.h"
#include "Envelope.h"

struct Voice
{
    int note = -1;       // MIDI note actualmente activa
    float freq = 0.0f;   // frecuencia correspondiente
    float velocityGain = 0.997f;
    float osc2Gain = 1.0f;
    float panLeft, panRight;


    OscillatorPolyBLEP osc1;
    OscillatorPolyBLEP osc2;
    Envelope env;

    void startNote(int midiNote)
    {
        note = midiNote;
        freq = 440.0f * std::pow(2.0f, (note - 69) / 12.0f); // MIDI → Hz
        osc1.setFrequency(freq);
        osc2.setFrequency(freq);
    }

    void stopNote()
    {
        note = -1;
        freq = 0.0f;

    }

    void reset()
    {
        stopNote();
        env.reset();
        osc1.reset();
        osc2.reset();
        panLeft = 0.707f;
        panRight = 0.707f;
    }

    float render(float input)
    {
        float sample1 = osc1.nextSample();
        float sample2 = osc2.nextSample()*osc2Gain;
        float output = sample1- sample2 + input;
        float envelope = env.nextValue();
        return output * envelope * velocityGain;
}

    void release()
    {
        env.release();
    }
    
    void updatePanning()
    {
        float panning = std::clamp((note- 60.0f) / 24.0f,-1.0f, 1.0f);
        panLeft = std::sin(PI_OVER_4 * (1.0f- panning));
        panRight = std::sin(PI_OVER_4 * (1.0f + panning));
    }
};