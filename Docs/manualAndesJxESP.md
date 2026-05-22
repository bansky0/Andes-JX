<div align="center">

# 🎹 Andes JX

## Manual de Usuario

**Sintetizador sustractivo polifónico**  
*Síntesis desde la latitud cero*

**Versión 1.0** · **Mayo 2026**

---

Por **NoiseRoomUIO**  
Quito, Ecuador

[github.com/bansky0/Andes-JX](https://github.com/bansky0/Andes-JX) · [@noiseroom.uio](https://www.instagram.com/noiseroom.uio/)

</div>

---

<div align="center">

*Un sintetizador sustractivo polifónico desarrollado en C++ con JUCE,*  
*inspirado en la geografía andina del Ecuador.*

</div>

---

## Tabla de contenidos

### Parte 1 — Fundamentos

1. [Introducción](#1-introducción)
2. [Concepto del instrumento](#2-concepto-del-instrumento)
3. [Instalación y primeros pasos](#3-instalación-y-primeros-pasos)
4. [Descripción general de la interfaz](#4-descripción-general-de-la-interfaz)

### Parte 2 — Práctica

5. [Motor de síntesis](#5-motor-de-síntesis)
   - 5.1 [Oscillators](#51-oscillators)
   - 5.2 [Filter](#52-filter)
   - 5.3 [Envelopes](#53-envelopes)
   - 5.4 [Modulation](#54-modulation)
   - 5.5 [Performance](#55-performance)
   - 5.6 [Master](#56-master)
   - 5.7 [Sistema de presets](#57-sistema-de-presets)

6. [Tutoriales de sound design](#6-tutoriales-de-sound-design)
   - 6.1 [Tutorial 1 — Bass: Cotacachi Ostinato](#61-tutorial-1--bass-cotacachi-ostinato)
   - 6.2 [Tutorial 2 — Pad: Cayambe 5th](#62-tutorial-2--pad-cayambe-5th)
   - 6.3 [Tutorial 3 — Lead: Cotopaxi Acid](#63-tutorial-3--lead-cotopaxi-acid)

7. [Técnicas de sound design](#7-técnicas-de-sound-design)
   - 7.1 [Trabajando con el filtro](#71-trabajando-con-el-filtro)
   - 7.2 [Modulación interesante](#72-modulación-interesante)
   - 7.3 [Movimiento interno](#73-movimiento-interno)

### Parte 3 — Referencia

8. [Aprendiendo DSP con Andes JX](#8-aprendiendo-dsp-con-andes-jx)
   - 8.1 [Lo que necesitas para comenzar](#81-lo-que-necesitas-para-comenzar)
   - 8.2 [La arquitectura en tres capas](#82-la-arquitectura-en-tres-capas)
   - 8.3 [Archivos clave para estudiar](#83-archivos-clave-para-estudiar)
   - 8.4 [Rutas de estudio sugeridas](#84-rutas-de-estudio-sugeridas)
   - 8.5 [Cómo los parámetros se conectan con el algoritmo](#85-cómo-los-parámetros-se-conectan-con-el-algoritmo)
   - 8.6 [Dónde viven las referencias académicas en el código](#86-dónde-viven-las-referencias-académicas-en-el-código)
   - 8.7 [Una nota sobre la documentación bilingüe](#87-una-nota-sobre-la-documentación-bilingüe)

9. [Banco de presets](#9-banco-de-presets)
   - 9.1 [Init](#91-init)
   - 9.2 [Familia Bass](#92-familia-bass-7-presets)
   - 9.3 [Familia Pad](#93-familia-pad-6-presets)
   - 9.4 [Familia Lead](#94-familia-lead-6-presets)
   - 9.5 [Familia Brass / Wind / Organ](#95-familia-brass--wind--organ-6-presets)
   - 9.6 [Familia Keys / Pluck / FX](#96-familia-keys--pluck--fx-8-presets)

10. [Solución de problemas y FAQ](#10-solución-de-problemas-y-faq)
    - 10.1 [Problemas genéricos de plugins](#101-problemas-genéricos-de-plugins)
    - 10.2 [Preguntas específicas de Andes JX](#102-preguntas-específicas-de-andes-jx)

11. [Glosario](#11-glosario)

12. [Créditos y referencias](#12-créditos-y-referencias)
    - 12.1 [Equipo de Andes JX](#121-equipo-de-andes-jx)
    - 12.2 [Este manual](#122-este-manual)
    - 12.3 [Agradecimientos](#123-agradecimientos)
    - 12.4 [Referencias](#124-referencias)
    - 12.5 [Licencia y distribución](#125-licencia-y-distribución)
    - 12.6 [Contacto y soporte](#126-contacto-y-soporte)

---

## 1. Introducción

### ¿Qué es Andes JX?

Andes JX es un plugin sintetizador sustractivo polifónico (VST3) desarrollado por NoiseRoomUIO en Quito, Ecuador. Combina dos osciladores anti-aliased, dos modelos de filtro intercambiables —un *state-variable filter* transparente y un *Moog ladder* no lineal—, envolventes ADSR independientes para amplitud y filtro, un LFO global para modulación y un banco de 33 presets de fábrica organizados por familia sonora.

El diseño del instrumento toma como punto de partida la calidez y el carácter de los sintetizadores analógicos clásicos, pero introduce una identidad sonora y visual enraizada en la geografía andina del Ecuador. Los nombres de presets, la paleta de colores y la estética general hacen referencia a lugares reales: volcanes, nevados, lagunas de altura y páramos.

Andes JX también es un **proyecto educativo open-source**. Su código fuente completo está publicado bajo GPL v3 y documentado de forma bilingüe —inglés y español—, de modo que pueda estudiarse como referencia para aprender DSP y desarrollo de plugins de audio. Esta doble naturaleza —herramienta de producción y recurso de aprendizaje— atraviesa cada aspecto del instrumento.

### ¿Para quién es este manual?

Este manual está escrito para *productores musicales* y *sound designers* que ya tienen una familiaridad básica con la síntesis sustractiva: sabes qué hacen un oscilador, una envolvente y un filtro en términos generales, y has usado al menos otro sintetizador software antes.

Si eres completamente nuevo en síntesis, el manual seguirá siendo comprensible, pero puede resultarte útil tener a mano algún recurso introductorio sobre síntesis para ciertos conceptos más profundos. Al final de este manual encontrarás un breve [glosario](#11-glosario) que cubre los términos más importantes.

Si eres un *sound designer* experimentado o *desarrollador DSP*, encontrarás aquí información práctica para usar Andes JX como instrumento; para profundizar en la implementación técnica, la referencia recomendada es el código fuente del proyecto.

### Cómo usar este manual

El manual está estructurado en tres partes:

1. **Fundamentos** (secciones 1–4) — qué es Andes JX, cómo suena, cómo instalarlo y qué hace cada parte de la interfaz.
2. **Práctica** (secciones 5–7) — el motor de síntesis en detalle, tres tutoriales paso a paso de *sound design* y técnicas avanzadas.
3. **Referencia** (secciones 8–12) — rutas de aprendizaje desde el código, el banco completo de presets con descripciones, solución de problemas, glosario y créditos.

El manual puede leerse de forma secuencial o usarse como documento de consulta. Si eres nuevo en Andes JX, se recomienda trabajar las secciones 1–4 en orden antes de abrir el plugin: te ahorrará tiempo más adelante.

> 🎓 **Sobre los bloques educativos de este manual.** Las secciones marcadas con el ícono 🎓 contienen contexto opcional sobre la teoría o las decisiones de diseño detrás de una característica. Si solo quieres usar Andes JX como herramienta, puedes omitirlas sin problema. Si quieres entender *por qué* funciona como funciona, son el camino más corto hacia la lógica detrás del instrumento.

---

## 2. Concepto del instrumento

### Filosofía sonora

Andes JX fue diseñado alrededor de tres intenciones principales:

**Calidez sin nostalgia.** El Roland JX-8P y el JX-10 son referencias fundamentales para el proyecto, pero Andes JX no es una emulación. El objetivo nunca fue reproducir un instrumento vintage, sino heredar una manera de pensar el sonido —suave, musical, expresiva e inmediata— y combinarla con técnicas modernas de DSP. El resultado es un instrumento familiar para cualquiera que haya tocado *polysynths* analógicos, pero que no pretende pertenecer a otra época.

**Dos caracteres de filtro en un solo instrumento.** Andes JX incluye dos modelos de filtro que producen colores sonoros genuinamente distintos: un **state-variable filter** transparente para timbres limpios y modernos, y un **Moog ladder** con saturación no lineal para un carácter cálido, agresivo y distintivo. Cambiar entre ellos modifica profundamente la personalidad del instrumento. La elección es una decisión creativa, no solo técnica.

**Pocos controles, resultados profundos.** En lugar de ofrecer decenas de parámetros, Andes JX apuesta por un conjunto enfocado de aproximadamente 30 controles cuidadosamente seleccionados. Cada parámetro tiene un rol sonoro claro y un rango calibrado. La intención es que un productor pueda llegar rápidamente al sonido deseado sin perderse entre menús y submenús.

### Identidad andina

La identidad cultural de Andes JX no es decorativa. Está integrada en tres aspectos concretos del instrumento:

**Identidad visual.** La interfaz utiliza una paleta azul grisácea inspirada en la piedra andina de alta montaña, acompañada de texto en tonos *off-white* suaves. No hay colores brillantes ni acentos neón; el lenguaje visual refleja la sobriedad serena de la alta montaña más que el exceso visual típico de muchos plugins modernos.

**Nombres de presets.** Cada preset de fábrica toma el nombre de una ubicación andina real del Ecuador acompañado de un término musical: *Cotopaxi Acid*, *Cayambe 5th*, *Imbabura Pedal*, *Quilotoa Aqua*, *Páramo Sostenuto*. Los nombres no son asociaciones aleatorias; cada uno relaciona el carácter geográfico del lugar con una cualidad musical del sonido. *Cotopaxi*, un estratovolcán activo, da nombre a un *lead* con resonancia explosiva. *Cayambe*, un nevado atravesado por la línea ecuatorial, inspira un *pad* etéreo. La nomenclatura funciona también como una pequeña lección de geografía integrada en el instrumento.

**Marco cultural.** Andes JX plantea la síntesis sustractiva como una técnica universal que puede reinterpretarse desde un lugar específico. Quito se encuentra a 2.850 metros sobre el nivel del mar, sobre la línea ecuatorial. Esa es la latitud cero desde la que habla este instrumento: no como folclor o estilización, sino como el origen material del proyecto.

### Alcance y limitaciones

Andes JX 1.0 es un sintetizador sustractivo enfocado. Hace bien aquello para lo que la síntesis sustractiva fue diseñada: bajos, leads, pads, plucks, texturas evolutivas y timbres analógicos clásicos. No está diseñado para:

- **FM o wavetable synthesis.** Los osciladores utilizan formas de onda clásicas (*sine*, *saw*, *square*, *triangle*, *PWM*), sin modulación de fase entre ellas ni *wavetable scanning*.

- **Sample playback o procesamiento granular.** Andes JX no es un sampler ni un instrumento granular.

- **Efectos integrados.** No existen efectos de chorus, delay, reverb o distorsión dentro de la cadena de señal. El instrumento está pensado para ser procesado externamente con los efectos que mejor se adapten a cada producción. Versiones futuras podrían reconsiderar esta decisión.

- **Matrices complejas de modulación.** La modulación en Andes JX es directa: el LFO se enruta hacia pitch, PWM y filter cutoff. Las envolventes controlan amplitud y filtro. No existe una matriz de modulación de ruteo libre.

Estas limitaciones son deliberadas. Un instrumento más complejo sería más difícil de aprender, más difícil de leer desde el código fuente y menos útil como herramienta pedagógica. Andes JX sacrifica amplitud de funciones en favor de accesibilidad y profundidad.

---

## 3. Instalación y primeros pasos

### Requisitos del sistema

| Requisito | Windows | macOS |
|-------------|---------|-------|
| Sistema operativo | Windows 10 o superior (64-bit) | macOS 10.13 o superior (Intel + Apple Silicon) |
| Formato de plugin | VST3 | VST3, AU |
| DAW | Cualquier host con soporte VST3 (Reaper, Ableton Live, FL Studio, Cubase, Studio One, Bitwig, etc.) | Cualquier host con soporte VST3 (compatibilidad AU para Logic Pro próximamente) |
| RAM | 200 MB recomendados | 200 MB recomendados |
| CPU | Dual-core moderno o superior | Dual-core moderno o superior |

Andes JX está desarrollado con JUCE 7 y sigue la especificación estándar VST3. Si tu DAW soporta cualquier otro plugin VST3 moderno, también soportará Andes JX.

### Instalación

El instalador de Andes JX funciona mediante el proceso estándar de doble clic. La instalación es directa:

1. Descarga el instalador correspondiente a tu plataforma desde el Instagram de NoiseRoomUIO o el sitio web del desarrollador (o desde la página de *releases* de GitHub una vez esté disponible).
2. Haz doble clic sobre el instalador y sigue las instrucciones en pantalla.
3. Presiona **Next / Continue** en cada paso hasta completar la instalación.
4. Abre tu DAW y realiza un *rescan* de plugins. Andes JX debería aparecer en la lista de instrumentos bajo **NoiseRoomUIO**.

El instalador coloca el plugin en la ubicación estándar para VST3:

- **Windows**: `C:\Program Files\Common Files\VST3\AndesJX.vst3`
- **macOS**: `/Library/Audio/Plug-Ins/VST3/AndesJX.vst3`

> ⚠️ **Reinstalación o actualización a una nueva versión.** Si necesitas reinstalar Andes JX o actualizar desde una versión anterior, elimina manualmente el archivo `.vst3` antiguo antes de ejecutar el nuevo instalador. Esto evita conflictos de versión que pueden confundir el escáner de plugins de tu DAW.

### Cargando Andes JX en tu DAW

Cada DAW maneja la carga de plugins de forma ligeramente distinta, pero el procedimiento general es el siguiente:

1. Crea una nueva pista MIDI o de instrumento.
2. Desde el slot de instrumento de la pista, busca **NoiseRoomUIO → Andes JX** (o simplemente **Andes JX**, dependiendo de tu DAW).
3. La ventana del plugin se abrirá con el estado por defecto cargado.
4. Conecta un controlador MIDI o utiliza el piano roll / teclado virtual de tu DAW para tocar notas.

Si Andes JX no aparece en tu DAW después de instalarlo, verifica que el DAW esté configurado para escanear la carpeta VST3 donde se instaló el plugin. La mayoría de DAWs modernos hacen esto automáticamente, pero algunos requieren un *rescan* manual (normalmente en **Preferences → Plugins → Rescan**).

### Tu primer sonido (60 segundos)

Una vez que Andes JX esté cargado:

1. Haz clic en el menú desplegable **PRESET** en la parte inferior de la interfaz.
2. Selecciona **Cotacachi Ostinato** (familia Bass) o cualquier preset que llame tu atención.
3. Toca una nota. Deberías escuchar sonido inmediatamente.
4. Mientras sostienes una nota, gira lentamente el knob **CUTOFF** en la sección Filter. El brillo del sonido cambiará.
5. Suelta la nota. Observa cómo el sonido se desvanece gradualmente: esa es la etapa de *release* de la envolvente de amplitud.

Eso es todo. Ahora estás tocando un sintetizador diseñado a 2.850 metros sobre el nivel del mar.

---

## 4. Descripción general de la interfaz

![Andes JX Interface](screenshots/ScreenshotAndesJX.png)

La interfaz de Andes JX está dividida en seis zonas. Cada una agrupa controles relacionados dentro de una sección coherente. La distribución visual se lee naturalmente de arriba hacia abajo y de izquierda a derecha, siguiendo el flujo de señal típico de un sintetizador sustractivo.

> 📍 **Sobre esta sección.** Este es el mapa del instrumento: dónde se encuentra cada zona, qué tipo de controles agrupa y qué rol cumple dentro del flujo de señal. El comportamiento detallado de cada parámetro individual (rango exacto, curva de respuesta, efecto sonoro) se desarrolla en la sección 5 — *Motor de síntesis*, donde cada zona tiene su propia subsección dedicada.

### Zona 1 — Oscillators

Ubicada en la **parte superior izquierda** de la interfaz. Esta zona contiene las fuentes sonoras y todos los parámetros que moldean la salida cruda de los osciladores antes de llegar al filtro.

**Controles por oscilador:**

- Oscillator 1 — selector de waveform (*sine / saw / square / triangle / PWM*).
- Oscillator 2 — selector de waveform + controles dedicados de afinación:
  (**COARSE** en semitonos, **FINE** en cents, desplazamiento de **OCTAVE**).
  Estos controles permiten desafinar Osc 2 respecto a Osc 1, base del clásico sonido sustractivo de dos osciladores.

**Controles globales de esta zona:**

- **MIX** (knob central) — mezcla entre Osc 1 (izquierda) y Osc 2 (derecha), mostrada en tiempo real como una proporción `osc1:osc2`.
- **WIDTH** — control global de apertura estéreo con una distribución de notas similar a la de un piano.
- **NOISE** — generador de ruido independiente añadido a la mezcla.
- **TUNE** — afinación global del instrumento en cents (equivalente a un *master tune*).

Esta zona es la fuente sonora del sintetizador. Todo lo demás en el instrumento moldea lo que estos controles producen.

### Zona 2 — Performance

Ubicada en la **parte superior derecha** de la interfaz. Contiene:

- Selector de modo **GLIDE** (*Off / Legato / Always*) junto con los knobs de *glide rate* y *glide bend*.
- Selector **VOICE** mono/poly.
- Knob bipolar **PWM/VIB** (un solo control, dos funciones: valores positivos producen vibrato y valores negativos producen *pulse-width modulation*).
- Knob **VEL FLTR**, que controla cómo la velocidad MIDI afecta el *filter cutoff*.

Esta zona define cómo responde el instrumento al tocarlo: su comportamiento entre notas, voces y dinámica.

### Zona 3 — Filter

Ubicada en la **parte media izquierda** de la interfaz. Contiene:

- El selector **TYPE** (*SVF / Moog*).
- Los dos knobs principales **CUTOFF** y **RESO**.
- Los controles de modulación del filtro:
  **ENV AMT** (*envelope amount*),
  **KEY TRCK** (*key tracking*),
  **KEY CNTR** (*key center*).

Esta zona es el moldeador sonoro principal. El filtro es donde vive gran parte del carácter de Andes JX, y la elección del modelo de filtro es una de las decisiones más importantes dentro de cada preset.

### Zona 4 — Envelopes

Ubicada en la **parte media derecha** de la interfaz. Contiene dos envolventes ADSR una junto a la otra:

- La envolvente **AMP** (izquierda), que controla la amplitud de cada nota.
- La envolvente **FILTER** (derecha), que controla la evolución temporal del *filter cutoff*.

Cada envolvente posee cuatro faders verticales:

- **A** (*Attack*)
- **D** (*Decay*)
- **S** (*Sustain*)
- **R** (*Release*)

### Zona 5 — Modulation

Ubicada en la **parte inferior izquierda** de la interfaz. Contiene:

- **LFO RATE** — velocidad del LFO global.
- **VCF MOD** — cantidad de modulación del LFO aplicada al *filter cutoff*.

El LFO también puede modular el pitch (vibrato) y el ancho de pulso (PWM) mediante el knob **PWM/VIB** en la zona *Performance*. Esto significa que el LFO es compartido globalmente dentro del instrumento, pero sus destinos se controlan de forma independiente.

### Zona 6 — Master

Esta zona agrupa los dos controles globales del instrumento: selección de presets y nivel final de salida. Por equilibrio visual, ambos controles están ubicados en distintas posiciones de la interfaz:

- El selector desplegable **PRESET** — ubicado en la parte inferior central, justo debajo del logo Andes JX.
- El knob **OUTPUT** — ubicado en la esquina inferior derecha.

Esta zona es donde guardas y recuperas sonidos, y donde defines el nivel final de salida antes de que el audio abandone el plugin.

### Lectura de valores

Todos los knobs de Andes JX muestran su valor actual, ya sea dentro del knob (los cuatro knobs principales: **MIX**, **CUTOFF**, **RESO** y **OUTPUT**) o mediante una pequeña etiqueta ubicada sobre el control (knobs secundarios). Se utilizan tres convenciones de display, según el parámetro:

**Knobs que muestran una unidad física real:**

- **Decibeles** (`dB`) para el nivel de OUTPUT.
- **Semitonos** (`st`) para parámetros de afinación (tune del oscilador, octava).
- **Cents** (`c`) para afinación fina.
- **Hertz** (`Hz`) para la velocidad del LFO.
- **Nombres de nota** (`C4`, `A#3`, etc.) para el *filter key center*.

**Knobs cuyo valor es naturalmente un porcentaje** porque controlan una cantidad normalizada sin otra unidad subyacente: NOISE, MIX, RESONANCIA, niveles de sustain y los knobs bipolares de cantidad (ENV AMT, VEL FLTR, PWM/VIB, VCF MOD, KEY TRCK).

**Knobs expuestos como porcentaje pero que internamente se mapean a un rango no lineal** (frecuencia logarítmica o tiempo exponencial): CUTOFF, GLIDE RATE y todas las etapas de envolvente (A/D/R). En estos, el porcentaje muestra la *posición del knob*, no el valor físico subyacente. El rango real y el mapeo están documentados parámetro por parámetro en la sección 5 de este manual — por ejemplo, CUTOFF muestra `75 %` en la GUI pero se mapea a aproximadamente `5 kHz` (ver sección 5.2).

Los parámetros bipolares muestran siempre su signo (`+25 %`, `-50 %`) para que puedas identificar inmediatamente de qué lado del cero se encuentra el valor.

> 🎓 **Por qué algunos valores utilizan espacios de alineación.** Al mover un knob, notarás que valores de un solo dígito como `+5 %` aparecen con un espacio adicional a la izquierda para alinearse visualmente con valores de dos dígitos como `+50 %`. Esto es intencional: los números permanecen alineados dentro de la interfaz en lugar de “saltar” lateralmente mientras el parámetro cambia. Es un pequeño detalle tipográfico que hace que la GUI se sienta más sólida y pulida durante movimientos rápidos.

## 5. Motor de síntesis

Esta sección es la referencia técnica central del manual. Recorre cada parámetro de Andes JX, zona por zona, incluyendo rangos exactos, valores por defecto y descripciones detalladas de lo que hace cada control. Si estás leyendo el manual de principio a fin, este es el punto donde conviene bajar el ritmo.

Para cada parámetro, el manual incluye:

- Una línea compacta de **datos técnicos** (rango, mapeo, valor por defecto) como referencia rápida.
- Una **descripción narrativa** de qué hace el control y cómo afecta al sonido.
- Cuando es útil, un **tip práctico** marcado con 💡 para aprovechar mejor el parámetro.
- Cuando corresponde, un **bloque educativo** marcado con 🎓 que explica la decisión de diseño o DSP detrás del control.

> 📝 **Sobre los valores por defecto mostrados en este manual.** Los valores por defecto listados para cada parámetro corresponden al preset **Init** de Andes JX 1.0 — el estado con el que el plugin se carga al crear una nueva instancia. Estos valores fueron elegidos para producir un sonido inmediato y con carácter (dos *saws* separadas por una octava, mezcladas equitativamente y procesadas mediante el filtro Moog con un barrido percusivo de envolvente), en lugar de un estado completamente “vacío”. La idea es que un usuario nuevo pueda presionar una tecla y escuchar algo musical desde el primer momento.

---

### 5.1 Oscillators

La sección de osciladores es la fuente sonora de Andes JX. Dos osciladores (Osc 1 y Osc 2) generan las waveforms principales; un generador de ruido y un control de apertura estéreo completan la paleta sonora base. Todo en esta sección pasa primero por el mezclador y luego hacia el filtro.

### Controles por oscilador

#### Osc 1 — Selector de waveform

`Opciones: Sine · Saw · Square · Triangle · PWM` · `Por defecto: Saw`

Selecciona la waveform generada por Oscillator 1. Cada waveform posee un carácter armónico distinto:

- **Sine** — tono puro, sin armónicos. Redondo y limpio. Útil como capa base o para sub-bass.
- **Saw** — rica en armónicos. La waveform clásica de la síntesis sustractiva; funciona especialmente bien con el filtro.
- **Square** — solo armónicos impares, con carácter hueco. Funciona bien para sonidos tipo woodwind o timbres retro de videojuegos.
- **Triangle** — armónicos impares que decaen rápidamente. Más suave que *square*, con más cuerpo que *sine*.
- **PWM** — onda de pulso con modulación de ancho de pulso. Produce un carácter grueso y animado cuando el LFO modula el ancho de pulso (ver knob PWM/VIB en la sección 5.5).

> 🎓 **Sobre el anti-aliasing PolyBLEP.** Todas las waveforms no sinusoidales de Andes JX utilizan **PolyBLEP** (*Polynomial Band-Limited Step*) para suprimir artefactos de aliasing. En un sintetizador digital, generar ingenuamente una *saw* o una *square* produce parciales de alta frecuencia que se repliegan hacia el rango audible. PolyBLEP es una técnica desarrollada por Välimäki y Huovilainen (2007) que suaviza las discontinuidades de estas waveforms, manteniéndolas limpias incluso en frecuencias altas. Esta es una de las razones por las que las waveforms de Andes JX suenan pulidas en lugar de agresivas o ásperas.

#### Osc 2 — Selector de waveform

`Opciones: Sine · Saw · Square · Triangle · PWM` · `Por defecto: Saw`

Las mismas opciones que Osc 1. Ambos osciladores pueden utilizar la misma waveform (para sonidos tipo unison más densos) o waveforms distintas (para mezclas armónicas más ricas). Una combinación clásica: *Saw* en Osc 1 + *Square* en Osc 2 para un *lead* grueso.

#### Osc 2 — COARSE

`Rango: -24 a +24 semitonos` · `Paso: 1` · `Por defecto: -12` · `Bipolar`

Desplazamiento de afinación gruesa de Osc 2 respecto a Osc 1, medido en semitonos. El valor por defecto de `-12` coloca Osc 2 una octava por debajo de Osc 1, produciendo el sonido clásico y robusto de dos osciladores que define el preset Init.

Intervalos musicales comunes para probar:

- **+12** — Osc 2 una octava arriba de Osc 1. Añade brillo.
- **-12** — Osc 2 una octava abajo de Osc 1 (por defecto). Añade peso.
- **+7** — quinta justa superior. Muy útil para *leads*.
- **-5** — cuarta justa inferior. Funciona bien para sonidos retro tipo videojuegos.

> 💡 **Tip**: Andes JX muestra este valor incluyendo el signo (`+7 st`, `-12 st`) para que puedas identificar inmediatamente la dirección del desplazamiento.

#### Osc 2 — FINE

`Rango: -50 a +50 cents` · `Paso: 0.1` · `Skew: 0.3` · `Por defecto: 0` · `Bipolar`

Desplazamiento de afinación fina de Osc 2 respecto a Osc 1, medido en *cents* (1/100 de semitono). Se utiliza para crear pequeñas diferencias de afinación entre ambos osciladores, base fundamental del sonido sustractivo “grueso” y amplio.

> 💡 **Tip**: Ajusta FINE entre **+5 y +15 cents** para obtener la sensación clásica de desafinación analógica. Una ligera desafinación genera *beating* entre los osciladores, añadiendo movimiento y calidez. Valores por encima de 25 cents comienzan a percibirse claramente desafinados.

> 🎓 **Sobre el skew de este knob.** El knob FINE utiliza un **skew de 0.3** aplicado a su rango. En términos simples: el recorrido del knob se “expande” cerca del centro, de modo que pequeños movimientos producen cambios finos y controlables, mientras que movimientos más grandes hacia los extremos modifican el valor más rápidamente. Sin este skew, encontrar valores sutiles como `+8 cents` sería extremadamente difícil porque el knob saltaría en incrementos demasiado bruscos. La misma técnica de skew se utiliza en el knob GLIDE BEND (sección 5.4) por la misma razón: el control musical fino es más importante que una respuesta perfectamente uniforme en todo el rango.

#### Osc 2 — OCTAVE

`Rango: -2 a +2 octavas` · `Paso: 1` · `Por defecto: 0` · `Bipolar`

Desplazamiento de octava aplicado a Osc 2 además del ajuste COARSE. Útil cuando necesitas intervalos grandes (más allá de 24 semitonos) o cuando deseas reservar COARSE para ajustes interválicos más precisos mientras OCTAVE maneja los saltos amplios.

### Controles globales de esta zona

#### MIX

`Rango: 0–100 %` · `Por defecto: 50 %` · `Display: proporción osc1:osc2`

El knob central de la sección de osciladores. Mezcla entre Osc 1 (izquierda) y Osc 2 (derecha). En `0 %` escuchas únicamente Osc 1; en `100 %`, únicamente Osc 2; en `50 %`, ambos al mismo nivel. El display muestra la proporción real (`50:50`, `70:30`, `100:0`) para visualizar rápidamente el balance.

> 💡 **Tip**: En la mayoría de presets, mezclar ambos osciladores entre `40:60` y `60:40` produce el sonido más rico. Llevar la mezcla completamente hacia un oscilador (`0:100` o `100:0`) es útil cuando una waveform ya posee suficiente carácter por sí sola.

#### WIDTH

`Rango: 0–100 %` · `Paso: 1` · `Por defecto: 50 %`

Control global de apertura estéreo. En `0 %`, las voces permanecen centradas en mono. A medida que aumentas WIDTH, las notas graves permanecen cerca del centro mientras las notas agudas se abren progresivamente en el campo estéreo, produciendo una distribución similar a la de un piano.

El valor por defecto de `50 %` le da a Andes JX una sensación espacial inmediata sin exagerar la imagen estéreo.

> 💡 **Tip**: Para pads y texturas amplias, un WIDTH entre `40–70 %` suele funcionar bien. Para basses, reduce WIDTH por debajo de `20 %` o llévalo a cero; las frecuencias graves suelen sonar más sólidas cuando permanecen cerca del centro.

#### NOISE

`Rango: 0–100 %` · `Paso: 1` · `Por defecto: 0 %`

Generador independiente de ruido blanco añadido a la mezcla. Añade una cualidad aireada cuando se utiliza sutilmente, o un ataque percusivo cuando se utiliza en niveles altos.

> 💡 **Tip**: Una pequeña cantidad de ruido (`5–15 %`) durante el ataque de un sonido, combinada con un decay rápido de envolvente, puede simular el componente de aire de un instrumento de viento o el clic de un martillo.

#### TUNE

`Rango: -100 a +100 cents` · `Paso: 0.1` · `Por defecto: 0` · `Bipolar`

Afinación global del instrumento en *cents*. Equivalente a un *master tune*. Útil para adaptar Andes JX a instrumentos afinados ligeramente fuera de A=440 Hz (por ejemplo, afinación orquestal A=442) o para realizar pequeñas transposiciones globales.

> 💡 **Tip**: Déjalo en `0` salvo que tengas una razón específica para reajustar la afinación global. Para desafinación musical entre osciladores, utiliza el knob FINE.

---

### 5.2 Filter

El filtro es donde vive gran parte del carácter de Andes JX. Dos modelos de filtro pueden seleccionarse en tiempo real, cada uno con una personalidad sonora claramente distinta. El cutoff y la resonancia moldean directamente el timbre; la cantidad de envolvente, el key tracking, el key center y la modulación por LFO animan el filtro a lo largo del tiempo y del teclado.

### TYPE — Selector de modelo de filtro

`Opciones: SVF · Moog` · `Por defecto: Moog`

Selecciona entre los dos modelos de filtro implementados en Andes JX:

- **SVF** (*State Variable Filter*) — filtro basado en la implementación *topology-preserving* de Andrew Simper. Limpio, transparente y predecible. La resonancia se comporta de manera suave en todo el rango de cutoff. Ideal para sonidos modernos y limpios donde el filtro debe moldear el timbre sin añadir demasiado carácter propio.

- **Moog** (*Moog Ladder*) — implementación digital no lineal del clásico filtro ladder Moog, basada en el diseño de Antti Huovilainen (2004). Añade calidez, saturación y un carácter analógico reconocible. La resonancia posee una personalidad más agresiva, especialmente en valores altos. Ideal para basses cálidos, leads agresivos y cualquier sonido donde el filtro forme parte del timbre y no solo del tono.

El preset Init comienza utilizando el **filtro Moog**, de modo que el primer contacto con Andes JX transmita inmediatamente su carácter analógico. Cambiar a SVF transforma el mismo preset en algo más limpio y moderno.

> 🎓 **¿Por qué dos filtros?** La mayoría de sintetizadores incluyen un único carácter de filtro. Andes JX incorpora dos porque producen colores sonoros genuinamente distintos, y elegir entre ellos forma parte del proceso creativo de cada preset. Algunos sonidos (como un pad moderno y limpio) funcionan mejor con el SVF; otros (como un bass grueso) necesitan el Moog. Cambiar el tipo de filtro sobre un mismo patch puede transformar completamente su personalidad. Vale la pena compararlos mientras diseñas un sonido.

### CUTOFF

`Rango: 0–100 %` · `Paso: 0.1` · `Mapea a: 80 Hz – 20 kHz logarítmico` · `Por defecto: 75 %`

El knob más importante de la sección Filter. Define la frecuencia de corte del filtro: el punto a partir del cual las frecuencias altas comienzan a ser atenuadas.

El mapeo es logarítmico de modo que movimientos iguales del knob produzcan cambios iguales en la brillantez *percibida*, sin importar en qué parte del recorrido te encuentres. Un movimiento pequeño cerca de la parte baja del rango se escucha como un paso del mismo tamaño que un movimiento mucho mayor en Hz cerca de la parte alta.

El valor por defecto de `75 %` mantiene el filtro parcialmente cerrado, dejando espacio para que la envolvente abra y cierre el cutoff de forma musical. Con el filtro completamente abierto (`100 %`), el efecto de la envolvente sobre el cutoff sería casi imperceptible.

> 💡 **Tip**: La mayor parte del rango musicalmente útil del filtro se encuentra entre `20–70 %`. Por debajo de `10 %`, el sonido se vuelve extremadamente oscuro o desaparece casi por completo; por encima de `90 %`, el filtro está prácticamente abierto y su efecto es mínimo.

> 🎓 **Sobre el mapeo logarítmico del cutoff.** La percepción humana de la altura es aproximadamente logarítmica: cada octava es una duplicación de la frecuencia, pero el oído escucha cada octava como un paso del mismo tamaño. El mapeo exponencial de CUTOFF refleja esta percepción — pasar del `25 %` al `50 %` del knob eleva el cutoff la misma cantidad de octavas que pasar del `50 %` al `75 %`. Sin este mapeo, la región más musical del filtro (unos cientos de Hz a unos pocos kHz, donde vive la mayor parte del timbre) quedaría aplastada en una porción mínima del recorrido del knob, y el extremo superior se sentiría inservible. La fórmula utilizada internamente es `Hz = 80 × 250^(position)`, donde `position` es el knob normalizado a `0–1`, cubriendo desde `80 Hz` en el extremo inferior hasta `20 kHz` en el superior. El mismo principio exponencial se aplica a LFO RATE y a los knobs de tiempo de las envolventes (ver sección 5.3) por la misma razón perceptual.

### RESO — Resonancia

`Rango: 0–100 %` · `Paso: 1` · `Por defecto: 15 %`

La resonancia (también llamada *Q* o *emphasis*) incrementa las frecuencias justo alrededor del punto de cutoff. En `0 %`, el filtro atenúa suavemente; a medida que aumentas la resonancia, aparece un pico alrededor de la frecuencia de corte, produciendo eventualmente el clásico carácter “wah” o resonante.

Los dos modelos de filtro responden de manera distinta a la resonancia:

- **SVF**: resonancia suave y controlada. Incluso en `100 %` se mantiene musical.
- **Moog**: resonancia más agresiva y con posibilidad de auto-oscilación en valores altos. Entre `90–100 %`, el filtro puede sostener un tono propio incluso sin señal de entrada.

El valor por defecto de `15 %` añade un poco de carácter al preset Init sin entrar todavía en un territorio claramente resonante.

> 💡 **Tip**: Para leads estilo acid, lleva la resonancia entre `70–90 %` utilizando el filtro Moog y modula el cutoff mediante la envolvente del filtro. Esa es la base del clásico sonido tipo 303.

### ENV AMT — Cantidad de envolvente del filtro

`Rango: -100 a +100 %` · `Paso: 0.1` · `Mapea a: ±48 semitonos (±4 octavas)` · `Por defecto: 50 %` · `Bipolar`

Define cuánto la envolvente del filtro (configurada en la sección 5.3) modula la frecuencia de cutoff a lo largo del tiempo. Este parámetro es lo que hace posibles los barridos de filtro. En sus valores extremos, la envolvente puede mover el cutoff hasta cuatro octavas hacia arriba o hacia abajo desde su posición de reposo.

- **Valores positivos** abren el filtro al tocar una nota y luego lo cierran mientras la envolvente decae.
- **Valores negativos** cierran el filtro durante el ataque y lo abren a medida que la envolvente decae; menos común, pero interesante para sonidos percusivos.
- **Cero** significa que la envolvente no afecta el cutoff (la envolvente sigue funcionando internamente, solo que sin efecto audible).

El valor por defecto de `+50 %` es el responsable del barrido característico del filtro en el preset Init, equivalente a una profundidad de modulación de aproximadamente dos octavas.

> 💡 **Tip**: Para plucks y basses, prueba ENV AMT entre `+30 y +60 %` con un decay rápido. Para pads, valores más bajos (`+10 a +20 %`) generan un movimiento sutil del filtro que añade vida sin resultar evidente.

> 🎓 **La modulación vive en el dominio de los semitonos.** ENV AMT no se mide en Hz de desplazamiento de cutoff, sino en **semitonos**. La forma de la envolvente mueve el cutoff hacia arriba o abajo en pasos de *altura*, no en pasos de *frecuencia*. Esto es coherente con el resto de la red de modulación de Andes JX: la modulación de LFO al cutoff (VCF MOD), la modulación por velocidad (VEL FLTR), el key tracking (KEY TRCK) y el aftertouch están todos expresados en semitonos, se suman entre sí y solo al final se convierten en Hz. La ventaja es consistencia musical: un barrido de envolvente "dos octavas hacia arriba" se siente igual estés con CUTOFF en `30 %` o en `70 %`, porque la operación ocurre en el dominio de la altura, donde el oído percibe pasos iguales.

### KEY TRCK — Keyboard tracking

`Rango: 0–200 %` · `Paso: 1` · `Por defecto: 100 %`

Define cuánto el cutoff sigue la altura de la nota tocada. Con key tracking en `0 %`, el cutoff permanece en la misma frecuencia sin importar qué tecla toques; con `100 %` (valor por defecto), el cutoff sube y baja exactamente un semitono por tecla, siguiendo el pitch de la nota.

Esto es fundamental para mantener un brillo relativo consistente a lo largo del teclado. Sin key tracking, las notas graves pueden sonar opacas (porque el cutoff queda demasiado alto respecto a su fundamental) y las notas agudas demasiado delgadas (porque el cutoff queda demasiado bajo respecto a sus armónicos).

> 💡 **Tip**: Para la mayoría de presets, un keytracking entre `30–100 %` produce una respuesta equilibrada. Para sonidos tipo acid donde quieres que el carácter del filtro permanezca fijo sin importar la nota, utiliza `0 %`. Para sonidos tipo campana, donde el brillo debe aumentar fuertemente con el pitch, prueba valores superiores a `100 %` acercándote al máximo de `200 %`.

### KEY CNTR — Centro de keyboard tracking

`Rango: MIDI 24–96` · `Paso: 1` · `Por defecto: 60 (C4)` · `Display: nombre de nota`

Define la nota de referencia para el keyboard tracking. En esta nota el cutoff no recibe desplazamiento; por debajo, el cutoff se reduce según la cantidad de keytracking; por encima, aumenta.

El rango cubre seis octavas de uso musical práctico (C1 a C7), más que suficiente para la mayoría de situaciones musicales. El valor por defecto de MIDI 60 (C4, “middle C”) es un punto de partida razonable para la mayoría de contextos.

> 💡 **Tip**: Hacer doble clic sobre el knob lo devuelve a **C4** (MIDI 60), el estándar de middle C. Si estás diseñando patches de bass, mover el centro hacia **C2** (MIDI 36) puede hacer que el keytracking se sienta más natural en las octavas graves.

### Resumen de modulación del filtro

El cutoff es el parámetro más modulado de Andes JX. Cinco fuentes distintas pueden afectarlo simultáneamente:

1. **Envolvente del filtro** (controlada aquí mediante ENV AMT y moldeada en la sección 5.3).
2. **Keyboard tracking** (controlado aquí mediante KEY TRCK y KEY CNTR).
3. **Velocidad MIDI** (controlada mediante VEL FLTR en la sección 5.5).
4. **LFO** (controlado mediante VCF MOD en la sección 5.4).
5. **Movimiento manual** del knob CUTOFF (o automatización MIDI CC desde el host).

Las cinco fuentes se suman para determinar el cutoff real en cada instante. Esta capacidad de modulación es una de las razones por las que los barridos de filtro en Andes JX se sienten vivos y dinámicos.

---

### 5.3 Envelopes

Andes JX posee dos envolventes ADSR: una para amplitud (que define cómo cada nota crece y desaparece) y otra para el filtro (que define cómo el cutoff cambia a lo largo del tiempo). Ambas envolventes tienen los mismos cuatro parámetros — Attack, Decay, Sustain y Release — pero afectan aspectos distintos del sonido y poseen **valores por defecto deliberadamente diferentes** en el preset Init.

### Qué significa ADSR

Si eres nuevo en envolventes ADSR, aquí va la versión corta:

- **Attack (A)** — cuánto tarda la envolvente en alcanzar su nivel máximo después de presionar una nota.
- **Decay (D)** — cuánto tarda la envolvente en caer desde el máximo hasta el nivel de sustain.
- **Sustain (S)** — el nivel que la envolvente mantiene mientras la nota sigue presionada.
- **Release (R)** — cuánto tarda la envolvente en caer a cero después de soltar la nota.

Attack, Decay y Release son parámetros de *tiempo*. Sustain es un parámetro de *nivel* (controla “cuánto”, no “cuánto tiempo”).

### La envolvente de amplitud (AMP)

La envolvente de amplitud controla el volumen de cada nota desde que se presiona hasta que desaparece. Siempre está activa (no puede deshabilitarse) y es la envolvente más fundamental de cualquier sintetizador.

#### A — Attack

`Rango: 0–100 %` · `Paso: 1` · `Por defecto: 0 %` · `Mapea a: ~0 ms–10 s`

Tiempo que tarda la amplitud en llegar a su nivel máximo después de presionar una nota.

- En `0 %`, el ataque es instantáneo (sonidos percusivos y definidos: basses, plucks, leads).
- Entre `30–50 %`, el ataque se vuelve audible como un *swell* (pads, strings).
- Por encima de `70 %`, el ataque es lo suficientemente lento como para funcionar como efecto de *fade in* por sí solo.

> 💡 **Tip**: Incluso pequeñas cantidades de attack (`2–5 %`) pueden suavizar el clic que algunas waveforms producen al inicio de la nota, haciendo que el sonido se perciba menos agresivo sin convertirse en un *swell* evidente.

#### D — Decay

`Rango: 0–100 %` · `Paso: 1` · `Por defecto: 50 %` · `Mapea a: ~0 ms–10 s`

Tiempo que tarda la amplitud en caer desde el nivel máximo hasta el nivel de sustain. Solo es audible si el sustain está por debajo de `100 %`.

#### S — Sustain

`Rango: 0–100 %` · `Paso: 1` · `Por defecto: 100 %` · `Nivel, no tiempo`

Nivel de amplitud que la envolvente mantiene mientras la nota permanece presionada. Con `100 %` (valor por defecto), la nota se reproduce a volumen completo mientras mantienes la tecla presionada — la etapa de decay no tiene efecto audible porque no existe un nivel inferior hacia el cual decaer. Con valores menores, la nota cae hacia ese nivel después de la etapa de decay.

Para sonidos percusivos (plucks, basses sin sustain), ajusta este parámetro a `0 %`; la nota desaparecerá completamente después del decay incluso si la tecla continúa presionada.

#### R — Release

`Rango: 0–100 %` · `Paso: 1` · `Por defecto: 30 %` · `Mapea a: ~0 ms–10 s`

Tiempo que tarda la nota en desaparecer después de soltar la tecla. Releases largos generan superposición entre notas (útil para pads); releases cortos detienen el sonido casi inmediatamente (útil para leads staccato o basses definidos).

> 💡 **Tip**: En interpretación polifónica, ten cuidado con releases muy largos combinados con alta polifonía: las colas de notas superpuestas pueden volver una progresión de acordes confusa o embarrada rápidamente. Reduce el release o utiliza el modo Mono (sección 5.5) para obtener un resultado más limpio.

### La envolvente del filtro (FILTER)

La envolvente del filtro posee la misma estructura A/D/S/R que la envolvente de amplitud, pero controla el *filter cutoff* en lugar de la amplitud. La cantidad con la que afecta el cutoff está determinada por el knob **ENV AMT** en la sección 5.2.

Si ENV AMT está en `0 %`, la envolvente continúa funcionando internamente, pero no produce ningún efecto audible. A medida que aumentas ENV AMT, la modulación sobre el cutoff se vuelve más evidente.

Los cuatro parámetros funcionan de la misma forma que en la envolvente de amplitud:

- **A — Attack**: `Rango: 0–100 %` · `Por defecto: 0 %`
- **D — Decay**: `Rango: 0–100 %` · `Por defecto: 30 %`
- **S — Sustain**: `Rango: 0–100 %` · `Por defecto: 0 %`
- **R — Release**: `Rango: 0–100 %` · `Por defecto: 25 %`

> 🎓 **Por qué las dos envolventes tienen valores por defecto diferentes.** Los valores por defecto de la envolvente de amplitud (`A=0, D=50, S=100, R=30`) hacen que la nota permanezca sostenida a volumen completo mientras la tecla esté presionada y luego desaparezca suavemente al soltarla. Los valores de la envolvente del filtro (`A=0, D=30, S=0, R=25`) hacen que el cutoff se abra rápidamente al inicio de la nota y luego decaiga velozmente hacia cero, independientemente de si la tecla sigue presionada. Combinadas, ambas producen el clásico carácter de **filtered pluck** del preset Init: un barrido percusivo de filtro en cada ataque mientras la amplitud continúa sostenida. Esta asimetría intencional entre ambas envolventes es la base de innumerables sonidos de leads, basses y plucks en la historia de la síntesis sustractiva.

### Curvas de envolvente en Andes JX

Las envolventes de Andes JX utilizan **curvas estilo analógico** en lugar de rampas perfectamente lineales. Esto significa:

- Las curvas de attack son exponenciales (suben rápidamente y luego se desaceleran al acercarse al máximo).
- Las curvas de decay y release también son exponenciales (caen rápidamente y luego desaceleran al acercarse al destino).

Esta forma no lineal imita el comportamiento de los generadores de envolvente analógicos y produce una sensación más musical y orgánica que las envolventes matemáticamente lineales.

El rango de tiempo cubierto por cada knob de etapa abarca **aproximadamente desde 4 ms (knob en 0) hasta 7.4 segundos (knob en 100)**. El mapeo es exponencial, igual que en CUTOFF y LFO RATE: movimientos iguales del knob producen cambios iguales en el tiempo *percibido*, aunque los milisegundos subyacentes cambien por un factor mucho mayor en la parte alta. La zona central del knob (alrededor del `50 %`) corresponde a aproximadamente `170 ms`, una duración típica de "audible pero no lenta" para una etapa de envolvente. El mismo mapeo se comparte entre la envolvente de amplitud y la envolvente del filtro.

> 💡 **Tip**: Debido a su naturaleza exponencial, tiempos muy cortos de envolvente (por debajo de `10 %`) producen respuestas rápidas y agresivas. Tiempos largos (por encima de `70 %`) generan cambios lentos y graduales. La zona media (`30–60 %`) cubre la mayoría de escenarios musicales.

> 🎓 **Dos capas de exponencial.** Las envolventes de Andes JX son exponenciales de dos maneras distintas, y vale la pena distinguirlas. Primero, el *knob de tiempo* es exponencial: pasos iguales del knob dan pasos iguales multiplicativos en tiempo (10 ms → 100 ms → 1 s se siente como incrementos iguales). Segundo, *la forma de la envolvente misma* es exponencial: una vez que arranca una etapa, el nivel se aproxima a su destino geométricamente, como un capacitor cargándose o descargándose en un circuito analógico. Ambas decisiones son deliberadas. El knob exponencial cubre el enorme rango dinámico desde plucks rápidos hasta pads lentos sin zonas muertas. La forma exponencial imita el hardware analógico y, en el caso de la envolvente de amplitud, coincide con nuestra percepción logarítmica de la sonoridad — una caída exponencial en amplitud se *escucha* como un fade suave y lineal en volumen.

### 5.4 Modulation

La sección de modulación de Andes JX gira alrededor de un único LFO global (*Low-Frequency Oscillator*) compartido entre todas las voces. Aunque el LFO es una sola fuente de modulación, su salida puede enviarse hacia **tres destinos distintos** mediante tres controles independientes:

1. **Pitch** — vibrato, controlado por el lado positivo del knob PWM/VIB (en la sección Performance, 5.5).
2. **Pulse width** — modulación PWM, controlada por el lado negativo del knob PWM/VIB (también en la sección 5.5).
3. **Filter cutoff** — controlado mediante VCF MOD, en esta sección.

La zona Modulation de la interfaz expone directamente solo dos de estos controles: **LFO RATE** (la velocidad del LFO) y **VCF MOD** (la cantidad de modulación aplicada al filtro). Los destinos de pitch y PWM se encuentran en la sección Performance porque están estrechamente relacionados con la interpretación.

### LFO RATE

`Rango: 0–1 normalizado` · `Mapea a: ~0.018 Hz–20 Hz exponencial` · `Por defecto: 0.81 (~7 Hz)`

Define la velocidad del LFO global. El display muestra la frecuencia real en Hz con tres decimales de precisión, para que siempre puedas saber exactamente qué tan rápido está funcionando el LFO.

El mapeo es exponencial, de forma similar al knob CUTOFF: pequeños movimientos en la parte baja del rango producen cambios pequeños en Hz, mientras que el mismo movimiento en la parte alta genera cambios mucho mayores. Esto permite un control fino sobre velocidades lentas (donde cada fracción de Hz importa) sin perder acceso a velocidades rápidas en la parte superior del rango.

El valor por defecto de `0.81` produce aproximadamente `7 Hz`, una velocidad ubicada dentro del rango natural de vibrato utilizado por instrumentos de cuerda y cantantes.

> 💡 **Tip**: Rangos útiles según el propósito musical:
> - **Movimiento lento de filtro en pads**: `0.1–1 Hz` (posición del knob alrededor de `0.4–0.6`).
> - **Vibrato natural**: `5–7 Hz` (posición alrededor de `0.75–0.85`).
> - **Tremolo rápido / efectos tipo shimmer**: `10–20 Hz` (posición por encima de `0.9`).

> 🎓 **Sobre el mapeo exponencial de velocidad.** La percepción humana de la “velocidad” en música es aproximadamente exponencial: duplicar una velocidad de 1 Hz a 2 Hz se percibe como un cambio similar a duplicarla de 8 Hz a 16 Hz, aunque la diferencia absoluta en Hz sea mucho mayor en el segundo caso. El mapeo exponencial de Andes JX sigue esta percepción, de modo que movimientos equivalentes del knob producen cambios percibidos similares sin importar la posición del control. Internamente, la fórmula utilizada es `freq = exp(7 × position − 4)`, cubriendo aproximadamente desde `0.018 Hz` hasta `20 Hz`.

### VCF MOD — Cantidad de LFO sobre el filtro

`Rango: 0–100 %` · `Paso: 1` · `Por defecto: 0 %`

Define cuánto el LFO modula el *filter cutoff*. En `0 %` (valor por defecto), el LFO no afecta al filtro; en valores más altos, el cutoff oscila alrededor de su posición actual siguiendo la velocidad definida por LFO RATE.

> 💡 **Tip**: Una modulación sutil del filtro (`5–15 %`) añade movimiento orgánico a pads y sonidos sostenidos sin convertirse en un efecto evidente. Valores más altos (`30–60 %`) producen el clásico “filter wobble” utilizado en dub, trip-hop y texturas electrónicas.

### Sobre la waveform del LFO

El LFO de Andes JX utiliza una **waveform sinusoidal** (*sine*). Esta elección se hizo por su suavidad y musicalidad: la modulación sinusoidal se percibe orgánica en vibrato, suave en PWM y natural en barridos de filtro. La versión 1.0 no incluye selector de waveform para el LFO.

---

### 5.5 Performance

La zona Performance agrupa los controles que definen cómo Andes JX responde a la interpretación: transición entre notas, modo de voces, modulación expresiva proveniente del LFO y la relación entre velocidad MIDI y filtro.

### GLIDE — Selector de modo de glide

`Opciones: Off · Legato · Always` · `Por defecto: Off`

Define cuándo se aplica glide (portamento) entre notas:

- **Off** (por defecto) — sin glide. Cada nota se reproduce en su pitch exacto.
- **Legato** — el glide solo se aplica cuando las notas se superponen (interpretación legato). Las notas separadas no tienen glide. Este comportamiento coincide con el de muchos sintetizadores monofónicos clásicos y suele sentirse como la opción más musical.
- **Always** — el glide se aplica entre todas las notas, independientemente de si se superponen o no.

### GLIDE RATE

`Rango: 0–100 %` · `Paso: 1` · `Por defecto: 35 %`

Define la velocidad del glide entre dos notas. En `0 %`, el glide es prácticamente instantáneo (equivalente a no tener glide); en valores altos, la transición se vuelve más lenta.

El glide se calcula en el **dominio logarítmico de la frecuencia** (en semitonos), no en Hz. De ahí se desprenden dos consecuencias importantes:

- Un deslizamiento de una octava y uno de un semitono toman un tiempo proporcional a GLIDE RATE — se *sienten a la misma velocidad* independientemente del tamaño del intervalo.
- Subir y bajar a través del mismo intervalo musical son perfectamente simétricos en duración y trayectoria.

Esto coincide con la manera en que una voz humana o un instrumento sin trastes se mueve entre notas, y es la razón por la cual el glide de Andes JX se siente musicalmente natural a lo largo de todo el teclado.

> 💡 **Tip**: Para líneas musicales de bass y lead, valores entre `20–50 %` suelen funcionar bien. Por encima de `70 %`, el glide se vuelve muy lento y comienza a sentirse más como un efecto de deslizamiento exagerado que como una transición natural entre notas.

> 🎓 **Por qué importa el glide en dominio logarítmico.** Una implementación ingenua del glide suaviza la *frecuencia en Hz* con un filtro pasa-bajos simple. Sobre el papel se ve correcta, pero suena mal en un instrumento: como la percepción de altura es logarítmica, deslizar de `100 Hz` a `200 Hz` (una octava) recorre la misma distancia percibida que deslizar de `1000 Hz` a aproximadamente `1100 Hz` (apenas ~1.6 semitonos), pero un filtro en Hz tomaría el mismo tiempo para ambos. El resultado es un portamento que *se apresura* en el registro grave y *se arrastra* en el agudo, y que se siente asimétrico según vaya hacia arriba o hacia abajo. Suavizar en cambio el *logaritmo* de la frecuencia — equivalentemente, suavizar en semitonos — produce un glide donde tiempo igual significa recorrido en altura igual, en ambas direcciones. Es una de las pequeñas pero definitorias diferencias entre un portamento que "se siente bien" y uno que "se siente raro".

### GLIDE BEND

`Rango: -36 a +36 semitonos` · `Paso: 0.01` · `Skew: 0.4` · `Por defecto: 0` · `Bipolar`

Define qué tan arriba o abajo de la nota objetivo comienza el glide. En `0` (valor por defecto), el glide realiza una transición suave entre la nota anterior y la nueva nota — el comportamiento más común. Con valores positivos, el glide sobrepasa la nota objetivo hacia arriba y luego cae. Con valores negativos, cae por debajo de la nota objetivo antes de subir.

> 💡 **Tip**: Valores entre `-2 y -5 semitonos` producen un efecto tipo *scoop* que recuerda la manera en que un cantante o saxofonista se aproxima a una nota desde abajo. Valores entre `+12 y +24` generan glides descendentes dramáticos útiles para transiciones y *risers*.

### VOICE — Modo de polifonía

`Opciones: Mono · Poly` · `Por defecto: Poly`

Cambia entre reproducción monofónica y polifónica:

- **Poly** (por defecto) — reproducción polifónica, con hasta 16 voces simultáneas. Varias notas pueden sonar al mismo tiempo. Es el modo estándar para acordes, pads y capas armónicas.
- **Mono** — reproducción monofónica. Solo una nota puede sonar a la vez. Las nuevas notas interrumpen las anteriores. Fundamental para líneas tradicionales de bass, lead y acid donde la superposición de notas puede ensuciar el resultado.

> 💡 **Tip**: El modo Mono combina especialmente bien con glide en modo Legato. Ajustar GLIDE en **Legato** y VOICE en **Mono** produce el comportamiento clásico de un sintetizador monofónico con transiciones suaves entre notas superpuestas.

### PWM/VIB — Modulación bipolar de pitch y PWM

`Rango: -100 a +100 %` · `Paso: 0.1` · `Por defecto: 0 %` · `Bipolar`

Este es uno de los controles más distintivos de Andes JX: un **único knob bipolar con dos funciones diferentes** dependiendo del lado del cero en el que te encuentres.

- **Valores positivos (`+1 a +100`)** — aplican modulación del LFO sobre el pitch. Esto produce **vibrato**, cuya profundidad depende del valor del knob. Valores altos generan un vibrato más amplio.
- **Valores negativos (`-1 a -100`)** — aplican modulación del LFO sobre el ancho de pulso. Esto produce **PWM** sobre las waveforms PWM de los osciladores, controlando la profundidad mediante el valor absoluto del knob.
- **Cero** — ni vibrato ni PWM están activos.

El display refleja este comportamiento: valores positivos muestran simplemente la profundidad (`+45.0`), mientras que valores negativos muestran la profundidad etiquetada como PWM (`PWM 30.0`).

> 🎓 **¿Por qué un solo knob con dos funciones?** Vibrato y PWM son dos destinos de modulación distintos provenientes del mismo LFO. Muchos sintetizadores los presentan como controles separados, lo que suele provocar que los usuarios utilicen solo uno y nunca exploren el otro. Andes JX los combina en un único knob con una diferenciación visual clara (signo y etiqueta), ahorrando espacio en el panel e incentivando la exploración de ambos comportamientos. Esta es también una de las razones por las que el LFO es global en lugar de tener un LFO independiente para cada destino: un solo LFO, tres destinos, tres controles de cantidad.

> 💡 **Tip**: Para vibrato natural, ajusta este knob entre `+15 y +30 %`. Para vibrato evidente (tipo lead Moog), entre `+50 y +80 %`. Para PWM, selecciona primero la waveform PWM en el oscilador y luego ajusta el knob entre `-30 y -60 %` para escuchar el clásico efecto grueso y animado de modulación de pulso.

### VEL FLTR — Cantidad de velocidad MIDI sobre el filtro

`Rango: -100 a +100 %` · `Paso: 1` · `Por defecto: 0 %` · `Bipolar` · `Estado especial: OFF`

Define cómo la velocidad MIDI (la intensidad con la que presionas una tecla) afecta el *filter cutoff*:

- **Valores positivos** — notas tocadas con mayor intensidad abren más el filtro. Las notas suaves suenan más oscuras; las fuertes, más brillantes. Es la configuración más musical y expresiva.
- **Valores negativos** — notas fuertes cierran más el filtro. Menos común, pero útil para dinámicas invertidas.
- **Cero** — la velocidad MIDI no afecta el cutoff.
- **OFF** (estado especial mostrado cuando el knob baja de `-90`) — el seguimiento de velocidad se desactiva completamente a nivel interno. A nivel audible, produce el mismo resultado que `0 %`, pero internamente omite el cálculo correspondiente.

> 💡 **Tip**: Para líneas expresivas de lead y bass, prueba valores entre `+30 y +60 %`. El filtro responderá a tu dinámica de interpretación, haciendo que las notas suaves suenen cálidas y las notas fuertes más brillantes, de forma similar al comportamiento de instrumentos acústicos.

> 🎓 **¿Por qué OFF existe como estado separado?** Internamente, el cálculo de velocity tracking del filtro tiene un pequeño costo computacional. Cuando VEL FLTR está en `0`, el cálculo sigue ejecutándose (simplemente multiplicado por cero). Cuando el parámetro se encuentra en `OFF` (por debajo de `-90`), el cálculo se omite completamente. El resultado audible es idéntico, pero en sistemas reproduciendo muchas voces simultáneamente, los ciclos de CPU ahorrados pueden importar. El umbral OFF está documentado en `PluginEditor.cpp` y `PluginProcessor.cpp` como un contrato entre la GUI y el hilo de audio.

---

### 5.6 Master

La zona Master agrupa los dos controles globales del instrumento: selección de presets y nivel final de salida. Por equilibrio visual, ambos controles están ubicados en diferentes posiciones dentro de la interfaz.

### Selector PRESET

Ubicado en la parte inferior central de la interfaz, justo debajo del logo Andes JX. El menú desplegable PRESET se documenta en detalle en la sección 5.7 (*Sistema de presets*).

### OUTPUT

`Rango: -24 a +6 dB` · `Paso: 0.1` · `Por defecto: 0 dB`

Define el nivel general de salida de Andes JX en decibeles (dB). Esta es la etapa final antes de que el audio salga del plugin hacia tu DAW.

El rango cubre desde `-24 dB` (atenuación considerable, útil para colocar Andes JX discretamente debajo de otros instrumentos) hasta `+6 dB` (ganancia adicional, útil cuando un patch suena más bajo que el resto del proyecto).

El valor por defecto de `0 dB` corresponde a **unity gain**: Andes JX entrega exactamente el mismo nivel que produce internamente el motor de síntesis, sin amplificación ni atenuación adicional.

Internamente, la etapa de salida incorpora una función de **soft limiting basada en una curva sigmoidal**. A niveles moderados el comportamiento es prácticamente transparente, pero al acercarse y superar `0 dB` comienza a introducir una saturación suave con enriquecimiento de armónicos impares. El resultado es un carácter más cálido y denso, similar al comportamiento de circuitos analógicos o etapas valvulares empujadas suavemente.

Esto significa que OUTPUT no funciona únicamente como un control de volumen: también puede utilizarse como una forma sutil de coloración tonal.

> 💡 **Tip**: Si un patch está saturando en tu DAW, reduce el knob OUTPUT entre `3–6 dB` en lugar de bajar el fader del canal. Esto mantiene intacta la estructura general de mezcla y te da un punto de referencia consistente. Si buscas un carácter más agresivo o cálido, puedes empujar OUTPUT hacia valores positivos; la saturación sigmoidal añadirá densidad armónica sin el clipping duro típico de una saturación digital abrupta.

### Sobre el display del knob OUTPUT

OUTPUT es uno de los cuatro knobs “principales” de Andes JX (junto con MIX, CUTOFF y RESO). Utiliza el diseño visual de **dos líneas**: el valor numérico arriba (por ejemplo `0.0`) y la unidad `dB` debajo. Los otros tres knobs principales utilizan displays más compactos que priorizan el valor sobre la unidad. Esta diferencia visual refuerza el rol de OUTPUT como control maestro del instrumento: está diseñado para destacar.

---

### 5.7 Sistema de presets

Andes JX incluye **33 presets de fábrica** organizados en cinco familias sonoras: **Bass**, **Pad**, **Lead**, **Brass / Wind / Organ** y **Keys / Pluck / FX**. El primer preset del banco es **Init**, el punto de partida neutro documentado a lo largo de las secciones anteriores de este manual. Los 32 restantes son ejemplos de *sound design* diseñados para mostrar el rango sonoro del instrumento.

El catálogo completo de presets, junto con la descripción individual de cada uno, se documenta en la sección 9 (*Banco de presets*) de este manual.

### Menú desplegable PRESET

Ubicado en la **parte inferior central de la interfaz**, justo debajo del logo Andes JX. Al hacer clic, se abre un menú con los 33 presets de fábrica, seguido de un separador y una última entrada llamada **Custom** (id 1000).

Para cargar un preset, simplemente selecciónalo desde la lista. El estado completo del sintetizador cambiará inmediatamente para reflejar los parámetros del preset elegido.

### El estado "Custom"

La entrada **Custom** se comporta de manera diferente a los presets de fábrica:

- **No puede seleccionarse directamente** haciendo clic sobre ella. El menú ignora los clics sobre Custom porque no funciona como un preset tradicional, sino como un marcador de estado.
- **Custom se activa automáticamente** en el momento en que modificas cualquier parámetro mientras un preset de fábrica está cargado. El menú cambia de mostrar el nombre del preset a mostrar “Custom”, indicando que el estado actual ya no coincide exactamente con ningún preset guardado.
- **Volver a cargar un preset de fábrica** elimina el estado Custom y el menú vuelve a mostrar el nombre del preset correspondiente.

> 🎓 **¿Por qué existe Custom?** Sin este estado, el menú estaría “mintiendo” al usuario. Imagina cargar `Cotopaxi Acid` y luego mover el cutoff a la mitad. El sonido ya no es exactamente `Cotopaxi Acid`, pero el menú seguiría mostrando ese nombre. El estado Custom mantiene la interfaz honesta: tan pronto como te alejas de un preset guardado, el nombre cambia a “Custom” para indicar que estás explorando un estado nuevo. Recargar el preset original (o cualquier otro) desactiva Custom.

### Cargar y guardar presets desde tu DAW

Andes JX no implementa un sistema propio de guardado/carga de presets en la versión 1.0. Para guardar un patch personalizado:

1. Utiliza el sistema de presets de plugins de tu DAW (la mayoría de DAWs incluyen un menú o botón en la parte superior de la ventana del plugin para guardar y cargar el estado completo del instrumento).
2. O simplemente guarda el proyecto completo del DAW; el estado del plugin quedará almacenado junto con el proyecto.

Tu DAW guardará el estado completo del plugin (los 32 parámetros), y Andes JX restaurará correctamente ese estado la próxima vez que abras el preset o proyecto.

### Persistencia del estado Custom entre sesiones

Un detalle sutil pero útil: cuando tu DAW guarda un proyecto, Andes JX serializa **tres cosas simultáneamente**: los valores de los 32 parámetros, el índice del preset actualmente seleccionado y el estado de Custom. Esto significa que:

- Si cargas un preset de fábrica, modificas un knob y el menú cambia a **Custom**, ese sonido modificado quedará guardado como Custom dentro del proyecto.
- La próxima vez que abras el proyecto, Andes JX restaurará exactamente los mismos parámetros y el menú seguirá mostrando **Custom**, indicando que el estado sigue siendo una variación de un preset existente y no un preset de fábrica nuevo.
- Internamente, el plugin verifica que el estado restaurado coincida realmente con lo que fue guardado; si por cualquier motivo los parámetros difieren del preset original, el estado Custom se reactiva automáticamente para mantener coherencia entre el sonido y la interfaz.

En la práctica, esto significa que no perderás trabajo entre sesiones. Un patch que dejaste en estado Custom dentro de un proyecto reaparecerá exactamente igual al volver a abrir el DAW. La única situación donde el estado Custom se pierde es cuando **cierras la instancia del plugin sin guardar el proyecto**; en ese caso, las modificaciones son volátiles y desaparecen cuando la instancia del plugin se descarga.

> 💡 **Tip**: Cuando diseñes sonidos que quieras conservar, es buena práctica guardarlos inmediatamente como presets del DAW, incluso si todavía no están “terminados”. Así podrás reutilizarlos desde cualquier proyecto y no solo desde la sesión actual.

### Organización de presets en el menú

La lista de presets sigue el orden definido en el código fuente. El primer preset siempre es **Init**, seguido de los 32 presets organizados por familia:

```text
Init
─────────────
Bass — Cotacachi Ostinato
Bass — Imbabura Pedal
Bass — Chimborazo Sub
…
─────────────
Pad — Cayambe 5th
Pad — Páramo Sostenuto
…
─────────────
Lead — Cotopaxi Acid
…
(y así sucesivamente con Brass, Wind, Organ, Keys, Pluck y FX)
─────────────
Custom (visible únicamente cuando el estado difiere de cualquier preset de fábrica)
```

Para el catalogo completo con descripciones, revisar la sección 9.

## 6. Tutoriales de sound design

Esta sección contiene tres tutoriales paso a paso que recrean tres presets de fábrica desde cero, comenzando siempre desde el estado **Init**. El objetivo no es simplemente cargar un preset — eso ya puedes hacerlo desde el menú desplegable — sino entender **cómo se construye cada sonido**, parámetro por parámetro, para que puedas aplicar las mismas técnicas en tus propios diseños.

Los tres tutoriales cubren tres familias sonoras distintas:

1. **Bass** — *Cotacachi Ostinato*, un bass monofónico tipo Moog con una envolvente de filtro marcada.
2. **Pad** — *Cayambe 5th*, un pad polifónico amplio con dos osciladores separados por una quinta.
3. **Lead** — *Cotopaxi Acid*, un lead estilo acid polifónico con el filtro Moog llevado al límite.

Para cada tutorial:

- Comienza desde el preset **Init** (cárgalo desde el menú PRESET).
- Sigue los pasos en orden.
- Lee la prueba de escucha para verificar que llegaste al resultado esperado.
- Prueba las variaciones para ampliar tu comprensión del sonido.

---

### 6.1 Tutorial 1 — Bass: Cotacachi Ostinato

### Qué vamos a construir

Un bass monofónico enfocado, con alta resonancia y una envolvente de filtro pronunciada en cada nota. El carácter es elástico y percusivo, pensado para patrones repetitivos de bajo (la palabra “ostinato” describe una figura musical que se repite persistentemente; el volcán Cotacachi se eleva con esa misma presencia sólida y recurrente sobre el horizonte de Imbabura).

### Paso 1 — Comenzar desde Init

Carga el preset **Init**. Escucharás dos *saws* separadas por una octava, un filtro Moog al 75 % y un barrido percusivo de envolvente. Vamos a transformar eso en un bass enfocado.

### Paso 2 — Configurar los osciladores para bass

Ajusta **MIX** a `100:0` (gira el knob completamente hacia la izquierda, valor `0`). Para este bass queremos únicamente Osc 1: una sola *saw* limpia. Mantén ambas waveforms en Saw.

Reinicia **COARSE** a `0` (offset de afinación de Osc 2). Aunque Osc 2 esté silenciado, mantenerlo afinado al unísono significa que, si más adelante aumentas MIX para experimentar, ambos osciladores no entrarán en conflicto.

Ajusta **OCTAVE** (en la zona global de Oscillators) a `-2`. Esto desplaza todo el instrumento dos octavas hacia abajo, colocando el rango de interpretación firmemente en territorio de bass.

Reduce **WIDTH** a `15 %`. Los basses funcionan mejor cuando permanecen cerca del centro; demasiado estéreo en frecuencias graves reduce definición en parlantes pequeños y sistemas mono compatibles.

### Paso 3 — Configurar el filtro

Cambia **TYPE** a **Moog** (debería ya estar en Moog desde Init, pero verifícalo). El modelo Moog le da a este bass su calidez analógica y el carácter saturado que aparece con resonancias altas.

Ajusta **CUTOFF** a `55 %`. Esta es la posición base del filtro cuando la envolvente no está actuando — más oscura que Init, porque queremos que el bass se sienta profundo y sólido.

Lleva **RESO** a `75 %`. Es un valor alto, cercano al territorio de auto-oscilación, y es lo que le da al bass su cualidad vocal y elástica.

Ajusta **ENV AMT** a `+38 %`. La envolvente del filtro abrirá el cutoff en cada ataque de nota y luego lo cerrará nuevamente, produciendo el clásico golpe percusivo del bass.

Reduce **KEY TRCK** a `45 %`. Queremos algo de keytracking para que las notas agudas mantengan definición, pero no un seguimiento completo — eso volvería demasiado brillante el rango alto.

### Paso 4 — Dar forma a la envolvente del filtro

La envolvente del filtro determina la forma del barrido de cutoff en cada nota. Ajusta:

- **A** (*Attack*) → `0` (instantáneo)
- **D** (*Decay*) → `48`
- **S** (*Sustain*) → `0` (el cutoff decae completamente)
- **R** (*Release*) → `18`

A=0 + S=0 significa que el filtro se abre instantáneamente en cada ataque y luego decae completamente, independientemente de cuánto tiempo mantengas la nota presionada. Esta es la base del carácter percusivo del bass.

### Paso 5 — Dar forma a la envolvente de amplitud

La envolvente de amplitud define cómo se comporta el volumen. Ajusta:

- **A** (*Attack*) → `0` (instantáneo)
- **D** (*Decay*) → `38`
- **S** (*Sustain*) → `70`
- **R** (*Release*) → `25`

A diferencia de la envolvente del filtro, la amplitud mantiene un sustain del 70 %. La nota continúa sonando mientras mantengas la tecla presionada; el decay simplemente reduce el nivel desde el máximo inicial hasta ese sustain.

### Paso 6 — Configurar el comportamiento interpretativo

Cambia **VOICE** a **Mono**. Las líneas de bass normalmente se interpretan una nota a la vez; el modo Mono evita superposición entre notas y mantiene claridad.

Ajusta **GLIDE** a **Legato** y **GLIDE RATE** a `49 %`. En modo legato, deslizarse de una nota a otra sin soltar la primera genera una transición suave de pitch — característica típica de líneas de bass expresivas.

Ajusta **GLIDE BEND** a `+1` semitono. Esto añade un pequeño sobrepaso ascendente al inicio de cada glide, un detalle sutil que hace que el bass se sienta más orgánico y menos mecánico.

### Paso 7 — Toque final

Reduce **LFO RATE** a aproximadamente `0.20` (~`0.6 Hz`). Aunque el LFO todavía no está ruteado a ningún destino (VCF MOD = `0`, PWM/VIB = `0`), mantenerlo en una velocidad lenta hace que cualquier experimento posterior de modulación parta desde una base musical y no desde una vibración excesivamente rápida.

### Prueba de escucha

Toca una nota grave en tu teclado (alrededor de C2). Deberías escuchar:

- Un ataque inmediato y percusivo con un barrido rápido de filtro.
- Un tono enfocado, ligeramente resonante y prácticamente centrado en mono.
- Una nota que se mantiene a volumen moderado después del ataque inicial.

Ahora toca dos notas en legato (presiona la segunda antes de soltar la primera): el pitch debería deslizarse suavemente entre ambas, con un pequeño sobrepaso apenas perceptible al inicio.

### Variaciones

- **Más agresividad**: lleva **RESO** hasta `90 %` para acercarte a la auto-oscilación.
- **Más movimiento**: aumenta **VCF MOD** a `15 %` para introducir un *filter wobble* lento.
- **Carácter diferente**: cambia **TYPE** a **SVF** para escuchar el mismo patch con un filtro más limpio y menos coloreado.
- **Wobble bass**: aumenta **GLIDE RATE** a `80 %` para obtener deslizamientos más lentos y evidentes.

> 🎓 **Sobre la resonancia en los dos modelos de filtro.** Aunque ambos filtros utilizan el mismo rango de resonancia, no reaccionan de la misma manera. El filtro Moog ladder introduce una atenuación más marcada en frecuencias altas y una saturación no lineal suave, produciendo un carácter más cálido y redondo incluso con resonancia elevada. El SVF, en cambio, conserva más contenido armónico en la parte alta del espectro, por lo que la resonancia se percibe más brillante, definida y “filuda”.  
>
> En la práctica, esto significa que un mismo valor de resonancia (`75 %`, por ejemplo) puede sonar mucho más agresivo y brillante en el SVF, mientras que el Moog mantiene un carácter más oscuro y denso. Cambiar entre ambos filtros no solo modifica el cutoff: cambia el balance armónico completo del sonido.

---

### 6.2 Tutorial 2 — Pad: Cayambe 5th

### Qué vamos a construir

Un pad polifónico amplio, con dos osciladores separados por una quinta justa, envolventes lentas y un movimiento suave de LFO. Cayambe es un nevado atravesado por la línea ecuatorial — el único lugar del planeta donde la nieve se encuentra exactamente sobre la latitud cero. Este patch intenta capturar esa sensación: capas lentas, suspendidas y expansivas.

### Paso 1 — Comenzar desde Init

Carga **Init**. Antes de continuar, cambia **TYPE** a **SVF**. Para este pad queremos el carácter más limpio y brillante del SVF; la saturación y el comportamiento más oscuro del filtro Moog volverían el sonido demasiado denso para el tipo de atmósfera que buscamos.

### Paso 2 — Configurar los osciladores para el intervalo de quinta

Ajusta la **waveform de OSC 2** a **Square** (Osc 1 permanece en Saw). La combinación Saw + Square produce dos colores armónicos complementarios: la riqueza armónica de la saw junto con el carácter hueco de la square.

Ajusta **COARSE** a `-7` semitonos. Esto coloca Osc 2 una quinta justa por debajo de Osc 1 — el intervalo que da nombre a este preset.

Ajusta **FINE** a `-6.3` cents. Una ligera desafinación entre osciladores genera *beating* orgánico y evita que la quinta suene matemáticamente perfecta (y por tanto demasiado rígida).

Ajusta **MIX** a `60:40` (valor del knob `40`, favoreciendo ligeramente Osc 1). Este balance deja la saw al frente mientras la square añade cuerpo y profundidad debajo.

Lleva **WIDTH** a `95 %`. Los pads suelen beneficiarse de una apertura estéreo extrema; este es uno de los pocos casos donde acercarse al máximo resulta musicalmente apropiado.

### Paso 3 — Configurar el filtro

Mantén **CUTOFF** en `75 %` (igual que en Init). Este pad no necesita un filtro particularmente cerrado; su carácter proviene más de la forma de las envolventes y del movimiento lento que de un filtrado agresivo.

Aumenta **RESO** a `25 %`. Una pequeña cantidad de resonancia le da una ligera “voz” al movimiento del filtro sin volverlo demasiado pronunciado.

Ajusta **ENV AMT** a `+42 %`. La envolvente del filtro abrirá y cerrará suavemente el cutoff a lo largo del tiempo, contribuyendo al carácter evolutivo del pad.

Añade **VCF MOD** en `10 %`. Una pequeña cantidad de modulación LFO sobre el cutoff añade respiración continua al sonido: movimiento perceptible, pero nunca exagerado.

### Paso 4 — Dar forma a las envolventes lentas

Aquí es donde realmente vive el carácter del pad. Ajusta la **envolvente del filtro** (lado FILTER):

- **A** (*Attack*) → `90` (swell muy lento)
- **D** (*Decay*) → `80`
- **S** (*Sustain*) → `72`
- **R** (*Release*) → `80` (release largo)

Ahora ajusta la **envolvente de amplitud** (lado AMP):

- **A** (*Attack*) → `90`
- **D** (*Decay*) → `80`
- **S** (*Sustain*) → `80`
- **R** (*Release*) → `80`

Ambas envolventes son lentas en todas sus etapas. Al presionar una nota, el sonido tarda aproximadamente 2 segundos en alcanzar su nivel máximo; al soltarla, tarda otros 2 segundos en desaparecer. Esa lentitud es lo que hace que el pad se sienta suspendido en el tiempo.

### Paso 5 — Añadir modulación suave con LFO

Reduce **LFO RATE** a aproximadamente `0.30` (~`0.45 Hz`, muy lento). A esta velocidad, el LFO completa un ciclo cada 2–3 segundos — lo suficientemente lento como para sentirse como respiración y no como un wobble evidente.

Añade **PWM/VIB** en `+5` (lado positivo, vibrato muy sutil). El vibrato es tan leve que apenas se percibe conscientemente, pero evita que el pitch se sienta completamente estático.

### Paso 6 — Balance final

Reduce **OUTPUT** a `-4 dB`. Los pads suelen percibirse más fuertes de lo que realmente parecen porque sostienen múltiples voces simultáneamente durante largos periodos. Reducir 4 dB deja margen dinámico suficiente para apilar acordes sin generar clipping.

### Prueba de escucha

Toca un acorde simple (por ejemplo C-E-G) y **manténlo presionado** durante al menos 3 segundos. Deberías escuchar:

- Un swell lento desde silencio hasta volumen completo.
- Una imagen estéreo amplia que llena el campo estéreo.
- Movimiento continuo y suave del filtro.
- Un vibrato apenas perceptible.
- Un fade lento después de soltar el acorde.

Ahora toca varios acordes consecutivos sin esperar a que desaparezcan completamente: los releases largos se superpondrán entre sí, creando una nube continua de sonido.

### Variaciones

- **Más movimiento**: aumenta **VCF MOD** a `25 %` para un barrido de filtro más evidente.
- **Pad más contenido**: reduce todas las etapas de envolvente alrededor de `50 %` para un sonido más rápido y menos expansivo.
- **Otro color armónico**: cambia la waveform de Osc 2 a Triangle para un pad más suave y cálido.
- **Experimento de desafinación**: aumenta **FINE** hasta `-15` cents para obtener un *beating* mucho más evidente entre los osciladores.

> 🎓 **Sobre el estéreo amplio en pads.** En Andes JX, WIDTH distribuye las voces dentro del campo estéreo según la nota interpretada, utilizando una disposición inspirada en la espacialidad natural de un piano. En acordes polifónicos, cada voz ocupa una posición ligeramente distinta, generando una imagen estéreo amplia y orgánica en lugar de un simple efecto de paneo artificial.  
>
> Con valores altos de WIDTH (`90–100 %`), los pads adquieren una sensación envolvente y tridimensional especialmente útil para texturas lentas y atmosféricas. La ligera variación espacial entre voces evita que el sonido se perciba rígido o excesivamente centrado, incluso cuando varias notas sostienen el mismo acorde durante largos periodos.

---

### 6.3 Tutorial 3 — Lead: Cotopaxi Acid

### Qué vamos a construir

Un lead estilo acid polifónico, con el filtro Moog llevado a una zona altamente resonante, sin keytracking y con una envolvente de filtro pronunciada en cada nota. Cotopaxi es uno de los estratovolcánes activos más altos y peligrosos del mundo — este patch toma prestada esa energía: afilada, ascendente y con un carácter que atraviesa la mezcla.

### Paso 1 — Comenzar desde Init

Carga **Init**. Verifica que **TYPE** esté configurado en **Moog** — este lead depende completamente del comportamiento del filtro Moog con resonancias altas.

### Paso 2 — Configurar los osciladores para un lead a la quinta superior

Ajusta la **waveform de OSC 2** a **Square**. La combinación Saw + Square aporta dos texturas armónicas distintas al lead.

Ajusta **COARSE** a `+7` semitonos (una quinta justa por encima de Osc 1, inverso del pad Cayambe).

Ajusta **FINE** a `-7.1` cents para generar una ligera desafinación y engrosar el sonido.

Ajusta **MIX** a `35:65` (favoreciendo Osc 2, la square afinada a la quinta superior). Esto coloca la square como voz principal mientras la saw añade riqueza armónica por debajo.

Ajusta **OCTAVE** a `+1`. Esto desplaza todo el instrumento una octava arriba, llevando el rango de interpretación hacia territorio de lead.

Reduce **WIDTH** a `20 %`. Los leads suelen funcionar mejor cuando permanecen relativamente enfocados dentro de la imagen estéreo; un lead demasiado amplio puede perder presencia y definición dentro de la mezcla.

### Paso 3 — Configurar el filtro para carácter acid

Ajusta **CUTOFF** a `65 %`. Lo suficientemente cerrado para que la envolvente del filtro tenga espacio para abrirse dramáticamente.

Lleva **RESO** a `65 %`. La resonancia alta es una de las características fundamentales del sonido acid; este valor entra claramente en territorio resonante sin llegar todavía a auto-oscilación.

Ajusta **ENV AMT** a `+55 %`. La envolvente del filtro abrirá el cutoff agresivamente en cada ataque de nota.

Ajusta **KEY TRCK** a `0 %`. **Esto es fundamental para el carácter acid**: sin keytracking, el cutoff permanece en la misma frecuencia independientemente de la nota que toques. Como resultado, las notas agudas suenan relativamente más oscuras que las graves porque su fundamental se acerca al cutoff. Ese comportamiento desigual a través del teclado es una parte esencial del carácter clásico acid.

Añade una pequeña cantidad de **VCF MOD** en `12 %`. Un poco de modulación lenta sobre el cutoff mantiene las notas sostenidas vivas y en movimiento.

### Paso 4 — Dar forma a las envolventes para un lead percusivo

La envolvente del filtro es la responsable principal del ataque acid. Ajusta la **envolvente del filtro**:

- **A** (*Attack*) → `60`
- **D** (*Decay*) → `30`
- **S** (*Sustain*) → `0`
- **R** (*Release*) → `25`

El Attack en `60 %` es poco común para un lead acid — normalmente el filtro abre instantáneamente. Aquí, el ataque lento produce un efecto de apertura progresiva (*blooming*): el filtro se abre gradualmente durante el inicio de la nota en lugar de hacerlo de golpe.

Ahora ajusta la **envolvente de amplitud**:

- **A** (*Attack*) → `0`
- **D** (*Decay*) → `25`
- **S** (*Sustain*) → `12`
- **R** (*Release*) → `50`

La amplitud entra instantáneamente, pero el sustain cae rápidamente hasta un nivel muy bajo. Combinado con el attack lento del filtro, esto produce una interacción interesante: la amplitud disminuye mientras el filtro todavía continúa abriéndose. El resultado es un lead que “florece” tímbricamente después del ataque inicial.

### Paso 5 — Configurar el comportamiento interpretativo

Ajusta **GLIDE** a **Always** con **GLIDE RATE** en `34 %`. El glide constante entre todas las notas — incluso sin tocar legato — le da al lead su carácter resbaladizo y vocal.

Mantén **VOICE** en **Poly**. Aunque el acid clásico es monofónico, este preset utiliza polifonía para permitir acordes ocasionales y reinterpretar el concepto acid desde una perspectiva más moderna.

### Paso 6 — LFO y balance final

Ajusta **LFO RATE** a `0.55` (~`3 Hz`, velocidad moderada). El LFO modulando el filtro a esta velocidad añade un wobble claramente perceptible en notas sostenidas.

Mantén **OUTPUT** en `0 dB`.

### Prueba de escucha

Toca una nota alrededor de C4. Deberías escuchar:

- Un ataque inmediato que rápidamente cae a un nivel bajo.
- Un filtro que continúa abriéndose gradualmente durante el inicio de la nota.
- Un wobble continuo proveniente del LFO.
- Una resonancia claramente audible como parte del timbre.

Ahora toca una secuencia de notas (por ejemplo C, E♭, G, B♭). Cada transición debería deslizarse suavemente, y notarás que las notas agudas suenan más oscuras que las graves. Ese comportamiento desigual proviene de haber desactivado el keytracking y es una parte esencial del sonido acid.

### Variaciones

- **Acid auto-oscilante**: lleva **RESO** a `90 %` para acercarte a auto-oscilación.
- **Ataque más agresivo**: reduce **filter A** a `0` para un barrido instantáneo del filtro.
- **Acid monofónico**: cambia **VOICE** a **Mono** + **GLIDE** a **Legato** para acercarte al comportamiento clásico tipo TB-303.
- **Lead más moderno y brillante**: cambia **TYPE** a **SVF** para obtener un carácter más limpio y filudo.

> 🎓 **Por qué los sonidos acid desactivan el keytracking.** En la mayoría de patches, el keytracking se utiliza para mantener el brillo relativamente consistente en todo el teclado: notas graves más oscuras y notas agudas más brillantes. Pero en el sonido acid, el filtro mismo se convierte en parte central del instrumento. Al desactivar el keytracking, el cutoff permanece fijo mientras las notas cambian de pitch. Esto provoca que una misma línea melódica cambie radicalmente de color tímbrico dependiendo de la nota tocada.  
>
> Esa inestabilidad tímbrica — donde el filtro deja de “seguir” al teclado — es una de las razones por las que las líneas acid se sienten tensas, impredecibles y expresivas. El Roland TB-303, el instrumento que definió el sonido acid, tampoco incluía control de keytracking: esa aparente limitación terminó convirtiéndose en parte esencial de su identidad sonora.

## 7. Técnicas de sound design

Los tutoriales anteriores mostraron cómo construir tres sonidos específicos. Esta sección toma distancia y se enfoca en **técnicas transversales**: estrategias que pueden aplicarse a muchos patches distintos y que comienzan a aparecer naturalmente después de pasar algo de tiempo con el instrumento. Piensa en esto menos como recetas y más como principios: maneras de entender Andes JX que te ayudarán a llegar más rápido al sonido que tienes en mente.

Las tres técnicas cubren:

- **7.1 Trabajando con el filtro** — cómo elegir entre SVF y Moog, y cómo interactúan cutoff, resonancia y envolvente.
- **7.2 Modulación interesante** — cómo utilizar el LFO global a través de sus tres destinos.
- **7.3 Movimiento interno** — cómo darle vida a sonidos que de otra forma se sentirían estáticos.

---

### 7.1 Trabajando con el filtro

El filtro es la herramienta más expresiva de Andes JX. Dos cosas marcan una diferencia enorme: elegir el tipo de filtro correcto para el sonido que buscas y entender cómo interactúan cutoff, resonancia y envolvente.

### Elegir entre SVF y Moog

Los dos modelos de filtro de Andes JX no son simplemente “dos sabores de lo mismo”. Poseen personalidades realmente distintas y funcionan mejor en contextos musicales diferentes.

**Elige SVF cuando busques**:

- Filtrado limpio y transparente que no añada demasiada coloración propia.
- Resonancia predecible y controlada incluso en valores extremos.
- Sonidos modernos y pulidos (leads limpios, pads brillantes, basses precisos).
- Que el filtro funcione más como moldeador tonal que como fuente de carácter.

**Elige Moog cuando busques**:

- Calidez, saturación y carácter analógico.
- Resonancia más agresiva y densa.
- Sonidos clásicos de síntesis sustractiva (acid leads, basses gruesos, pads saturados).
- Que el filtro forme parte del timbre y no solo sea un proceso aplicado al timbre.

Un ejercicio útil: carga cualquier preset y alterna entre SVF y Moog. Los mismos parámetros producen resultados claramente distintos. El Moog atenúa más las frecuencias altas y añade saturación armónica; el SVF conserva más contenido espectral y mantiene un sonido más brillante y definido. Una vez que internalizas esa diferencia, elegir entre ambos filtros se vuelve intuitivo.

### El triángulo cutoff–resonancia–envolvente

Tres parámetros trabajan juntos para definir el comportamiento del filtro en Andes JX: **CUTOFF** (dónde se encuentra el filtro), **RESO** (cuánto enfatiza ese punto) y **ENV AMT** (cuánto lo mueve la envolvente). Entender su interacción es mucho más útil que pensar en cada parámetro de forma aislada.

Algunos patrones importantes:

**Cutoff estático (ENV AMT = 0)**: el filtro simplemente moldea el tono. Útil para pads donde quieres un carácter fijo o para leads donde el timbre permanece estable entre notas.

**Barrido suave de envolvente (ENV AMT = +20 a +40 %)**: el cutoff se abre ligeramente durante el ataque y luego vuelve a asentarse. Añade vida orgánica sin resultar evidente. La mayoría de sonidos “naturales” viven aquí.

**Barrido fuerte de envolvente (ENV AMT = +50 a +80 %)**: apertura dramática del filtro en cada nota. La firma sonora de plucks, basses con ataque y leads expresivos.

**Barrido invertido (ENV AMT negativo)**: el filtro se cierra durante el ataque y se abre mientras la envolvente decae. Menos común, pero útil para clicks percusivos o efectos tipo reverse.

La resonancia amplifica todos estos comportamientos. Un barrido suave con poca resonancia se siente delicado; el mismo barrido con resonancia alta se vuelve vocal y expresivo. La resonancia actúa como multiplicador de la personalidad del filtro.

### Evitar basses embarrados

Un error común: cerrar demasiado el cutoff en patches de bass. En solo puede sonar enorme, pero dentro de una mezcla desaparece rápidamente. La fundamental de una nota grave (por ejemplo A1 = 55 Hz) ya ocupa una región muy baja del espectro; si además el filtro corta gran parte de los armónicos superiores, el bass pierde definición y deja de ser audible en parlantes pequeños.

Un punto de partida útil: ajusta el cutoff aproximadamente **dos octavas por encima** de la nota más grave que planeas tocar. En la práctica, esto suele significar valores de cutoff alrededor de `40–60 %`. Luego utiliza la envolvente del filtro para añadir ataque y carácter sin cerrar todavía más el espectro.

---

### 7.2 Modulación interesante

El LFO global de Andes JX puede modular tres destinos: pitch (vibrato), pulse width (PWM) y filter cutoff (VCF MOD). Lo interesante aparece cuando utilizas **más de un destino al mismo tiempo**.

### Usos clásicos de un solo destino

Estos son los puntos de partida más directos para cada destino:

- **Solo vibrato** (PWM/VIB entre `+30 y +50`) — vibrato natural para leads.
- **Solo PWM** (PWM/VIB entre `-30 y -60`, usando waveform PWM) — modulación gruesa y animada para pads y stabs.
- **Solo filter wobble** (VCF MOD entre `30–60 %`) — movimiento rítmico de filtro, base de muchas texturas dub y electrónicas.

Cada uno funciona bien por separado. El objetivo inicial es aprender cómo se siente cada destino individualmente.

### Combinando destinos

El terreno realmente interesante aparece cuando comienzas a combinar destinos de modulación.

En Andes JX, el LFO puede modular simultáneamente:

- **Pitch + filtro** (vibrato + filter wobble)
- **PWM + filtro**

Pero **pitch y PWM no pueden utilizarse al mismo tiempo**, porque ambos comparten el mismo knob bipolar (**PWM/VIB**): valores positivos activan vibrato y valores negativos activan PWM.

Esto significa que el instrumento siempre te obliga a elegir entre una modulación orientada al pitch o una orientada al ancho de pulso, manteniendo el filtro como tercer destino independiente.

**Vibrato + filter wobble**: un vibrato sutil (`+15 %`) combinado con una modulación suave del filtro (`10 %`) a la misma velocidad de LFO produce un sonido que se siente “vivo” sin que ninguna modulación sea evidente por sí sola. El cerebro percibe una respiración unificada en lugar de dos efectos separados.

**Filtro lento + PWM lento**: con velocidades de LFO muy bajas (alrededor de `0.2 Hz`), combinar modulación de filtro y PWM hace que un pad evolucione continuamente. Cada ciclo produce un color armónico ligeramente distinto.

**PWM rápido + filter wobble rápido**: a velocidades altas de LFO (por encima de `8 Hz`), la combinación de PWM y modulación de filtro produce un efecto tembloroso y extremadamente sintético, cercano a un chorus artificial. Muy útil para pads etéreos y texturas sci-fi.

La idea importante es esta: **solo tienes un LFO**. Todos los destinos activos se mueven a la misma velocidad. Esa limitación es deliberadamente creativa — te obliga a encontrar velocidades que funcionen simultáneamente para todas las modulaciones activas, produciendo movimiento coherente en lugar de varias modulaciones compitiendo entre sí.

### Elegir la velocidad del LFO musicalmente

La velocidad del LFO determina el comportamiento de todas las modulaciones simultáneamente. Algunas regiones musicales útiles:

- **0.1–0.5 Hz** — respiración lenta. Cambio orgánico y continuo. Ideal para pads.
- **1–3 Hz** — wobble suave. Movimiento audible sin convertirse en un efecto dominante. Bueno para leads sostenidos y basses atmosféricos.
- **5–7 Hz** — rango natural de vibrato. Similar al vibrato de cantantes e instrumentos de cuerda.
- **10–15 Hz** — tremolo / efecto rápido. La modulación se convierte en un evento musical evidente.
- **Más de 15 Hz** — entra en rango audible. La modulación comienza a percibirse más como tono que como movimiento.

Cuando el LFO modula el filtro a velocidades muy altas (por encima de `15 Hz`), pueden aparecer sidebands alrededor de la frecuencia de cutoff — una forma de FM a través del filtro. Es un efecto poco convencional y vale la pena experimentarlo.

---

### 7.3 Movimiento interno

Un patch que no cambia se siente muerto. La diferencia entre un sonido que mantiene la atención y uno que rápidamente se convierte en “fondo” es el **movimiento interno**: pequeños cambios continuos que ocurren aunque no hagas nada.

Andes JX ofrece varias maneras de añadir movimiento interno. Estas son las más efectivas.

### Desafinación entre osciladores

Cuando dos osciladores están perfectamente afinados, producen un tono estático. En cuanto introduces una ligera desafinación mediante **FINE**, comienzan a generar *beating*: fluctuaciones periódicas de amplitud que hacen que el sonido cobre vida.

Un rango útil para empezar: **FINE entre `+5 y +15` cents**. Por debajo de eso, el beating es demasiado lento; por encima de `25` cents, los osciladores comienzan a percibirse como dos pitches separados más que como un único sonido en movimiento.

La desafinación funciona prácticamente en cualquier patch. Incluso un bass simple mejora cuando utilizas MIX = `50:50` y unos pocos cents de desafinación; deja de sentirse completamente digital y comienza a percibirse más orgánico.

### Modulación lenta del filtro

Un cutoff completamente estático suele sentirse rígido. Incluso una cantidad mínima de modulación de filtro (VCF MOD entre `5–10 %`, LFO RATE alrededor de `0.3 Hz`) hace que un pad parezca respirar. El oyente probablemente no perciba conscientemente la modulación, pero sí percibe la ausencia de inmovilidad absoluta.

Es una de las técnicas más simples de sound design y también una de las más efectivas. Dos pads idénticos — uno estático y otro con movimiento sutil de filtro — se perciben como sonidos completamente distintos.

### Apertura estéreo

Un sonido mono ocupa un único punto en el espacio. Un sonido amplio ocupa una región completa del campo estéreo. El cerebro percibe los sonidos amplios como más grandes y complejos, incluso cuando el contenido armónico es exactamente el mismo.

Para pads, WIDTH es fundamental — valores entre `60–95 %` añaden profundidad y presencia. Para leads y basses, WIDTH debería mantenerse relativamente bajo (`0–30 %`) porque la definición y el enfoque suelen ser más importantes que la amplitud estéreo.

Una técnica útil: combina WIDTH moderado (`40–60 %`) con una ligera desafinación (FINE = `+8`). Obtendrás un sonido simultáneamente amplio y en movimiento, perceptualmente mucho más grande que cualquiera de los dos efectos por separado.

### Envolventes lentas en notas sostenidas

Si sostienes una nota durante varios segundos y el sonido permanece exactamente igual todo el tiempo, el oído pierde interés rápidamente. Las envolventes lentas — incluso en parámetros donde normalmente no pensarías usarlas — solucionan esto.

Dos ejemplos efectivos en Andes JX:

- **Envolvente lenta de filtro en pads**: ajusta D y S de la envolvente FILTER para que el cutoff derive lentamente hacia otra posición durante varios segundos. La nota sostenida se siente evolutiva.
- **LFO lento de filtro en basses largos**: incluso un bass se beneficia de un VCF MOD muy sutil (`~5 %`) con velocidades lentas. El filtro respira lentamente debajo de la nota.

### El principio detrás de estas técnicas

Todas estas técnicas comparten la misma idea: **el sistema auditivo humano percibe el cambio con mucha más facilidad que los estados absolutos**. Un sonido que cambia — aunque sea ligeramente — mantiene la atención porque el cerebro continúa siguiendo ese cambio. Un sonido completamente estático termina volviéndose invisible.

El movimiento interno no es decoración. Es lo que separa un sonido sintetizado de una simple demostración de sintetizador.

## 8. Aprendiendo DSP con Andes JX

Andes JX es tanto un proyecto educativo open-source como un instrumento musical. El código fuente completo está publicado bajo GPL v3 y documentado de forma bilingüe (inglés y español), de modo que pueda estudiarse como referencia para aprender DSP y desarrollo de plugins de audio.

Esta sección está dirigida a quienes quieren ir más allá de *usar* Andes JX y comenzar a *entender* cómo funciona internamente. Puedes leer esta sección sin abrir nunca un editor de código — funciona como un mapa general de la arquitectura — o puedes utilizarla como guía para estudiar realmente el código fuente.

El repositorio se encuentra en:  
[github.com/bansky0/Andes-JX](https://github.com/bansky0/Andes-JX)

---

### 8.1 Lo que necesitas para comenzar

Esta sección asume:

- Familiaridad básica con C++ (puedes leer definiciones de funciones y jerarquías de clases).
- Conocimiento general de qué es JUCE (no necesitas dominar toda la API).
- Cierta intuición sobre señales de audio (samples, sample rate y diferencia entre dominio temporal y frecuencial).

Si no tienes todos estos conocimientos, el código sigue siendo relativamente accesible gracias a la documentación bilingüe, pero obtendrás mucho más complementando la lectura con material introductorio sobre DSP y JUCE.

---

### 8.2 La arquitectura en tres capas

Andes JX está organizado en tres capas claramente separadas, cada una con una única responsabilidad:

```text
┌──────────────────────────────────────────────────┐
│                 Capa GUI                         │
│   PluginEditor + 6 LookAndFeels personalizados   │
│   (interfaz visual, sin procesamiento de audio)  │
├──────────────────────────────────────────────────┤
│               Capa Plugin                        │
│   PluginProcessor + APVTS + Sistema de Presets   │
│   (parámetros, MIDI, integración con el host y   │
│    manejo de estado)                             │
├──────────────────────────────────────────────────┤
│                 Capa DSP                         │
│   Synth → Voice → Oscillators + Filter + EG      │
│   (procesamiento puro de audio, sin dependencias │
│    de interfaz gráfica)                          │
└──────────────────────────────────────────────────┘
```

La capa DSP no conoce nada sobre la GUI; la GUI nunca toca directamente los buffers de audio. Esta separación es una de las razones por las que el proyecto resulta amigable para estudiar: puedes leer cualquier capa sin necesidad de entender completamente las demás primero.

---

### 8.3 Archivos clave para estudiar

El repositorio contiene muchos archivos. Estos son los más valiosos desde un punto de vista pedagógico, organizados según lo que enseñan.

#### Fundamentos

Estos dos archivos definen constantes globales y pequeñas utilidades. Léelos primero — gran parte del resto del proyecto asume sus definiciones.

- `Source/Constants.h` — constantes globales: número de parámetros, límite de polifonía, factor de oversampling y frecuencia de actualización del LFO. Cada constante incluye comentarios explicando dónde se utiliza y por qué tiene ese valor.
- `Source/Utils.h` — pequeñas utilidades matemáticas utilizadas en distintas partes del motor de síntesis.

#### Primitivas de síntesis

- `Source/Envelope.h` — generador ADSR estilo analógico. Archivo autocontenido y relativamente corto. Uno de los mejores puntos de entrada para comenzar a entender el motor de síntesis.
- `Source/NoiseGenerator.h` — generador de ruido blanco. Incluso más simple que la envolvente. Funciona bien como un “hello world” de generación de audio.
- `Source/Oscillator.h` — fachada para la generación de waveforms de los osciladores. Encapsula el uso del oscilador PolyBLEP más complejo.
- `Source/OscillatorPolyBLEP.cpp` — implementación real del oscilador anti-aliased. Contiene la técnica PolyBLEP junto con la referencia a Välimäki & Huovilainen (2007).

#### Filtros

- `Source/IFilter.h` — interfaz abstracta que define el contrato que todos los filtros deben implementar. Léelo primero para entender cómo los modelos SVF y Moog pueden intercambiarse dinámicamente.
- `Source/SVF.h` — implementación del *state variable filter* (Simper 2013), utilizando un diseño *topology-preserving*.
- `Source/LadderFilter.h` — núcleo del filtro Moog ladder con saturación no lineal (Huovilainen 2004).
- `Source/MoogFilter.h` y `Source/SVFFilter.h` — clases adaptadoras que encapsulan los filtros reales dentro del contrato `IFilter`.

#### Voz y orquestación de síntesis

- `Source/Voice.h` — describe lo que hace una única voz polifónica: osciladores, mezclador, filtro, envolventes y salida. Aquí se observa cómo todas las piezas interactúan dentro de una sola nota.
- `Source/Synth.h` y `Source/Synth.cpp` — motor principal de síntesis: manejo de voces, MIDI, polifonía, glide y ruteo de modulación. Es el archivo DSP más grande y el que conecta todo el sistema.

#### Capa Plugin

- `Source/PluginProcessor.h` y `Source/PluginProcessor.cpp` — implementación del `AudioProcessor` de JUCE para Andes JX. Contiene el layout de parámetros (`createParameterLayout`), el banco de presets (`createPrograms`) y el puente entre los cambios de parámetros provenientes del host y el motor de síntesis.

#### Capa GUI

- `Source/PluginEditor.h` y `Source/PluginEditor.cpp` — interfaz gráfica: controles, layout y conexión de parámetros mediante attachments de APVTS.
- `Source/LookAndFeel/AndesTheme.h` — fuente central de colores, tipografías y espaciados.
- `Source/LookAndFeel/AndesStyleHelpers.h` — primitivas reutilizables de dibujo (paneles redondeados, estados hover/press y tipografías).
- `Source/LookAndFeel/*.h` — seis LookAndFeels personalizados, uno para cada tipo de control. Se recomienda comenzar por `ToggleLookAndFeel.h` como ejemplo base; los demás siguen la misma estructura.

---

### 8.4 Rutas de estudio sugeridas

El orden en el que lees el código importa. Distintos objetivos llevan a distintos recorridos dentro del proyecto. Aquí tienes cinco rutas, organizadas desde la más corta (una tarde) hasta la más extensa (varias semanas).

#### Ruta 1 — “Quiero entender cómo se genera una sola nota” (~2 horas)

La ruta más corta con sentido práctico. Objetivo: seguir el recorrido de una nota desde la entrada MIDI hasta la salida de audio.

1. `Constants.h` y `Utils.h` — lectura rápida, solo para saber qué contienen.
2. `Envelope.h` — léelo completo; entiende cómo una envolvente ADSR genera samples.
3. `Oscillator.h` — revisa la interfaz pública (puedes omitir los detalles internos de PolyBLEP).
4. `Voice.h` — revisa `noteOn()` y `render()` para entender cómo se combinan envolvente, oscilador y filtro.

Al final, entenderás cómo Andes JX produce un sample de audio. Todo lo demás son extensiones y variaciones sobre esa base.

#### Ruta 2 — “Quiero entender polifonía y MIDI” (~4 horas)

Objetivo: entender cómo conviven múltiples notas y cómo los eventos MIDI controlan el sintetizador.

1. Completa primero la Ruta 1.
2. `Synth.h` — revisa la definición de la clase y la documentación de sus miembros.
3. `Synth.cpp` — enfócate en `noteOn()`, `noteOff()`, `render()` y la lógica de asignación de voces.
4. `PluginProcessor.cpp::processBlock()` — observa cómo los eventos MIDI provenientes del host terminan convirtiéndose en llamadas hacia `Synth`.

Al final, entenderás cómo Andes JX maneja hasta 16 voces simultáneas y distribuye eventos MIDI entre ellas.

#### Ruta 3 — “Quiero entender las implementaciones de filtros” (~4 horas)

Objetivo: entender los dos modelos de filtro y cómo conviven detrás de una misma interfaz.

1. `IFilter.h` — revisa el contrato de la interfaz y el bloque de comentarios que explica el patrón *strategy*.
2. `SVF.h` — el filtro más simple de los dos. Lee los comentarios del algoritmo e identifica los integradores.
3. `LadderFilter.h` — el más complejo. Presta atención a la saturación no lineal basada en `tanh` y al lugar donde se aplica.
4. `MoogFilter.h` y `SVFFilter.h` — adaptadores. Observa cómo traducen entre las implementaciones reales y el contrato `IFilter`.
5. `Voice.h` — revisa cómo la voz utiliza `IFilter*` de manera polimórfica sin saber qué modelo de filtro está activo.

Al final, entenderás ambos algoritmos de filtro y cómo el patrón *strategy* permite cambiar entre ellos dinámicamente en tiempo de ejecución.

#### Ruta 4 — “Quiero entender cómo la GUI habla con el motor de audio” (~4 horas)

Objetivo: entender el sistema de parámetros que conecta interfaz y DSP.

1. `PluginProcessor.h` y `PluginProcessor.cpp::createParameterLayout()` — revisa cómo cada parámetro es declarado junto con su rango, valor por defecto y formateador.
2. `Synth.h::update()` (y su implementación correspondiente en `Synth.cpp`) — observa cómo los valores de parámetros llegan al motor de audio.
3. `PluginEditor.cpp::initialiseAttachments()` — revisa cómo cada control de la GUI se conecta con su parámetro mediante el sistema de attachments APVTS de JUCE.
4. `PluginEditor.cpp::initialisePolyToggle()` y `initialiseFilterTypeControl()` — ejemplos de binding manual en casos donde los attachments APVTS no aplican directamente (parámetros Choice).

Al final, entenderás el flujo bidireccional completo: el usuario mueve un knob → APVTS notifica → el motor de audio se actualiza en el siguiente bloque; O la automatización del host cambia un parámetro → APVTS notifica → el control visual refleja el cambio.

#### Ruta 5 — “Quiero entender el sistema visual” (~4 horas)

Objetivo: entender cómo la capa LookAndFeel construye una identidad visual coherente.

1. `LookAndFeel/AndesTheme.h` — colores, tipografías y constantes de espaciado.
2. `LookAndFeel/AndesStyleHelpers.h` — primitivas de dibujo (`drawPanel`, `applyInteractionState`, `makeUIFont`).
3. `LookAndFeel/AndesBaseLookAndFeel.h` — clase base utilizada por los seis LookAndFeels concretos.
4. `LookAndFeel/ToggleLookAndFeel.h/.cpp` — comienza por este; funciona como ejemplo canónico completo de header + implementación.
5. Los demás LookAndFeels (`SegmentedButtonLookAndFeel.h`, `SecondaryKnobLookAndFeel.h`, `FaderLookAndFeel.h`, `ComboBoxLookAndFeel.h`, `KnobPrincipalLookAndFeel.h`) siguen el mismo patrón con complejidad progresiva.

Al final, entenderás cómo un sistema visual unificado puede construirse a partir de un pequeño conjunto de constantes de tema + primitivas de dibujo + LookAndFeels específicos por widget.

---

### 8.5 Cómo los parámetros se conectan con el algoritmo

Un ejercicio mental útil: toma cualquier control de la interfaz y sigue el recorrido de su valor hasta el hilo de audio. Aquí tienes tres ejemplos concretos que ilustran el patrón.

#### Ejemplo 1 — El knob CUTOFF

1. **Capa GUI**: el usuario gira el knob CUTOFF. `KnobPrincipalLookAndFeel` redibuja el control visual; el `juce::Slider` reporta el nuevo valor normalizado (0–100) hacia su attachment de APVTS.

2. **Capa Plugin**: el APVTS recibe el valor y lo almacena bajo el ID de parámetro `filterFreq`. El host también es notificado (de modo que el cambio aparece en las pistas de automatización).

3. **Transición hacia la capa DSP**: al inicio del siguiente bloque de audio, `Synth::update()` lee el valor actual de `filterFreq` desde el APVTS y lo convierte en una frecuencia real en Hz (utilizando un mapeo logarítmico). El resultado se almacena dentro del estado interno de `Synth`.

4. **Capa DSP**: cuando cada `Voice` renderiza su salida, lee la frecuencia de cutoff actual desde `Synth` y la aplica al filtro activo (SVF o Moog). El siguiente sample de audio refleja inmediatamente el cambio.

Este patrón se repite para todos los parámetros, con variaciones dependiendo del tipo de dato específico (Float, Choice o Bool) y de las conversiones necesarias.

#### Ejemplo 2 — El selector TYPE (cambio de modelo de filtro)

Este caso es más interesante porque cambiar de filtro implica cambiar dinámicamente el tipo de objeto utilizado en tiempo de ejecución.

1. El usuario hace clic en el botón segmentado **TYPE** (SVF o Moog).
2. `SegmentedButtonLookAndFeel` redibuja el control visual; el `SegmentedControl` padre reporta el nuevo índice (0 o 1).
3. Debido a que `filterType` es un `AudioParameterChoice`, no puede utilizar un attachment estándar de APVTS. En su lugar, el editor utiliza el **dual-binding pattern** documentado en `PluginEditor.cpp::initialiseFilterTypeControl()`: un callback manual `onChange` escribe el valor hacia el parámetro, mientras un listener `parameterChanged` maneja cambios externos.
4. `Synth::update()` lee el nuevo valor de `filterType` y cambia el puntero `IFilter*` dentro de cada `Voice` hacia la implementación correspondiente.
5. Desde el siguiente sample en adelante, todas las voces utilizan el nuevo filtro.

El cambio dinámico es invisible para el resto del código porque todo interactúa a través de `IFilter*` (el patrón *strategy* en acción).

#### Ejemplo 3 — El knob PWM/VIB (un knob, dos funciones)

Este ejemplo muestra cómo un único parámetro puede controlar distintos procesos DSP dependiendo de su signo.

1. El usuario mueve el knob **PWM/VIB** hacia un valor positivo o negativo (rango `-100` a `+100`).
2. El APVTS almacena el valor bajo el parámetro `vibrato`.
3. `Synth::update()` lee el valor y:
   - Si es positivo, configura la cantidad de modulación LFO→pitch y pone en cero la modulación LFO→PWM.
   - Si es negativo, configura la cantidad de modulación LFO→PWM y pone en cero la modulación LFO→pitch.
   - Si es cero, ambas cantidades de modulación quedan en cero.
4. Cada voz aplica luego el LFO de manera correspondiente durante el siguiente bloque de audio.

Un único parámetro termina controlando elegantemente dos rutas distintas de modulación, concentrando toda la lógica de ruteo dentro de `Synth::update()`.

---

### 8.6 Dónde viven las referencias académicas dentro del código

Andes JX está construido sobre técnicas DSP publicadas académicamente. Cada una de las cuatro referencias algorítmicas principales aparece citada en el archivo donde se aplica, permitiendo conectar la implementación directamente con el paper original:

| Algoritmo | Referencia | Archivo |
|-----------|-----------|------|
| **PolyBLEP** anti-aliasing | Välimäki & Huovilainen (2007), *IEEE Signal Processing Magazine* 24(2):116-125 | `OscillatorPolyBLEP.cpp` |
| **Moog ladder** no lineal | Huovilainen (2004), DAFx-04 Naples | `LadderFilter.h` |
| **SVF** topology-preserving | Simper (2013), *Cytomic Technical Papers* | `SVF.h` |
| **BLIT-DSF** (referencia histórica) | Stilson & Smith (1996), ICMC | `Oscillator.h` (preservado como alternativa histórica) |

Si quieres leer los papers originales, las referencias están formateadas académicamente dentro de los encabezados de cada archivo. Las implementaciones presentes en Andes JX no son traducciones literales de los papers — son adaptaciones prácticas diseñadas para ejecutar los algoritmos eficientemente en tiempo real.

---

### 8.7 Una nota sobre la documentación bilingüe

Cada archivo header en Andes JX incluye un bloque de documentación al inicio que explica:

- **El propósito del módulo** (qué hace ese archivo).
- **Su rol arquitectónico** (dónde se ubica dentro del proyecto).
- **Notas importantes** (decisiones de diseño, contratos con otros archivos y criterios de optimización).

Estos bloques están escritos tanto en inglés como en español, utilizando los prefijos `EN:` y `ES:`:

```cpp
/*
    Module: Voice
    Purpose:
        EN: A single polyphonic voice. Owns its own oscillators,
            envelope generators and filter pointer.
        ES: Una única voz polifónica. Posee sus propios osciladores,
            generadores de envolvente y puntero a filtro.
    ...
*/
```

Lo mismo ocurre con los comentarios inline distribuidos a lo largo del código: cuando una decisión no resulta obvia, ambos idiomas explican el razonamiento detrás de la implementación. Esta decisión fue tomada deliberadamente para hacer el proyecto más accesible a estudiantes hispanohablantes que, de otro modo, podrían enfrentarse a una barrera de idioma al leer material técnico escrito exclusivamente en inglés.

Si solo lees uno de los dos idiomas, el código sigue estando completo — ambas versiones contienen exactamente la misma información. El bilingüismo existe por accesibilidad, no porque la información esté dividida entre idiomas.

## 9. Banco de presets

Andes JX incluye **33 presets de fábrica** organizados en cinco familias sonoras. Cada preset está nombrado a partir de una ubicación real de los Andes ecuatorianos, combinada con un término musical que describe su carácter. Los nombres no son aleatorios — cada combinación conecta una geografía específica con una cualidad sonora específica.

Esta sección funciona como un catálogo completo con descripciones de todos los presets. Úsala como referencia para encontrar un punto de partida adecuado para el sonido que buscas, o como inspiración para tus propios diseños.

### Cómo leer esta sección

Cada entrada incluye:

- El **nombre cultural en negrita** (con las tildes y caracteres correctos tal como se escriben en Ecuador).
- El **nombre mostrado en el menú desplegable** entre paréntesis — exactamente como aparece en el selector PRESET dentro del plugin.
- Una **descripción de 2–3 líneas** que cubre el carácter sonoro, la técnica principal que define el patch y su contexto de uso típico.

Por ejemplo:

> **Cotacachi Ostinato** *(mostrado como: "Bass - Cotacachi Ostinato")*  
> Descripción...

Los nombres dentro del menú desplegable utilizan únicamente caracteres ASCII por compatibilidad multiplataforma. Si deseas buscar un preset por su nombre cultural, el texto entre paréntesis es el que debes localizar dentro del plugin.

---

### 9.1 Init

El punto de partida neutro. Se carga automáticamente al crear una nueva instancia de Andes JX. Dos osciladores saw separados por una octava atraviesan el filtro Moog, acompañados por una envolvente de filtro percusiva y una amplitud con sustain. Funciona como el “lienzo en blanco” para diseñar sonidos propios — todos los tutoriales de la sección 6 comienzan desde aquí.

---

### 9.2 Familia Bass (7 presets)

Siete presets enfocados en el registro grave, cada uno con una identidad claramente diferenciada para evitar solapamiento cuando se interpretan sobre las mismas notas MIDI. La familia cubre desde sub bass puro hasta acid squelch, pasando por drones sostenidos y portamentos estilo fretless.

### Cotacachi Ostinato *(mostrado como: "Bass - Cotacachi Ostinato")*

Un bass mono enfocado a través del filtro Moog con resonancia alta (`75 %`) y una envolvente de filtro pronunciada. Octave en `-2` lo coloca firmemente en territorio sub, mientras el glide en modo Legato añade transiciones expresivas entre notas. Diseñado para patrones repetitivos de bajo donde un carácter elástico y definido impulsa el groove. (Este es el bass reconstruido paso a paso en el tutorial 6.1.)

### Imbabura Pedal *(mostrado como: "Bass - Imbabura Pedal")*

Un pedal grave oscuro y sostenido construido a partir de una combinación Saw + Square desafinada `-10.9` cents, procesada a través del filtro Moog con cutoff al `32 %` y velocity tracking invertido (`filterVelocity = -40`). Pensado para permanecer debajo de la mezcla aportando base armónica más que actividad melódica — las notas largas se sienten sólidas y pesadas.

### Chimborazo Sub *(mostrado como: "Bass - Chimborazo Sub")*

El sub-bass más puro del banco: un único oscilador sine transpuesto `-12` semitonos, procesado por el SVF con cutoff muy bajo (`26 %`) y en modo mono. Prácticamente no existe contenido armónico por encima de la fundamental. Ideal cuando necesitas peso en graves sin competir espectralmente con otros elementos de la mezcla.

### Tungurahua Wobble *(mostrado como: "Bass - Tungurahua Wobble")*

Bass wobble modulado por LFO con un diseño particularmente interesante: la cantidad de envolvente del filtro es **negativa** (`-24`), por lo que el cutoff *se cierra* durante el ataque y luego se abre mientras la envolvente decae. Combinado con una modulación fuerte de filtro por LFO (`28 %`), esto genera el carácter wobble típico sobre una combinación Saw + PWM. Ideal para dubstep, drum & bass y contextos donde el bass funciona también como elemento rítmico.

### Sincholagua Staccato *(mostrado como: "Bass - Sincholagua Staccato")*

Bass corto y percusivo construido con waveforms Square + Triangle a través del filtro Moog, acompañado de un release rápido de amplitud (`25`). El glide en modo Legato solo se activa en notas superpuestas. El resultado es un sonido seco y definido, ideal para líneas staccato donde cada nota necesita distinguirse claramente sin arrastrarse sobre la siguiente.

### Atacazo Pick *(mostrado como: "Bass - Atacazo Pick")*

Un bass tipo picked bass con filtro SVF y una pequeña cantidad de ruido añadida al ataque (`10 %`) para simular el golpe inicial de una cuerda pulsada. Combina Triangle + Square con una ligera desafinación. El sustain es cero (`envSustain = 0`), por lo que cada nota funciona como un evento percusivo breve, similar al comportamiento de un bajo real tocado con púa.

### Chiles Portamento *(mostrado como: "Bass - Chiles Portamento")*

Bass estilo fretless con glide prominente. Osciladores Triangle + Saw aportan calidez, mientras el filtro SVF mantiene el tono limpio. El release largo de amplitud (`65`) deja resonar las notas, y el glide en modo Legato con velocidad moderada (`40`) produce el deslizamiento característico entre pitches propio de un instrumento fretless.

---

### 9.3 Familia Pad (6 presets)

Seis presets atmosféricos diferenciados más por estructura que por simple timbre. Cada pad utiliza una combinación distinta de waveforms, carácter de filtro y comportamiento de envolventes para ocupar un espacio sonoro propio dentro de la familia.

### Cayambe 5th *(mostrado como: "Pad - Cayambe 5th")*

Un pad polifónico amplio con dos osciladores separados por una quinta justa (`oscTune = -7`), procesados a través del SVF para mantener limpieza espectral. Las envolventes lentas (attack y release alrededor de `80–90 %`) y el estéreo extremadamente abierto (`95 %`) producen una sensación suspendida y expansiva. (Este es el pad reconstruido paso a paso en el tutorial 6.2.)

### Páramo Sostenuto *(mostrado como: "Pad - Paramo Sostenuto")*

Las envolventes más largas de todo el banco — todas las etapas flotan entre `70–80 %`. Osciladores Saw + Triangle a través del SVF con una modulación suave de filtro por LFO producen un pad que parece estático al principio y revela movimiento solo después de varios segundos. Diseñado para música ambiental donde el tiempo mismo se convierte en parte del sonido.

### Altar Pianissimo *(mostrado como: "Pad - Altar Pianissimo")*

Pad íntimo construido a partir de dos waveforms triangle con el SVF en cutoff moderado (`42 %`) y resonancia alta (`60 %`). El nombre “pianissimo” proviene de la forma suave de las envolventes más que del nivel de salida. El carácter es delicado, cercano y ligeramente resonante — el equivalente sonoro de un espacio interior pequeño.

### Pululahua Crescendo *(mostrado como: "Pad - Pululahua Crescendo")*

Pad con fuerte respuesta de velocity hacia el filtro (`filterVelocity = +48`). Las notas suaves suenan oscuras y contenidas; las notas fuertes abren el filtro dramáticamente, produciendo crescendos naturales según la dinámica de interpretación. El ataque extremadamente lento (`envAttack = 100`) hace que incluso las notas más fuertes necesiten tiempo para desarrollarse completamente.

### Corazón Strings *(mostrado como: "Pad - Corazon Strings")*

Un ensemble clásico de synth strings — osciladores Saw + Square desafinados `-7.1` cents, procesados a través del SVF con envolventes moderadas. WIDTH en `82 %` genera una sensación de sección distribuida en el espacio. El sustain permanece completamente abierto mientras mantengas las notas, funcionando tanto para acordes sostenidos como para líneas melódicas.

### Iliniza Shimmer *(mostrado como: "Pad - Iliniza Shimmer")*

Pad brillante de registro alto transpuesto dos octavas arriba (`octave = +2`). Osciladores Sine + Triangle a través del SVF con un LFO rápido sobre el filtro (`rate ≈ 0.42`) producen una textura shimmer continua que se sitúa encima de otros elementos en lugar de debajo. Muy útil para capas cinematográficas en registros altos.

---

### 9.4 Lead family (6 presets)

Six melodic presets with strong individual character. Three are saw-based for cutting through a mix; three use distinct waveform identities (square, triangle, mixed) to provide alternative timbral options.

### Cotopaxi Acid *(displayed as: "Lead - Cotopaxi Acid")*

A 303-style acid lead through the Moog filter with high resonance (65 %) and a marked filter envelope sweep (+55). Glide is set to Always, so every note transition slides — characteristic of acid playing. Keytracking is at zero, producing the classic acid behavior where filter timbre stays fixed regardless of pitch. (Recreated step by step in tutorial 6.3.)

### Pichincha Unison *(displayed as: "Lead - Pichincha Unison")*

A unison lead built from two saws with strong detuning (-17.2 cents) for a thick, supersaw-like character. The very long attack on the amplitude envelope (envAttack = 100) gives notes a swelling quality before they reach full volume. Use for sustained lead lines where character and width matter more than punch.

### Sangay Fortissimo *(displayed as: "Lead - Sangay Fortissimo")*

An aggressive lead combining Saw + PWM waveforms with the Moog filter at moderate resonance. The long amplitude release (60) lets notes ring out, while the +12 semitone offset on Osc 2 adds octave brightness. Built for loud passages where the lead needs presence and authority.

### Rucu Mono *(displayed as: "Lead - Rucu Mono")*

A monophonic lead with **negative velocity tracking** (filterVelocity = -50): louder notes close the filter, softer notes open it. This inverted dynamic produces an unusual expressive quality where playing harder makes the sound darker rather than brighter. Octave -1 places it in mid-range bass-lead territory.

### Guagua Flute *(displayed as: "Lead - Guagua Flute")*

A breathy flute-like lead. Saw + Triangle waveforms through the Moog filter with soft attack (filterAttack = 47) and small noise addition (noise = 3) simulate the breath component of a flute. Vibrato is set to a subtle +3 — enough to feel organic without becoming an obvious wobble. Octave -2 places it in low flute register.

### Rumiñahui Square *(displayed as: "Lead - Ruminahui Square")*

A delicate pluck based on a Square + Saw combination with no glide, fast envelope decay (envDecay = 10), and zero sustain. The character is precise and short, ideal for melodic figures where each note is articulated separately. The SVF filter keeps the tone clean, letting the square's hollow character come through.

---

### 9.5 Brass / Wind / Organ family (6 presets)

Six orchestral and aerial presets covering the registers a wind ensemble would occupy. The family is internally divided into three sub-groups by prefix in the dropdown: **Wind**, **Brass**, and **Organ**.

### Carihuairazo Horn *(displayed as: "Wind - Carihuairazo Horn")*

A soft solo horn built from Triangle + Square waveforms with Osc 2 transposed +12 semitones. Filter envelope is negative (-18) — the cutoff closes on attack rather than opening, simulating the muted character of a softly blown horn. Through the SVF for transparency. Best for solo melodic passages.

### Cóndor Tutti *(displayed as: "Brass - Condor Tutti")*

A wide brass ensemble with Saw + Square oscillators (Osc 2 transposed +12 semitones for octave brightness) through the Moog filter for warm character. The slow filter attack (filterAttack = 16) combined with high amplitude attack (envAttack = 65) gives the patch a swelling quality on each note. Stereo width at 72 % spreads it like a section spread across a stage.

### Antisana Fanfare *(displayed as: "Brass - Antisana Fanfare")*

A fanfare-style synth-brass built from two Saw oscillators with +14 cents detune for richness. Strong filter envelope (+48) creates the characteristic brass attack, and the Moog filter at moderate cutoff adds warm saturation. Made for short, declamatory phrases where the attack is the musical event.

### Sumaco Rotary *(displayed as: "Organ - Sumaco Rotary")*

An organ patch with rotary speaker emulation suggested through PWM modulation (vibrato = -12, applying LFO to pulse width on the Square waveforms). High resonance (55 %) at moderate cutoff gives the body a vocal quality typical of tone-wheel organs. Output trimmed to -4 dB to leave headroom.

### Llanganates Clarinet *(displayed as: "Wind - Llanganates Clarinet")*

A dark clarinet patch in the bass-clarinet register. Square + Triangle waveforms (the square providing the clarinet's characteristic odd-harmonic content) through the SVF at low cutoff (42 %). Zero attack and decay on the filter envelope keeps the timbre static — what you set is what you hear, like an acoustic instrument.

### Ilaló Whistle *(displayed as: "Wind - Ilalo Whistle")*

A bright whistle/piccolo voice. Sine + Triangle waveforms through SVF at moderate cutoff (38 %) with high resonance (72 %), producing the focused tonal character of a high-register whistle. Slight detune (-0.7 cents) on Osc 2 plus a small vibrato (+10) keeps it from feeling too synthetic.

---

### 9.6 Keys / Pluck / FX family (8 presets)

Eight presets organized into three sub-groups: **Keys** (3 sustained tonal percussives), **Pluck** (2 short tonal articulations), and **FX** (3 abstract textures). The variety here is the widest of any family — these presets cover roles that don't fit neatly elsewhere.

### Mojanda Rhodes *(displayed as: "Keys - Mojanda Rhodes")*

A Rhodes-style electric piano built from Triangle + Square waveforms through the SVF. Moderate envelope shaping (filterDecay 16, envDecay 32) produces the tine-like decay that defines Rhodes timbre. Use for jazzy chords, ballad accompaniment, or anywhere a softer keyboard voice is needed.

### Yanahurco Kalimba *(displayed as: "Keys - Yanahurco Kalimba")*

A dry, short kalimba voice with Triangle as Osc 1 and Sine as Osc 2 (the Sine adds purity to the fundamental), both transposed +12. Very fast amplitude decay (envDecay = 22) and zero sustain produce the characteristic kalimba pluck — present and brief, with no lingering tail.

### Quilindaña Steelpan *(displayed as: "Keys - Quilindana Steelpan")*

A metallic steelpan voice. Sine + Triangle waveforms with Osc 2 transposed +12 and -8 cents detune (the detune is what creates the metallic shimmer). Negative keytracking (-8) means high notes get darker, mimicking the natural behavior of a real steelpan as you move toward its higher pans.

### Pasochoa Pizzicato *(displayed as: "Pluck - Pasochoa Pizzicato")*

An orchestral pizzicato. Square + Triangle in mono-summed mix (oscMix = 0), through the SVF with very fast amplitude decay (envDecay = 8) and zero sustain. Output trimmed to -3 dB. The result is a focused, percussive tonal blip — perfect for pizzicato string lines or staccato accents.

### Cuicocha Bubble *(displayed as: "Pluck - Cuicocha Bubble")*

A quirky pluck with LFO modulation on the filter (filterLFO = 16, lfoRate = 0.38). Sine + Triangle waveforms transposed -12, filter at moderate cutoff with high resonance (52 %). Each note has a "bubble" quality from the filter modulation acting on the short envelope. Great for playful melodic figures.

### Cajas PWM *(displayed as: "FX - Cajas PWM")*

A wide PWM pad-texture combining Saw and PWM waveforms an octave apart. Vibrato at -8 routes the LFO to PWM modulation, producing the characteristic thick, animated pulse character. Long releases (envRelease = 68) and stereo width at 65 % give it presence as an evolving background element rather than a foreground voice.

### Quilotoa Aqua *(displayed as: "FX - Quilotoa Aqua")*

A velocity-sensitive watery texture. Saw + Triangle through the SVF with high cutoff (72 %) and high resonance (42 %), and strong negative velocity tracking (filterVelocity = -42 — soft notes open the filter, hard notes close it). The slow filter LFO modulation (rate 0.24, amount 14) creates the rippling water effect. Wide stereo at 75 %.

### Reventador Ghost *(displayed as: "FX - Reventador Ghost")*

A breathy spectral texture using Triangle + Saw through SVF with moderate cutoff (38 %) and Always-on glide (every note slides). The slow attack and long release create a haunted, drifting quality — notes never start cleanly and never end cleanly. Best for ambient pads, transitional textures, and sound design contexts where the absence of sharp edges is the point.

---

### Matriz de referencia rápida

Si estás buscando un sonido por **técnica** más que por familia, esta matriz te orienta hacia presets relevantes:

| Buscando... | Prueba estos presets |
|---|---|
| **Carácter acid** | Cotopaxi Acid, Cotacachi Ostinato |
| **Sub bass puro** | Chimborazo Sub |
| **Pads estéreo amplios** | Cayambe 5th, Páramo Sostenuto, Iliniza Shimmer |
| **Envolvente / velocity negativa** | Tungurahua Wobble, Carihuairazo Horn, Rucu Mono, Quilotoa Aqua |
| **Glide / portamento pronunciado** | Chiles Portamento, Cotopaxi Acid, Reventador Ghost |
| **Percusivo / pluck** | Pasochoa Pizzicato, Yanahurco Kalimba, Rumiñahui Square |
| **Movimiento impulsado por LFO** | Tungurahua Wobble, Cuicocha Bubble, Quilotoa Aqua |
| **Respuesta expresiva a velocity** | Pululahua Crescendo, Quilotoa Aqua, Rucu Mono |

Esto no es exhaustivo — muchos presets comparten características entre sí. Úsalo como punto de partida y luego explora libremente.

## 10. Solución de problemas y FAQ

Esta sección cubre dos tipos de preguntas: **problemas genéricos de plugins** que aplican a la mayoría de instrumentos VST3 y **preguntas específicas de Andes JX** relacionadas con comportamientos particulares de este sintetizador. Si tu problema no aparece aquí, la sección de Issues del repositorio de GitHub es el lugar adecuado para reportarlo.

[Issues de Andes-JX en GitHub](https://github.com/bansky0/Andes-JX/issues)

### 10.1 Problemas genéricos de plugins

### Andes JX no aparece en mi DAW después de instalarlo

Tres cosas que deberías revisar, en este orden:

1. **¿El archivo `.vst3` está en la carpeta correcta?**  
   En Windows debería encontrarse en:  
   `C:\Program Files\Common Files\VST3\AndesJX.vst3`  
   En macOS:  
   `/Library/Audio/Plug-Ins/VST3/AndesJX.vst3`  
   Si el instalador lo colocó en otro lugar, cópialo manualmente a una de estas rutas.

2. **¿Tu DAW ha escaneado nuevos plugins?**  
   La mayoría de DAWs realizan el escaneo automáticamente al iniciar, pero algunos requieren hacerlo manualmente. Busca un botón llamado **Rescan** o **Refresh** dentro de las preferencias de plugins de tu DAW.

3. **¿Tu DAW soporta VST3 de 64 bits?**  
   Andes JX funciona únicamente en 64 bits. Algunos DAWs muy antiguos (anteriores aproximadamente a 2018) pueden no tener soporte VST3 completo.

### No escucho sonido cuando toco notas

En orden de probabilidad:

1. **Revisa el nivel de OUTPUT**: si está en `-24 dB`, el sonido será prácticamente inaudible. Devuélvelo hacia `0 dB`.

2. **Revisa la envolvente de amplitud**: si AMP Sustain está en `0 %` y el Decay es muy rápido, la nota puede desaparecer antes de que la percibas claramente. Como prueba, ajusta temporalmente AMP Sustain a `100 %`.

3. **Verifica que el MIDI esté llegando al plugin**: la mayoría de DAWs muestran un indicador de actividad MIDI. Si no hay señal MIDI entrando, el sintetizador no tiene nada que reproducir.

4. **Revisa el ruteo de audio de tu DAW**: la salida del plugin debe estar conectada a una pista que llegue al master output.

### El consumo de CPU es demasiado alto

Tres lugares donde revisar:

1. **Reduce la polifonía**: si estás tocando muchas notas simultáneamente, estás activando muchas voces. Cambia **VOICE** a **Mono** si el patch lo permite.

2. **Ajusta el buffer size de tu DAW**: dependiendo del sistema, un buffer más pequeño o más grande puede mejorar el rendimiento. Si Andes JX parece ser el cuello de botella, experimenta con distintos tamaños de buffer.

3. **Desactiva instancias que no estés utilizando**: especialmente importante si tienes múltiples Andes JX cargados simultáneamente en el proyecto.

### Mi proyecto del DAW no recupera mis patches personalizados

Andes JX 1.0 no incluye un sistema propio de guardado/carga de presets. Para conservar un patch personalizado:

- Utiliza el sistema de presets de plugins de tu DAW (la mayoría incluyen un botón de “save state” en la parte superior de la ventana del plugin).
- Guarda el proyecto del DAW antes de cerrarlo — el estado completo del plugin se almacena junto con el proyecto.

Si cerraste el proyecto sin guardar y perdiste tu trabajo personalizado, ese trabajo se perdió definitivamente. Andes JX 1.0 no incluye autosave.

### 10.2 Preguntas específicas de Andes JX

### ¿Por qué el menú desplegable muestra "Custom" en lugar del nombre de un preset?

Porque has modificado al menos un parámetro después de cargar un preset de fábrica. Andes JX cambia automáticamente el display a “Custom” en el momento en que el estado actual deja de coincidir con cualquier preset guardado. Volver a cargar un preset de fábrica elimina el estado Custom. Consulta la sección 5.7 para más detalles.

### ¿Por qué no puedo seleccionar "Custom" desde el menú desplegable?

Porque no es un preset — es un marcador de estado. Seleccionarlo manualmente no tendría sentido (no existe un preset “Custom” definido para cargar). Solo aparece como etiqueta visual cuando el estado actual ha sido modificado respecto a un preset de fábrica.

### ¿Por qué el knob PWM/VIB tiene valores negativos?

Porque es un único knob con dos funciones distintas dependiendo del lado del cero en el que te encuentres:

- Valores **positivos** producen vibrato (el LFO modula el pitch).
- Valores **negativos** producen PWM (el LFO modula el ancho de pulso).

El display refleja esta lógica: los valores positivos muestran simplemente el número; los negativos aparecen etiquetados como “PWM”. Consulta la sección 5.5 para más detalles.

### ¿Por qué la envolvente del filtro hace que mi sonido desaparezca?

Si ENV AMT del filtro es positivo, el Sustain de la envolvente FILTER está en `0` y el CUTOFF es bajo, el filtro se abrirá durante el ataque y luego se cerrará completamente al decaer la envolvente — potencialmente hasta el punto donde el sonido deja de ser audible. Puedes solucionarlo aumentando CUTOFF, reduciendo ENV AMT, aumentando FILTER Sustain o combinando varias de estas opciones.

### ¿Por qué VEL FLTR a veces muestra "OFF"?

Cuando VEL FLTR baja de `-90`, el display muestra “OFF” en lugar de un valor numérico. Este es un estado interno especial que desactiva completamente el cálculo de velocity tracking, reduciendo ligeramente la carga de CPU. A nivel audible, “OFF” y `0` producen exactamente el mismo resultado (ningún efecto de velocity sobre el filtro); la diferencia existe únicamente a nivel de implementación.

### ¿Por qué algunos nombres de presets aparecen sin tildes en el menú desplegable (Páramo → Paramo, Cóndor → Condor)?

El menú desplegable utiliza únicamente caracteres ASCII por compatibilidad multiplataforma (algunos sistemas operativos y navegadores de plugins manejan caracteres acentuados de manera inconsistente). Los nombres culturales con sus tildes correctas están documentados en la sección 9 de este manual.

### ¿Puedo cambiar la waveform del LFO?

No en la versión 1.0. El LFO utiliza exclusivamente una waveform sine. Esta fue una decisión de diseño deliberada para mantener la modulación enfocada y musical; la modulación sinusoidal se percibe orgánica en los tres destinos del LFO (vibrato, PWM y filtro).

### ¿Andes JX soporta modulación a audio rate (FM)?

No. Los osciladores no se modulan entre sí y el LFO permanece en rango sub-audio (máximo ~`20 Hz`). Andes JX es un sintetizador puramente sustractivo; FM y modulación a audio rate están fuera del alcance conceptual del instrumento.

### ¿Por qué mi preset Init suena distinto al de otra persona?

No debería. El preset Init tiene una definición fija dentro del código fuente. Si el tuyo suena distinto, la causa más probable es que tú (o el host) modificaron un parámetro sin darte cuenta. Vuelve a cargar Init desde el menú desplegable para restaurar el estado canónico.

### ¿Puedo asignar parámetros a mi controlador MIDI mediante MIDI Learn?

Sí. Al hacer clic derecho sobre cualquier knob de Andes JX aparecerá el menú estándar de automatización del host (la interfaz exacta depende de tu DAW). Una vez que el parámetro está expuesto al host, puedes asignarlo mediante el sistema de MIDI mapping de tu DAW. Andes JX no implementa un sistema propio de MIDI Learn — depende completamente del host.

### ¿Se añadirá soporte AU para macOS?

Está planificado, pero no comprometido para una versión específica. Consulta el repositorio del proyecto para futuras actualizaciones.

[Repositorio Andes-JX en GitHub](https://github.com/bansky0/Andes-JX)

---

## 11. Glosario

Este glosario cubre términos de síntesis utilizados a lo largo de este manual. Las definiciones son prácticas y están orientadas a cómo se utiliza cada término dentro de Andes JX, no como explicaciones académicas exhaustivas.

### ADSR

Siglas de **Attack**, **Decay**, **Sustain** y **Release**. Las cuatro etapas de un generador de envolvente. Attack, Decay y Release son parámetros de tiempo; Sustain es un parámetro de nivel. Consulta la sección 5.3 para una explicación completa.

### Aliasing

Artefactos no deseados de alta frecuencia que aparecen cuando una señal digital contiene frecuencias por encima de la mitad del sample rate. En sintetizadores, el aliasing produce sidebands audibles con carácter metálico o áspero. Andes JX utiliza **PolyBLEP** para reducir aliasing en sus osciladores.

### Anti-aliasing

Técnicas utilizadas para prevenir aliasing durante la generación digital de audio. Ver **PolyBLEP**.

### Attack

La primera etapa de una envolvente ADSR: el tiempo que tarda la envolvente en alcanzar su valor máximo después de presionar una nota.

### Bipolar

Parámetro que posee valores positivos y negativos, utilizando cero como punto central. Ejemplos en Andes JX: COARSE, FINE, TUNE, ENV AMT, VEL FLTR y PWM/VIB.

### Cents

Unidad de afinación equivalente a 1/100 de un semitono. Se utiliza para ajustes finos de pitch. El knob FINE en Andes JX trabaja en cents.

### Cutoff

Frecuencia a partir de la cual un filtro low-pass comienza a eliminar frecuencias altas. Es el control más expresivo dentro de la síntesis sustractiva. Consulta la sección 5.2.

### Decay

La segunda etapa de una envolvente ADSR: el tiempo que tarda la envolvente en caer desde el valor máximo hasta el nivel de sustain.

### Detuning

Pequeña diferencia de afinación entre dos osciladores. Produce *beating* y añade cuerpo al sonido. En Andes JX, el knob FINE es la herramienta principal para esto.

### Envelope

Función que modifica un parámetro a lo largo del tiempo, activada por un evento de nota. La forma de envolvente más común es ADSR. Andes JX incluye dos envolventes: una para amplitud y otra para cutoff de filtro.

### Filter

Circuito (o equivalente digital) que atenúa o enfatiza determinadas frecuencias de una señal. La síntesis sustractiva gira alrededor de los filtros: partir de una waveform rica armónicamente y remover partes de ella. Andes JX incluye dos modelos de filtro: SVF y Moog ladder.

### Glide

También llamado **portamento**. Transición suave de pitch entre dos notas consecutivas en lugar de un salto instantáneo. Andes JX incluye tres modos de glide: Off, Legato y Always.

### Keytracking

Modulación que vincula un parámetro al pitch de la nota tocada. El uso más común es hacer que el cutoff del filtro siga el pitch de la nota para mantener un brillo relativamente constante a lo largo del teclado.

### Legato

Técnica interpretativa donde una nueva nota se toca antes de liberar la anterior, produciendo notas superpuestas. En Andes JX, el glide puede configurarse para aplicarse únicamente durante transiciones legato.

### LFO

**Low-Frequency Oscillator**. Oscilador que opera por debajo del rango audible (generalmente menos de `20 Hz`) y se utiliza como fuente de modulación en lugar de fuente sonora. Andes JX incluye un único LFO global ruteado hacia pitch, pulse width y cutoff de filtro.

### Mono / Monofónico

Modo de reproducción donde solo una nota puede sonar al mismo tiempo. Las nuevas notas interrumpen las anteriores. Opuesto de polifónico. Utilizado típicamente para basses, leads y líneas acid.

### Moog ladder

Diseño específico de filtro creado originalmente por Robert Moog para sus sintetizadores en los años 60. Conocido por su sonido cálido, saturación no lineal y resonancia agresiva. El filtro Moog de Andes JX es una implementación digital basada en Huovilainen (2004).

### Note On / Note Off

Eventos MIDI enviados cuando una tecla es presionada (Note On) y liberada (Note Off). Incluyen el número de nota (qué tecla) y velocity (qué tan fuerte fue presionada).

### Oscillator

Componente que genera la waveform base de un sonido. En Andes JX, dos osciladores por voz producen waveforms sine, saw, square, triangle y PWM.

### PolyBLEP

**Polynomial Band-Limited Step**. Técnica anti-aliasing utilizada en los osciladores de Andes JX. Suaviza las discontinuidades presentes en las waveforms saw y square para reducir artefactos de alta frecuencia no deseados. Referencia: Välimäki & Huovilainen (2007).

### Polyphony

Capacidad de reproducir múltiples notas simultáneamente. Andes JX soporta hasta 16 voces simultáneas.

### Portamento

Ver **Glide**.

### PWM

**Pulse Width Modulation**. Modulación del duty cycle (la relación entre tiempo encendido/apagado) de una waveform de pulso. Cuando se aplica mediante un LFO, produce un carácter grueso y animado. En Andes JX, PWM es tanto una waveform disponible como uno de los destinos del LFO.

### Release

La cuarta etapa de una envolvente ADSR: el tiempo que tarda la envolvente en caer hasta cero después de liberar la nota.

### Resonance

También llamada **Q** o **emphasis**. Realce aplicado a las frecuencias alrededor del cutoff del filtro. Resonancia baja produce filtrado suave; resonancia alta genera un carácter vocal tipo “wow”. En valores extremos, el filtro Moog puede entrar en auto-oscilación.

### Saw / Sawtooth

Waveform con una subida lineal seguida de una caída instantánea, produciendo un espectro armónico muy rico (todos los armónicos enteros presentes). Es la waveform clásica de la síntesis sustractiva.

### Self-oscillation

Condición donde un filtro comienza a generar una onda sinusoidal en su frecuencia de cutoff incluso sin señal de entrada, causada por resonancia extremadamente alta. El filtro Moog de Andes JX puede entrar en auto-oscilación cerca de `100 %` de resonancia.

### Semitone

La distancia más pequeña dentro de la escala temperada occidental de 12 tonos. El knob COARSE en Andes JX mueve Osc 2 en semitonos.

### Sine

Waveform pura sin armónicos, únicamente la fundamental. Utilizada como capa base o para sub-bass.

### Square

Waveform que alterna entre dos niveles fijos, produciendo únicamente armónicos impares. Tiene un carácter hueco.

### Subtractive synthesis

Método de síntesis que comienza con waveforms ricas armónicamente (saw, square) y las moldea eliminando frecuencias mediante filtros y modulando amplitud mediante envolventes. Andes JX es un sintetizador sustractivo.

### Sustain

La tercera etapa de una envolvente ADSR: el nivel en el que la envolvente permanece mientras la nota se mantiene presionada. A diferencia de las demás etapas ADSR, Sustain es un nivel y no un tiempo.

### SVF

**State Variable Filter**. Diseño de filtro capaz de producir simultáneamente salidas low-pass, high-pass y band-pass. El SVF de Andes JX utiliza el diseño topology-preserving de Andrew Simper (2013), conocido por su comportamiento limpio y predecible incluso en configuraciones extremas.

### Triangle

Waveform con subidas y bajadas lineales, produciendo armónicos impares que decaen rápidamente. Más suave que una square, con más cuerpo que una sine.

### Velocity

Valor MIDI (`0–127`) que representa qué tan fuerte fue presionada una tecla. Se utiliza como control expresivo. Andes JX mapea velocity hacia el cutoff del filtro mediante el knob VEL FLTR.

### Vibrato

Modulación periódica de pitch, normalmente mediante un LFO lento. Imita la oscilación natural de pitch de cantantes e instrumentos de cuerda. En Andes JX, vibrato corresponde a la función positiva del knob PWM/VIB.

### Voice

En un sintetizador polifónico, unidad independiente capaz de reproducir una nota. Andes JX dispone de 16 voces, permitiendo hasta 16 notas simultáneas.

### Waveform

Forma de la salida de un oscilador a lo largo del tiempo. Diferentes waveforms poseen distinto contenido armónico y, por lo tanto, distintos timbres. Andes JX incluye cinco: sine, saw, square, triangle y PWM.

## 12. Créditos y referencias

### 12.1 Equipo de Andes JX

| Persona | Contribuciones | Afiliación |
|--------|---------------|-------------|
| **Jhonatan López-Pilco** | Concepto, diseño y arquitectura general del proyecto. Implementación del motor DSP (osciladores, filtros, envolventes y modulación). Diseño e implementación de la GUI. Documentación bilingüe del código. | LASINAC, FIS, Escuela Politécnica Nacional, Quito, Ecuador |
| **Valeria Villarreal-Villarreal** | Implementación de componentes DSP complementarios. Testing, validación y retroalimentación de diseño para el instrumento. | Instituto Tecnológico de Artes del Ecuador (ITAE), Guayaquil, Ecuador (principal) · Universidad de las Américas (UDLA), Quito, Ecuador (secundaria) |

### 12.2 Este manual

Escrito por **Jhonatan López-Pilco**.

**Primera edición**: mayo 2026.  
**Versión cubierta**: Andes JX 1.0.  
**Ubicación**: Quito, Ecuador.

El manual se distribuye junto al código fuente dentro del repositorio de Andes JX:

[github.com/bansky0/Andes-JX](https://github.com/bansky0/Andes-JX)

Las futuras ediciones reflejarán actualizaciones del plugin e incorporarán correcciones.

### 12.3 Agradecimientos

El desarrollo de Andes JX no ocurrió en aislamiento. Varias personas y comunidades han contribuido a que este proyecto exista, y corresponde reconocerlas explícitamente.

#### Mentor

**PhD José Francisco Lucio-Naranjo** — profesor y coordinador de LASINAC (Laboratorio de Acústica) en la Escuela Politécnica Nacional, Quito. Su mentoría y apertura hicieron posible imaginar Andes JX no solo como una herramienta de producción musical, sino también como un producto académico, abriendo la puerta a su presentación en contextos de investigación.

#### Familia

**Guillermo López y Teresa Pilco** — por su apoyo incondicional a lo largo de este trabajo, incluso cuando gran parte de lo que implica permanece opaco para ellos. Los proyectos independientes de largo plazo que este manual documenta solo son sostenibles con ese tipo de base.

**Fabián Jácome** — cofundador de NoiseRoomUIO y primo del autor. Aportó retroalimentación sobre diseño GUI durante el desarrollo y ha sido un colaborador constante dentro del contexto más amplio de NoiseRoomUIO.

#### Testing

Andes JX fue probado durante el desarrollo por el autor y por Valeria Villarreal-Villarreal. Se espera realizar un beta testing más amplio en futuras versiones; si deseas participar, la sección de Issues del repositorio está abierta para retroalimentación.

[Issues de Andes-JX en GitHub](https://github.com/bansky0/Andes-JX/issues)

#### Comunidades

**La comunidad JUCE** — por sostener el framework sobre el que Andes JX está construido, y por los foros y ejemplos de código que han servido como referencia durante el desarrollo.

**La comunidad The Audio Programmer** — el Discord y canal de YouTube de Joshua Hodge han sido un punto de referencia para el desarrollo de plugins de audio, especialmente para desarrolladores hispanohablantes acercándose al tema. La comunidad que él sostiene, junto con la presencia de Matthijs Hollemans en ella, ha hecho el aprendizaje de programación de audio considerablemente más accesible.

**Ear Candy Technologies** — empresa independiente de plugins basada en Ciudad de México que ha construido activamente comunidad alrededor del desarrollo de plugins de audio para desarrolladores latinoamericanos e hispanohablantes. Sus workshops educativos abiertos y repositorios open-source (incluyendo la serie *intro-audio-plugins*) han contribuido a reducir la barrera de entrada para desarrolladores de habla hispana que ingresan a este campo. Andes JX forma parte del mismo movimiento más amplio que ellos han ayudado a impulsar.

#### Linaje open-source

Andes JX existe dentro de una cadena de proyectos open-source de sintetizadores que han permitido la transmisión de conocimiento entre generaciones. Corresponde reconocer ese linaje:

**Paul Kellett** — autor de **MDA JX10**, un sintetizador freeware VST lanzado a inicios de los años 2000 como parte del paquete de plugins MDA. Kellett liberó el código fuente como open source, un acto de generosidad que ha permitido la propagación de conocimiento sobre síntesis durante más de dos décadas. Gran parte de los tutoriales modernos de síntesis virtual analógica dentro del ecosistema JUCE pueden rastrearse indirectamente hasta este gesto.

**Matthijs Hollemans** — modernizó el código de MDA JX10, lo portó a JUCE y C++ moderno, y lo utilizó como proyecto ejemplo para su libro *Creating Synthesizer Plug-Ins with C++ and JUCE* (The Audio Programmer, 2023). Su proyecto, llamado **JX11**, es la referencia metodológica directa detrás de Andes JX. El patrón de documentación bilingüe, el enfoque del layout de parámetros y varias implementaciones DSP dentro de Andes JX toman JX11 como punto de partida. El código fuente del proyecto de Hollemans se encuentra en:

[BuildASynthPluginBook GitHub Repository](https://github.com/TheAudioProgrammer/BuildASynthPluginBook)

y se distribuye bajo licencia MIT.

El “JX” en **Andes JX** carga este doble linaje: un homenaje a los sintetizadores hardware Roland JX-8P y JX-10 de los años 80, y una continuación de la línea pedagógica open-source JX10 → JX11. El nombre es pequeño, pero la genealogía detrás de él es real.

---

### 12.4 Referencias

#### Fundamentos metodológicos

Estos dos libros fueron las referencias metodológicas principales del proyecto:

- Hollemans, M. (2023). *Creating Synthesizer Plug-Ins with C++ and JUCE*. The Audio Programmer.
- Pirkle, W. C. (2021). *Designing Software Synthesizer Plug-Ins in C++ with Audio DSP*. 2nd edition, Routledge.

#### Algoritmos de estado del arte implementados en el código

Cada una de estas referencias aparece citada en el archivo fuente donde se aplica la técnica correspondiente:

- Välimäki, V., & Huovilainen, A. (2007). *Antialiasing Oscillators in Subtractive Synthesis*. IEEE Signal Processing Magazine, 24(2), 116-125. → Técnica PolyBLEP utilizada en los osciladores (`OscillatorPolyBLEP.cpp`).
- Huovilainen, A. (2004). *Non-Linear Digital Implementation of the Moog Ladder Filter*. Proceedings of DAFx-04, Naples. → Modelo de filtro Moog con saturación no lineal (`LadderFilter.h`).
- Simper, A. (2013). *Linear Trapezoidal State Variable Filter SVF in state increment form*. Cytomic Technical Papers. → Modelo SVF topology-preserving (`SVF.h`).
- Stilson, T., & Smith, J. (1996). *Alias-Free Digital Synthesis of Classic Analog Waveforms*. Proceedings of the International Computer Music Conference (ICMC). → Técnica BLIT-DSF preservada como alternativa histórica en `Oscillator.h`.

#### Proyectos open-source referenciados

- Kellett, P. (inicios de los años 2000). *MDA JX10*. Parte del paquete MDA plug-in suite, liberado como open-source por su autor. Fundación histórica del linaje JX10 → JX11 → Andes JX.
- Hollemans, M. (2023). *JX11*. Código fuente que acompaña *Creating Synthesizer Plug-Ins with C++ and JUCE*.

[BuildASynthPluginBook GitHub Repository](https://github.com/TheAudioProgrammer/BuildASynthPluginBook)

Licencia MIT.

#### Frameworks y herramientas

- **JUCE** — framework multiplataforma en C++ para aplicaciones y plugins de audio.

[JUCE Official Website](https://juce.com)

- **Projucer** — herramienta de gestión de proyectos de JUCE, utilizada para administrar la configuración de compilación de Andes JX.

---

### 12.5 Licencia y distribución

Andes JX se distribuye bajo la licencia **GNU General Public License v3 (GPL v3)**.

Esto significa que eres libre de:

- Utilizar el software para cualquier propósito.
- Estudiar cómo funciona y modificarlo.
- Distribuir copias del software original.
- Distribuir versiones modificadas.

Con la condición de que cualquier trabajo derivado también sea distribuido bajo GPL v3, preservando la libertad del software para otros usuarios (el llamado efecto *copyleft*).

Para los detalles legales completos, consulta el archivo `LICENSE` dentro del repositorio.

---

### 12.6 Contacto y soporte

Para reportar bugs, sugerir funcionalidades o realizar preguntas técnicas sobre el código, utiliza la sección de Issues del repositorio de GitHub. Mantener el soporte técnico centralizado en GitHub permite que las respuestas permanezcan públicas y puedan beneficiar también a otros usuarios.

[Issues de Andes-JX en GitHub](https://github.com/bansky0/Andes-JX/issues)

Para noticias sobre NoiseRoomUIO, futuros lanzamientos de plugins y contenido relacionado con sound design:

📷 **Instagram**:  
[@noiseroom.uio](https://www.instagram.com/noiseroom.uio/)

---

<div align="center">

*Fin del Manual de Usuario de Andes JX 1.0.*

**Desarrollado en Quito, Ecuador — a 2.850 metros sobre el nivel del mar.**

</div>