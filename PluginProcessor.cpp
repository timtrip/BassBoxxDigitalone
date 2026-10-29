#include "PluginProcessor.h"
#include "PluginEditor.h"

BossBassOneAudioProcessor::BossBassOneAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "BOSS_BASS_ONE", createParameterLayout()) {}

juce::AudioProcessorValueTreeState::ParameterLayout BossBassOneAudioProcessor::createParameterLayout() {
    using namespace juce;
    std::vector<std::unique_ptr<RangedAudioParameter>> p;
    p.push_back(std::make_unique<AudioParameterFloat>("pickup", "Pickup Position", NormalisableRange<float>(0.05f, 0.95f, 0.001f), 0.72f));
    p.push_back(std::make_unique<AudioParameterFloat>("sub", "Sub", NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.35f));
    p.push_back(std::make_unique<AudioParameterFloat>("drive", "Drive", NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.12f));
    p.push_back(std::make_unique<AudioParameterFloat>("pluck", "Pluck", NormalisableRange<float>(0.15f, 1.0f, 0.001f), 0.68f));
    return { p.begin(), p.end() };
}

void BossBassOneAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    currentSampleRate = sampleRate;
    stringBuffer.setSize(1, juce::roundToInt(sampleRate / 20.0) + samplesPerBlock + 8);
    stringBuffer.clear();
    writeIndex = 0;
    feedbackState = amplitude = amplitudeTarget = 0.0f;
    driveSmoother.reset(sampleRate, 0.02);
}
void BossBassOneAudioProcessor::releaseResources() {}
bool BossBassOneAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
    const auto out = layouts.getMainOutputChannelSet();
    return out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo();
}

void BossBassOneAudioProcessor::pluck(float frequency, float velocity) {
    targetFrequency = juce::jlimit(20.0f, 1000.0f, frequency);
    currentFrequency = targetFrequency;
    const int length = juce::jlimit(2, stringBuffer.getNumSamples() - 2,
        juce::roundToInt(static_cast<float>(currentSampleRate) / currentFrequency));
    const float pick = parameters.getRawParameterValue("pluck")->load();
    for (int i = 0; i < length; ++i) {
        const float noise = random.nextFloat() * 2.0f - 1.0f;
        const float soften = 1.0f - pick * 0.34f;
        const float prev = stringBuffer.getSample(0, (writeIndex + stringBuffer.getNumSamples() - 1) % stringBuffer.getNumSamples());
        stringBuffer.setSample(0, writeIndex, noise * (0.65f + pick * 0.35f) + prev * soften * 0.12f);
        writeIndex = (writeIndex + 1) % stringBuffer.getNumSamples();
    }
    amplitudeTarget = juce::jlimit(0.0f, 1.0f, velocity);
    amplitude = 0.0f;
}

float BossBassOneAudioProcessor::readDelay(float delaySamples) const noexcept {
    const int size = stringBuffer.getNumSamples();
    float position = static_cast<float>(writeIndex) - delaySamples;
    while (position < 0.0f) position += static_cast<float>(size);
    const int i0 = static_cast<int>(position) % size;
    const int i1 = (i0 + 1) % size;
    const float frac = position - std::floor(position);
    return stringBuffer.getSample(0, i0) + frac * (stringBuffer.getSample(0, i1) - stringBuffer.getSample(0, i0));
}

void BossBassOneAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) {
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();
    const float pickup = parameters.getRawParameterValue("pickup")->load();
    const float subMix = parameters.getRawParameterValue("sub")->load();
    const float drive = parameters.getRawParameterValue("drive")->load();
    auto* left = buffer.getWritePointer(0);
    auto* right = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : left;
    auto event = midi.begin();
    const auto end = midi.end();
    for (int i = 0; i < buffer.getNumSamples(); ++i) {
        while (event != end && event->samplePosition <= i) {
            const auto msg = event->getMessage();
            if (msg.isNoteOn()) pluck(static_cast<float>(juce::MidiMessage::getMidiNoteInHertz(msg.getNoteNumber())), msg.getFloatVelocity());
            else if (msg.isNoteOff() && msg.getNoteNumber() == juce::roundToInt(juce::MidiMessage::getMidiNoteFromHertz(currentFrequency))) amplitudeTarget = 0.0f;
            ++event;
        }

        currentFrequency += (targetFrequency - currentFrequency) * 0.0008f;
        amplitude += (amplitudeTarget - amplitude) * (amplitudeTarget > amplitude ? 0.0035f : 0.000025f);
        const float period = static_cast<float>(currentSampleRate) / juce::jmax(20.0f, currentFrequency);
        const float body = readDelay(period * pickup) - readDelay(period * (pickup + 0.035f));
        const float loop = readDelay(period);
        feedbackState = loop * 0.498f + feedbackState * 0.498f;
        const float damp = 0.99925f - pickup * 0.0007f;
        stringBuffer.setSample(0, writeIndex, feedbackState * damp);
        writeIndex = (writeIndex + 1) % stringBuffer.getNumSamples();

        subPhase += juce::MathConstants<float>::twoPi * currentFrequency / static_cast<float>(currentSampleRate);
        if (subPhase >= juce::MathConstants<float>::twoPi) subPhase -= juce::MathConstants<float>::twoPi;
        const float sub = std::sin(subPhase) * amplitude;
        const float in = (body * 2.8f + sub * subMix * 0.72f) * amplitude;
        const float gain = 1.0f + drive * 18.0f;
        const float out = std::tanh(in * gain) / std::tanh(gain);
        left[i] = right[i] = out * 0.85f;
    }
}

juce::AudioProcessorEditor* BossBassOneAudioProcessor::createEditor() { return new BossBassOneAudioProcessorEditor(*this); }
void BossBassOneAudioProcessor::getStateInformation(juce::MemoryBlock& dest) {
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, dest);
}
void BossBassOneAudioProcessor::setStateInformation(const void* data, int size) {
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, size));
    if (xml && xml->hasTagName(parameters.state.getType())) parameters.replaceState(juce::ValueTree::fromXml(*xml));
}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new BossBassOneAudioProcessor(); }
