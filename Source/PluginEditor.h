#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class TremoloEditor : public juce::AudioProcessorEditor
{
public:
    TremoloEditor(TremoloProcessor&);
    ~TremoloEditor() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    TremoloProcessor& processor;

    // The two knobs
    juce::Slider rateSlider;
    juce::Slider depthSlider;
    juce::Label  rateLabel;
    juce::Label  depthLabel;

    // Attachments keep sliders in sync with the AudioProcessorValueTreeState
    juce::AudioProcessorValueTreeState::SliderAttachment rateAttachment;
    juce::AudioProcessorValueTreeState::SliderAttachment depthAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TremoloEditor)
};
