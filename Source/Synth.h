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
#include "Oscillator.h"

class Synth
{
public:
    Synth();
    static constexpr int MAX_VOICES = 8;
    int numVoices{};
    float envAttack{};
    float envDecay{};
    float envSustain{};
    float envRelease{};
    float oscMix{};
    float detune{};
    float tune{};
    float pitchBend{};
    float stereoWidth{};
    float volumeTrim{};
    float velocitySensitivity = 0.0f;
    bool ignoreVelocity = false;
    float lfoRateHz = 5.0f;      
    float lfoDepthSemis = 0.0f; 
    juce::LinearSmoothedValue<float> outputLevelSmoother;

    void allocateResources(double sampleRate, int samplesPerBlock);
    void deallocateResources();
    void reset();
    void render(float** outputBuffers, int sampleCount);
    void midiMessage(uint8_t data0, uint8_t data1, uint8_t data2);
    void startVoice(int v, int note, int velocity);
    int findVoiceForNote(int note) const;
    int findFreeVoice(int note) const;
    float calcBaseFreq(int v, int note) const;
    void controlChange(uint8_t data1, uint8_t data2);
    float noiseMix{};
    void setLfoRateHz(float hz)
    {
        lfoRateHz = hz;
        lfo.setFrequency(lfoRateHz / float(LFO_MAX));
    }
    
    void setLfoDepthSemis(float semis)
    {
        lfoDepthSemis = semis;
    }


private:
    void noteOn(int note, int velocity);
    void noteOff(int note);
    void restartMonoVoice(int note, int velocity);
    void shiftQueuedNotes();
    int nextQueuedNote();

    bool sustainPedalPressed{};
    int queuedVelocity[MAX_VOICES]{};
    
    float sampleRate{};
    std::array<Voice, MAX_VOICES> voices;
    NoiseGenerator noiseGen;
    juce::Random rng;
    
    Oscillator lfo;
    int lfoCounter = 0;
    static constexpr int LFO_MAX = 32;
    float lfoValue = 0.0f;
    float lfoPitchMul = 1.0f;
};