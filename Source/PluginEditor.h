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
    void updateRateControls();

    TremoloProcessor& processor;

    // --- Tremolo controls ---
    juce::ToggleButton syncButton { "Sync to BPM" };
    juce::Slider       rateSlider;
    juce::ComboBox     divisionBox;
    juce::Slider       depthSlider;
    juce::Label        rateLabel;
    juce::Label        depthLabel;

    // --- Wah controls ---
    juce::ToggleButton wahEnabledButton { "Wah On" };
    juce::Slider       wahDepthSlider;
    juce::Label        wahDepthLabel;
    juce::ComboBox     wahDivisionBox;
    juce::Label        wahDivisionLabel;
    juce::ComboBox     wahOffsetBox;
    juce::Label        wahOffsetLabel;
    juce::Label        wahSectionLabel;

    // --- Tremolo attachments ---
    juce::AudioProcessorValueTreeState::ButtonAttachment   syncAttachment;
    juce::AudioProcessorValueTreeState::SliderAttachment   rateAttachment;
    juce::AudioProcessorValueTreeState::ComboBoxAttachment divisionAttachment;
    juce::AudioProcessorValueTreeState::SliderAttachment   depthAttachment;

    // --- Wah attachments ---
    juce::AudioProcessorValueTreeState::ButtonAttachment   wahEnabledAttachment;
    juce::AudioProcessorValueTreeState::SliderAttachment   wahDepthAttachment;
    juce::AudioProcessorValueTreeState::ComboBoxAttachment wahDivisionAttachment;
    juce::AudioProcessorValueTreeState::ComboBoxAttachment wahOffsetAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TremoloEditor)
};
