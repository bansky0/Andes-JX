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
#include "SVFFilter.h" 
#include "MoogFilter.h"

Synth::Synth()
{
    sampleRate = 48000.0f; // originally 44100.0f
    setFilterType(FilterType::SVF);
}

Synth::~Synth()
{

}

void Synth::setOsc1Wave(WaveType wt)
{
    osc1Wave = wt;
    for (int v = 0; v < MAX_VOICES; ++v)
        voices[v].osc1.setWaveType(osc1Wave);
}

void Synth::setOsc2Wave(WaveType wt)
{
    osc2Wave = wt;
    for (int v = 0; v < MAX_VOICES; ++v)
        voices[v].osc2.setWaveType(osc2Wave);
}

float Synth::calculateKeyTrackedCutoff(int midiNote, float baseCutoff) const
{
    // Calcular distancia desde el centro en semitonos
    const float semitoneDistance = float(midiNote - filterKeycenterNote);

    // Convertir a multiplicador de frecuencia
    // 1 semitono = factor de 2^(1/12) ≈ 1.059463
    const float frequencyRatio = std::exp2(semitoneDistance / 12.0f);

    // Aplicar tracking amount (0 = sin tracking, 1 = tracking completo)
    // Interpolación entre baseCutoff (sin tracking) y baseCutoff*ratio (tracking completo)
    const float trackedCutoff = baseCutoff * std::pow(frequencyRatio, filterKeytrackAmount);

    return trackedCutoff;
}

void Synth::setCCState(const CCState& s)
{
    cc = s;
    sustainPedalPressed = cc.sustain;
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
void Synth::restartMonoVoice(int note, int velocity)
{
    Voice& voice = voices[0];

    voice.osc1.setWaveType(osc1Wave);
    voice.osc2.setWaveType(osc2Wave);

    const float freq = calcBaseFreq(0, note);
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

    // ============================================================
    // MEJORA CLAVE: Manejo inteligente del envelope para legato
    // ============================================================
    Envelope& env = voice.env;

    // Actualizar parámetros del envelope
    const float relMult = 0.25f + 1.75f * cc.release;
    const float atkMult = 0.25f + 1.75f * cc.attack;
    env.attackMultiplier = std::pow(envAttack, 1.0f / atkMult);
    env.decayMultiplier = envDecay;
    env.sustainLevel = envSustain;
    env.releaseMultiplier = std::pow(envRelease, 1.0f / relMult);

    // Solo hacer attack() si el envelope está muy bajo o en release
    if (env.level < 0.1f || voice.released)
    {
        // Envelope casi silencioso o voz en release: hacer attack completo
        env.attack();
    }
    // Si env.level >= 0.1f y NO está en release: 
    // mantener level actual para transición legato suave
    // (solo actualizamos los parámetros arriba, sin retriggear)

    // ============================================================
    // Lo mismo para el filter envelope
    // ============================================================
    Envelope& fe = voice.filterEnv;
    fe.attackMultiplier = filterEnvAttack;
    fe.decayMultiplier = filterEnvDecay;
    fe.sustainLevel = filterEnvSustain;
    fe.releaseMultiplier = filterEnvRelease;

    if (fe.level < 0.1f || voice.released)
    {
        fe.attack();
    }

    // ============================================================
    // Velocity
    // ============================================================
    const float velNorm = juce::jlimit(0.0f, 1.0f, (float)velocity / 127.0f);
    const float velCurve = velNorm * velNorm;
    const float amt = ignoreVelocity ? 0.0f : juce::jlimit(0.0f, 1.0f, velocitySensitivity);

    // amt=0 -> gain=1, amt=1 -> gain=velCurve
    voice.velocityGain = (1.0f - amt) + amt * velCurve;

    // ============================================================
    // IMPORTANTE: Marcar voz como activa (NO released)
    // ============================================================
    voice.note = note;
    voice.released = false;  // ¡CRÍTICO! La voz ahora está activa
    voice.updatePanning(0);
}

void Synth::controlChange(uint8_t data1, uint8_t data2)
{
    // All Notes Off / All Sound Off (CC 120-127)
    if (data1 >= 0x78)
    {
        for (int v = 0; v < MAX_VOICES; ++v)
            voices[v].reset();
        sustainPedalPressed = false;
        return;  // Salir después de procesar
    }

    switch (data1)
    {
    case 0x40:  // CC64 = Sustain Pedal (Damper)
    {
        bool pedalDown = (data2 >= 64);

        // Si se SUELTA el pedal (transición de pressed -> not pressed)
        if (sustainPedalPressed && !pedalDown)
        {
            sustainPedalPressed = false;

            // Hacer release de todas las voces que fueron marcadas
            for (int v = 0; v < MAX_VOICES; ++v)
            {
                Voice& voice = voices[v];

                // Solo en voces que:
                // 1. Están marcadas como "released" (tecla soltada con pedal)
                // 2. Tienen envelope activo
                if (voice.released && voice.env.isActive())
                {
                    voice.env.release();
                    voice.filterEnv.release();
                }
            }
        }
        else if (!sustainPedalPressed && pedalDown)
        {
            // Se PRESIONA el pedal
            sustainPedalPressed = true;
        }
        break;
    }

    case 0x4A:  // CC74 usualmente es cutoff/brightness, pero el libro usa 0x4A como "Filter +"
    {
        cc.filterPlus = data2 / 127.0f;
        break;
    }

    case 0x4B:  // "Filter -"
    {
        cc.filterMinus = data2 / 127.0f;
        break;
    }

    default:
        break;
    }
}
/*
void Synth::controlChange(uint8_t data1, uint8_t data2)
{
    if (data1 >= 0x78)
    {
        for (int v = 0; v < MAX_VOICES; ++v)
            voices[v].reset();
        sustainPedalPressed = false;
    }
    switch (data1)
    {
    case 0x4A: // CC74 usualmente es cutoff/brightness, pero el libro usa 0x4A como “Filter +”
    {
        cc.filterPlus = data2 / 127.0f;
        break;
    }
    case 0x4B: // “Filter -”
    {
        cc.filterMinus = data2 / 127.0f;
        break;
    }
    default: break;
    }
}
*/
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
    
    voice.osc1.setWaveType(osc1Wave);
    voice.osc2.setWaveType(osc2Wave);
    
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
    
    //float vel = 0.004f * float((velocity + 64) * (velocity + 64))- 8.0f;
    //voice.velocityGain = 0.01f * vel;
    // velocity viene 0..127
        const float velNorm = juce::jlimit(0.0f, 1.0f, (float)velocity / 127.0f);

        // curva musical típica (más control en valores bajos)
        const float velCurve = velNorm * velNorm;              // pow(v, 2)

        // amount 0..1 (si ignoreVelocity -> 0)
        const float amt = ignoreVelocity ? 0.0f : juce::jlimit(0.0f, 1.0f, velocitySensitivity);

        // mezcla: amt=0 => sin velocity (gain=1), amt=1 => full velocity (gain=vCurve)
        const float g = (1.0f - amt) * 1.0f + amt * velCurve;

        voice.velocityGain = g;

    // Sincronizar fase en modo PWM
    if (lfoDepthSemis == 0.0f && pwmDepth > 0.0f) {
        voice.osc2.syncPhase(voice.osc1);  // Sincronizar con osc1
    }

    voice.osc2Gain = oscMix;
    Envelope& env = voice.env;

    // multiplicadores por CC (performance)
    const float atkMult = 0.25f + 1.75f * cc.attack;   // [0.25..2.0]
    const float relMult = 0.25f + 1.75f * cc.release;  // [0.25..2.0]

    // Configurar ADSR
    env.attackMultiplier = std::pow(envAttack, 1.0f / atkMult);
    env.decayMultiplier = envDecay;
    env.sustainLevel = envSustain;
    env.releaseMultiplier = std::pow(envRelease, 1.0f / relMult);// envRelease * relMult;

    // Iniciar ataque
    env.attack();

    Envelope& fe = voice.filterEnv;

    fe.attackMultiplier = filterEnvAttack;
    fe.decayMultiplier = filterEnvDecay;
    fe.sustainLevel = filterEnvSustain;
    fe.releaseMultiplier = filterEnvRelease;

    fe.attack();

    // Modular profundidad del filter envelope según velocity
    //const float velNorm = juce::jlimit(0.0f, 1.0f, (float)velocity / 127.0f);
    //const float velCurve = velNorm * velNorm;  // curva cuadrática para mejor respuesta
    //const float amt = velocitySensitivity;
    //voice.velocityGain = (1.0f - amt) + amt * velCurve;

    // Filtro (tu implementación actual: escala profundidad del filter env)
    voice.filterEnvDepthMultiplier = (1.0f - filterVelocityAmount) + filterVelocityAmount * velCurve;
    // filterVelocityAmount de 0 a 1:
    // - Si amount = 0: sin efecto (multiplier = 1)
    // - Si amount = 1: rango completo (vel baja = 0.5x, vel alta = 1.5x)
    //const float velRange = filterVelocityAmount;
    //voice.filterEnvDepthMultiplier = 1.0f - velRange + (2.0f * velRange * velCurve);
    // Resultado: vel=0 → 1-amount, vel=64 → 1.0, vel=127 → 1+amount
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
    // Actualizar key tracking (remover nota del stack)
    releaseKey(note);

    // ========================================================================
    // MODO MONO
    // ========================================================================
    if (numVoices == 1)
    {
        // Solo procesar si la nota que suena es la que estamos soltando
        if (voices[0].note != note)
            return;  // Esta nota no está sonando, ignorar

        // Verificar si hay otra tecla aún presionada
        const int topNote = topKey();  // última tecla presionada o -1

        if (topNote >= 0)
        {
            // HAY otra tecla presionada: cambiar a esa nota (legato)
            // IMPORTANTE: velocity=0 indica que es un cambio legato sin retriggear
            restartMonoVoice(topNote, 0);
            return;
        }

        // NO hay otras teclas presionadas: hacer release
        if (!sustainPedalPressed)
        {
            // Sin pedal: release inmediato
            voices[0].release();
        }
        else
        {
            // Con pedal: marcar como "released" para release posterior
            voices[0].released = true;
        }

        return;
    }

    // ========================================================================
    // MODO POLY
    // ========================================================================

    // Buscar TODAS las voces que están tocando esta nota
    // (puede haber múltiples si se retriggereó la misma nota)
    for (int v = 0; v < MAX_VOICES; ++v)
    {
        Voice& voice = voices[v];

        // Si esta voz está tocando la nota que soltamos
        if (voice.note == note)
        {
            if (!sustainPedalPressed)
            {
                // Sin pedal: release inmediato
                voice.release();
            }
            else
            {
                // Con pedal: marcar como "released" para release posterior
                voice.released = true;
            }
        }
    }
}

void Synth::allocateResources(double sampleRate_, int /*samplesPerBlock*/)
{
    sampleRate = static_cast<float>(sampleRate_);
    setFilterType(filterType);
    outputLevelSmoother.reset(sampleRate_, 0.02);
    for (int v = 0; v < MAX_VOICES; ++v)
    {
        voices[v].osc1.prepare(sampleRate);
        voices[v].osc2.prepare(sampleRate);
        voices[v].osc1.setWaveType(osc1Wave);
        voices[v].osc2.setWaveType(osc2Wave);
        if (voices[v].filter)                // <-- defensivo
            voices[v].filter->prepare(sampleRate);
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
    //modWheel = 0.0f;
    lastNote = -1;
    keyDown.fill(false);
    keyStackSize = 0;
    //aftertouch = 0.0f;
    cc.filterPlus = 0.0f;
    cc.filterMinus = 0.0f;
    filterZipSemis = 0.0f;
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
            
            // Vibrato depth modulado por Mod Wheel (CC1)
            const float baseVibratoSemis = lfoDepthSemis;     // parámetro APVTS
            const float modWheelCurved = cc.modWheel * cc.modWheel;
            constexpr float kModWheelMaxSemis = 2.0f;
            const float effectiveVibratoSemis = baseVibratoSemis + (kModWheelMaxSemis * modWheelCurved);

            const float vibratoPitchMul = std::exp2((lfoSine * effectiveVibratoSemis) / 12.0f);
            /*
            const float modWheelSemis = modWheel * 12.0f;
            const float totalVibratoSemis = lfoDepthSemis + modWheelSemis;
            const float vibratoPitchMul = std::exp2((lfoSine * totalVibratoSemis) / 12.0f);
            */

            lfoPitchMul = vibratoPitchMul;

            const float totalPwmDepth = juce::jlimit(0.0f, 0.45f, pwmDepth);
            const float pwmWidth = juce::jlimit(0.05f, 0.95f, 0.5f + lfoSine * totalPwmDepth);

            // --- MODULACIÓN GLOBAL (en semitonos) ---
            const float filterCtlSemis = 12.0f * (cc.filterPlus - cc.filterMinus);
            const float pressureSemis = 12.0f * cc.aftertouch;
            const float lfoSemis = (filterLFODepthSemis + pressureSemis) * lfoSine;

            const float filterModSemisTarget = filterCtlSemis + lfoSemis;

            // --- SMOOTHING GLOBAL (una sola vez) ---
            const float dt = float(LFO_MAX) / sampleRate;
            const float tau = 0.02f;
            const float a = 1.0f - std::exp(-dt / tau);

            filterZipSemis += a * (filterModSemisTarget - filterZipSemis);
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
                // 4) FILTRO (NUEVO)
                // Base cutoff modulada por CC + key tracking
                //const float noteFreq = voice.freqCurrent;
                //const float keyTrackAmount = 0.3f;  // 0 = sin tracking, 1 = tracking total

                // Base desde APVTS modulada multiplicativamente por CC
                //float cutoffHz = filterCutoff * (1.0f + 2.0f * cc.brightness)  // x1 a x3
                //    + noteFreq * keyTrackAmount;
                // 
                // Base cutoff modulado por CC brightness
                /*
                float baseCutoff = filterCutoff * (1.0f + 2.0f * cc.brightness);  // x1 a x3

                // Aplicar key tracking usando la nota MIDI
                float cutoffHz = calculateKeyTrackedCutoff(voice.note, baseCutoff);
                // offset manual del cutoff (como keytracking, NO oscilatorio)
                const float filterCtlSemis = 12.0f * (cc.filterPlus - cc.filterMinus);
                cutoffHz *= std::exp2(filterCtlSemis / 12.0f);
                // LFO + aftertouch (oscilatorio)
                const float pressure = cc.aftertouch;                 // 0..1
                const float pressureSemis = 12.0f * pressure;
                const float filterModSemis = (filterLFODepthSemis + pressureSemis) * lfoSine;
                cutoffHz *= std::exp2(filterModSemis / 12.0f);
                cutoffHz /= pitchBend;
                cutoffHz = std::clamp(cutoffHz, 80.0f, 0.45f * sampleRate);

                // Resonancia base + CC
                float Q = filterResonance + 5.0f * cc.resonance;  // +0 a +5
                Q = std::clamp(Q, 0.5f, 10.0f);
                
                const float dt = float(LFO_MAX) / sampleRate;  // sampleRate real del motor
                const float tau = 0.02f;                       // 20 ms (ajusta)
                const float a = 1.0f - std::exp(-dt / tau);

                if (voice.cutoffZipHz <= 0.0f) voice.cutoffZipHz = cutoffHz; // init
                voice.cutoffZipHz += a * (cutoffHz - voice.cutoffZipHz);

                voice.setFilter(cutoffHz, Q);
                */
                // --- FILTRO ---
                float baseCutoff = filterCutoff * (1.0f + 2.0f * cc.brightness);
                float cutoffHz = calculateKeyTrackedCutoff(voice.note, baseCutoff);

                // aplica modulador suavizado (incluye CC +/- + LFO + aftertouch)
                cutoffHz *= std::exp2(filterZipSemis / 12.0f);

                // “detalle pitch bend” estilo libro
                cutoffHz /= pitchBend;

                cutoffHz = std::clamp(cutoffHz, 80.0f, 0.45f * sampleRate);

                float res01 = std::clamp(filterResonance + 0.5f * cc.resonance, 0.0f, 1.0f);
                voice.setFilter(cutoffHz, res01);
                /*
                float Q = filterResonance + 5.0f * cc.resonance;
                Q = std::clamp(Q, 0.5f, 10.0f);

                const float normalizedResonance = (Q - 0.5f) / (10.0f - 0.5f);

                const float fe = voice.filterEnv.nextValue();
                cutoffHz *= std::exp2((fe * filterEnvAmountSemis) / 12.0f);
                //cutoffHz *= std::exp2((fe * filterEnvAmountSemis * voice.filterEnvDepthMultiplier) / 12.0f);


                voice.setFilter(cutoffHz, normalizedResonance);
                */

            };
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
       
        const float gain = outputLevelSmoother.getNextValue();
        outL *= gain;
        outR *= gain;

        // CC11 Expression (modulación, no APVTS)
        float expr = cc.expression;   // [0..1]  (asumiendo que ya se setea cc desde setCCState)
        expr = expr * expr;           // curva perceptual opcional (más control en valores bajos)
        outL *= expr;
        outR *= expr;

        float driveL = outL * volumeTrim;
        float driveR = outR * volumeTrim;

        outL = driveL / (1.0f + std::abs(driveL));
        outR = driveR / (1.0f + std::abs(driveR));

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
        // Channel Pressure
        case 0xD0:
        {
            cc.aftertouch = (data1 & 0x7F) / 127.0f;;        // o variable dedicada
            break;
        }
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

void Synth::setFilterType(FilterType type)
{
    const bool pointersReady = (voices[0].filter != nullptr);
    if (filterType == type && pointersReady)
        return;

    filterType = type;

    for (int v = 0; v < MAX_VOICES; ++v)
    {
        Voice& voice = voices[v];

        switch (filterType)
        {
        case FilterType::SVF:
            voice.filter = &voice.svfFilter;
            break;

        case FilterType::Moog:
            voice.filter = &voice.moogFilter;
            break;

        default:
            voice.filter = &voice.svfFilter;
            break;
        }

        voice.filter->setSampleRate(sampleRate);
        voice.filter->reset();
    }
}