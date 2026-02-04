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
    sampleRate = 48000.0f; // originally 44100.0f


}
/*
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
*/
/*
void Synth::shiftQueuedNotes()
{
    for (int tmp = MAX_VOICES - 1; tmp > 0; tmp--) {
        voices[tmp].note = voices[tmp - 1].note;
        //voices[tmp].release();
    }
}
*/
void Synth::restartMonoVoice(int note, int /*velocity*/)
{
    Voice& voice = voices[0];

    const float freq = calcBaseFreq(0, note);

    // actualizar nota base
    voice.freq = freq;
    voice.freqTarget = freq;

    const bool wasLegato = true;

    bool shouldGlide = false;
    if (glideMode == 2) shouldGlide = true;           // Always
    else if (glideMode == 1) shouldGlide = wasLegato; // Legato
    // glideMode==0 => Off

    voice.glideRateThisNote = shouldGlide ? glideRate : 1.0f;

    // Arranque de frecuencia
    if (!shouldGlide || lastNote < 0)
    {
        voice.freqCurrent = freq;
    }
    else
    {
        const int noteDistance = note - lastNote;
        const float startSemis = float(noteDistance) - glideBend;
        voice.freqCurrent = freq * std::pow(1.059463094359f, -startSemis);
    }
    lastNote = note;

    voice.osc1.setFrequency(voice.freqCurrent * pitchBend * lfoPitchMul);
    voice.osc2.setFrequency(voice.freqCurrent * pitchBend * detune * lfoPitchMul);

    // legato = NO retrigger envelope
    // AGREGAR: Actualizar envelope sin retrigger
    Envelope& env = voice.env;
    
    /*
    // Si estaba en release, volver a sustain
    if (env.level < envSustain) {
        // Retomar attack suavemente
        env.attack();
    }
    */
    // Actualizar parámetros del envelope
    env.attackMultiplier = envAttack;
    env.decayMultiplier = envDecay;
    env.sustainLevel = envSustain;
    env.releaseMultiplier = envRelease;

    /*
    // Actualizar velocity solo si se proporciona
    if (velocity > 0 && !ignoreVelocity) {
        float vel = 0.004f * float((velocity + 64) * (velocity + 64)) - 8.0f;
        voice.velocityGain = 0.01f * vel;
    }
    */
    //voice.env.level += SILENCE + SILENCE;
    voice.note = note;
    voice.released = false;
    voice.updatePanning(0);
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
        
        // Mod wheel (CC1)
        case 0x01: 
        modWheel = 0.000005f * float(data2 * data2);
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
    
    // 1) Detecta legato ANTES de startNote()
    const bool wasLegato = (numVoices == 1) && isPlayingLegatoStyle();

    // 2) Marca la nota como activa
    voice.startNote(note);
    voice.randomPan = rng.nextFloat() * 2.0f - 1.0f;
    //voice.stereoWidth = stereoWidth;
    //voice.updatePanning(v);
    
    const float freq = calcBaseFreq(v, note);
    voice.freqTarget = freq;
    
    
    // 3) Decide si debe hacer glide según modo
    bool shouldGlide = false;
    
    if (numVoices == 1) {  // SOLO en modo mono
        if (glideMode == 2) shouldGlide = true;
        else if (glideMode == 1) shouldGlide = wasLegato;
    }
    /*
    if (glideMode == 2) shouldGlide = true;           // Always
    else if (glideMode == 1) shouldGlide = wasLegato; // Legato
    // glideMode == 0 => Off
    */


    voice.glideRateThisNote = shouldGlide ? glideRate : 1.0f;

    // 4) Punto de arranque
    if (!shouldGlide || lastNote < 0)
        {
            voice.freqCurrent = freq; // sin glide: cae directo
        }
        else
        {
            const int noteDistance = note - lastNote;
            const float startSemis = float(noteDistance) - glideBend;
            voice.freqCurrent = freq * std::pow(1.059463094359f, -startSemis);
        }

        if (numVoices == 1) {
            lastNote = note;
        }
        /*
        lastNote = note;
        */
        // osciladores arrancan con freqCurrent (como ya tienes)
        voice.osc1.setFrequency(voice.freqCurrent * pitchBend);
        voice.osc1.reset();
        voice.osc2.setFrequency(voice.freqCurrent * pitchBend * detune);
        voice.osc2.reset();
    
    float vel = 0.004f * float((velocity + 64) * (velocity + 64))- 8.0f;
    voice.velocityGain = 0.01f * vel;
    

    // Sincronizar fase en modo PWM
    if (lfoDepthSemis == 0.0f && pwmDepth > 0.0f) {
        voice.osc2.syncPhase(voice.osc1);  // Sincronizar con osc1
    }

    voice.osc2Gain = oscMix;
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
    if (ignoreVelocity) { velocity = 80; }

    const bool wasLegato = legatoOnThisNoteOn(note);

    pushKey(note);

    if (numVoices == 1)
    {
        if (!voices[0].env.isActive() || !wasLegato)
        {
            // primer ataque (o no-legato): retrigger normal
            startVoice(0, note, velocity);
        }
        else
        {
            // legato mono: no attack, solo cambio de nota (+glide)
            restartMonoVoice(note, velocity);
        }
        return;
    }

    // --- POLY ---

    int v = findFreeVoice(note);
    startVoice(v, note, velocity);
    /*
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
    */

    
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
    // actualiza key tracking
    releaseKey(note);
    
    if (numVoices == 1)
    {
        // Si la nota liberada era la que estaba asignada a la voz mono:
        if (voices[0].note == note)
        {
            const int next = topKey(); // última tecla aún presionada, o -1

            if (next >= 0)
            {
                // seguir tocando con la nota siguiente sin retrigger (legato)
                restartMonoVoice(next, 0);
            }
            else
            {
                // no hay teclas: release real
                if (sustainPedalPressed)
                {
                    voices[0].note = SUSTAIN; // con sustain
                }
                else
                {
                    voices[0].release();
                    voices[0].note = -1; // NO 0
                }
            }
        }

        return;
    }

    /*
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
    */
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

    // --- POLY ---
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
                voices[v].note = -1;
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
    
    lfo.prepare(sampleRate_);
    lfo.setWaveType(WaveType::Sine);
    lfo.setFrequency(lfoRateHz); 
    lfo.reset();
    lfoCounter = 0;
    lfoPitchMul = 1.0f;

}
/*
void Synth::deallocateResources()
{
    
}
*/

void Synth::reset()
{
    for (int v = 0; v < MAX_VOICES; ++v) 
    {
        voices[v].reset();
    }
    noiseGen.reset();
    pitchBend = 1.0f;
    sustainPedalPressed = false;
    modWheel = 0.0f;
    lastNote = 0;
}

void Synth::render(float** outputBuffers, int sampleCount)
{
    float* outputBufferLeft  = outputBuffers[0];
    float* outputBufferRight = outputBuffers[1];

    // 1) Pre-update por voz (1 vez por bloque)
    for (int v = 0; v < MAX_VOICES; ++v)
    {
        Voice& voice = voices[v];
        if (voice.env.isActive())
        {
            voice.stereoWidth = stereoWidth;
            voice.updatePanning(v);

            //voice.osc1.setFrequency(voice.freqCurrent * pitchBend * lfoPitchMul);
            //voice.osc2.setFrequency(voice.freqCurrent * pitchBend * detune * lfoPitchMul);
        }
    }

    // 2) Render samples
    for (int sample = 0; sample < sampleCount; ++sample)
    {
        // ---- LFO update cada 32 samples ----
        if (++lfoCounter >= LFO_MAX)
        {
            lfoCounter = 0;
            const float lfoSine = lfo.nextSample(); // -1..+1

            const float modWheelSemis = modWheel * 12.0f;
            const float totalVibratoSemis = lfoDepthSemis + modWheelSemis;
            const float vibratoPitchMul = std::exp2((lfoSine * totalVibratoSemis) / 12.0f);

            lfoPitchMul = vibratoPitchMul;

            const float totalPwmDepth = juce::jlimit(0.0f, 0.45f, pwmDepth);
            const float pwmWidth = juce::jlimit(0.05f, 0.95f, 0.5f + lfoSine * totalPwmDepth);

            // actualizar voces activas SOLO cuando el LFO cambia
            for (int v = 0; v < MAX_VOICES; ++v)
            {
                Voice& voice = voices[v];
                if (!voice.env.isActive())
                    continue;

                // 1) glide one-pole (libro)star
                voice.freqCurrent += voice.glideRateThisNote * (voice.freqTarget - voice.freqCurrent);

                // 2) aplicar a osciladores
                voice.osc1.setFrequency(voice.freqCurrent * pitchBend * vibratoPitchMul);
                voice.osc2.setFrequency(voice.freqCurrent * pitchBend * detune * vibratoPitchMul);

                // 3) PWM (si aplica)
                if (pwmDepth > 0.0f)
                    voice.osc2.setPulseWidth(pwmWidth);
            }
        }

        const float noise = noiseGen.nextValue() * noiseMix;

        float outL = 0.0f;
        float outR = 0.0f;

        for (int v = 0; v < MAX_VOICES; ++v)
        {
            Voice& voice = voices[v];
            if (voice.env.isActive())
            {
                const float mono = voice.render(noise, pwmDepth > 0.0f);
                outL += mono * voice.panLeft;
                outR += mono * voice.panRight;
            }
        }

        float driveL = outL * volumeTrim;
        float driveR = outR * volumeTrim;

        outL = driveL / (1.0f + std::abs(driveL));
        outR = driveR / (1.0f + std::abs(driveR));

        const float gain = outputLevelSmoother.getNextValue();
        outL *= gain;
        outR *= gain;

        if (outputBufferRight != nullptr)
        {
            outputBufferLeft[sample]  = outL;
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
            voice.env.reset();
            voice.note = -1;
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
                    voice.osc1.setFrequency(voice.freqCurrent * pitchBend * lfoPitchMul);
                    voice.osc2.setFrequency(voice.freqCurrent * pitchBend * detune * lfoPitchMul);

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



void Synth::setLfoRateHz(float hz)
{
    lfoRateHz = hz;
    lfo.setFrequency(hz);
}



void Synth::setLfoDepthSemis(float semis)
{
    lfoDepthSemis = semis;
}



void Synth::setPwmDepth(float depth)
{
    pwmDepth = depth;
}


bool Synth::isPlayingLegatoStyle() const
{
    return keyStackSize > 1;
    /*
    int held = 0;
    for (int i = 0; i < MAX_VOICES; ++i)
    {
        if (voices[i].note > 0) { held += 1; }
    }
    return held > 0;
    */
}

bool Synth::noteIsDown(int note) const
{
    if (note < 0 || note > 127) return false;
    return keyDown[note];
}

void Synth::pushKey(int note)
{
    if (note < 0 || note > 127) return;
    if (keyDown[note]) return;

    keyDown[note] = true;
    keyStack[keyStackSize++] = note;
}

void Synth::releaseKey(int note)
{
    if (note < 0 || note > 127) return;
    if (!keyDown[note]) return;

    keyDown[note] = false;

    // remover del stack manteniendo orden
    for (int i = 0; i < keyStackSize; ++i)
    {
        if (keyStack[i] == note)
        {
            for (int j = i; j < keyStackSize - 1; ++j)
                keyStack[j] = keyStack[j + 1];
            --keyStackSize;
            break;
        }
    }
}

int Synth::topKey() const
{
    return (keyStackSize > 0) ? keyStack[keyStackSize - 1] : -1;
}

bool Synth::legatoOnThisNoteOn(int note) const
{
    // Legato real: entra un noteOn mientras hay alguna tecla presionada distinta a esa
    // (antes de hacer pushKey(note)).
    if (note < 0 || note > 127) return false;

    if (keyStackSize == 0) return false;
    if (keyStackSize == 1 && noteIsDown(note)) return false; // repeticion
    return true;
}