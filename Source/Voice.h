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
    float freq = 0.0f;   // frecuencia correspondiente, base frequency
    float velocityGain = 0.997f;
    float osc2Gain = 1.0f;
    float panLeft, panRight;
    float randomPan = 0.0f; // [-1..1]
    float stereoWidth = 1.0f;

    //int age = 0;          // contador simple
    bool released = false; // flag simple

    OscillatorPolyBLEP osc1;
    OscillatorPolyBLEP osc2;
    Envelope env;

    void startNote(int midiNote)
    {
        note = midiNote;
        released = false;
        //freq = 440.0f * std::pow(2.0f, (note - 69) / 12.0f); // MIDI → Hz
    }

    void stopNote()
    {
        note = -1;
        freq = 0.0f;
        released = false;
    }

    void reset()
    {
        stopNote();
        released = false;
        //age = 0;

        env.reset();
        osc1.reset();
        osc2.reset();

        panLeft = 0.707f;
        panRight = 0.707f;
    }

    float render(float noise, bool usePwm)
    {
        // si se activa esta parte es el jx tradicional (sonido tradicional)
        float sample1 = osc1.nextSample();
        //float sample2 = osc2.nextSample() * osc2Gain;
        float sample2 = (usePwm ? osc2.squarePWM() : osc2.nextSample()) * osc2Gain;
        float output = sample1 - sample2 + noise;
        float envelope = env.nextValue();
        return output * envelope * velocityGain;
        
        /*
        // Balance entre osciladores (en lugar de suma directa) sonido más gordo
        float osc1Sample = osc1.nextSample() * (1.0f - osc2Gain);
        float osc2Sample = osc2.nextSample() * osc2Gain;

        // Suma balanceada + ruido
        float output = osc1Sample + osc2Sample + noise;

        // Aplicar envelope
        output *= env.nextValue() * velocityGain;

        return output;
        */
}

    void release()
    {
        released = true;
        env.release();
    }
    
    void updatePanning(int /*voiceIndex*/)
    {
        // 1. "t" define qué tanto efecto estéreo aplicamos (0 en graves, 1 en agudos)
    // Notas bajas ( < 40) estarán en el centro (mono).
        float t = std::clamp((note - 40.0f) / 48.0f, 0.0f, 1.0f);

        // 2. Determinamos el lado según el índice de voz (Voz 0, 2, 4... Izq | Voz 1, 3, 5... Der)
        //float side = (voiceIndex % 2 == 0) ? -0.8f : 0.8f;

        float pianoDirection = 1.0f;

        // 3. Mezclamos el lado fijo con un toque de tu randomPan original para dar variedad
        float panningBase = (pianoDirection * 0.8f) + (randomPan * 0.2f);

        // 4. Aplicamos el ancho estéreo (stereoWidth) y la intensidad por nota (t)
        float panning = panningBase * stereoWidth * t;

        // 5. Ley de potencia constante (para que no baje el volumen al panear)
        panLeft = std::sin(PI_OVER_4 * (1.0f - panning));
        panRight = std::sin(PI_OVER_4 * (1.0f + panning));
    }
};