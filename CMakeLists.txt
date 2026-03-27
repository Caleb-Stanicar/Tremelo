# Tremolo / Auto-Pan / Wah audio plugin
cmake_minimum_required(VERSION 3.22)
project(Tremolo VERSION 0.1.0)

# Tell CMake where JUCE lives
add_subdirectory(JUCE)

juce_add_plugin(Tremolo
    COMPANY_NAME "MyPlugins"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    PLUGIN_MANUFACTURER_CODE Mypl
    PLUGIN_CODE Trem
    FORMATS VST3 AU
    PRODUCT_NAME "Tremolo Auto-Pan Wah"
)

juce_generate_juce_header(Tremolo)

target_sources(Tremolo
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
)

target_compile_features(Tremolo PRIVATE cxx_std_17)

target_compile_definitions(Tremolo PUBLIC JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(Tremolo
    PRIVATE
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
