#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class TremoloEditor : public juce::AudioProcessorEditor,
                      private juce::Button::Listener
{
public:
    TremoloEditor(TremoloProcessor&);
    ~TremoloEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void buttonClicked(juce::Button*) override;
    void updateRateControls(); // show/hide rate vs division based on sync toggle

    TremoloProcessor& processor;

    juce::ToggleButton syncButton { "Sync to BPM" };
    juce::Slider       rateSlider;
    juce::ComboBox     divisionBox;
    juce::Slider       depthSlider;
    juce::Label        rateLabel;
    juce::Label        depthLabel;

    juce::AudioProcessorValueTreeState::ButtonAttachment   syncAttachment;
    juce::AudioProcessorValueTreeState::SliderAttachment   rateAttachment;
    juce::AudioProcessorValueTreeState::ComboBoxAttachment divisionAttachment;
    juce::AudioProcessorValueTreeState::SliderAttachment   depthAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TremoloEditor)
};
