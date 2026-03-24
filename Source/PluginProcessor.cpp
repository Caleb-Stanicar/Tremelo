#include "PluginProcessor.h"
#include "PluginEditor.h"

const juce::StringArray TremoloProcessor::divisionNames { "1/32", "1/16", "1/8", "1/4", "1/2", "1/1", "2/1" };
const float TremoloProcessor::divisionMultipliers[numDivisions] { 0.125f, 0.25f, 0.5f, 1.0f, 2.0f, 4.0f, 8.0f };

static juce::AudioProcessorValueTreeState::ParameterLayout createParameters()
{
    return {
        std::make_unique<juce::AudioParameterBool>(
            "sync", "Sync to BPM", false   // default: off (free Hz mode)
        ),
        std::make_unique<juce::AudioParameterFloat>(
            "rate", "Rate",
            juce::NormalisableRange<float>(0.1f, 20.0f, 0.01f),
            4.0f   // default: 4 Hz
        ),
        std::make_unique<juce::AudioParameterChoice>(
            "division", "Division",
            TremoloProcessor::divisionNames,
            3      // default index 3 = "1/4"
        ),
        std::make_unique<juce::AudioParameterFloat>(
            "depth", "Depth",
            juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
            0.8f
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

void TremoloProcessor::prepareToPlay(double sampleRate, int)
{
    currentSampleRate = sampleRate;
    lfoPhase = 0.0f;
}

void TremoloProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                    juce::MidiBuffer&)
{
    bool  sync  = *parameters.getRawParameterValue("sync") > 0.5f;
    float depth = *parameters.getRawParameterValue("depth");

    float rateHz;

    if (sync)
    {
        // Read BPM from the DAW — fall back to 120 if unavailable
        if (auto* playHead = getPlayHead())
            if (auto pos = playHead->getPosition())
                if (auto bpm = pos->getBpm())
                    currentBpm = *bpm;

        int   divIndex      = (int) *parameters.getRawParameterValue("division");
        float beatsPerCycle = divisionMultipliers[divIndex];
        float beatsPerSec   = (float) currentBpm / 60.0f;
        rateHz = beatsPerSec / beatsPerCycle;
    }
    else
    {
        rateHz = *parameters.getRawParameterValue("rate");
    }

    float phaseIncrement = (2.0f * juce::MathConstants<float>::pi * rateHz)
                           / (float) currentSampleRate;

    int numChannels = buffer.getNumChannels();
    int numSamples  = buffer.getNumSamples();

    for (int sample = 0; sample < numSamples; ++sample)
    {
        // Left channel: LFO as normal
        // Right channel: LFO offset by π (half cycle), so panning sweeps left↔right
        float lfoLeft  = 1.0f - depth * 0.5f * (1.0f - std::sin(lfoPhase));
        float lfoRight = 1.0f - depth * 0.5f * (1.0f - std::sin(lfoPhase + juce::MathConstants<float>::pi));

        for (int channel = 0; channel < numChannels; ++channel)
        {
            float* channelData = buffer.getWritePointer(channel);
            float  lfoValue    = (channel == 0) ? lfoLeft : lfoRight;
            channelData[sample] *= lfoValue;
        }

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
