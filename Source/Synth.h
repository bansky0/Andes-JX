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
#include "Constants.h"

class Synth
{
public:
    Synth();
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
    float pwmDepth = 0.0f;
    
    int glideMode = 0;
    float glideRate = 1.0f;
    float glideBend = 0.0f;

    juce::LinearSmoothedValue<float> outputLevelSmoother;

    void allocateResources(double sampleRate, int samplesPerBlock);
    //void deallocateResources();
    void reset();
    void render(float** outputBuffers, int sampleCount);
    void midiMessage(uint8_t data0, uint8_t data1, uint8_t data2);
    void startVoice(int v, int note, int velocity);
    int findVoiceForNote(int note) const;
    int findFreeVoice(int note) const;
    float calcBaseFreq(int v, int note) const;
    void controlChange(uint8_t data1, uint8_t data2);
    void setLfoRateHz(float hz);
    void setLfoDepthSemis(float semis);
    void setPwmDepth(float depth);

    float noiseMix{};

private:
    // --- Key tracking ---
    std::array<bool, 128> keyDown{};     // true si la tecla está presionada
    std::array<int, 128>  keyStack{};    // stack de notas presionadas (orden)
    int keyStackSize = 0;

    bool noteIsDown(int note) const;
    void pushKey(int note);
    void releaseKey(int note);
    int  topKey() const;                // última nota presionada o -1
    bool legatoOnThisNoteOn(int note) const; // true si había otra tecla presionada antes de este noteOn

    void noteOn(int note, int velocity);
    void noteOff(int note);
    void restartMonoVoice(int note, int velocity);
    //void shiftQueuedNotes();
    //int nextQueuedNote();

    bool isPlayingLegatoStyle() const;
    int lastNote = -1;


    bool sustainPedalPressed{};
    //int queuedVelocity[MAX_VOICES]{};
    
    float sampleRate{};
    std::array<Voice, MAX_VOICES> voices;
    NoiseGenerator noiseGen;
    juce::Random rng;
    
    Oscillator lfo;
    int lfoCounter = 0;
    float lfoValue = 0.0f;
    float lfoPitchMul = 1.0f;
    float modWheel = 0.0f;


};