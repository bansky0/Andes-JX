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
    Envelope& env = voice.env;

    // Configurar ADSR
    env.attackMultiplier  = envAttack;
    env.decayMultiplier   = envDecay;
    env.sustainLevel      = envSustain;
    env.releaseMultiplier = envRelease;

    // Iniciar ataque
    env.attack();

    // Nota
    voice.startNote(note);
    voice.velocityGain = (velocity / 127.0f);
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
    voice.osc.prepare(sampleRate);
}
void Synth::deallocateResources()
{
    
}
void Synth::reset()
{
    voice.reset();
    noiseGen.reset();
}
void Synth::render(float** outputBuffers, int sampleCount)
{
    float* outputBufferLeft = outputBuffers[0];
    float* outputBufferRight = outputBuffers[1];

    for (int sample = 0; sample < sampleCount; ++sample) {
        float noise = noiseGen.nextValue() * noiseMix;

        float output = 0.0f;

        if (voice.env.isActive()) {
            output = voice.render(noise);
        }
        else
            output = 0.0f;

        outputBufferLeft[sample] = output;

        if (outputBufferRight != nullptr) {
            outputBufferRight[sample] = output;
        }
    }
    /*
    if (!voice.env.isActive()) {
        voice.env.reset();
    }
    */
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
    }
}

