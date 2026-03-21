#include "PluginEditor.h"

TremoloEditor::TremoloEditor(TremoloProcessor& p)
    : AudioProcessorEditor(&p),
      processor(p),
      syncAttachment    (p.parameters, "sync",     syncButton),
      rateAttachment    (p.parameters, "rate",     rateSlider),
      divisionAttachment(p.parameters, "division", divisionBox),
      depthAttachment   (p.parameters, "depth",    depthSlider)
{
    setSize(300, 220);

    syncButton.addListener(this);
    addAndMakeVisible(syncButton);

    // Rate knob (free Hz mode)
    rateSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    rateSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    addAndMakeVisible(rateSlider);

    rateLabel.setText("Rate (Hz)", juce::dontSendNotification);
    rateLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(rateLabel);

    // Division dropdown (sync mode)
    divisionBox.addItemList(TremoloProcessor::divisionNames, 1);
    addAndMakeVisible(divisionBox);

    // Depth knob
    depthSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    depthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    addAndMakeVisible(depthSlider);

    depthLabel.setText("Depth", juce::dontSendNotification);
    depthLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(depthLabel);

    updateRateControls();
}

TremoloEditor::~TremoloEditor()
{
    syncButton.removeListener(this);
}

void TremoloEditor::buttonClicked(juce::Button*)
{
    updateRateControls();
}

void TremoloEditor::updateRateControls()
{
    bool synced = syncButton.getToggleState();
    rateSlider.setVisible(!synced);
    rateLabel.setVisible(!synced);
    divisionBox.setVisible(synced);
}

void TremoloEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(30, 30, 30));
    g.setColour(juce::Colours::white);
    g.setFont(18.0f);
    g.drawFittedText("Tremolo", getLocalBounds().removeFromTop(40),
                     juce::Justification::centred, 1);
}

void TremoloEditor::resized()
{
    auto area = getLocalBounds().reduced(10);
    area.removeFromTop(40);

    // Sync toggle spans the full width at the top
    syncButton.setBounds(area.removeFromTop(24));
    area.removeFromTop(8);

    auto leftHalf  = area.removeFromLeft(area.getWidth() / 2);
    auto rightHalf = area;

    // Left side: rate knob (free) or division dropdown (sync) — same space
    rateLabel.setBounds(leftHalf.removeFromTop(20));
    rateSlider.setBounds(leftHalf);
    divisionBox.setBounds(rateLabel.getBounds().getUnion(rateSlider.getBounds()).reduced(0, 30));

    // Right side: depth knob
    depthLabel.setBounds(rightHalf.removeFromTop(20));
    depthSlider.setBounds(rightHalf);
}
