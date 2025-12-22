/*
  ==============================================================================

    Synth.cpp
    Created: 10 Nov 2025 6:45:11pm
    Author:  Jhonatan

  ==============================================================================
*/

#include "Synth.h"
#include "Utils.h"

Synth::Synth()
{
    sampleRate = 48000.0f; // originallly 44100.0f
}
void Synth::noteOn(int note, int velocity)
{
    voice.note = note;
    voice.updatePanning();

    float freq = 440.0f*std::exp2((float(note-69)+tune)/12.0f);
    voice.velocityGain = (velocity / 127.0f) * 0.5f;
   
    // osc1 / osc2: PolyBLEP usa frecuencia (no period)
    voice.osc1.setFrequency(freq*pitchBend);
    voice.osc1.reset();
    
    voice.osc2.setFrequency(freq* pitchBend *detune);
    voice.osc2.reset();
    voice.osc2Gain = oscMix;
    //voice.osc2Gain = voice.velocityGain * oscMix;
    
    Envelope& env = voice.env;

    // Configurar ADSR
    env.attackMultiplier  = envAttack;
    env.decayMultiplier   = envDecay;
    env.sustainLevel      = envSustain;
    env.releaseMultiplier = envRelease;

    // Iniciar ataque
    env.attack();

}

void Synth::noteOff(int note)
{
    if (voice.note == note)
    {
        voice.env.release();
        voice.note = -1;
    }
}

void Synth::allocateResources(double sampleRate_, int /*samplesPerBlock*/)
{
    sampleRate = static_cast<float>(sampleRate_);
    voice.osc1.prepare(sampleRate);
    voice.osc2.prepare(sampleRate);
}
void Synth::deallocateResources()
{
    
}
void Synth::reset()
{
    voice.reset();
    noiseGen.reset();
    pitchBend = 1.0f;
}
void Synth::render(float** outputBuffers, int sampleCount)
{
    float* outputBufferLeft = outputBuffers[0];
    float* outputBufferRight = outputBuffers[1];

    for (int sample = 0; sample < sampleCount; ++sample) {
        float noise = noiseGen.nextValue() * noiseMix;

        float outL = 0.0f;
        float outR = 0.0f;

        if (voice.env.isActive()) {
            float mono = voice.render(noise);
            outL = mono * voice.panLeft;
            outR = mono * voice.panRight;
        }
        if (outputBufferRight != nullptr)
        {
            outputBufferLeft[sample] = outL;
            outputBufferRight[sample] = outR;
        }
        else
        {
            outputBufferLeft[sample] = 0.5f * (outL + outR);
        }
    }
       
    protectYourEars(outputBufferLeft, sampleCount);
    protectYourEars(outputBufferRight, sampleCount);
}

void Synth::midiMessage(uint8_t data0, uint8_t data1, uint8_t data2)
{
    switch (data0 & 0xF0) {
        // Note off
        case 0x80:
            noteOff(data1 & 0x7F);
            break;

        // Note on
        case 0x90: {
            uint8_t note = data1 & 0x7F;
            uint8_t velo = data2 & 0x7F;
            if (velo > 0) {
                noteOn(note, velo);
            } else {
                noteOff(note);
            }
            break;
        }
        // Pitch bend
        case 0xE0:
        {
            pitchBend = std::exp(0.000014102f * float((data1 + 128 * data2) - 8192));

            // Pitch bend en tiempo real: actualizar osciladores si hay nota activa
            if (voice.env.isActive())
            {
                float baseFreq = 440.0f * std::exp2((float(voice.note - 69) + tune) / 12.0f);

                voice.osc1.setFrequency(baseFreq * pitchBend);
                voice.osc2.setFrequency(baseFreq * pitchBend * detune);
            }
            break;
        }
    }
}