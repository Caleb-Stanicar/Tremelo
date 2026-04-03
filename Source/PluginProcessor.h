#pragma once
#include <JuceHeader.h>

class TremoloProcessor : public juce::AudioProcessor
{
public:
    TremoloProcessor();
    ~TremoloProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    const juce::String getName() const override { return "Tremolo"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return "Default"; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    juce::AudioProcessorValueTreeState parameters;

    // Tremolo division options
    static constexpr int numDivisions = 7;
    static const juce::StringArray divisionNames;
    static const float divisionMultipliers[numDivisions];

    // Wah division options: 1/16, 1/8, 1/4, 1/2, 1/1
    static constexpr int numWahDivisions = 5;
    static const juce::StringArray wahDivisionNames;
    static const float wahDivisionMultipliers[numWahDivisions]; // beats per LFO cycle

    // Wah offset options: none, 1/16, 1/8, 1/4 (in beats)
    static constexpr int numWahOffsets = 4;
    static const juce::StringArray wahOffsetNames;
    static const float wahOffsetBeats[numWahOffsets];

private:
    float lfoPhase = 0.0f;
    float wahPhase = 0.0f;
    double currentSampleRate = 44100.0;
    double currentBpm = 120.0;

    // State variable filter state per channel (for wah)
    float svfLow[2]  = { 0.0f, 0.0f };
    float svfBand[2] = { 0.0f, 0.0f };

    // Cached parameter pointers — avoids string lookups in processBlock
    std::atomic<float>* syncParam        = nullptr;
    std::atomic<float>* rateParam        = nullptr;
    std::atomic<float>* divisionParam    = nullptr;
    std::atomic<float>* depthParam       = nullptr;
    std::atomic<float>* wahEnabledParam  = nullptr;
    std::atomic<float>* wahDepthParam    = nullptr;
    std::atomic<float>* wahDivisionParam = nullptr;
    std::atomic<float>* wahOffsetParam   = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TremoloProcessor)
};
