/*
  ==============================================================================

    Synth.cpp
    Created: 10 Nov 2025 6:45:11pm
    Author:  Jhonatan

  ==============================================================================
*/

#include "Synth.h"
#include "Utils.h"
#include "Constants.h"

Synth::Synth()
{
    sampleRate = 48000.0f; // originallly 44100.0f
}

int Synth::nextQueuedNote()
{
    int held = 0;
    for (int v = MAX_VOICES - 1; v > 0; v--) 
    {
        if (voices[v].note > 0) { held = v; }
    }
    if (held > 0) 
    {
        int note = voices[held].note;
        voices[held].note = 0;
        return note;
    }
    return 0;
}

void Synth::shiftQueuedNotes()
{
    for (int tmp = MAX_VOICES - 1; tmp > 0; tmp--) {
        voices[tmp].note = voices[tmp - 1].note;
        //voices[tmp].release();
    }
}

void Synth::restartMonoVoice(int note, int velocity)
{
    Voice& voice = voices[0];

    float freq = calcBaseFreq(0, note);
    voice.freq = freq;
    voice.env.level += SILENCE + SILENCE;
    voice.note = note;
    voice.released = false;
    voice.updatePanning(0);
    /*
    if (velocity >= 0)
        voice.velocityGain = (velocity / 127.0f) * 0.5f;
    */
    voice.osc1.setFrequency(freq * pitchBend);
    voice.osc2.setFrequency(freq * pitchBend * detune);
}

void Synth::controlChange(uint8_t data1, uint8_t data2)
{

    switch (data1) {
        // All notes off
    default:
        if (data1 >= 0x78) {
            for (int v = 0; v < MAX_VOICES; ++v) {
                voices[v].reset();
            }
            sustainPedalPressed = false;
        }
        break;
        // Sustain pedal
    case 0x40:
        sustainPedalPressed = (data2 >= 64);
        if (!sustainPedalPressed) { // add this
            noteOff(SUSTAIN);
        }
        break;
    }
}

float Synth::calcBaseFreq(int v, int note) const
{
    // offset en semitonos: note + (ANALOG * v)
    // + tune (tambi	n en semitonos)
    const float semis = (float(note) + ANALOG * float(v) + tune);
    return 440.0f * std::exp2((semis - 69.0f) / 12.0f);
}

int Synth::findVoiceForNote(int note) const
{
    for (int v = 0; v < MAX_VOICES; ++v)
        if (voices[v].note == note && voices[v].env.isActive())
            return v;
    return -1;
}

int Synth::findFreeVoice(int note) const
{
    // 1) Reuse same note if already playing
    if (int same = findVoiceForNote(note); same >= 0)
        return same;

    // 2) Prefer voices already released (least audible to steal)
    int best = -1;
    float bestLevel = 1e9f;

    for (int v = 0; v < MAX_VOICES; ++v)
    {
        const auto& voice = voices[v];
        if (voice.env.isActive() && voice.released)
        {
            if (voice.env.level < bestLevel)
            {
                bestLevel = voice.env.level;
                best = v;
            }
        }
    }
    if (best >= 0) return best;

    // 3) Otherwise steal the quietest voice not in attack
    best = 0;
    bestLevel = 1e9f;
    for (int v = 0; v < MAX_VOICES; ++v)
    {
        const auto& voice = voices[v];
        if (voice.env.level < bestLevel && !voice.env.isInAttack())
        {
            bestLevel = voice.env.level;
            best = v;
        }
    }
    return best;
}

void Synth::startVoice(int v, int note, int velocity)
{
    Voice& voice = voices[v];
    voice.randomPan = rng.nextFloat() * 2.0f - 1.0f;
    voice.stereoWidth = stereoWidth;
    voice.startNote(note);
    voice.updatePanning(v);

    //float freq = 440.0f * std::exp2((float(note - 69) + tune) / 12.0f);
    const float freq = calcBaseFreq(v, note);
    voice.freq = freq;
    voice.velocityGain = (velocity / 127.0f) * 0.5f;

    // osc1 / osc2: PolyBLEP usa frecuencia (no period)
    voice.osc1.setFrequency(freq * pitchBend);
    voice.osc1.reset();

    voice.osc2.setFrequency(freq * pitchBend * detune);
    voice.osc2.reset();
    voice.osc2Gain = oscMix;
    //voice.osc2Gain = voice.velocityGain * oscMix;

    Envelope& env = voice.env;

    // Configurar ADSR
    env.attackMultiplier = envAttack;
    env.decayMultiplier = envDecay;
    env.sustainLevel = envSustain;
    env.releaseMultiplier = envRelease;

    // Iniciar ataque
    env.attack();
}

void Synth::noteOn(int note, int velocity)
{
    if (numVoices == 1)
    {
        // Legato SOLO si aún hay una nota "held"
        if (voices[0].note > 0)
        {
            shiftQueuedNotes();
            restartMonoVoice(note, velocity);   // NO reinicia attack
        }
        else
        {
            startVoice(0, note, velocity);      // SÍ reinicia attack
        }
        return;
    }

    int v = findFreeVoice(note);
    startVoice(v, note, velocity);
        /*
        if (voices[0].env.isActive())
        {
            if (voices[0].note > 0)
            {
                shiftQueuedNotes();
            }
            // Legato
            restartMonoVoice(note, velocity);
        }
        else
        {
            startVoice(0, note, velocity);
        }
    }
    else
    {
        int v = findFreeVoice(note);
        startVoice(v, note, velocity);
    }
    */
}

void Synth::noteOff(int note)
{
    if (numVoices == 1)
    {
        // libera la voz 0 si es esa nota
        if (voices[0].note == note)
        {
            if (sustainPedalPressed)
                voices[0].note = SUSTAIN;
            else
            {
                voices[0].release();
                voices[0].note = 0; // clave para que el próximo noteOn NO sea legato
            }

            int queued = nextQueuedNote();
            if (queued > 0)
                restartMonoVoice(queued, -1);
        }

        // además, limpia cualquier nota en la cola
        for (int v = 1; v < MAX_VOICES; ++v)
            if (voices[v].note == note) voices[v].note = 0;

        return;
    }
    /*
    if ((numVoices == 1) && (voices[0].note == note))
    {
        int queuedNote = nextQueuedNote();
        if (queuedNote > 0)
        {
            restartMonoVoice(queuedNote, -1);
        }
    }
    */
    for (int v = 0; v < MAX_VOICES; v++) 
    {
        if (voices[v].note == note) 
        {
            if (sustainPedalPressed) 
            {
                voices[v].note = SUSTAIN;
            }
            else {
                voices[v].release();
                voices[v].note = 0;
            }
        }
    }
}

void Synth::allocateResources(double sampleRate_, int /*samplesPerBlock*/)
{
    sampleRate = static_cast<float>(sampleRate_);
    outputLevelSmoother.reset(sampleRate_, 0.02);
    for (int v = 0; v < MAX_VOICES; ++v)
    {
        voices[v].osc1.prepare(sampleRate);
        voices[v].osc2.prepare(sampleRate);
        voices[v].reset(); // opcional pero recomendable al preparar
    }  
}
void Synth::deallocateResources()
{
    
}
void Synth::reset()
{
    for (int v = 0; v < MAX_VOICES; ++v) 
    {
        voices[v].reset();
    }
    noiseGen.reset();
    pitchBend = 1.0f;
    sustainPedalPressed = false;
}
void Synth::render(float** outputBuffers, int sampleCount)
{
    float* outputBufferLeft = outputBuffers[0];
    float* outputBufferRight = outputBuffers[1];

    // 1) Pre-update por voz (1 vez por bloque)
    for (int v = 0; v < MAX_VOICES; ++v)
    {
        Voice& voice = voices[v];

        if (voice.env.isActive())
        {
            voice.stereoWidth = stereoWidth;   // <- update real-time
            voice.updatePanning(v);

            voice.osc1.setFrequency(voice.freq * pitchBend);
            voice.osc2.setFrequency(voice.freq * pitchBend * detune);
        }
    }

    // 2) Render samples
    for (int sample = 0; sample < sampleCount; ++sample) 
    {
        float noise = noiseGen.nextValue() * noiseMix;

        float outL = 0.0f;
        float outR = 0.0f;

        for (int v = 0; v < MAX_VOICES; ++v)
        {
            Voice& voice = voices[v];

            if (voice.env.isActive())
            {
                float mono = voice.render(noise);
                outL += mono * voice.panLeft;
                outR += mono * voice.panRight;
            }
        }

        float driveL = outL * volumeTrim;
        float driveR = outR * volumeTrim;

        outL = driveL / (1.0f + std::abs(driveL));
        outR = driveR / (1.0f + std::abs(driveR));

        float gain = outputLevelSmoother.getNextValue();

        outL *= gain;
        outR *= gain;

        
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
        // 3) Limpieza de voces inactivas (1 vez por bloque)
        for (int v = 0; v < MAX_VOICES; ++v)
        {
            Voice& voice = voices[v];

            if (numVoices == 1 && v > 0)
                continue;

            if (!voice.env.isActive())
            {
                //Resetear envelope cuando ya murio
                voice.env.reset();
                voice.note = -1; // opcional: marca voz libre
                voice.released = false;
                voice.randomPan = 0.0f;
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
            //break;
            // Pitch bend en tiempo real: actualizar osciladores si hay nota activa
            for (int v = 0; v < MAX_VOICES; ++v)
            {
                Voice& voice = voices[v];
                if (voice.env.isActive())
                {
                    voice.osc1.setFrequency(voice.freq * pitchBend);
                    voice.osc2.setFrequency(voice.freq * pitchBend * detune);
                }
            }
            break;
        }
        // Control change
        case 0xB0:
            controlChange(data1, data2);
            break;
    }
}