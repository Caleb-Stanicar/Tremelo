#include "PluginEditor.h"

TremoloEditor::TremoloEditor(TremoloProcessor& p)
    : AudioProcessorEditor(&p),
      processor(p),
      syncAttachment       (p.parameters, "sync",        syncButton),
      rateAttachment       (p.parameters, "rate",        rateSlider),
      divisionAttachment   (p.parameters, "division",    divisionBox),
      depthAttachment      (p.parameters, "depth",       depthSlider),
      wahEnabledAttachment (p.parameters, "wahEnabled",  wahEnabledButton),
      wahDepthAttachment   (p.parameters, "wahDepth",    wahDepthSlider),
      wahDivisionAttachment(p.parameters, "wahDivision", wahDivisionBox),
      wahOffsetAttachment  (p.parameters, "wahOffset",   wahOffsetBox)
{
    setSize(300, 460);

    // --- Tremolo ---
    syncButton.addListener(this);
    addAndMakeVisible(syncButton);

    rateSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    rateSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    addAndMakeVisible(rateSlider);

    rateLabel.setText("Rate (Hz)", juce::dontSendNotification);
    rateLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(rateLabel);

    divisionBox.addItemList(TremoloProcessor::divisionNames, 1);
    addAndMakeVisible(divisionBox);

    depthSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    depthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    addAndMakeVisible(depthSlider);

    depthLabel.setText("Depth", juce::dontSendNotification);
    depthLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(depthLabel);

    // --- Wah ---
    wahSectionLabel.setText("Wah", juce::dontSendNotification);
    wahSectionLabel.setJustificationType(juce::Justification::centred);
    wahSectionLabel.setFont(juce::FontOptions(15.0f).withStyle("Bold"));
    addAndMakeVisible(wahSectionLabel);

    wahEnabledButton.addListener(this);
    addAndMakeVisible(wahEnabledButton);

    wahDepthSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    wahDepthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    addAndMakeVisible(wahDepthSlider);

    wahDepthLabel.setText("Depth", juce::dontSendNotification);
    wahDepthLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(wahDepthLabel);

    wahDivisionBox.addItemList(TremoloProcessor::wahDivisionNames, 1);
    addAndMakeVisible(wahDivisionBox);

    wahDivisionLabel.setText("Rate", juce::dontSendNotification);
    wahDivisionLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(wahDivisionLabel);

    wahOffsetBox.addItemList(TremoloProcessor::wahOffsetNames, 1);
    addAndMakeVisible(wahOffsetBox);

    wahOffsetLabel.setText("Offset", juce::dontSendNotification);
    wahOffsetLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(wahOffsetLabel);

    updateRateControls();
}

TremoloEditor::~TremoloEditor()
{
    syncButton.removeListener(this);
    wahEnabledButton.removeListener(this);
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

    // Divider between tremolo and wah sections
    g.setColour(juce::Colour(80, 80, 80));
    g.drawHorizontalLine(222, 10.0f, (float)(getWidth() - 10));
}

void TremoloEditor::resized()
{
    auto area = getLocalBounds().reduced(10);
    area.removeFromTop(40); // title

    // --- Tremolo section ---
    syncButton.setBounds(area.removeFromTop(24));
    area.removeFromTop(8);

    auto tremoloArea = area.removeFromTop(130);
    {
        auto left  = tremoloArea.removeFromLeft(tremoloArea.getWidth() / 2);
        auto right = tremoloArea;

        rateLabel.setBounds(left.removeFromTop(20));
        rateSlider.setBounds(left);
        divisionBox.setBounds(rateLabel.getBounds().getUnion(rateSlider.getBounds()).reduced(0, 30));

        depthLabel.setBounds(right.removeFromTop(20));
        depthSlider.setBounds(right);
    }

    area.removeFromTop(20); // space for divider line

    // --- Wah section ---
    wahSectionLabel.setBounds(area.removeFromTop(20));
    area.removeFromTop(4);

    wahEnabledButton.setBounds(area.removeFromTop(24));
    area.removeFromTop(8);

    auto wahKnobArea = area.removeFromTop(100);
    {
        auto left  = wahKnobArea.removeFromLeft(wahKnobArea.getWidth() / 2);
        auto right = wahKnobArea;

        wahDepthLabel.setBounds(left.removeFromTop(20));
        wahDepthSlider.setBounds(left);

        wahDivisionLabel.setBounds(right.removeFromTop(20));
        wahDivisionBox.setBounds(right.removeFromTop(24));
    }

    area.removeFromTop(4);
    wahOffsetLabel.setBounds(area.removeFromTop(20));
    wahOffsetBox.setBounds(area.removeFromTop(24));
}
