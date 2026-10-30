#pragma once
#include "PluginProcessor.h"

class BassBoxxDigitalOneAudioProcessorEditor final : public juce::AudioProcessorEditor {
public:
    explicit BassBoxxDigitalOneAudioProcessorEditor(BassBoxxDigitalOneAudioProcessor&);
    void paint(juce::Graphics&) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent&) override;
    void mouseDrag(const juce::MouseEvent&) override;
private:
    BassBoxxDigitalOneAudioProcessor& processor;
    juce::Slider pickup, sub, drive, pluck, slide, wah;
    juce::ComboBox subWave;
    juce::Label pickupLabel, subLabel, driveLabel, pluckLabel, slideLabel, wahLabel, subWaveLabel, title, subtitle;
    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    std::unique_ptr<Attachment> pickupAttach, subAttach, driveAttach, pluckAttach, slideAttach, wahAttach;
    std::unique_ptr<ComboAttachment> subWaveAttach;
    void styleSlider(juce::Slider&, juce::Label&, const juce::String&);
    void setPickupFromMouse(juce::Point<float>);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BassBoxxDigitalOneAudioProcessorEditor)
};
