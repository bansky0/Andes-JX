/*
  ==============================================================================

    Synth.cpp
    Created: 10 Nov 2025 6:45:11pm
    Author:  Jhonatan

  ==============================================================================
*/

/*
    Module: Synth (implementation)
    Purpose:
        EN: Implements the polyphonic synthesis engine declared in Synth.h.
            Hosts the voice pool, MIDI routing, voice allocation, key
            tracking, global LFO, per-block modulation, and the main audio
            render loop.
        ES: Implementa el motor de síntesis polifónica declarado en
            Synth.h. Alberga el pool de voces, enrutado MIDI, asignación
            de voces, key tracking, LFO global, modulación por bloque y
            el bucle principal de audio.

    Structure / Estructura:
        EN:
          1. Construction and filter selection
          2. Global modulation helpers (key tracking, CC snapshot)
          3. Note handling (startVoice, restartMonoVoice, noteOn, noteOff)
          4. Voice allocation (findVoiceForNote, findFreeVoice)
          5. Resource management (allocateResources, reset)
          6. Main audio render loop
          7. MIDI dispatch
          8. Key stack helpers (last-note-priority legato)
          9. Filter type switching
        ES:
          1. Construcción y selección de filtro
          2. Helpers de modulación global (key tracking, snapshot de CC)
          3. Manejo de notas (startVoice, restartMonoVoice, noteOn, noteOff)
          4. Asignación de voces (findVoiceForNote, findFreeVoice)
          5. Gestión de recursos (allocateResources, reset)
          6. Bucle principal de renderizado de audio
          7. Despacho MIDI
          8. Helpers del key stack (legato con prioridad a última nota)
          9. Cambio de tipo de filtro
*/

#include "Synth.h"
#include "Utils.h"
#include "Constants.h"
#include "SVFFilter.h"
#include "MoogFilter.h"


// ============================================================================
//  1. CONSTRUCTION AND FILTER SELECTION
//     CONSTRUCCIÓN Y SELECCIÓN DE FILTRO
// ============================================================================

// EN: Default sample rate is used only if the host never calls
//     allocateResources() before the first render(). The initial filter
//     type bootstraps the filter pointers inside each Voice.
// ES: La sample rate por defecto solo se usa si el host no llama a
//     allocateResources() antes del primer render(). El tipo de filtro
//     inicial inicializa los punteros de filtro dentro de cada Voice.
Synth::Synth()
{
    sampleRate = 48000.0f;
    setFilterType(FilterType::SVF);
}

Synth::~Synth()
{
}


// ----------------------------------------------------------------------------
//  Oscillator waveform setters / Setters de forma de onda
// ----------------------------------------------------------------------------

// EN: Propagate a waveform change to all voices. Called by PluginProcessor
//     whenever the osc1Wave or osc2Wave APVTS parameter changes.
// ES: Propaga un cambio de forma de onda a todas las voces. Lo llama
//     PluginProcessor cuando cambia el parámetro APVTS osc1Wave u osc2Wave.
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


// ============================================================================
//  2. GLOBAL MODULATION HELPERS
//     HELPERS DE MODULACIÓN GLOBAL
// ============================================================================

// EN: Applies the key-tracking curve to the base cutoff.
//     The cutoff is multiplied by  2^(semitones/12 * trackAmount), so:
//       - filterKeytrackAmount = 0  leaves the cutoff unchanged
//       - filterKeytrackAmount = 1  makes the cutoff follow the pitch 1:1
//         (one octave of cutoff shift per octave of key distance from
//         filterKeycenterNote)
// ES: Aplica la curva de key tracking al cutoff base.
//     Se multiplica el cutoff por  2^(semitonos/12 * trackAmount), así:
//       - filterKeytrackAmount = 0  deja el cutoff sin cambios
//       - filterKeytrackAmount = 1  hace que el cutoff siga al pitch 1:1
//         (una octava de desplazamiento del cutoff por cada octava de
//         distancia respecto a filterKeycenterNote)
float Synth::calculateKeyTrackedCutoff(int midiNote, float baseCutoff) const
{
    const float semitoneDistance = float(midiNote - filterKeycenterNote);

    // EN: Semitone distance -> frequency ratio: 1 semitone = 2^(1/12).
    // ES: Distancia en semitonos -> razón de frecuencia: 1 semitono = 2^(1/12).
    const float frequencyRatio = std::exp2(semitoneDistance / 12.0f);

    // EN: pow(ratio, amount) interpolates smoothly between
    //     "no tracking" (amount=0 -> pow=1) and "full tracking"
    //     (amount=1 -> pow=ratio).
    // ES: pow(ratio, amount) interpola suavemente entre
    //     "sin tracking" (amount=0 -> pow=1) y "tracking completo"
    //     (amount=1 -> pow=ratio).
    const float trackedCutoff = baseCutoff * std::pow(frequencyRatio, filterKeytrackAmount);

    return trackedCutoff;
}


// EN: Copies the current MIDI CC snapshot into the engine and mirrors the
//     sustain-pedal flag into the private field used by the note-off path.
// ES: Copia el snapshot actual de los CC MIDI al motor y refleja la
//     bandera del pedal de sustain en el campo privado usado por note-off.
void Synth::setCCState(const CCState& s)
{
    cc = s;
    sustainPedalPressed = cc.sustain;
}
// ============================================================================
//  3. NOTE HANDLING
//     MANEJO DE NOTAS
// ============================================================================

// EN: Restarts voice 0 without triggering a new attack (monophonic legato).
//     When one key is already held and a new key is pressed, the envelope
//     is kept where it is while the oscillators glide to the new pitch.
//     This matches the classic Minimoog behavior and is the reason the
//     "legato style" feels smooth rather than percussive.
// ES: Reinicia la voz 0 sin disparar un ataque nuevo (legato monofónico).
//     Cuando hay una tecla ya pulsada y se presiona otra, la envolvente se
//     mantiene donde está mientras los osciladores hacen glide hacia el
//     nuevo pitch. Esto replica el comportamiento clásico del Minimoog y
//     es la razón de que el legato se sienta suave, no percusivo.
void Synth::restartMonoVoice(int note, int velocity)
{
    Voice& voice = voices[0];

    voice.osc1.setWaveType(osc1Wave);
    voice.osc2.setWaveType(osc2Wave);

    const float freq = calcBaseFreq(0, note);
    voice.freqTarget = freq;

    // EN: In this function we always come from a legato situation
    //     (noteOn detected another key was held), so wasLegato is true.
    // ES: En esta función siempre venimos de una situación legato
    //     (noteOn detectó que había otra tecla pulsada), así que
    //     wasLegato es verdadero.
    const bool wasLegato = true;

    // EN: Decide whether this note should glide, based on the glide mode:
    //       0 = off, 1 = legato only, 2 = always.
    // ES: Decidir si esta nota debe hacer glide, según el modo:
    //       0 = off, 1 = solo legato, 2 = siempre.
    bool shouldGlide = false;
    if (glideMode == 2)      shouldGlide = true;
    else if (glideMode == 1) shouldGlide = wasLegato;

    voice.glideRateThisNote = shouldGlide ? glideRate : 1.0f;

    // EN: Frequency starting point. Without glide, jump directly to the
    //     target. With glide, start N semitones away (glideBend offsets
    //     this to give a subtle "drift" at the beginning of the slide).
    // ES: Punto de arranque en frecuencia. Sin glide, saltar directo al
    //     objetivo. Con glide, empezar N semitonos lejos (glideBend
    //     desplaza esto para dar un "drift" sutil al inicio del deslizamiento).
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


    // ------------------------------------------------------------------------
    //  Smart envelope handling for legato
    //  Manejo inteligente de la envolvente para legato
    // ------------------------------------------------------------------------

    Envelope& env = voice.env;

    // EN: CC73 (attack time) and CC72 (release time) scale the envelope
    //     shape factors. Range [0.25, 2.0] means the user can halve or
    //     double the envelope speed from the MIDI controller.
    // ES: CC73 (tiempo de attack) y CC72 (tiempo de release) escalan los
    //     factores de forma de la envolvente. Rango [0.25, 2.0] significa
    //     que el usuario puede dividir o duplicar la velocidad de la
    //     envolvente desde el controlador MIDI.
    const float relMult = 0.25f + 1.75f * cc.release;
    const float atkMult = 0.25f + 1.75f * cc.attack;
    env.attackMultiplier = std::pow(envAttack, 1.0f / atkMult);
    env.decayMultiplier = envDecay;
    env.sustainLevel = envSustain;
    env.releaseMultiplier = std::pow(envRelease, 1.0f / relMult);

    // EN: Key decision: only restart the attack if the envelope is very
    //     low (< 0.1) or if the voice was already in release. Otherwise
    //     we keep the current level for a smooth legato transition and
    //     only update the parameters above. This is what makes legato
    //     playing feel connected rather than re-attacked on every note.
    // ES: Decisión clave: solo reiniciar el attack si la envolvente está
    //     muy baja (< 0.1) o si la voz estaba en release. En otro caso se
    //     mantiene el nivel actual para una transición legato suave y
    //     solo se actualizan los parámetros de arriba. Esto es lo que
    //     hace que el legato se sienta conectado y no re-atacado en cada
    //     nota.
    if (env.level < 0.1f || voice.released)
    {
        env.attack();
    }

    // EN: Same smart-retrigger logic applied to the filter envelope.
    // ES: La misma lógica de retrigger inteligente aplicada a la
    //     envolvente del filtro.
    Envelope& fe = voice.filterEnv;
    fe.attackMultiplier = filterEnvAttack;
    fe.decayMultiplier = filterEnvDecay;
    fe.sustainLevel = filterEnvSustain;
    fe.releaseMultiplier = filterEnvRelease;

    if (fe.level < 0.1f || voice.released)
    {
        fe.attack();
    }


    // ------------------------------------------------------------------------
    //  Velocity response / Respuesta a la velocity
    // ------------------------------------------------------------------------

    // EN: Quadratic velocity curve ( v^2 ) gives more control at low
    //     values, which is the standard musical response.
    // ES: Curva cuadrática de velocity ( v^2 ) da más control en valores
    //     bajos, que es la respuesta musical estándar.
    const float velNorm = juce::jlimit(0.0f, 1.0f, (float)velocity / 127.0f);
    const float velCurve = velNorm * velNorm;
    const float amt = ignoreVelocity ? 0.0f
        : juce::jlimit(0.0f, 1.0f, velocitySensitivity);

    // EN: Blend between "no velocity" (gain = 1) and "full velocity"
    //     (gain = velCurve) using velocitySensitivity as the amount.
    // ES: Mezcla entre "sin velocity" (gain = 1) y "velocity completo"
    //     (gain = velCurve) usando velocitySensitivity como cantidad.
    voice.velocityGain = (1.0f - amt) + amt * velCurve;


    // EN: Critical bookkeeping: the voice is active again, not released.
    //     Forgetting this line would leave the voice in release state
    //     even though a new key is being held.
    // ES: Bookkeeping crítico: la voz vuelve a estar activa, no liberada.
    //     Olvidar esta línea dejaría la voz en estado release aunque se
    //     esté pulsando una nueva tecla.
    voice.note = note;
    voice.released = false;
    voice.updatePanning(0);
}

// ----------------------------------------------------------------------------
//  MIDI Control Change handler / Handler de Control Change MIDI
// ----------------------------------------------------------------------------

// EN: Processes a MIDI CC message. Three cases are handled here:
//       - CC 120-127 (All Notes Off / All Sound Off): hard reset voices
//       - CC 64 (Sustain Pedal): deferred release on pedal release
//       - CC 74 / 75 (Filter + / Filter -): manual cutoff offset
//     Other CCs (mod wheel, expression, brightness, attack, release,
//     aftertouch) are expected to arrive through setCCState() from
//     PluginProcessor.
// ES: Procesa un mensaje CC MIDI. Aquí se atienden tres casos:
//       - CC 120-127 (All Notes Off / All Sound Off): reset duro de voces
//       - CC 64 (pedal de sustain): release diferido al soltar el pedal
//       - CC 74 / 75 (Filter + / Filter -): offset manual del cutoff
//     El resto de CCs (mod wheel, expression, brightness, attack, release,
//     aftertouch) se esperan vía setCCState() desde PluginProcessor.
void Synth::controlChange(uint8_t data1, uint8_t data2)
{
    // EN: CC 120-127 are the "channel mode messages" block. Any of them
    //     (All Sound Off, All Notes Off, Omni, Mono, Poly) triggers a
    //     hard reset of the voice pool.
    // ES: Los CC 120-127 son el bloque de "channel mode messages".
    //     Cualquiera de ellos (All Sound Off, All Notes Off, Omni, Mono,
    //     Poly) dispara un reset duro del pool de voces.
    if (data1 >= 0x78)
    {
        for (int v = 0; v < MAX_VOICES; ++v)
            voices[v].reset();
        sustainPedalPressed = false;
        return;
    }

    switch (data1)
    {
        // EN: CC64 - Sustain Pedal. The MIDI convention is that values >= 64
        //     mean "pressed". The interesting case is pedal release: we must
        //     fire release() on every voice that was left pending (flagged
        //     released = true while the pedal was held).
        // ES: CC64 - Pedal de Sustain. La convención MIDI es que valores >= 64
        //     significan "pisado". El caso interesante es al soltar el pedal:
        //     hay que disparar release() en toda voz que quedó pendiente
        //     (marcada released = true mientras el pedal estaba pisado).
    case 0x40:
    {
        bool pedalDown = (data2 >= 64);

        // EN: Transition pressed -> released: flush the pending releases.
        // ES: Transición pisado -> soltado: ejecutar los release pendientes.
        if (sustainPedalPressed && !pedalDown)
        {
            sustainPedalPressed = false;

            for (int v = 0; v < MAX_VOICES; ++v)
            {
                Voice& voice = voices[v];

                // EN: A voice is "pending release" when:
                //       1. Its key has been lifted (released == true), and
                //       2. Its envelope is still audible.
                //     noteOff() sets the flag without calling release();
                //     that call is delayed until this point.
                // ES: Una voz está "pendiente de release" cuando:
                //       1. Su tecla ya fue soltada (released == true), y
                //       2. Su envolvente sigue siendo audible.
                //     noteOff() activa la bandera sin llamar a release();
                //     esa llamada se retrasa hasta aquí.
                if (voice.released && voice.env.isActive())
                {
                    voice.env.release();
                    voice.filterEnv.release();
                }
            }
        }
        else if (!sustainPedalPressed && pedalDown)
        {
            // EN: Transition released -> pressed. No voice action is
            //     needed here; noteOff() will start marking voices as
            //     "pending release" from now on.
            // ES: Transición soltado -> pisado. No hay que tocar voces
            //     aquí; a partir de ahora noteOff() empezará a marcar
            //     voces como "pendientes de release".
            sustainPedalPressed = true;
        }
        break;
    }

    // EN: CC74 (Filter +) and CC75 (Filter -) act as a manual bipolar
    //     cutoff offset. They are normalized to [0, 1] and combined in
    //     render() as a semitone displacement:
    //         filterCtlSemis = 12 * (filterPlus - filterMinus)
    // ES: CC74 (Filter +) y CC75 (Filter -) actúan como offset bipolar
    //     manual del cutoff. Se normalizan a [0, 1] y en render() se
    //     combinan como desplazamiento en semitonos:
    //         filterCtlSemis = 12 * (filterPlus - filterMinus)
    case 0x4A:
    {
        cc.filterPlus = data2 / 127.0f;
        break;
    }

    case 0x4B:
    {
        cc.filterMinus = data2 / 127.0f;
        break;
    }

    default:
        break;
    }
}
// ============================================================================
//  4. VOICE ALLOCATION
//     ASIGNACIÓN DE VOCES
// ============================================================================

// EN: MIDI note number -> oscillator base frequency in Hz.
//     Three offsets are combined in the semitone domain before the
//     exponential conversion:
//       - note         : the MIDI note number itself
//       - ANALOG * v   : per-voice analog drift (see Constants::ANALOG).
//                        Each polyphonic voice gets a tiny detune
//                        proportional to its index, emulating vintage
//                        oscillator instability and adding "thickness".
//       - tune         : global tuning offset from the preset
//     Then  f = 440 * 2^((semis - 69) / 12)  centered on A4 = MIDI 69.
// ES: Número de nota MIDI -> frecuencia base del oscilador en Hz.
//     Se combinan tres desplazamientos en semitonos antes de la
//     conversión exponencial:
//       - note         : el número de nota MIDI
//       - ANALOG * v   : drift analógico por voz (ver Constants::ANALOG).
//                        Cada voz polifónica recibe un detune minúsculo
//                        proporcional a su índice, emulando la
//                        inestabilidad de osciladores vintage y añadiendo
//                        "grosor".
//       - tune         : offset global de afinación del preset
//     Luego  f = 440 * 2^((semis - 69) / 12)  centrado en A4 = MIDI 69.
float Synth::calcBaseFreq(int v, int note) const
{
    const float semis = (float(note) + ANALOG * float(v) + tune);
    return 440.0f * std::exp2((semis - 69.0f) / 12.0f);
}


// EN: Returns the voice index currently playing `note`, or -1 if none.
//     "Currently playing" means the voice is assigned to this note AND
//     its envelope is still audible; a released voice whose envelope has
//     died is not a match.
// ES: Devuelve el índice de la voz que está tocando `note`, o -1 si
//     ninguna lo hace. "Está tocando" significa que la voz está asignada
//     a esta nota Y su envolvente aún es audible; una voz liberada cuya
//     envolvente ya murió no cuenta.
int Synth::findVoiceForNote(int note) const
{
    for (int v = 0; v < MAX_VOICES; ++v)
        if (voices[v].note == note && voices[v].env.isActive())
            return v;
    return -1;
}


// EN: Classic voice-allocation strategy with three priority levels:
//
//       Level 1 (retrigger): if the same note is already playing, reuse
//           that voice. Prevents duplicate voices stacking on the same
//           key and saves polyphony.
//
//       Level 2 (graceful steal): among voices already in release,
//           steal the quietest one. Musically imperceptible because the
//           stolen voice was fading out anyway.
//
//       Level 3 (hard steal): if no voice is free or released, steal
//           the quietest voice that is NOT in its attack phase. Avoiding
//           attack-phase voices protects the most-audible transients.
//
//     This ordering is standard in analog-style polyphonic synths and
//     keeps voice stealing inaudible in almost all musical situations.
//
// ES: Estrategia clásica de asignación de voces con tres niveles de
//     prioridad:
//
//       Nivel 1 (retrigger): si la misma nota ya suena, reusar esa voz.
//           Evita apilar voces duplicadas en la misma tecla y ahorra
//           polifonía.
//
//       Nivel 2 (robo amable): entre voces ya en release, robar la más
//           silenciosa. Musicalmente imperceptible porque la voz robada
//           estaba desvaneciéndose.
//
//       Nivel 3 (robo duro): si ninguna está libre ni en release, robar
//           la voz más silenciosa que NO esté en attack. Evitar la fase
//           de attack protege los transitorios más audibles.
//
//     Este orden es estándar en sintes polifónicos analog-style y
//     mantiene el robo inaudible en la mayoría de situaciones musicales.
int Synth::findFreeVoice(int note) const
{
    // EN: Level 1: reuse same-note voice if one exists.
    // ES: Nivel 1: reusar voz con la misma nota si existe.
    if (int same = findVoiceForNote(note); same >= 0)
        return same;

    // EN: Level 2: pick the quietest voice already in release.
    // ES: Nivel 2: elegir la voz en release con menor nivel.
    int   best = -1;
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

    // EN: Level 3: steal the quietest voice not in attack.
    //     Starting best = 0 ensures we always return a valid index even
    //     if every voice is in attack (unlikely but possible during
    //     rapid chord presses).
    // ES: Nivel 3: robar la voz más silenciosa que no esté en attack.
    //     Inicializar best = 0 garantiza que siempre devolvemos un índice
    //     válido aunque todas las voces estén en attack (improbable pero
    //     posible al pulsar acordes rápidos).
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

// EN: Starts voice `v` playing `note` with the given velocity. Sets up
//     every per-voice piece: oscillator waveforms and frequencies, glide
//     start point, amplitude envelope, filter envelope, velocity gain
//     and filter-envelope depth modulation. Called from the note-on path
//     for both mono (after attack retrigger) and poly modes.
// ES: Arranca la voz `v` tocando `note` con la velocidad dada. Configura
//     cada pieza de la voz: formas de onda y frecuencias de los
//     osciladores, punto de inicio del glide, envolvente de amplitud,
//     envolvente de filtro, ganancia de velocity y modulación de la
//     profundidad de la envolvente de filtro. Se llama desde el note-on
//     tanto en modo mono (tras retrigger de attack) como en poly.
void Synth::startVoice(int v, int note, int velocity)
{
    Voice& voice = voices[v];

    voice.osc1.setWaveType(osc1Wave);
    voice.osc2.setWaveType(osc2Wave);

    // EN: Detect legato BEFORE calling startNote(). We treat it as legato
    //     only in monophonic mode; in poly mode every note gets its own
    //     voice, so legato does not apply.
    // ES: Detectar legato ANTES de llamar startNote(). Solo se considera
    //     legato en modo monofónico; en poly cada nota recibe su propia
    //     voz, así que legato no aplica.
    const bool wasLegato = (numVoices == 1) && isPlayingLegatoStyle();

    voice.startNote(note);

    // EN: Per-voice random pan offset in [-1, 1]. Combined later with
    //     stereoWidth inside updatePanning() to give each voice a subtly
    //     different stereo placement.
    // ES: Offset aleatorio de paneo por voz en [-1, 1]. Se combina luego
    //     con stereoWidth dentro de updatePanning() para dar a cada voz
    //     una ubicación estéreo sutilmente distinta.
    voice.randomPan = rng.nextFloat() * 2.0f - 1.0f;

    const float freq = calcBaseFreq(v, note);
    voice.freqTarget = freq;


    // ------------------------------------------------------------------------
    //  Glide / Glide
    // ------------------------------------------------------------------------

    // EN: Glide is only enabled in mono mode. In poly mode, each note
    //     starts at its own target frequency without pitch interpolation.
    // ES: El glide solo se habilita en mono. En poly cada nota arranca en
    //     su frecuencia objetivo sin interpolación de pitch.
    bool shouldGlide = false;

    if (numVoices == 1)
    {
        if (glideMode == 2)      shouldGlide = true;           // EN: always  |  ES: siempre
        else if (glideMode == 1) shouldGlide = wasLegato;      // EN: legato  |  ES: legato
        // EN: glideMode == 0 means off.
        // ES: glideMode == 0 significa apagado.
    }

    voice.glideRateThisNote = shouldGlide ? glideRate : 1.0f;

    // EN: Frequency starting point. Without glide, land directly on the
    //     target. With glide, begin some semitones away (offset by
    //     glideBend) so the slide is audible.
    // ES: Punto de arranque de la frecuencia. Sin glide, caer directo en
    //     el objetivo. Con glide, empezar a algunos semitonos de
    //     distancia (desplazados por glideBend) para que el deslizamiento
    //     sea audible.
    if (!shouldGlide || lastNote < 0)
    {
        voice.freqCurrent = freq;
    }
    else
    {
        const int   noteDistance = note - lastNote;
        const float startSemis = float(noteDistance) - glideBend;
        voice.freqCurrent = freq * std::pow(1.059463094359f, -startSemis);
    }

    if (numVoices == 1)
        lastNote = note;


    // ------------------------------------------------------------------------
    //  Oscillator setup / Configuración de osciladores
    // ------------------------------------------------------------------------

    // EN: Osciladores arrancan en freqCurrent (no en freqTarget) para que
    //     el glide se escuche desde la primera muestra.
    //     Reset clears the internal phase so every note starts from the
    //     same phase state, which removes accumulated detune between
    //     retriggers of the same note.
    // ES: Oscillators start at freqCurrent (not freqTarget) so that the
    //     glide is heard from the first sample.
    //     Reset limpia la fase interna para que cada nota arranque desde
    //     el mismo estado, eliminando el detune acumulado entre
    //     retriggers de la misma nota.
    voice.osc1.setFrequency(voice.freqCurrent * pitchBend);
    voice.osc1.reset();
    voice.osc2.setFrequency(voice.freqCurrent * pitchBend * detune);
    voice.osc2.reset();


    // ------------------------------------------------------------------------
    //  Velocity / Velocity
    // ------------------------------------------------------------------------

    // EN: Quadratic velocity curve ( v^2 ) gives more control at low
    //     values, a standard musical response.
    // ES: Curva cuadrática de velocity ( v^2 ) da más control en valores
    //     bajos, una respuesta musical estándar.
    const float velNorm = juce::jlimit(0.0f, 1.0f, (float)velocity / 127.0f);
    const float velCurve = velNorm * velNorm;
    const float amt = ignoreVelocity ? 0.0f
        : juce::jlimit(0.0f, 1.0f, velocitySensitivity);

    // EN: Blend between "no velocity" (gain = 1) and "full velocity"
    //     (gain = velCurve) using velocitySensitivity as the mix amount.
    // ES: Mezcla entre "sin velocity" (gain = 1) y "velocity completo"
    //     (gain = velCurve) usando velocitySensitivity como la cantidad.
    voice.velocityGain = (1.0f - amt) * 1.0f + amt * velCurve;


    // ------------------------------------------------------------------------
    //  PWM phase sync / Sync de fase para PWM
    // ------------------------------------------------------------------------

    // EN: When PWM is active and vibrato is off, force osc2 to share
    //     osc1's phase so the PWM modulation is coherent across voices.
    //     If vibrato is also on, the extra detune would break the sync
    //     anyway, so we skip it.
    // ES: Cuando la modulación PWM está activa y no hay vibrato, forzar
    //     a osc2 a compartir la fase de osc1 para que la modulación PWM
    //     sea coherente entre voces. Si además hay vibrato, el detune
    //     extra rompería la sincronización, así que se omite.
    if (lfoDepthSemis == 0.0f && pwmDepth > 0.0f)
        voice.osc2.syncPhase(voice.osc1);

    voice.osc2Gain = oscMix;


    // ------------------------------------------------------------------------
    //  Amplitude envelope / Envolvente de amplitud
    // ------------------------------------------------------------------------

    Envelope& env = voice.env;

    // EN: CC73 (attack) and CC72 (release) scale the envelope speed
    //     within [0.25, 2.0]. The user can halve or double attack/release
    //     in real time from a MIDI controller.
    // ES: CC73 (attack) y CC72 (release) escalan la velocidad de la
    //     envolvente en el rango [0.25, 2.0]. El usuario puede dividir o
    //     duplicar attack/release en tiempo real desde un controlador
    //     MIDI.
    const float atkMult = 0.25f + 1.75f * cc.attack;
    const float relMult = 0.25f + 1.75f * cc.release;

    env.attackMultiplier = std::pow(envAttack, 1.0f / atkMult);
    env.decayMultiplier = envDecay;
    env.sustainLevel = envSustain;
    env.releaseMultiplier = std::pow(envRelease, 1.0f / relMult);

    env.attack();


    // ------------------------------------------------------------------------
    //  Filter envelope / Envolvente de filtro
    // ------------------------------------------------------------------------

    Envelope& fe = voice.filterEnv;

    fe.attackMultiplier = filterEnvAttack;
    fe.decayMultiplier = filterEnvDecay;
    fe.sustainLevel = filterEnvSustain;
    fe.releaseMultiplier = filterEnvRelease;

    fe.attack();


    // ------------------------------------------------------------------------
    //  Filter envelope depth modulation by velocity
    //  Modulación de la profundidad de la envolvente de filtro por velocity
    // ------------------------------------------------------------------------

    // EN: filterVelocityAmount controls how much velocity scales the
    //     filter envelope depth:
    //       - amount = 0:  no effect (multiplier stays at 1)
    //       - amount = 1:  full range (multiplier goes from ~0 at low
    //                      velocity up to velCurve at full velocity)
    //     Used in render() to multiply the filter envelope before it is
    //     added to the cutoff.
    // ES: filterVelocityAmount controla cuánto escala la velocity a la
    //     profundidad de la envolvente de filtro:
    //       - amount = 0:  sin efecto (el multiplier se queda en 1)
    //       - amount = 1:  rango completo (el multiplier va de ~0 en
    //                      velocity baja hasta velCurve en velocity alta)
    //     Se usa en render() para multiplicar la envolvente de filtro
    //     antes de sumarla al cutoff.
    voice.filterEnvDepthMultiplier =
        (1.0f - filterVelocityAmount) + filterVelocityAmount * velCurve;
}

// ----------------------------------------------------------------------------
//  Note-on and note-off handlers / Handlers de note-on y note-off
// ----------------------------------------------------------------------------

// EN: Note-on handler. Paths diverge between mono and poly modes:
//       - Mono: either retrigger voice 0 with a full attack, or perform
//         a legato transition via restartMonoVoice() if a key was
//         already held.
//       - Poly: allocate a voice through findFreeVoice() and delegate
//         everything to startVoice().
// ES: Handler de note-on. Los caminos divergen entre modo mono y poly:
//       - Mono: o bien retriggerear la voz 0 con un attack completo, o
//         hacer una transición legato con restartMonoVoice() si ya había
//         una tecla pulsada.
//       - Poly: asignar una voz con findFreeVoice() y delegar todo a
//         startVoice().
void Synth::noteOn(int note, int velocity)
{
    velocity = juce::jlimit(0, 127, velocity);
    keyVelocities[note] = velocity;

    // EN: Check the key stack BEFORE pushing this note. If another key
    //     was already held, this is a legato situation.
    // ES: Consultar el stack de teclas ANTES de empujar esta nota. Si ya
    //     había otra tecla pulsada, estamos en situación de legato.
    const bool wasLegato = legatoOnThisNoteOn(note);

    pushKey(note);

    // EN: --- Mono mode ---
    //     Decide between full retrigger and smooth legato based on the
    //     current envelope state:
    //       - If voice is silent or it was NOT a legato noteOn, do a
    //         full startVoice(): attack from near-zero.
    //       - Otherwise, restartMonoVoice() preserves the envelope level
    //         and only updates pitch and parameters (classic Minimoog
    //         behavior).
    // ES: --- Modo mono ---
    //     Decidir entre retrigger completo y legato suave según el estado
    //     actual de la envolvente:
    //       - Si la voz está en silencio o NO fue un noteOn legato, hacer
    //         un startVoice() completo: attack desde casi-cero.
    //       - En otro caso, restartMonoVoice() preserva el nivel de la
    //         envolvente y solo actualiza pitch y parámetros
    //         (comportamiento clásico Minimoog).
    if (numVoices == 1)
    {
        if (!voices[0].env.isActive() || !wasLegato)
            startVoice(0, note, velocity);
        else
            restartMonoVoice(note, velocity);

        return;
    }

    // EN: --- Poly mode ---
    //     Voice allocation takes care of reuse/steal; startVoice()
    //     configures the chosen voice from scratch.
    // ES: --- Modo poly ---
    //     La asignación de voces se encarga de reuso/robo; startVoice()
    //     configura la voz elegida desde cero.
    const int v = findFreeVoice(note);
    startVoice(v, note, velocity);
}


// EN: Note-off handler. Two behaviors depending on the sustain pedal:
//       - Pedal up:   immediate release().
//       - Pedal down: mark the voice as `released` but keep it playing;
//                     the actual release() will fire from controlChange()
//                     when the pedal goes up.
//     Mono and poly branches differ because in mono we may switch to
//     another held key (last-note priority) instead of releasing.
// ES: Handler de note-off. Dos comportamientos según el pedal de sustain:
//       - Pedal suelto:  release() inmediato.
//       - Pedal pisado:  marcar la voz como `released` pero seguir
//                        sonando; el release() real se disparará desde
//                        controlChange() cuando se suelte el pedal.
//     Las ramas mono y poly difieren porque en mono podemos cambiar a
//     otra tecla aún pulsada (prioridad última-nota) en vez de liberar.
void Synth::noteOff(int note)
{
    // EN: Update the key-tracking stack first, regardless of mode.
    // ES: Actualizar primero el stack de key-tracking, sin importar el modo.
    releaseKey(note);


    // ------------------------------------------------------------------------
    //  Mono mode / Modo mono
    // ------------------------------------------------------------------------
    if (numVoices == 1)
    {
        // EN: If voice 0 is not playing the note we are releasing, there
        //     is nothing to do (e.g. the key was already overridden by a
        //     newer legato note).
        // ES: Si la voz 0 no está tocando la nota que liberamos, no hay
        //     nada que hacer (p. ej. la tecla ya fue sustituida por una
        //     nueva nota legato).
        if (voices[0].note != note)
            return;

        // EN: Last-note priority: if another key is still held, fall
        //     back to it using its remembered velocity. This is the
        //     behavior of classic monophonic synths.
        // ES: Prioridad a la última nota: si hay otra tecla aún pulsada,
        //     retomarla usando su velocity recordada. Es el
        //     comportamiento de los sintes monofónicos clásicos.
        const int topNote = topKey();

        if (topNote >= 0)
        {
            restartMonoVoice(topNote, keyVelocities[topNote]);
            return;
        }

        // EN: No other keys are held. Release immediately or defer if
        //     the sustain pedal is down.
        // ES: No hay otras teclas pulsadas. Liberar inmediatamente o
        //     diferir si el pedal de sustain está pisado.
        if (!sustainPedalPressed)
            voices[0].release();
        else
            voices[0].released = true;

        return;
    }


    // ------------------------------------------------------------------------
    //  Poly mode / Modo poly
    // ------------------------------------------------------------------------

    // EN: Release every voice assigned to this note. There may be more
    //     than one if the same note was retriggered before its previous
    //     envelope finished fading out.
    // ES: Liberar toda voz asignada a esta nota. Puede haber más de una
    //     si la misma nota se retriggereó antes de que su envolvente
    //     anterior terminara de desvanecerse.
    for (int v = 0; v < MAX_VOICES; ++v)
    {
        Voice& voice = voices[v];

        if (voice.note == note)
        {
            if (!sustainPedalPressed)
                voice.release();
            else
                voice.released = true;
        }
    }
}

// ============================================================================
//  5. RESOURCE MANAGEMENT
//     GESTIÓN DE RECURSOS
// ============================================================================

// EN: Called by PluginProcessor once before the first audio block. Stores
//     the sample rate, sets up the output level smoother's ramp time,
//     prepares each voice's oscillators and filter, and configures the
//     global LFO.
// ES: PluginProcessor lo llama una vez antes del primer bloque de audio.
//     Guarda la sample rate, configura el tiempo de rampa del smoother
//     de salida, prepara osciladores y filtro de cada voz y configura el
//     LFO global.
void Synth::allocateResources(double sampleRate_, int /*samplesPerBlock*/)
{
    sampleRate = static_cast<float>(sampleRate_);

    // EN: Re-apply the current filter type. This wires the `filter`
    //     pointer of each voice to the correct adapter (svfFilter or
    //     moogFilter) and calls prepare() on them.
    // ES: Re-aplicar el tipo de filtro actual. Esto conecta el puntero
    //     `filter` de cada voz con el adaptador correcto (svfFilter o
    //     moogFilter) y llama a prepare() sobre ellos.
    setFilterType(filterType);

    // EN: 20 ms smoothing ramp for output level changes. Avoids zipper
    //     noise when the user moves the volume control.
    // ES: Rampa de suavizado de 20 ms para cambios de nivel de salida.
    //     Evita zipper noise al mover el control de volumen.
    outputLevelSmoother.reset(sampleRate_, 0.02);

    for (int v = 0; v < MAX_VOICES; ++v)
    {
        voices[v].osc1.prepare(sampleRate);
        voices[v].osc2.prepare(sampleRate);
        voices[v].osc1.setWaveType(osc1Wave);
        voices[v].osc2.setWaveType(osc2Wave);

        // EN: Defensive check: filter pointer should be set by the
        //     setFilterType() call above, but guard against early calls
        //     where the pointer may still be null.
        // ES: Chequeo defensivo: el puntero filter debería estar
        //     asignado por el setFilterType() de arriba, pero nos
        //     protegemos por si la llamada fuera temprana y el puntero
        //     aún estuviera en null.
        if (voices[v].filter)
            voices[v].filter->prepare(sampleRate);

        voices[v].reset();
    }

    // EN: Global LFO setup. Sine wave is used because sub-audio
    //     modulation is usually expected to be smooth and symmetric.
    // ES: Configuración del LFO global. Se usa onda sinusoidal porque la
    //     modulación sub-audio normalmente se espera suave y simétrica.
    lfo.prepare(sampleRate_);
    lfo.setWaveType(WaveType::Sine);
    lfo.setFrequency(lfoRateHz);
    lfo.reset();

    lfoCounter = 0;
    lfoPitchMul = 1.0f;
}


// EN: Full engine reset. Called on preset changes, transport resets, and
//     any other context where the synth should start from a clean state.
//     Clears all voices, the shared noise generator, the key-tracking
//     stack, and the real-time modulation state (pitch bend, pedal,
//     filter CC offsets).
// ES: Reset completo del motor. Se llama al cambiar de preset, al
//     resetear el transporte y en cualquier contexto donde el synth
//     deba partir de un estado limpio. Limpia todas las voces, el
//     generador de ruido compartido, el stack de key tracking y el
//     estado de modulación en tiempo real (pitch bend, pedal, offsets
//     CC de filtro).
void Synth::reset()
{
    for (int v = 0; v < MAX_VOICES; ++v)
        voices[v].reset();

    noiseGen.reset();

    // EN: 1.0 is the pitch-bend neutral value (multiplicative, not
    //     additive). It multiplies the oscillator frequency by 1 -> no
    //     change. See the pitch-bend handler in midiMessage().
    // ES: 1.0 es el valor neutro de pitch-bend (multiplicativo, no
    //     aditivo). Multiplica la frecuencia del oscilador por 1 -> sin
    //     cambio. Ver el handler de pitch-bend en midiMessage().
    pitchBend = 1.0f;

    sustainPedalPressed = false;
    lastNote = -1;

    // EN: Keyboard state. keyVelocities defaults to 100 (mid-velocity)
    //     so that any fallback to an already-held key after a note-off
    //     has a reasonable loudness even if the original velocity was
    //     not recorded.
    // ES: Estado del teclado. keyVelocities por defecto es 100 (velocity
    //     media) para que cualquier fallback a una tecla aún pulsada
    //     tras un note-off tenga una sonoridad razonable aunque la
    //     velocity original no se haya registrado.
    keyDown.fill(false);
    keyStackSize = 0;
    keyVelocities.fill(100);

    cc.filterPlus = 0.0f;
    cc.filterMinus = 0.0f;
    filterZipSemis = 0.0f;
}

// ============================================================================
//  6. MAIN AUDIO RENDER LOOP
//     BUCLE PRINCIPAL DE RENDERIZADO DE AUDIO
// ============================================================================

// EN: Renders `sampleCount` stereo frames into the output buffers. This
//     function is the hottest path in the synth: it runs once per audio
//     block from the host (typically every few milliseconds) and must
//     stay allocation-free and lock-free.
//
//     The loop is organized in three stages:
//       (1) Per-voice pre-update done once per block (panning).
//       (2) Sample-by-sample loop:
//             - LFO update every LFO_MAX samples (sub-audio rate).
//             - Per-voice filter cutoff calculation and voice render.
//             - Stereo summing, output gain, expression CC, soft clip.
//       (3) Per-block cleanup: recycle voices whose envelopes have died.
//
// ES: Renderiza `sampleCount` cuadros estéreo en los buffers de salida.
//     Esta función es la ruta más caliente del synth: corre una vez por
//     bloque de audio del host (normalmente cada pocos milisegundos) y
//     debe permanecer libre de reservas y libre de locks.
//
//     El bucle se organiza en tres etapas:
//       (1) Pre-update por voz una vez por bloque (paneo).
//       (2) Bucle muestra a muestra:
//             - Actualización del LFO cada LFO_MAX muestras (sub-audio).
//             - Cálculo de cutoff por voz y render de voz.
//             - Suma estéreo, ganancia de salida, CC de expression,
//               soft clip.
//       (3) Limpieza por bloque: reciclar voces con envolvente muerta.
void Synth::render(float** outputBuffers, int sampleCount)
{
    float* outputBufferLeft = outputBuffers[0];
    float* outputBufferRight = outputBuffers[1];


    // ------------------------------------------------------------------------
    //  (1) Per-voice pre-update / Pre-update por voz
    // ------------------------------------------------------------------------

    // EN: Panning depends on the active MIDI note and the global stereo
    //     width, both of which are constant across the block. Computing
    //     them once per block (not per sample) saves CPU with no
    //     audible difference.
    // ES: El paneo depende de la nota MIDI activa y del stereo width
    //     global, ambos constantes dentro del bloque. Calcularlos una
    //     vez por bloque (no por muestra) ahorra CPU sin diferencia
    //     audible.
    for (int v = 0; v < MAX_VOICES; ++v)
    {
        Voice& voice = voices[v];
        if (voice.env.isActive())
        {
            voice.stereoWidth = stereoWidth;
            voice.updatePanning(v);
        }
    }


    // ------------------------------------------------------------------------
    //  (2) Sample-by-sample loop / Bucle muestra a muestra
    // ------------------------------------------------------------------------

    for (int sample = 0; sample < sampleCount; ++sample)
    {
        keyVelocities.fill(100);

        // --------------------------------------------------------------------
        //  LFO update (sub-audio rate) / Actualización del LFO (sub-audio)
        // --------------------------------------------------------------------

        // EN: The LFO only runs every LFO_MAX samples (see Constants.h).
        //     This is safe because LFO frequencies are well below 20 Hz;
        //     aliasing is not a concern. Inside the block we recompute:
        //       - Vibrato pitch multiplier (LFO + mod wheel mix).
        //       - PWM pulse width for osc2.
        //       - Smoothed filter modulation in semitones (filter CC
        //         offset + filter-LFO + aftertouch).
        //       - Per-voice glide, oscillator frequencies, PWM width.
        // ES: El LFO solo corre cada LFO_MAX muestras (ver Constants.h).
        //     Es seguro porque las frecuencias del LFO están muy por
        //     debajo de 20 Hz; el aliasing no es problema. Dentro del
        //     bloque recalculamos:
        //       - Multiplicador de pitch para vibrato (LFO + mod wheel).
        //       - Ancho de pulso PWM para osc2.
        //       - Modulación suavizada de filtro en semitonos (offset
        //         de CC de filtro + LFO-filtro + aftertouch).
        //       - Glide, frecuencias de osc y ancho PWM por voz.
        if (++lfoCounter >= LFO_MAX)
        {
            lfoCounter = 0;
            const float lfoSine = lfo.nextSample();   // EN/ES: -1 .. +1

            // EN: Vibrato depth combines the preset's lfoDepthSemis plus
            //     the mod wheel (CC1) up to +2 semitones extra. The
            //     modWheel^2 curve gives gentler response near zero.
            // ES: La profundidad del vibrato combina lfoDepthSemis del
            //     preset más la rueda de modulación (CC1) hasta +2
            //     semitonos extra. La curva modWheel^2 da respuesta más
            //     suave cerca de cero.
            const float baseVibratoSemis = lfoDepthSemis;
            const float modWheelCurved = cc.modWheel * cc.modWheel;
            constexpr float kModWheelMaxSemis = 2.0f;
            const float effectiveVibratoSemis =
                baseVibratoSemis + (kModWheelMaxSemis * modWheelCurved);

            // EN: Convert the semitone modulation into a frequency
            //     multiplier: 2^(semis/12).
            // ES: Convertir la modulación en semitonos a un multiplicador
            //     de frecuencia: 2^(semis/12).
            const float vibratoPitchMul =
                std::exp2((lfoSine * effectiveVibratoSemis) / 12.0f);

            lfoPitchMul = vibratoPitchMul;

            // EN: PWM modulation. pwmDepth is clamped to 0.45 so the
            //     final pulse width stays inside the safe [0.05, 0.95]
            //     range that the oscillator enforces anyway.
            // ES: Modulación PWM. pwmDepth se limita a 0.45 para que el
            //     ancho de pulso final se mantenga dentro del rango
            //     seguro [0.05, 0.95] que el oscilador impone de todos
            //     modos.
            const float totalPwmDepth = juce::jlimit(0.0f, 0.45f, pwmDepth);
            const float pwmWidth = juce::jlimit(0.05f, 0.95f,
                0.5f + lfoSine * totalPwmDepth);


            // ----------------------------------------------------------------
            //  Global filter modulation / Modulación global de filtro
            // ----------------------------------------------------------------

            // EN: Three sources contribute to the filter cutoff
            //     modulation in the semitone domain:
            //       - Manual bipolar offset from CC74/75 (filterPlus/Minus),
            //         scaled to +/-12 semitones.
            //       - Aftertouch (channel pressure), scaled to +12 semis.
            //       - Filter LFO depth, modulated by the LFO sine itself.
            // ES: Tres fuentes contribuyen a la modulación del cutoff en
            //     el dominio de semitonos:
            //       - Offset bipolar manual desde CC74/75 (filterPlus/Minus),
            //         escalado a +/-12 semitonos.
            //       - Aftertouch (presión de canal), escalado a +12 semis.
            //       - Profundidad del LFO de filtro, modulada por el
            //         sine del LFO mismo.
            const float filterCtlSemis = 12.0f * (cc.filterPlus - cc.filterMinus);
            const float pressureSemis = 12.0f * cc.aftertouch;
            const float lfoSemis = (filterLFODepthSemis + pressureSemis) * lfoSine;
            const float filterModSemisTarget = filterCtlSemis + lfoSemis;

            // EN: One-pole smoothing of the filter modulation. dt is the
            //     time between LFO updates (LFO_MAX / sampleRate), and
            //     tau = 20 ms is the smoothing time constant. The
            //     coefficient a = 1 - exp(-dt/tau) is the standard IIR
            //     form for a first-order lowpass.
            //     This prevents zipper noise when the CC or the filter
            //     LFO change abruptly.
            // ES: Suavizado de un polo de la modulación del filtro. dt
            //     es el tiempo entre actualizaciones del LFO
            //     (LFO_MAX / sampleRate) y tau = 20 ms es la constante
            //     de tiempo del suavizado. El coeficiente
            //     a = 1 - exp(-dt/tau) es la forma IIR estándar para un
            //     lowpass de primer orden.
            //     Esto previene zipper noise cuando el CC o el LFO del
            //     filtro cambian abruptamente.
            const float dt = float(LFO_MAX) / sampleRate;
            const float tau = 0.02f;
            const float a = 1.0f - std::exp(-dt / tau);

            filterZipSemis += a * (filterModSemisTarget - filterZipSemis);            // actualizar voces activas SOLO cuando el LFO cambia
            // ----------------------------------------------------------------
                        //  Per-voice LFO-rate updates
                        //  Actualizaciones por voz al ritmo del LFO
                        // ----------------------------------------------------------------

                        // EN: Glide, oscillator frequencies, PWM width and filter
                        //     cutoff are all updated here, once every LFO_MAX samples.
                        //     These quantities change slowly enough that updating
                        //     them at the LFO rate is indistinguishable from
                        //     per-sample updating, but much cheaper.
                        // ES: Glide, frecuencias de osciladores, ancho PWM y cutoff
                        //     de filtro se actualizan aquí, una vez cada LFO_MAX
                        //     muestras. Estas magnitudes cambian lo bastante lento
                        //     como para que actualizarlas al ritmo del LFO sea
                        //     indistinguible de hacerlo por muestra, pero mucho más
                        //     barato.
            for (int v = 0; v < MAX_VOICES; ++v)
            {
                Voice& voice = voices[v];
                if (!voice.env.isActive())
                    continue;

                // EN: One-pole glide toward freqTarget. When
                //     glideRateThisNote == 1.0 the voice snaps to the
                //     target instantly (no glide). Lower values produce
                //     a smooth portamento whose speed is independent of
                //     the interval size (a musical choice: sliding one
                //     octave feels as fast as sliding one semitone).
                // ES: Glide de un polo hacia freqTarget. Cuando
                //     glideRateThisNote == 1.0 la voz salta al objetivo
                //     al instante (sin glide). Valores menores producen
                //     un portamento suave cuya velocidad es
                //     independiente del tamaño del intervalo (elección
                //     musical: deslizar una octava se siente tan rápido
                //     como deslizar un semitono).
                voice.freqCurrent +=
                    voice.glideRateThisNote * (voice.freqTarget - voice.freqCurrent);

                // EN: Apply pitch bend and vibrato (multiplicative) to
                //     the oscillator frequencies. osc2 additionally
                //     carries the detune offset from the preset.
                // ES: Aplicar pitch bend y vibrato (multiplicativos) a
                //     las frecuencias de los osciladores. osc2 lleva
                //     además el offset de detune del preset.
                voice.osc1.setFrequency(voice.freqCurrent * pitchBend * vibratoPitchMul);
                voice.osc2.setFrequency(voice.freqCurrent * pitchBend * detune * vibratoPitchMul);

                if (pwmDepth > 0.0f)
                    voice.osc2.setPulseWidth(pwmWidth);


                // ------------------------------------------------------------
                //  Filter cutoff modulation / Modulación del cutoff
                // ------------------------------------------------------------

                // EN: Active strategy for the filter cutoff, step by step:
                //       1. Base cutoff with CC74 brightness boost (x1..x3)
                //       2. Apply key tracking from the preset.
                //       3. Apply the smoothed semitone offset (filterZipSemis)
                //          which already combines CC74/75 + aftertouch + LFO.
                //       4. Compensate for pitch bend (classic JX detail:
                //          bend up also brightens the sound slightly).
                //       5. Clamp to a safe range before handing it to the filter.
                //     Resonance is the preset value plus a small extra
                //     from CC71, clamped to [0, 1].
                //     Alternative historical formulations are preserved
                //     below for reference.
                // ES: Estrategia activa para el cutoff del filtro, paso
                //     a paso:
                //       1. Cutoff base con realce de brightness CC74 (x1..x3)
                //       2. Aplicar key tracking del preset.
                //       3. Aplicar el offset en semitonos ya suavizado
                //          (filterZipSemis), que combina CC74/75 +
                //          aftertouch + LFO.
                //       4. Compensar pitch bend (detalle clásico del JX:
                //          hacer bend hacia arriba también abre un poco
                //          el sonido).
                //       5. Limitar a un rango seguro antes de pasarlo al
                //          filtro.
                //     La resonancia es el valor del preset más un aporte
                //     pequeño de CC71, limitada a [0, 1].
                //     Formulaciones históricas alternativas se conservan
                //     más abajo como referencia.

                float baseCutoff = filterCutoff * (1.0f + 2.0f * cc.brightness);
                float cutoffHz = calculateKeyTrackedCutoff(voice.note, baseCutoff);

                cutoffHz *= std::exp2(filterZipSemis / 12.0f);
                cutoffHz /= pitchBend;
                cutoffHz = std::clamp(cutoffHz, 80.0f, 0.45f * sampleRate);

                const float res01 =
                    std::clamp(filterResonance + 0.5f * cc.resonance, 0.0f, 1.0f);

                voice.setFilter(cutoffHz, res01);


                /*
                    ------------------------------------------------------------
                    HISTORICAL REFERENCE / REFERENCIA HISTÓRICA
                    ------------------------------------------------------------

                    EN: Two earlier formulations of the per-voice filter
                        cutoff modulation are preserved here. They were
                        functional but were replaced for the reasons noted.

                        (A) Verbose per-voice composition, before the
                            global `filterZipSemis` smoother was introduced:

                              const float filterCtlSemis = 12.0f * (cc.filterPlus - cc.filterMinus);
                              cutoffHz *= std::exp2(filterCtlSemis / 12.0f);
                              const float pressure      = cc.aftertouch;
                              const float pressureSemis = 12.0f * pressure;
                              const float filterModSemis = (filterLFODepthSemis + pressureSemis) * lfoSine;
                              cutoffHz *= std::exp2(filterModSemis / 12.0f);
                              cutoffHz /= pitchBend;
                              cutoffHz  = std::clamp(cutoffHz, 80.0f, 0.45f * sampleRate);

                            Discarded because it recomputed the same
                            modulation in every voice. The active version
                            computes it once globally in filterZipSemis,
                            which is cheaper and also smoother (no risk
                            of per-voice zipper drift).

                        (B) Resonance expressed as Q in [0.5, 10] and
                            re-normalized to [0, 1] before being sent to
                            the filter wrapper, with the filter envelope
                            modulating the cutoff directly here rather
                            than being applied by the voice itself:

                              float Q = filterResonance + 5.0f * cc.resonance;
                              Q = std::clamp(Q, 0.5f, 10.0f);
                              const float normalizedResonance = (Q - 0.5f) / (10.0f - 0.5f);

                              const float fe = voice.filterEnv.nextValue();
                              cutoffHz *= std::exp2((fe * filterEnvAmountSemis) / 12.0f);

                              voice.setFilter(cutoffHz, normalizedResonance);

                            Discarded because it advanced the filter
                            envelope twice (once here, once inside
                            voice.render()), which drained the envelope
                            at double rate and caused inconsistent
                            timing with the amplitude envelope.

                    ES: Dos formulaciones anteriores de la modulación del
                        cutoff por voz se conservan aquí. Eran funcionales
                        pero fueron reemplazadas por las razones anotadas.

                        (A) Composición verbosa por voz, antes de que se
                            introdujera el suavizado global
                            `filterZipSemis`:

                              const float filterCtlSemis = 12.0f * (cc.filterPlus - cc.filterMinus);
                              cutoffHz *= std::exp2(filterCtlSemis / 12.0f);
                              const float pressure      = cc.aftertouch;
                              const float pressureSemis = 12.0f * pressure;
                              const float filterModSemis = (filterLFODepthSemis + pressureSemis) * lfoSine;
                              cutoffHz *= std::exp2(filterModSemis / 12.0f);
                              cutoffHz /= pitchBend;
                              cutoffHz  = std::clamp(cutoffHz, 80.0f, 0.45f * sampleRate);

                            Se descartó porque recalculaba la misma
                            modulación en cada voz. La versión activa la
                            calcula una sola vez en filterZipSemis, que
                            es más barato y además más suave (sin riesgo
                            de drift de zipper por voz).

                        (B) Resonancia expresada como Q en [0.5, 10] y
                            re-normalizada a [0, 1] antes de enviarse al
                            wrapper del filtro, con la envolvente de
                            filtro modulando el cutoff aquí directamente
                            en vez de aplicarla la voz misma:

                              float Q = filterResonance + 5.0f * cc.resonance;
                              Q = std::clamp(Q, 0.5f, 10.0f);
                              const float normalizedResonance = (Q - 0.5f) / (10.0f - 0.5f);

                              const float fe = voice.filterEnv.nextValue();
                              cutoffHz *= std::exp2((fe * filterEnvAmountSemis) / 12.0f);

                              voice.setFilter(cutoffHz, normalizedResonance);

                            Se descartó porque avanzaba la envolvente de
                            filtro dos veces (una aquí, otra dentro de
                            voice.render()), agotándola al doble del
                            ritmo y causando desfase con la envolvente de
                            amplitud.
                    ------------------------------------------------------------
                */
            }
}  // EN: end of LFO-rate update  |  ES: fin del update al ritmo del LFO


        // --------------------------------------------------------------------
        //  Per-sample stereo summing / Suma estéreo por muestra
        // --------------------------------------------------------------------

        // EN: One noise sample shared by all voices. This is cheaper
        //     than per-voice noise and, more importantly, produces a
        //     unified noise texture instead of multiple uncorrelated
        //     streams.
        // ES: Una muestra de ruido compartida por todas las voces. Más
        //     barato que tener ruido por voz y, sobre todo, produce una
        //     textura de ruido unificada en vez de varios streams no
        //     correlacionados.
const float noise = noiseGen.nextValue() * noiseMix;

float outL = 0.0f;
float outR = 0.0f;

// EN: Sum the active voices into the stereo buses using each
//     voice's pre-computed pan gains.
// ES: Sumar las voces activas en los buses estéreo usando las
//     ganancias de paneo precalculadas por voz.
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


// --------------------------------------------------------------------
//  Output gain, expression and soft clip
//  Ganancia de salida, expression y soft clip
// --------------------------------------------------------------------

// EN: Smoothed master output level (avoids zipper noise).
// ES: Nivel maestro de salida suavizado (evita zipper noise).
const float gain = outputLevelSmoother.getNextValue();
outL *= gain;
outR *= gain;

// EN: CC11 Expression. Quadratic curve (x^2) gives finer control
//     in the low range, which is the standard pedal feel.
//     Expression is applied per-sample on top of the output gain
//     because it is a real-time performance control.
// ES: CC11 Expression. Curva cuadrática (x^2) da control más fino
//     en el rango bajo, que es la sensación estándar del pedal.
//     Se aplica por muestra encima de la ganancia de salida
//     porque es un control de interpretación en tiempo real.
float expr = cc.expression;
expr = expr * expr;
outL *= expr;
outR *= expr;

// EN: Soft-clip stage. The rational function  x / (1 + |x|)
//     smoothly saturates as |x| grows: it equals x for small
//     inputs, asymptotes to +/-1 for large inputs, and is
//     monotonic and continuous. Cheaper than tanh and perceptually
//     similar in this role. volumeTrim drives the saturator.
// ES: Etapa de soft-clip. La función racional  x / (1 + |x|)
//     satura suavemente al crecer |x|: vale x en entradas
//     pequeñas, tiende a +/-1 en entradas grandes, es monótona
//     y continua. Más barata que tanh y perceptualmente similar
//     en este rol. volumeTrim alimenta al saturador.
const float driveL = outL * volumeTrim;
const float driveR = outR * volumeTrim;

outL = driveL / (1.0f + std::abs(driveL));
outR = driveR / (1.0f + std::abs(driveR));


// --------------------------------------------------------------------
//  Write to output buffers / Escritura a los buffers de salida
// --------------------------------------------------------------------

// EN: If the host expects stereo (both pointers valid), write L/R
//     separately. If mono (right buffer null), collapse to the
//     midpoint. This keeps the plugin usable in mono chains.
// ES: Si el host espera estéreo (ambos punteros válidos), escribir
//     L/R por separado. Si es mono (puntero derecho null),
//     colapsar al punto medio. Así el plugin sigue siendo usable
//     en cadenas mono.
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


    // ------------------------------------------------------------------------
    //  (3) Per-block cleanup / Limpieza por bloque
    // ------------------------------------------------------------------------

    // EN: Recycle voices whose envelopes have fully decayed. Doing this
    //     once per block (not per sample) is enough because a silent
    //     voice has already stopped contributing to the mix; reclaiming
    //     its slot a few ms later has no audible effect.
    //     In mono mode only voice 0 is relevant; the rest of the pool
    //     is skipped.
    // ES: Reciclar voces cuya envolvente ya decayó completamente.
    //     Hacerlo una vez por bloque (no por muestra) es suficiente
    //     porque una voz silenciosa ya dejó de contribuir a la mezcla;
    //     recuperar su slot unos pocos ms después no tiene efecto
    //     audible.
    //     En modo mono solo importa la voz 0; el resto del pool se
    //     omite.
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

    // EN: Final safety net: catch NaN/Inf or out-of-range samples that
    //     may have slipped through a numerical edge case in the DSP
    //     chain. See Utils.h for details.
    // ES: Red de seguridad final: atrapar NaN/Inf o muestras fuera de
    //     rango que hayan podido colarse en algún caso numérico extremo
    //     del DSP. Ver Utils.h para los detalles.
    protectYourEars(outputBufferLeft, sampleCount);
    protectYourEars(outputBufferRight, sampleCount);
}
// ============================================================================
//  7. MIDI DISPATCH
//     DESPACHO MIDI
// ============================================================================

// EN: Single entry point for raw MIDI messages coming from the host.
//     The status byte (data0) is masked with 0xF0 to extract the message
//     type, ignoring the MIDI channel. Four types are handled:
//       - 0x80  Note off
//       - 0x90  Note on (note-on with velocity 0 is treated as note-off,
//               a common MIDI convention)
//       - 0xE0  Pitch bend
//       - 0xB0  Control change
//       - 0xD0  Channel pressure (aftertouch)
// ES: Único punto de entrada para los mensajes MIDI crudos del host.
//     El byte de estado (data0) se enmascara con 0xF0 para extraer el
//     tipo de mensaje, ignorando el canal MIDI. Se atienden cuatro tipos:
//       - 0x80  Note off
//       - 0x90  Note on (note-on con velocity 0 se trata como note-off,
//               convención MIDI habitual)
//       - 0xE0  Pitch bend
//       - 0xB0  Control change
//       - 0xD0  Channel pressure (aftertouch)
void Synth::midiMessage(uint8_t data0, uint8_t data1, uint8_t data2)
{
    switch (data0 & 0xF0)
    {
        // --- Note off ---
    case 0x80:
        noteOff(data1 & 0x7F);
        break;

        // --- Note on ---
    case 0x90:
    {
        uint8_t note = data1 & 0x7F;
        uint8_t velo = data2 & 0x7F;
        if (velo > 0)
            noteOn(note, velo);
        else
            noteOff(note);
        break;
    }

    // --- Pitch bend ---
    case 0xE0:
    {
        // EN: MIDI pitch bend is a 14-bit value from 0 to 16383,
        //     centered at 8192. The exponential coefficient 0.000014102
        //     converts the offset into a pitch multiplier that spans
        //     roughly one semitone across the wheel's full throw;
        //     change this coefficient to widen or narrow the bend range.
        // ES: El pitch bend MIDI es un valor de 14 bits entre 0 y
        //     16383, centrado en 8192. El coeficiente exponencial
        //     0.000014102 convierte el offset en un multiplicador de
        //     pitch que abarca aproximadamente un semitono en todo el
        //     recorrido de la rueda; cambiar este coeficiente amplía
        //     o reduce el rango del bend.
        pitchBend = std::exp(0.000014102f * float((data1 + 128 * data2) - 8192));

        // EN: Apply the new pitch bend to all active voices
        //     immediately, so the bend feels responsive. Without
        //     this, the pitch bend would only take effect at the
        //     next LFO-rate update (up to LFO_MAX samples later).
        // ES: Aplicar el nuevo pitch bend a todas las voces activas
        //     inmediatamente, para que el bend se sienta responsivo.
        //     Sin esto, el pitch bend solo se aplicaría en el
        //     siguiente update al ritmo del LFO (hasta LFO_MAX
        //     muestras después).
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

    // --- Control change ---
    case 0xB0:
        controlChange(data1, data2);
        break;

        // --- Channel pressure (aftertouch) ---
    case 0xD0:
    {
        // EN: Channel pressure is a single-byte message; data1 is the
        //     pressure value [0, 127], data2 is unused.
        // ES: Channel pressure es un mensaje de un solo byte; data1
        //     es el valor de presión [0, 127], data2 no se usa.
        cc.aftertouch = (data1 & 0x7F) / 127.0f;
        break;
    }
    }
}


// ============================================================================
//  8. LFO AND PWM SETTERS
//     SETTERS DE LFO Y PWM
// ============================================================================

// EN: Setters are kept minimal. PluginProcessor calls them whenever the
//     associated APVTS parameter changes.
// ES: Los setters se mantienen mínimos. PluginProcessor los llama cuando
//     cambia el parámetro APVTS asociado.

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


// ============================================================================
//  9. KEY STACK HELPERS
//     HELPERS DEL KEY STACK
// ============================================================================

// EN: These helpers implement the last-note-priority legato discussed in
//     Synth.h. The stack stores every MIDI note currently held, in the
//     order they were pressed. The top of the stack is always the note
//     that should sound in monophonic mode.
// ES: Estos helpers implementan el legato con prioridad a la última nota
//     descrito en Synth.h. El stack guarda cada nota MIDI aún pulsada en
//     el orden en que fueron pulsadas. El tope del stack es siempre la
//     nota que debe sonar en modo monofónico.


// EN: True if more than one key is currently held. Used by startVoice()
//     to decide whether a noteOn is part of a legato passage.
// ES: Verdadero si hay más de una tecla aún pulsada. Lo usa startVoice()
//     para decidir si un noteOn forma parte de un pasaje legato.
bool Synth::isPlayingLegatoStyle() const
{
    return keyStackSize > 1;
}


// EN: Bounds-checked lookup of the keyDown flag array.
// ES: Consulta con chequeo de rango del arreglo keyDown.
bool Synth::noteIsDown(int note) const
{
    if (note < 0 || note > 127) return false;
    return keyDown[note];
}


// EN: Marks a note as pressed and appends it to the stack. Duplicate
//     pushes (same note pressed again without release) are ignored to
//     keep the stack consistent.
// ES: Marca una nota como pulsada y la agrega al stack. Los push
//     duplicados (misma nota pulsada de nuevo sin liberar) se ignoran
//     para mantener el stack consistente.
void Synth::pushKey(int note)
{
    if (note < 0 || note > 127) return;
    if (keyDown[note]) return;

    keyDown[note] = true;
    keyStack[keyStackSize++] = note;
}


// EN: Marks a note as released and removes it from the stack while
//     preserving the order of the remaining notes. The shift is O(n) in
//     the stack size, which is at most 128 and in practice very small.
// ES: Marca una nota como liberada y la elimina del stack manteniendo el
//     orden de las notas restantes. El desplazamiento es O(n) en el
//     tamaño del stack, que es a lo sumo 128 y en la práctica muy pequeño.
void Synth::releaseKey(int note)
{
    if (note < 0 || note > 127) return;
    if (!keyDown[note]) return;

    keyDown[note] = false;

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


// EN: Returns the most recently pressed note still held, or -1 if no
//     key is down. Used by noteOff() for the last-note-priority fallback
//     in monophonic mode.
// ES: Devuelve la última nota pulsada que aún se mantiene, o -1 si
//     ninguna tecla está pulsada. La usa noteOff() para el fallback de
//     prioridad a última nota en modo monofónico.
int Synth::topKey() const
{
    return (keyStackSize > 0) ? keyStack[keyStackSize - 1] : -1;
}


// EN: Returns true if `note` arrives while another key was already held,
//     which is the strict definition of a legato note-on.
//     Two corner cases are ruled out:
//       - Empty stack: not a legato (first key of the phrase).
//       - Stack of size 1 with `note` already down: a repetition of the
//         same key, which should retrigger normally.
// ES: Devuelve true si `note` llega mientras había otra tecla ya
//     pulsada, que es la definición estricta de un note-on legato.
//     Se descartan dos casos límite:
//       - Stack vacío: no es legato (primera tecla de la frase).
//       - Stack de tamaño 1 con `note` ya pulsada: una repetición de la
//         misma tecla, que debe retriggearse normalmente.
bool Synth::legatoOnThisNoteOn(int note) const
{
    if (note < 0 || note > 127) return false;

    if (keyStackSize == 0) return false;
    if (keyStackSize == 1 && noteIsDown(note)) return false;
    return true;
}


// ============================================================================
//  10. FILTER TYPE SWITCHING
//      CAMBIO DE TIPO DE FILTRO
// ============================================================================

// EN: Switches the active filter algorithm for every voice. This is the
//     runtime hook for the FilterType enum: given a target type, each
//     voice's `filter` pointer is reassigned to either its svfFilter or
//     its moogFilter member, and the chosen filter is prepared with the
//     current sample rate and reset.
//
//     The early-out at the top serves two purposes:
//       - Skip work when the type has not changed.
//       - Bootstrap the filter pointers on the very first call (from the
//         constructor), where voices[0].filter is still null.
//
// ES: Cambia el algoritmo de filtro activo para cada voz. Es el enganche
//     en tiempo de ejecución del enum FilterType: dado un tipo objetivo,
//     el puntero `filter` de cada voz se reasigna al miembro svfFilter o
//     moogFilter y el filtro elegido se prepara con la sample rate
//     actual y se resetea.
//
//     La salida temprana al inicio cumple dos roles:
//       - Saltar trabajo si el tipo no ha cambiado.
//       - Inicializar los punteros de filtro en la primerísima llamada
//         (desde el constructor), cuando voices[0].filter aún es null.
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