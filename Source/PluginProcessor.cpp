#include "PluginProcessor.h"
#include "PluginEditor.h"

// Define the two knobs: Rate (Hz) and Depth (0–1)
static juce::AudioProcessorValueTreeState::ParameterLayout createParameters()
{
    return {
        std::make_unique<juce::AudioParameterFloat>(
            "rate",          // parameter ID
            "Rate",          // display name
            juce::NormalisableRange<float>(0.1f, 20.0f, 0.01f),  // min, max, step
            4.0f             // default: 4 Hz
        ),
        std::make_unique<juce::AudioParameterFloat>(
            "depth",
            "Depth",
            juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
            0.8f             // default: 80% depth
        )
    };
}

TremoloProcessor::TremoloProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input",  juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "Parameters", createParameters())
{
}

void TremoloProcessor::prepareToPlay(double sampleRate, int /*samplesPerBlock*/)
{
    currentSampleRate = sampleRate;
    lfoPhase = 0.0f;  // reset LFO when playback starts
}

void TremoloProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                    juce::MidiBuffer& /*midiMessages*/)
{
    // Read current knob values
    float rate  = *parameters.getRawParameterValue("rate");
    float depth = *parameters.getRawParameterValue("depth");

    // How much the LFO phase advances each sample
    float phaseIncrement = (2.0f * juce::MathConstants<float>::pi * rate)
                           / (float)currentSampleRate;

    int numChannels = buffer.getNumChannels();
    int numSamples  = buffer.getNumSamples();

    for (int sample = 0; sample < numSamples; ++sample)
    {
        // LFO outputs a value from (1 - depth) to 1.0
        // When depth=1: gain swings 0 → 1 (full tremolo)
        // When depth=0: gain stays 1.0 (bypassed)
        float lfoValue = 1.0f - depth * 0.5f * (1.0f - std::sin(lfoPhase));

        // Apply the same gain to every channel at this sample
        for (int channel = 0; channel < numChannels; ++channel)
        {
            float* channelData = buffer.getWritePointer(channel);
            channelData[sample] *= lfoValue;
        }

        // Advance the LFO and wrap it at 2π to prevent float overflow
        lfoPhase += phaseIncrement;
        if (lfoPhase >= 2.0f * juce::MathConstants<float>::pi)
            lfoPhase -= 2.0f * juce::MathConstants<float>::pi;
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TremoloProcessor();
}

juce::AudioProcessorEditor* TremoloProcessor::createEditor()
{
    return new TremoloEditor(*this);
}
