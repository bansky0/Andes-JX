cmake_minimum_required(VERSION 3.22)

project(AndesJX VERSION 1.0.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(juce
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.8
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(juce)

# Identity of the original plugin. Verify the manufacturer code against existing builds.
set(ANDES_FORMATS VST3)
if(APPLE)
    list(APPEND ANDES_FORMATS AU)
endif()

juce_add_plugin(AndesJX
    COMPANY_NAME "NoiseRoomUIO"
    BUNDLE_ID "com.noiseroomuio.andesjx"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    PLUGIN_MANUFACTURER_CODE Manu
    PLUGIN_CODE NRAN
    FORMATS ${ANDES_FORMATS}
    COPY_PLUGIN_AFTER_BUILD FALSE
    PRODUCT_NAME "Andes-JX"
)
juce_generate_juce_header(AndesJX)

target_sources(AndesJX PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/Synth.cpp
    Source/OscillatorPolyBLEP.cpp
    Source/OscillatorDPW.cpp
    Source/LookAndFeel/ToggleLookAndFeel.cpp
)

juce_add_binary_data(AndesJXAssets SOURCES
    Assets/AndesFaderADSR.png
    Assets/AndesKnobPrincipal2.png
    Assets/AndesKnobSecondary.png
    Assets/backgroundAndesJX.png
)

target_link_libraries(AndesJX PRIVATE
    AndesJXAssets
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_core
    juce::juce_data_structures
    juce::juce_dsp
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags
)

target_compile_definitions(AndesJX PRIVATE
    JUCE_STRICT_REFCOUNTEDPOINTER=1
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
)
