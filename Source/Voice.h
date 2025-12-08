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

    OscillatorPolyBLEP osc;
    Envelope env;

    void startNote(int midiNote)
    {
        note = midiNote;
        freq = 440.0f * std::pow(2.0f, (note - 69) / 12.0f); // MIDI → Hz
        osc.setFrequency(freq);
    }

    void stopNote()
    {
        note = -1;
        freq = 0.0f;
        osc.reset();
    }

    void reset()
    {
        stopNote();
        env.reset();
    }

    float render(float noise)
    {
        if (!env.isActive())
            return 0.0f;

        float oscSignal = osc.nextSample();
        float envValue = env.nextValue();

        float mixed = oscSignal + noise;

        // Aplica ADSR + velocity
        return mixed * envValue * velocityGain;
    }

    void release()
    {
        env.release();
    }

};