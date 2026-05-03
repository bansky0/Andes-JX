<!-- ============================================================== -->
<!--                          AndesJX                               -->
<!-- ============================================================== -->

<div align="center">

# 🎹 AndesJX

**Polyphonic subtractive synthesizer inspired by the Ecuadorian Andean geography.**

**Sintetizador sustractivo polifónico inspirado en la geografía andina ecuatoriana.**

*Sintetizador desde la latitud cero.*

[![License: GPL v3](https://img.shields.io/badge/License-GPL_v3-blue.svg)](./LICENSE)
[![Platform: Windows | macOS](https://img.shields.io/badge/Platform-Windows_%7C_macOS-lightgrey.svg)]()
[![Format: VST3](https://img.shields.io/badge/Format-VST3-orange.svg)]()
[![Built with: JUCE](https://img.shields.io/badge/Built_with-JUCE-8b00ff.svg)](https://juce.com/)
[![NoiseRoomUIO](https://img.shields.io/badge/By-NoiseRoomUIO-c63d2f.svg)](https://www.instagram.com/noiseroom.uio/)

🌐 **Read in:** [English](#-andesjx--english-version) · [Español](#-andesjx--versión-en-español)

</div>

---

<!-- ============================================================== -->
<!--                       ENGLISH VERSION                          -->
<!-- ============================================================== -->

# 🇬🇧 AndesJX — English version

> **TL;DR** — AndesJX is a polyphonic subtractive synthesizer (VST3) inspired by the Andean geography of Ecuador. It combines a synthesis engine featuring two switchable filter models, 33 factory presets, and bilingual (English/Spanish) source code documentation designed to teach students and developers how DSP and audio plugin development actually work. Released under GPL v3. [Features](#-features) · [Get started](#-getting-started) · [Learn from the code](#-learn-from-the-code).

## 🌋 What is AndesJX?

**AndesJX** is a polyphonic subtractive synthesizer developed in C++ with the JUCE framework. It blends the warmth of classic analog synthesizers (especially the Roland JX family) with a sonic and visual identity rooted in the Andean landscapes of Ecuador: páramos, snow-capped mountains, lakes and volcanoes.

The plugin was born in 2025 as a product of **NoiseRoomUIO**, an independent audio laboratory based in Quito, designed for musicians and producers seeking an expressive tool with local character. Over time it evolved into something more: **an open educational project** where the code itself is meant to be read, studied and reused by students, teachers and developers who want to learn how a modern synthesizer is built from scratch.

## 🎯 Dual purpose: instrument + pedagogical resource

**As a musical instrument**, AndesJX provides a subtractive synthesis engine with two anti-aliased oscillators, two switchable filter models, independent ADSR envelopes for amplitude and filter, LFO modulation, configurable glide and 33 factory presets organized by sound family.

**As a pedagogical resource**, all source code is documented **bilingually (English/Spanish)**, organized into a three-layer architecture (DSP, plugin logic, GUI) and structured so each component can be studied independently. Students and professionals can use AndesJX as a starting point to understand how an audio plugin works internally, what design decisions sit behind each module, and how to modify or extend the code to build their own instruments.

## 🏗️ Architecture

AndesJX is structured in three clearly separated layers:

```
┌──────────────────────────────────────────────────┐
│              GUI Layer                           │
│   PluginEditor + 6 custom LookAndFeels           │
│   (theme, knobs, faders, combos, toggles)        │
├──────────────────────────────────────────────────┤
│            Plugin Layer                          │
│   PluginProcessor + APVTS + Preset System        │
│   (parameters, MIDI, state, host integration)    │
├──────────────────────────────────────────────────┤
│              DSP Layer                           │
│   Synth → Voice → Oscillators + Filter + EG      │
│   (pure audio processing, no UI dependencies)    │
└──────────────────────────────────────────────────┘
```

Each layer has a single, well-defined responsibility. The DSP layer has no knowledge of the GUI; the GUI layer never touches audio buffers directly. This separation makes the code easier to study, test and modify independently.

## 🔄 Signal flow inside a voice

When a note is played, audio flows through each polyphonic voice as follows:

```
MIDI ──▶ Synth ──▶ Voice (×N polyphony)
                    │
                    ├─▶ Osc 1 ─┐
                    ├─▶ Osc 2 ─┼─▶ Mixer ─▶ Filter ─▶ Amp Env ─▶ Output
                    ├─▶ Noise ─┘             ▲             ▲
                    │                        │             │
                    ├─▶ Filter Envelope ─────┘             │
                    └─▶ LFO (pitch / PWM / filter) ────────┘
```

The LFO is global (shared across voices), while everything else is per-voice. The filter is one of two interchangeable models (SVF or Moog ladder) selected at runtime.

## ✨ Features

### 🔊 Synthesis
- 2 oscillators (sine, saw, square, triangle, PWM)
- Anti-aliasing via **PolyBLEP**
- Continuous mix between osc1 / osc2
- Noise generator

### 🎚️ Filter
- **SVF** (state variable filter, topology-preserving)
- **Moog Ladder** with non-linear saturation
- Key tracking + key center
- Configurable filter envelope amount

### 🎛️ Modulation
- Independent ADSR (amplitude + filter)
- Global LFO (pitch / PWM / filter cutoff)
- Bipolar PWM/Vibrato knob

### 🎹 Performance
- Glide (off / legato / always)
- Mono / Poly modes
- Stereo width
- Velocity sensitivity (filter)

### ⚙️ Engine
- 2× oversampling
- Up to 16 voices polyphony
- 33 factory presets
- VST3 format (Windows + macOS)

## 🏔️ Preset bank

The 33 presets are organized into five families and follow the naming convention `[Family] – [Andean geography] [Musical term]`. Some examples:

| Family | Representative examples |
|---------|--------------------------|
| **Bass** | Cotacachi Ostinato · Imbabura Pedal · Chimborazo Sub |
| **Pad** | Cayambe 5th · Páramo Sostenuto · Altar Pianissimo |
| **Lead** | Cotopaxi Acid · Pichincha Unison · Sangay Fortissimo |
| **Brass / Wind / Organ** | Cóndor Tutti · Antisana Fanfare · Sumaco Rotary |
| **Keys / Pluck / FX** | Mojanda Rhodes · Cuicocha Bubble · Quilotoa Aqua |

Each name evokes a real place in the Ecuadorian Andes: volcanoes, snow-capped peaks, high-altitude lakes, páramos. The naming is not decorative — it is part of the instrument's cultural identity.

## 📜 Concept evolution

AndesJX did not start as it exists today. The first version (2025) focused on the 8 active volcanoes of Ecuador, with a warm three-tone palette and a simpler DSP architecture. As development progressed, the concept expanded:

- From **volcanoes only** to **complete Ecuadorian Andean geography** (snow peaks, lakes, páramos).
- From **8 presets** to **33 organized by sound family**.
- From **commercial product only** to **product + open educational resource** under GPL v3.

The original concept document (`concept.md`) is preserved in the repository as a record of the project's evolution.

## 🛠️ Technologies

- **Language**: C++17.
- **Framework**: [JUCE](https://juce.com/). AndesJX uses JUCE for general infrastructure (parameter management, MIDI, GUI, VST3 format), but the main components of the synthesis engine are **original implementations inspired by state-of-the-art technical literature**, not direct wrappers of JUCE classes.
- **Build**: [Projucer](https://juce.com/discover/projucer) (included with JUCE).
- **Target platforms**: Windows (VST3), macOS (VST3, AU planned).
- **No external dependencies** beyond JUCE.

## 📚 Learn from the code

AndesJX is designed so the source code itself serves as a learning resource. Suggested reading order:

**To understand the synthesis engine (DSP)**:
- `Source/Constants.h` and `Source/Utils.h` — project-wide constants and utilities.
- `Source/Envelope.h` — analog-style ADSR envelopes.
- `Source/Oscillator.h` and `OscillatorPolyBLEP.cpp` — anti-aliased waveform generation.
- `Source/IFilter.h`, `LadderFilter.h`, `SVF.h` — two filter models side by side.
- `Source/Voice.h` and `Source/Synth.cpp` — polyphonic voice management and MIDI handling.

**To understand the plugin architecture**:
- `Source/PluginProcessor.cpp` — the plugin's "brain": parameter management, presets, state.
- `Source/PluginEditor.cpp` — GUI, layout and parameter wiring.

**To understand the visual system**:
- `Source/LookAndFeel/AndesTheme.h` — centralized color palette and typography.
- `Source/LookAndFeel/AndesStyleHelpers.h` — reusable drawing primitives.
- `Source/LookAndFeel/*.h` — six custom LookAndFeels, one per control type.

Each file is **bilingually documented (EN/ES)** with blocks explaining the module's purpose, architectural role, and design decisions. Comments don't just say "what" the code does — they explain **why** it was done that way, what alternatives were considered, and what architectural contracts cross between files.

## 🚀 Getting started

### Use AndesJX as an instrument

1. Download the VST3 installer (coming soon for Windows and macOS from the NoiseRoomUIO website).
2. Place the `.vst3` file in your DAW's plugin folder:
   - **Windows**: `C:\Program Files\Common Files\VST3\`
   - **macOS**: `/Library/Audio/Plug-Ins/VST3/`
3. Open your DAW (Reaper, Ableton, FL Studio, Logic, Cubase, etc.), rescan plugins, and AndesJX will appear in the instrument list.

### Build from source

1. Clone the repository:
   ```bash
   git clone https://github.com/bansky0/Andes-JX.git
   cd Andes-JX
   ```
2. Install [JUCE](https://juce.com/get-juce) (version 7 or higher). Projucer is included.
3. Open the project's `.jucer` file with **Projucer**.
4. Export the project to your preferred IDE (Visual Studio for Windows, Xcode for macOS).
5. Build as VST3.

## 🔧 Use AndesJX as a starting point for your own project

AndesJX is released under GPL v3, which means you can:
- ✅ Fork and modify it freely for personal or educational use.
- ✅ Distribute your modified version, **as long as it is also released under GPL v3** (copyleft).
- ✅ Use it as the basis for courses, workshops and academic projects without restriction.

**Ideas for extending the project**:
- Add a third filter model by implementing the `IFilter` interface.
- Add output effects (chorus, delay, reverb) to the signal chain.
- Replace the cultural identity (visual palette, preset names, evoked geography) and create your own version.
- Implement new oscillator waveforms.
- Replace the GUI style by writing your own `LookAndFeel` classes.

If you want to contribute directly to the original project, pull requests are welcome but not the main goal — the project is designed for people to build **their own instruments** based on it.

## 📖 References

The development of AndesJX is built on recognized technical literature in the field of audio DSP and plugin development:

**Methodological foundations**:
- Hollemans, M., & Hodge, J. *Creating Synthesizer Plug-Ins with C++ and JUCE*. 
Independently published, 2023. (Primary methodological foundation of the project.)

Note: This work was developed using earlier iterations of the material (originally published as *Code Your Own Synth Plug-Ins with C++ and JUCE*). The 2023 edition constitutes the consolidated and updated version of that content.

- Pirkle, W. C. *Designing Software Synthesizer Plugins in C++ with Audio DSP*. 2nd edition, Routledge, 2021.

**State-of-the-art algorithms implemented in the code**:
- Välimäki, V., & Huovilainen, A. (2007). *Antialiasing Oscillators in Subtractive Synthesis*. IEEE Signal Processing Magazine, 24(2), 116-125. → PolyBLEP technique used in oscillators.
- Huovilainen, A. (2004). *Non-Linear Digital Implementation of the Moog Ladder Filter*. Proceedings of DAFx-04, Naples. → Moog filter model with non-linear saturation.
- Simper, A. (2013). *Linear Trapezoidal State Variable Filter SVF in state increment form*. Cytomic Technical Papers. → topology-preserving SVF model.

Each reference is cited in the code comments where the technique is applied. To go deeper, read AndesJX's source first, then go to the original publication for the full theoretical context.

## 👥 Team

| Person | Contributions | Affiliation |
|--------|---------------|-------------|
| **Jhonatan Guillermo López-Pilco** | Concept, design and overall project architecture. DSP engine implementation (oscillators, filters, envelopes, modulation). GUI design and implementation. Bilingual code documentation. | LASINAC, FIS, Escuela Politécnica Nacional, Quito, Ecuador |
| **Valeria Natacha Villarreal Villarreal** | Implementation of complementary DSP components. Testing, validation and design feedback for the instrument. | Instituto Tecnológico de Artes del Ecuador (ITAE), Guayaquil, Ecuador (primary) · Universidad de las Américas (UDLA), Quito, Ecuador (secondary) |

## 🎵 About NoiseRoomUIO

**NoiseRoomUIO** is an independent audio laboratory based in Quito, Ecuador, combining music production, sound design and technological research. The "UIO" in the name is Quito's airport code and marks the project's geographical origin.

AndesJX is the **third audio plugin** in the NoiseRoomUIO catalog, after **Classic Compressor CCMKI** (audio compressor) and **NoiseReverb** (impulse-response-based reverb captured in churches around Quito). The lab also produces hardware prototypes (Bluetooth speakers) and audiovisual projects.

📷 [@noiseroom.uio on Instagram](https://www.instagram.com/noiseroom.uio/)

## 📜 License

AndesJX is distributed under the **[GNU General Public License v3 (GPL v3)](./LICENSE)**.

You are free to use, study, modify and distribute the software, as long as derivative works are released under the same license. See the [LICENSE](./LICENSE) file for legal details.

## 🐛 Support

For bugs, feature suggestions or technical questions about the code, please use the [Issues](https://github.com/bansky0/Andes-JX/issues) section of this repository. Keeping technical support on GitHub means answers stay public and other users can benefit from them.

---

<div align="center">

🌐 [Switch to Spanish version ↓](#-andesjx--versión-en-español)

</div>

---
---

<!-- ============================================================== -->
<!--                       SPANISH VERSION                          -->
<!-- ============================================================== -->

# 🇪🇸 AndesJX — Versión en Español

> **TL;DR** — AndesJX es un sintetizador sustractivo polifónico (VST3) inspirado en la geografía andina del Ecuador. Combina un motor de síntesis con dos modelos de filtro conmutables, 33 presets de fábrica y código documentado en español/inglés pensado para que estudiantes y desarrolladores aprendan cómo funcionan realmente el DSP y el desarrollo de plugins. Licencia GPL v3. [Características](#-características) · [Cómo empezar](#-cómo-empezar) · [Aprende del código](#-aprende-del-código).

## 🌋 ¿Qué es AndesJX?

**AndesJX** es un sintetizador sustractivo polifónico desarrollado en C++ con el framework JUCE. Combina la calidez de los sintetizadores analógicos clásicos (especialmente la familia Roland JX) con una identidad sonora y visual propia, construida a partir de los paisajes andinos del Ecuador: páramos, nevados, lagunas y volcanes.

El plugin nació en 2025 como un producto de **NoiseRoomUIO**, un laboratorio independiente de audio con sede en Quito, pensado para músicos y productores que buscan una herramienta expresiva con carácter local. Con el tiempo evolucionó hacia algo más: **un proyecto educativo abierto** donde el código mismo está pensado para ser leído, estudiado y reutilizado por estudiantes, profesores y desarrolladores que quieran aprender cómo se construye un sintetizador moderno desde cero.

## 🎯 Doble propósito: instrumento + recurso pedagógico

**Como instrumento musical**, AndesJX ofrece un motor de síntesis sustractiva con dos osciladores con anti-aliasing, dos modelos de filtro conmutables, envolventes ADSR independientes para amplitud y filtro, modulación por LFO, glide configurable y un banco de 33 presets organizados por familias sonoras.

**Como recurso pedagógico**, todo el código fuente está documentado de forma **bilingüe (inglés/español)**, organizado en una arquitectura de tres capas (DSP, lógica del plugin, GUI) y estructurado para que cada componente pueda estudiarse de manera independiente. Estudiantes y profesionales pueden usar AndesJX como punto de partida para entender cómo funciona un plugin de audio por dentro, qué decisiones de diseño hay detrás de cada módulo y cómo modificar o extender el código para construir sus propios instrumentos.

## 🏗️ Arquitectura

AndesJX está estructurado en tres capas claramente separadas:

```
┌──────────────────────────────────────────────────┐
│              Capa GUI                            │
│   PluginEditor + 6 LookAndFeels custom           │
│   (tema, knobs, faders, combos, toggles)         │
├──────────────────────────────────────────────────┤
│            Capa Plugin                           │
│   PluginProcessor + APVTS + Sistema de Presets   │
│   (parámetros, MIDI, estado, integración host)   │
├──────────────────────────────────────────────────┤
│              Capa DSP                            │
│   Synth → Voice → Osciladores + Filtro + EG      │
│   (procesamiento puro de audio, sin UI)          │
└──────────────────────────────────────────────────┘
```

Cada capa tiene una responsabilidad única y bien definida. La capa DSP no conoce nada de la GUI; la capa GUI nunca toca buffers de audio directamente. Esta separación hace que el código sea más fácil de estudiar, probar y modificar de manera independiente.

## 🔄 Flujo de señal dentro de una voz

Cuando se toca una nota, el audio fluye a través de cada voz polifónica de la siguiente manera:

```
MIDI ──▶ Synth ──▶ Voice (×N polifonía)
                    │
                    ├─▶ Osc 1 ─┐
                    ├─▶ Osc 2 ─┼─▶ Mezclador ─▶ Filtro ─▶ Env Amp ─▶ Salida
                    ├─▶ Ruido ─┘                ▲           ▲
                    │                           │           │
                    ├─▶ Envolvente de Filtro ───┘           │
                    └─▶ LFO (pitch / PWM / filtro) ─────────┘
```

El LFO es global (compartido entre voces), mientras que el resto es por voz. El filtro es uno de dos modelos intercambiables (SVF o escalera Moog) seleccionado en tiempo de ejecución.

## ✨ Características

### 🔊 Síntesis
- 2 osciladores (sine, saw, square, triangle, PWM)
- Anti-aliasing mediante **PolyBLEP**
- Mezcla continua entre osc1 / osc2
- Generador de ruido

### 🎚️ Filtro
- **SVF** (state variable filter, topology-preserving)
- **Escalera Moog** con saturación no-lineal
- Key tracking + key center
- Cantidad de envolvente al filtro configurable

### 🎛️ Modulación
- ADSR independiente (amplitud + filtro)
- LFO global (pitch / PWM / cutoff del filtro)
- Knob bipolar PWM/Vibrato

### 🎹 Performance
- Glide (off / legato / always)
- Modos Mono / Poly
- Stereo width
- Sensibilidad a velocity (filtro)

### ⚙️ Motor
- Oversampling 2×
- Hasta 16 voces de polifonía
- 33 presets de fábrica
- Formato VST3 (Windows + macOS)

## 🏔️ Banco de presets

Los 33 presets se organizan en cinco familias y siguen la convención de nomenclatura `[Familia] – [Geografía andina] [Término musical]`. Algunos ejemplos:

| Familia | Ejemplos representativos |
|---------|--------------------------|
| **Bass** | Cotacachi Ostinato · Imbabura Pedal · Chimborazo Sub |
| **Pad** | Cayambe 5th · Páramo Sostenuto · Altar Pianissimo |
| **Lead** | Cotopaxi Acid · Pichincha Unison · Sangay Fortissimo |
| **Brass / Wind / Organ** | Cóndor Tutti · Antisana Fanfare · Sumaco Rotary |
| **Keys / Pluck / FX** | Mojanda Rhodes · Cuicocha Bubble · Quilotoa Aqua |

Cada nombre evoca un lugar real del Ecuador andino: volcanes, nevados, lagunas de altura, páramos. La nomenclatura no es decorativa — es parte de la identidad cultural del instrumento.

## 📜 Evolución del concepto

AndesJX no nació tal como existe hoy. La primera versión (2025) se centraba en los 8 volcanes activos del Ecuador, con una paleta de tres tonos cálidos y una arquitectura DSP más simple. A medida que el proyecto avanzó, el concepto se expandió:

- De **solo volcanes** a **geografía andina ecuatoriana completa** (nevados, lagunas, páramos).
- De **8 presets** a **33 organizados por familia sonora**.
- De **producto exclusivamente comercial** a **producto + recurso educativo abierto** bajo GPL v3.

El documento original del concepto (`concept.md`) se conserva en el repositorio como memoria de la evolución del proyecto.

## 🛠️ Tecnologías

- **Lenguaje**: C++17.
- **Framework**: [JUCE](https://juce.com/). AndesJX usa JUCE para la infraestructura general (gestión de parámetros, MIDI, GUI, formato VST3), pero los componentes principales del motor de síntesis son **implementaciones propias inspiradas en literatura técnica del estado del arte**, no wrappers directos de las clases de JUCE.
- **Build**: [Projucer](https://juce.com/discover/projucer) (incluido con JUCE).
- **Plataformas objetivo**: Windows (VST3), macOS (VST3, AU planeado).
- **Sin dependencias externas** más allá de JUCE.

## 📚 Aprende del código

AndesJX está diseñado para que el código fuente sirva como recurso de aprendizaje. Orden de lectura sugerido:

**Para entender el motor de síntesis (DSP)**:
- `Source/Constants.h` y `Source/Utils.h` — constantes y utilidades del proyecto.
- `Source/Envelope.h` — envolventes ADSR estilo analógico.
- `Source/Oscillator.h` y `OscillatorPolyBLEP.cpp` — generación de ondas con anti-aliasing.
- `Source/IFilter.h`, `LadderFilter.h`, `SVF.h` — dos modelos de filtro lado a lado.
- `Source/Voice.h` y `Source/Synth.cpp` — gestión de voces polifónicas y eventos MIDI.

**Para entender la arquitectura del plugin**:
- `Source/PluginProcessor.cpp` — el "cerebro" del plugin: gestión de parámetros, presets, estado.
- `Source/PluginEditor.cpp` — GUI, layout y conexión con parámetros.

**Para entender el sistema visual**:
- `Source/LookAndFeel/AndesTheme.h` — paleta de colores y tipografía centralizada.
- `Source/LookAndFeel/AndesStyleHelpers.h` — primitivas de dibujo reusables.
- `Source/LookAndFeel/*.h` — seis LookAndFeels custom, uno por tipo de control.

Cada archivo está **documentado de forma bilingüe (EN/ES)** con bloques que explican el módulo, su rol arquitectónico y las decisiones de diseño. Los comentarios no se limitan a "qué hace" el código — explican **por qué** se hizo de esa manera, qué alternativas se consideraron y qué contratos arquitectónicos cruzan entre archivos.

## 🚀 Cómo empezar

### Usar AndesJX como instrumento

1. Descarga el instalador VST3 (próximamente disponible para Windows y macOS desde la web de NoiseRoomUIO).
2. Coloca el archivo `.vst3` en la carpeta de plugins de tu DAW:
   - **Windows**: `C:\Program Files\Common Files\VST3\`
   - **macOS**: `/Library/Audio/Plug-Ins/VST3/`
3. Abre tu DAW (Reaper, Ableton, FL Studio, Logic, Cubase, etc.), escanea plugins nuevos y AndesJX aparecerá en la lista de instrumentos.

### Compilar desde el código fuente

1. Clona el repositorio:
   ```bash
   git clone https://github.com/bansky0/Andes-JX.git
   cd Andes-JX
   ```
2. Instala [JUCE](https://juce.com/get-juce) (versión 7 o superior). Projucer viene incluido.
3. Abre el archivo `.jucer` del proyecto con **Projucer**.
4. Exporta el proyecto a tu IDE preferido (Visual Studio en Windows, Xcode en macOS).
5. Compila como VST3.

## 🔧 Usar AndesJX como base para tu propio proyecto

AndesJX se distribuye bajo licencia GPL v3, lo que significa que puedes:
- ✅ Hacer un fork y modificarlo libremente para uso personal o educativo.
- ✅ Distribuir tu versión modificada, **siempre que también sea bajo GPL v3** (copyleft).
- ✅ Usarlo como base de cursos, talleres y proyectos académicos sin restricción.

**Ideas para extender el proyecto**:
- Agregar un tercer modelo de filtro implementando la interfaz `IFilter`.
- Añadir efectos de salida (chorus, delay, reverb) en la cadena.
- Cambiar la identidad cultural (paleta visual, nombres de presets, geografía evocada) y crear tu propia versión.
- Implementar nuevas formas de onda en los osciladores.
- Sustituir el estilo de la GUI escribiendo tus propios `LookAndFeel`.

Si quieres contribuir directamente al proyecto original, las pull requests son bienvenidas pero no son la prioridad principal — el proyecto está pensado para que la gente cree **sus propios instrumentos** a partir de él.

## 📖 Referencias

El desarrollo de AndesJX se apoya en bibliografía técnica reconocida en el campo del DSP de audio y el desarrollo de plugins:

**Pilares metodológicos**:
- Hollemans, M., & Hodge, J. *Creating Synthesizer Plug-Ins with C++ and JUCE*. 
Independently published, 2023. (Base metodológica principal del proyecto.)

Nota: El desarrollo se basó en versiones previas del material (publicadas originalmente como *Code Your Own Synth Plug-Ins with C++ and JUCE*). La edición de 2023 representa la versión consolidada y actualizada de dicho contenido.

- Pirkle, W. C. *Designing Software Synthesizer Plugins in C++ with Audio DSP*. 2ª edición, Routledge, 2021.

**Algoritmos del estado del arte implementados en el código**:
- Välimäki, V., & Huovilainen, A. (2007). *Antialiasing Oscillators in Subtractive Synthesis*. IEEE Signal Processing Magazine, 24(2), 116-125. → técnica PolyBLEP usada en los osciladores.
- Huovilainen, A. (2004). *Non-Linear Digital Implementation of the Moog Ladder Filter*. Proceedings of DAFx-04, Naples. → modelo de filtro Moog con saturación no-lineal.
- Simper, A. (2013). *Linear Trapezoidal State Variable Filter SVF in state increment form*. Cytomic Technical Papers. → modelo SVF topology-preserving.

Cada referencia está citada en los comentarios del código, en el archivo donde se aplica la técnica. Para profundizar, lee primero el código de AndesJX y luego ve a la fuente original para el contexto teórico completo.

## 👥 Equipo

| Persona | Aportes | Afiliación |
|---------|---------|------------|
| **Jhonatan Guillermo López-Pilco** | Concepto, diseño y arquitectura general del proyecto. Implementación del motor DSP (osciladores, filtros, envolventes, modulación). Diseño e implementación de la interfaz gráfica. Documentación bilingüe del código. | LASINAC, FIS, Escuela Politécnica Nacional, Quito, Ecuador |
| **Valeria Natacha Villarreal Villarreal** | Implementación de componentes DSP complementarios. Pruebas, validación y retroalimentación de diseño del instrumento. | Instituto Tecnológico de Artes del Ecuador (ITAE), Guayaquil, Ecuador (principal) · Universidad de las Américas (UDLA), Quito, Ecuador (secundaria) |

## 🎵 Acerca de NoiseRoomUIO

**NoiseRoomUIO** es un laboratorio independiente de audio con base en Quito, Ecuador, que combina producción musical, diseño sonoro e investigación tecnológica. El "UIO" del nombre es el código de aeropuerto de Quito y marca la geografía de origen del proyecto.

AndesJX es **el tercer plugin de audio** del catálogo de NoiseRoomUIO, después de **Classic Compressor CCMKI** (compresor de audio) y **NoiseReverb** (reverb basada en respuestas impulsivas grabadas en iglesias de Quito). El laboratorio también desarrolla prototipos de hardware (parlantes Bluetooth) y proyectos audiovisuales.

📷 [@noiseroom.uio en Instagram](https://www.instagram.com/noiseroom.uio/)

## 📜 Licencia

AndesJX se distribuye bajo la **[Licencia Pública General de GNU, versión 3 (GPL v3)](./LICENSE)**.

Tienes libertad para usar, estudiar, modificar y distribuir el software, siempre que las obras derivadas también se distribuyan bajo la misma licencia. Consulta el archivo [LICENSE](./LICENSE) para los detalles legales.

## 🐛 Soporte

Para reportar bugs, sugerir mejoras o hacer preguntas técnicas sobre el código, usa la sección [Issues](https://github.com/bansky0/Andes-JX/issues) de este repositorio. Mantener el soporte técnico en GitHub permite que las respuestas queden documentadas para que otros usuarios puedan beneficiarse.

---

<div align="center">

🌐 [Volver a la versión en inglés ↑](#-andesjx--english-version)

**Desarrollado en Quito, Ecuador — a 2850 metros sobre el nivel del mar.**

</div>