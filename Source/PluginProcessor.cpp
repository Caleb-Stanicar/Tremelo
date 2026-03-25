#include "PluginProcessor.h"
#include "PluginEditor.h"

const juce::StringArray TremoloProcessor::divisionNames { "1/32", "1/16", "1/8", "1/4", "1/2", "1/1", "2/1" };
const float TremoloProcessor::divisionMultipliers[numDivisions] { 0.125f, 0.25f, 0.5f, 1.0f, 2.0f, 4.0f, 8.0f };

const juce::StringArray TremoloProcessor::wahDivisionNames { "1/16", "1/8", "1/4", "1/2", "1/1" };
const float TremoloProcessor::wahDivisionMultipliers[numWahDivisions] { 0.25f, 0.5f, 1.0f, 2.0f, 4.0f };

const juce::StringArray TremoloProcessor::wahOffsetNames { "None", "1/16", "1/8", "1/4" };
const float TremoloProcessor::wahOffsetBeats[numWahOffsets] { 0.0f, 0.25f, 0.5f, 1.0f };

static juce::AudioProcessorValueTreeState::ParameterLayout createParameters()
{
    return {
        // --- Tremolo ---
        std::make_unique<juce::AudioParameterBool>(
            "sync", "Sync to BPM", false
        ),
        std::make_unique<juce::AudioParameterFloat>(
            "rate", "Rate",
            juce::NormalisableRange<float>(0.1f, 20.0f, 0.01f),
            4.0f
        ),
        std::make_unique<juce::AudioParameterChoice>(
            "division", "Division",
            TremoloProcessor::divisionNames,
            3   // default: "1/4"
        ),
        std::make_unique<juce::AudioParameterFloat>(
            "depth", "Depth",
            juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
            0.8f
        ),
        // --- Wah ---
        std::make_unique<juce::AudioParameterBool>(
            "wahEnabled", "Wah Enabled", false
        ),
        std::make_unique<juce::AudioParameterFloat>(
            "wahDepth", "Wah Depth",
            juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
            0.8f
        ),
        std::make_unique<juce::AudioParameterChoice>(
            "wahDivision", "Wah Division",
            TremoloProcessor::wahDivisionNames,
            2   // default: "1/4"
        ),
        std::make_unique<juce::AudioParameterChoice>(
            "wahOffset", "Wah Offset",
            TremoloProcessor::wahOffsetNames,
            0   // default: "None"
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
    wahPhase = 0.0f;
    svfLow[0]  = svfLow[1]  = 0.0f;
    svfBand[0] = svfBand[1] = 0.0f;
}

void TremoloProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                    juce::MidiBuffer&)
{
    // Always read BPM — needed for tremolo sync and wah
    if (auto* playHead = getPlayHead())
        if (auto pos = playHead->getPosition())
            if (auto bpm = pos->getBpm())
                currentBpm = *bpm;

    const float beatsPerSec = (float)currentBpm / 60.0f;
    const int   numChannels = buffer.getNumChannels();
    const int   numSamples  = buffer.getNumSamples();

    // -------------------------------------------------------------------------
    // Tremolo / Auto-pan
    // -------------------------------------------------------------------------
    {
        bool  sync  = *parameters.getRawParameterValue("sync") > 0.5f;
        float depth = *parameters.getRawParameterValue("depth");

        float rateHz;
        if (sync)
        {
            int   divIndex      = (int)*parameters.getRawParameterValue("division");
            float beatsPerCycle = divisionMultipliers[divIndex];
            rateHz = beatsPerSec / beatsPerCycle;
        }
        else
        {
            rateHz = *parameters.getRawParameterValue("rate");
        }

        const float phaseInc = (2.0f * juce::MathConstants<float>::pi * rateHz)
                               / (float)currentSampleRate;

        for (int s = 0; s < numSamples; ++s)
        {
            float lfoLeft  = 1.0f - depth * 0.5f * (1.0f - std::sin(lfoPhase));
            float lfoRight = 1.0f - depth * 0.5f * (1.0f - std::sin(lfoPhase + juce::MathConstants<float>::pi));

            for (int ch = 0; ch < numChannels; ++ch)
            {
                float* data = buffer.getWritePointer(ch);
                data[s] *= (ch == 0) ? lfoLeft : lfoRight;
            }

            lfoPhase += phaseInc;
            if (lfoPhase >= 2.0f * juce::MathConstants<float>::pi)
                lfoPhase -= 2.0f * juce::MathConstants<float>::pi;
        }
    }

    // -------------------------------------------------------------------------
    // Wah — state variable filter with LFO-swept center frequency
    //
    // The SVF produces a bandpass output. Its center frequency sweeps
    // exponentially between lowFreq and highFreq based on depth + LFO.
    // Q controls the resonant peak width (classic wah character).
    // -------------------------------------------------------------------------
    bool wahEnabled = *parameters.getRawParameterValue("wahEnabled") > 0.5f;

    if (wahEnabled)
    {
        const float wahDepth = *parameters.getRawParameterValue("wahDepth");

        const int   wahDivIdx      = (int)*parameters.getRawParameterValue("wahDivision");
        const float wahBeatsPerCycle = wahDivisionMultipliers[wahDivIdx];
        const float wahRateHz      = beatsPerSec / wahBeatsPerCycle;
        const float wahPhaseInc    = (2.0f * juce::MathConstants<float>::pi * wahRateHz)
                                     / (float)currentSampleRate;

        const int   wahOffsetIdx = (int)*parameters.getRawParameterValue("wahOffset");
        const float offsetPhase  = (wahOffsetBeats[wahOffsetIdx] / wahBeatsPerCycle)
                                   * 2.0f * juce::MathConstants<float>::pi;

        // Frequency sweep range — adjust these to taste
        constexpr float lowFreq  = 300.0f;
        constexpr float highFreq = 3000.0f;
        const float     midFreq  = std::sqrt(lowFreq * highFreq);   // geometric centre
        const float     logRange = std::log(highFreq / lowFreq);

        // Q controls resonant peak. Higher = more pronounced wah character.
        constexpr float q = 4.0f;

        for (int s = 0; s < numSamples; ++s)
        {
            // LFO: 0 → 1 sine wave
            const float lfo = 0.5f * (1.0f + std::sin(wahPhase + offsetPhase));

            // Exponential frequency sweep: sounds musical, mirrors how we hear pitch
            const float centerFreq = midFreq * std::exp(wahDepth * (lfo - 0.5f) * logRange);

            // SVF coefficient (Chamberlin topology, valid for fc << fs)
            const float f = 2.0f * std::sin(juce::MathConstants<float>::pi * centerFreq
                                            / (float)currentSampleRate);

            for (int ch = 0; ch < numChannels; ++ch)
            {
                float* data  = buffer.getWritePointer(ch);
                float  input = data[s];

                // Chamberlin state variable filter:
                //   high = input - low - Q*band  (highpass)
                //   band += f * high              (bandpass, used as output)
                //   low  += f * band              (lowpass)
                float high    = input - svfLow[ch] - q * svfBand[ch];
                svfBand[ch]  += f * high;
                svfLow[ch]   += f * svfBand[ch];

                data[s] = svfBand[ch];  // bandpass output = the wah
            }

            wahPhase += wahPhaseInc;
            if (wahPhase >= 2.0f * juce::MathConstants<float>::pi)
                wahPhase -= 2.0f * juce::MathConstants<float>::pi;
        }
    }
    else
    {
        // Clear filter memory so it starts clean next time wah is enabled
        svfLow[0] = svfLow[1] = svfBand[0] = svfBand[1] = 0.0f;
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
