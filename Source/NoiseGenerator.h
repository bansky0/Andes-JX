/*
  ==============================================================================
    NoiseGenerator.h

    ESP:
    Generador simple de ruido blanco para el sintetizador.
    Produce muestras pseudoaleatorias normalmente en rango [-1.0, 1.0] y se usa
    como fuente adicional de señal (mezcla audible o modulación).

    Este módulo es intencionalmente liviano y no depende de tono ni envolvente.
    Cualquier control dinámico (ganancia, paneo, filtrado) se aplica externamente
    en Voice o Synth.

    ENG:
    Simple white noise generator for the synthesizer.
    Produces pseudo-random samples typically in the range [-1.0, 1.0] and is used
    as an additional signal source (audible mix or modulation).

    This module is intentionally lightweight and independent from pitch or
    envelope. Any dynamic control (gain, panning, filtering) is applied externally
    in Voice or Synth.
  ==============================================================================
*/

#pragma once

#include <cstring> // Para memcpy
//#include <bit>     // Para std::bit_cast (C++20)
//------------------------------------------------------------------------------
// ESP: Clase encargada de generar ruido blanco. No mantiene fase ni frecuencia,
//      solo el estado interno del generador pseudoaleatorio.
// ENG: Class responsible for generating white noise. It does not maintain
//      phase or frequency, only the internal state of the pseudo-random generator.
//------------------------------------------------------------------------------


class NoiseGenerator
{
public:
//------------------------------------------------------------------------------
// ESP: Resetea el estado interno del generador pseudoaleatorio.
// ENG: Resets the internal state of the pseudo-random generator.
//------------------------------------------------------------------------------

    void reset()
    {
        noiseSeed = 22222;
    }
//------------------------------------------------------------------------------
// ESP: Devuelve una muestra de ruido blanco (pseudoaleatoria).
//      La amplitud final debe controlarse externamente.
// ENG: Returns a white noise (pseudo-random) sample.
//      Final amplitude should be controlled externally.
//------------------------------------------------------------------------------


    float nextValue()
    {
        // 1. Lineal Congruential Generator - LCG Step
        noiseSeed = noiseSeed * 196314165 + 907633515;

        // 2. Construct float using bits
        unsigned int r = (noiseSeed & 0x7FFFFF) + 0x40000000;

        // 3. Safe convertion

        // Compatible with old C++ (C++98/11/14/17)
        float noise;
        std::memcpy(&noise, &r, sizeof(float));

        //4. Range [-1, 1]
        return noise - 3.0f;
    }

private:
    unsigned int noiseSeed = 22222;
};
