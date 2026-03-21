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
    void getStateInformation(juce::MemoryBlock&) override {}
    void setStateInformation(const void*, int) override {}

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    juce::AudioProcessorValueTreeState parameters;

    // Note division options (shared with editor for building the ComboBox)
    static constexpr int numDivisions = 7;
    static const juce::StringArray divisionNames;
    static const float divisionMultipliers[numDivisions]; // beats per LFO cycle

private:
    float lfoPhase = 0.0f;
    double currentSampleRate = 44100.0;
    double currentBpm = 120.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TremoloProcessor)
};
