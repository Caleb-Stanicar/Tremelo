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

static float divisionToHz(int index, float beatsPerSec, const float* multipliers)
{
    return beatsPerSec / multipliers[index];
}

TremoloProcessor::TremoloProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input",  juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "Parameters", createParameters())
{
    syncParam        = parameters.getRawParameterValue("sync");
    rateParam        = parameters.getRawParameterValue("rate");
    divisionParam    = parameters.getRawParameterValue("division");
    depthParam       = parameters.getRawParameterValue("depth");
    wahEnabledParam  = parameters.getRawParameterValue("wahEnabled");
    wahDepthParam    = parameters.getRawParameterValue("wahDepth");
    wahDivisionParam = parameters.getRawParameterValue("wahDivision");
    wahOffsetParam   = parameters.getRawParameterValue("wahOffset");
}

void TremoloProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void TremoloProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState && xmlState->hasTagName(parameters.state.getType()))
        parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
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
        bool  sync  = *syncParam > 0.5f;
        float depth = *depthParam;

        float rateHz;
        if (sync)
            rateHz = divisionToHz((int)*divisionParam, beatsPerSec, divisionMultipliers);
        else
            rateHz = *rateParam;

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

            lfoPhase = std::fmod(lfoPhase + phaseInc, juce::MathConstants<float>::twoPi);
        }
    }

    // -------------------------------------------------------------------------
    // Wah — state variable filter with LFO-swept center frequency
    //
    // The SVF produces a bandpass output. Its center frequency sweeps
    // exponentially between lowFreq and highFreq based on depth + LFO.
    // Q controls the resonant peak width (classic wah character).
    // -------------------------------------------------------------------------
    bool wahEnabled = *wahEnabledParam > 0.5f;

    if (wahEnabled)
    {
        const float wahDepth = *wahDepthParam;

        const float wahRateHz   = divisionToHz((int)*wahDivisionParam, beatsPerSec, wahDivisionMultipliers);
        const float wahPhaseInc = (2.0f * juce::MathConstants<float>::pi * wahRateHz)
                                  / (float)currentSampleRate;

        const int   wahOffsetIdx    = (int)*wahOffsetParam;
        const float wahBeatsPerCycle = wahDivisionMultipliers[(int)*wahDivisionParam];
        const float offsetPhase     = (wahOffsetBeats[wahOffsetIdx] / wahBeatsPerCycle)
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

            // Clamp to safe range before computing SVF coefficient —
            // prevents f > 2 which breaks Chamberlin topology stability
            const float safeFreq = juce::jlimit(20.0f, (float)currentSampleRate * 0.49f, centerFreq);

            // SVF coefficient (Chamberlin topology, valid for fc << fs)
            const float f = 2.0f * std::sin(juce::MathConstants<float>::pi * safeFreq
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

                // Clamp output — resonance (Q=4) can push peaks above 0 dBFS
                data[s] = juce::jlimit(-1.0f, 1.0f, svfBand[ch]);
            }

            wahPhase = std::fmod(wahPhase + wahPhaseInc, juce::MathConstants<float>::twoPi);
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
