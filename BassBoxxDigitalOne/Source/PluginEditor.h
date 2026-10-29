#pragma once
#include "PluginProcessor.h"

class BassBoxxDigitalOneAudioProcessorEditor final : public juce::AudioProcessorEditor {
public:
    explicit BassBoxxDigitalOneAudioProcessorEditor(BassBoxxDigitalOneAudioProcessor&);
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    BassBoxxDigitalOneAudioProcessor& processor;
    juce::Slider pickup, sub, drive, pluck;
    juce::Label pickupLabel, subLabel, driveLabel, pluckLabel, title, subtitle;
    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<Attachment> pickupAttach, subAttach, driveAttach, pluckAttach;
    void styleSlider(juce::Slider&, juce::Label&, const juce::String&);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BassBoxxDigitalOneAudioProcessorEditor)
};
