/*
  ==============================================================================

    NoiseGenerator.h
    Created: 12 Nov 2025 1:55:23pm
    Author:  Jhonatan

  ==============================================================================
*/

/*
    Module: NoiseGenerator
    Purpose:
        EN: White noise source for the synthesizer. Produces uniformly
            distributed pseudo-random samples in the range [-1, 1).
        ES: Fuente de ruido blanco para el sintetizador. Produce muestras
            pseudoaleatorias uniformemente distribuidas en el rango [-1, 1).

    Main responsibilities:
        - EN: Maintain an internal seed for the pseudo-random sequence
        - EN: Generate one float sample per call without allocations
        - EN: Support reset to a known initial state
        - ES: Mantener una semilla interna para la secuencia pseudoaleatoria
        - ES: Generar una muestra float por llamada sin reservas de memoria
        - ES: Permitir reinicio a un estado inicial conocido

    Architectural role:
        EN: Used by Voice as the noise oscillator and by other modules that
            need a cheap, real-time-safe random source (e.g. envelope jitter
            or "analog" instabilities).
        ES: Usado por Voice como oscilador de ruido y por otros mos que
            necesitan una fuente aleatoria barata y segura para tiempo real
            (ej. jitter de envolventes o inestabilidades "analgicas").

    Notes:
        EN:
          - Combines a Linear Congruential Generator (LCG) with a direct
            IEEE-754 bit construction. This avoids divisions and yields
            uniform noise at near-zero CPU cost.
          - LCG constants come from "Numerical Recipes in C" (Press et al.,
            1992) and are widely used in audio (e.g. SuperCollider).
          - Not cryptographically secure: do NOT use for anything beyond
            audio synthesis.
        ES:
          - Combina un Generador Congruencial Lineal (LCG) con la
            construccin directa de un float IEEE-754. Esto evita divisiones
            y produce ruido uniforme con un costo de CPU casi nulo.
          - Las constantes del LCG provienen de "Numerical Recipes in C"
            (Press et al., 1992) y se usan ampliamente en audio (p. ej.
            SuperCollider).
          - No es criptogrficamente seguro: NO usar para nada fuera de
            s
ntesis de audio.
*/

#pragma once

#include <cstring>   // EN: for memcpy  |  ES: para memcpy
//#include <bit>     // EN: for std::bit_cast (C++20)  |  ES: para std::bit_cast (C++20)


class NoiseGenerator
{
public:
    // EN: Resets the seed to a known fixed value. Call before starting
    //     playback to ensure reproducible noise across runs.
    // ES: Reinicia la semilla a un valor fijo conocido. Llamar antes de
    //     iniciar la reproduccin para tener ruido reproducible entre runs.
    void reset()
    {
        noiseSeed = 22222;
    }

    // EN: Returns one pseudo-random sample in [-1, 1). Real-time safe:
    //     no allocations, no divisions, no branches.
    // ES: Devuelve una muestra pseudoaleatoria en [-1, 1). Segura para
    //     tiempo real: sin reservas, sin divisiones, sin saltos.
    float nextValue()
    {
        // -- Step 1: Linear Congruential Generator -------------------------
        // EN: Advances the seed using the LCG recurrence
        //         seed = seed * a + c
        //     The constants are the classic ones from Numerical Recipes.
        // ES: Avanza la semilla con la recurrencia del LCG
        //         seed = seed * a + c
        //     Las constantes son las clsicas de Numerical Recipes.
        noiseSeed = noiseSeed * 196314165 + 907633515;

        // -- Step 2: Build a float directly from random bits ---------------
        // EN: IEEE-754 trick. Mantissa is filled with 23 random bits
        //     (mask 0x7FFFFF); the exponent is forced to 128 (biased)
        //     by adding 0x40000000. The resulting bit pattern represents
        //     a float in [2.0, 4.0).
        // ES: Truco IEEE-754. La mantisa se llena con 23 bits aleatorios
        //     (mscara 0x7FFFFF); el exponente se fija en 128 (sesgado)
        //     sumando 0x40000000. El patrn de bits resultante representa
        //     un float en [2.0, 4.0).
        unsigned int r = (noiseSeed & 0x7FFFFF) + 0x40000000;

        // -- Step 3: Safe type punning (uint -> float) ---------------------
        // EN: Reinterpret the bits as a float. Both options below are
        //     equivalent; memcpy is portable, std::bit_cast requires C++20.
        // ES: Reinterpretar los bits como float. Ambas opciones son
        //     equivalentes; memcpy es portable, std::bit_cast requiere C++20.

        // OPTION A: C++20 (clean and modern)
        // float noise = std::bit_cast<float>(r);

        // OPTION B: Compatible with older C++ (C++98/11/14/17)
        float noise;
        std::memcpy(&noise, &r, sizeof(float));

        // -- Step 4: Map [2.0, 4.0) to [-1.0, 1.0) -------------------------
        // EN: A single subtraction shifts the range. No division needed.
        // ES: Una sola resta desplaza el rango. No hace falta divisin.
        return noise - 3.0f;
    }

private:
    // EN: Internal seed. Any non-zero value works; 22222 is arbitrary.
    // ES: Semilla interna. Cualquier valor no-cero funciona; 22222 es arbitrario.
    unsigned int noiseSeed = 22222;
};