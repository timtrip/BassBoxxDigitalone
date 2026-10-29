#pragma once
#include <JuceHeader.h>

class BossBassOneAudioProcessor final : public juce::AudioProcessor {
public:
    BossBassOneAudioProcessor();
    ~BossBassOneAudioProcessor() override = default;
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout&) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "Boss Bass One"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 4.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;

    juce::AudioProcessorValueTreeState parameters;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void pluck(float frequency, float velocity);
    float readDelay(float delaySamples) const noexcept;

    juce::AudioBuffer<float> stringBuffer;
    int writeIndex = 0;
    double currentSampleRate = 44100.0;
    float feedbackState = 0.0f;
    float currentFrequency = 55.0f;
    float targetFrequency = 55.0f;
    float amplitude = 0.0f;
    float amplitudeTarget = 0.0f;
    float subPhase = 0.0f;
    juce::Random random;
    juce::SmoothedValue<float> driveSmoother;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BossBassOneAudioProcessor)
};
