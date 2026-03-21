#pragma once
#include <JuceHeader.h>

class TremoloProcessor : public juce::AudioProcessor
{
public:
    TremoloProcessor();
    ~TremoloProcessor() override = default;

    // Called once before playback starts — use to initialise state
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}

    // Called repeatedly with a buffer of audio to process
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    // Plugin metadata
    const juce::String getName() const override { return "Tremolo"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    // Preset handling (keep minimal for now)
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return "Default"; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override {}
    void setStateInformation(const void*, int) override {}

    // Editor
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    // The two knobs exposed to the DAW
    juce::AudioProcessorValueTreeState parameters;

private:
    // Tracks the current phase of the LFO sine wave (0 to 2π)
    float lfoPhase = 0.0f;
    double currentSampleRate = 44100.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TremoloProcessor)
};
