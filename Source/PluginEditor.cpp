#include "PluginEditor.h"

TremoloEditor::TremoloEditor(TremoloProcessor& p)
    : AudioProcessorEditor(&p),
      processor(p),
      rateAttachment(p.parameters, "rate",  rateSlider),
      depthAttachment(p.parameters, "depth", depthSlider)
{
    setSize(300, 200);

    // Rate knob
    rateSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    rateSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    addAndMakeVisible(rateSlider);

    rateLabel.setText("Rate (Hz)", juce::dontSendNotification);
    rateLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(rateLabel);

    // Depth knob
    depthSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    depthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    addAndMakeVisible(depthSlider);

    depthLabel.setText("Depth", juce::dontSendNotification);
    depthLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(depthLabel);
}

void TremoloEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(30, 30, 30));  // dark background

    g.setColour(juce::Colours::white);
    g.setFont(18.0f);
    g.drawFittedText("Tremolo", getLocalBounds().removeFromTop(40),
                     juce::Justification::centred, 1);
}

void TremoloEditor::resized()
{
    // Split the window in half — one knob per side
    auto area = getLocalBounds().reduced(10);
    area.removeFromTop(40);  // leave room for title

    auto leftHalf  = area.removeFromLeft(area.getWidth() / 2);
    auto rightHalf = area;

    rateLabel.setBounds(leftHalf.removeFromTop(20));
    rateSlider.setBounds(leftHalf);

    depthLabel.setBounds(rightHalf.removeFromTop(20));
    depthSlider.setBounds(rightHalf);
}
