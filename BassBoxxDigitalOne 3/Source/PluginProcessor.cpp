#include "PluginProcessor.h"
#include "PluginEditor.h"

BassBoxxDigitalOneAudioProcessor::BassBoxxDigitalOneAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "BASS_BOXX_DIGITAL_ONE", createParameterLayout()) {}

juce::AudioProcessorValueTreeState::ParameterLayout BassBoxxDigitalOneAudioProcessor::createParameterLayout() {
    using namespace juce;
    std::vector<std::unique_ptr<RangedAudioParameter>> p;
    p.push_back(std::make_unique<AudioParameterFloat>("pickup", "Pickup Position", NormalisableRange<float>(0.05f, 0.95f, 0.001f), 0.72f));
    p.push_back(std::make_unique<AudioParameterFloat>("sub", "Sub", NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.35f));
    p.push_back(std::make_unique<AudioParameterFloat>("drive", "Drive", NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.12f));
    p.push_back(std::make_unique<AudioParameterFloat>("pluck", "Pluck", NormalisableRange<float>(0.15f, 1.0f, 0.001f), 0.68f));
    p.push_back(std::make_unique<AudioParameterFloat>("slide", "Slide Time", NormalisableRange<float>(0.0f, 500.0f, 1.0f), 0.0f, "ms"));
    p.push_back(std::make_unique<AudioParameterFloat>("wah", "Wah", NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.0f));
    p.push_back(std::make_unique<AudioParameterChoice>("subWave", "Sub Wave",
        StringArray { "Sine", "Triangle", "Square", "Saw" }, 0));
    return { p.begin(), p.end() };
}

void BassBoxxDigitalOneAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    currentSampleRate = sampleRate;
    juce::dsp::ProcessSpec wahSpec;
    wahSpec.sampleRate = sampleRate;
    wahSpec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    wahSpec.numChannels = 1;
    wahFilter.prepare(wahSpec);
    stringBuffer.setSize(1, juce::roundToInt(sampleRate / 20.0) + samplesPerBlock + 8);
    stringBuffer.clear();
    writeIndex = 0;
    feedbackState = amplitude = amplitudeTarget = 0.0f;
    wahFilter.reset();
    wahFilter.setType(juce::dsp::StateVariableTPTFilterType::bandpass);
    wahFilter.setResonance(0.85f);
    wahFilter.setCutoffFrequency(900.0f);
    wahPhase = 0.0f;
}
void BassBoxxDigitalOneAudioProcessor::releaseResources() {}
bool BassBoxxDigitalOneAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
    const auto out = layouts.getMainOutputChannelSet();
    return out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo();
}

void BassBoxxDigitalOneAudioProcessor::pluck(float frequency, float velocity) {
    baseFrequency = juce::jlimit(20.0f, 1000.0f, frequency);
    targetFrequency = juce::jlimit(20.0f, 1000.0f, baseFrequency * std::pow(2.0f, pitchBendSemitones / 12.0f));
    if (amplitude < 0.001f) {
        currentFrequency = targetFrequency;
        amplitude = 0.0f;
    }
    const int length = juce::jlimit(2, stringBuffer.getNumSamples() - 2,
        juce::roundToInt(static_cast<float>(currentSampleRate) / targetFrequency));
    const float pick = parameters.getRawParameterValue("pluck")->load();
    for (int i = 0; i < length; ++i) {
        const float noise = random.nextFloat() * 2.0f - 1.0f;
        const float soften = 1.0f - pick * 0.34f;
        const float prev = stringBuffer.getSample(0, (writeIndex + stringBuffer.getNumSamples() - 1) % stringBuffer.getNumSamples());
        stringBuffer.setSample(0, writeIndex, noise * (0.65f + pick * 0.35f) + prev * soften * 0.12f);
        writeIndex = (writeIndex + 1) % stringBuffer.getNumSamples();
    }
    amplitudeTarget = juce::jlimit(0.0f, 1.0f, velocity);
}

float BassBoxxDigitalOneAudioProcessor::readDelay(float delaySamples) const noexcept {
    const int size = stringBuffer.getNumSamples();
    float position = static_cast<float>(writeIndex) - delaySamples;
    while (position < 0.0f) position += static_cast<float>(size);
    const int i0 = static_cast<int>(position) % size;
    const int i1 = (i0 + 1) % size;
    const float frac = position - std::floor(position);
    return stringBuffer.getSample(0, i0) + frac * (stringBuffer.getSample(0, i1) - stringBuffer.getSample(0, i0));
}

void BassBoxxDigitalOneAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) {
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();
    const float pickup = parameters.getRawParameterValue("pickup")->load();
    const float subMix = parameters.getRawParameterValue("sub")->load();
    const float drive = parameters.getRawParameterValue("drive")->load();
    const float slideMs = parameters.getRawParameterValue("slide")->load();
    const float wahAmount = parameters.getRawParameterValue("wah")->load();
    const int subWave = juce::roundToInt(parameters.getRawParameterValue("subWave")->load());
    auto* left = buffer.getWritePointer(0);
    auto* right = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : left;
    auto event = midi.begin();
    const auto end = midi.end();
    for (int i = 0; i < buffer.getNumSamples(); ++i) {
        while (event != end) {
            const auto metadata = *event;
            if (metadata.samplePosition > i)
                break;
            const auto msg = metadata.getMessage();
            if (msg.isNoteOn()) {
                activeMidiNote = msg.getNoteNumber();
                pluck(static_cast<float>(juce::MidiMessage::getMidiNoteInHertz(activeMidiNote)), msg.getFloatVelocity());
            }
            else if (msg.isNoteOff() && msg.getNoteNumber() == activeMidiNote) {
                amplitudeTarget = 0.0f;
                activeMidiNote = -1;
            }
            else if (msg.isAllNotesOff() || msg.isAllSoundOff()) {
                amplitudeTarget = 0.0f;
                activeMidiNote = -1;
            }
            else if (msg.isPitchWheel()) {
                pitchBendSemitones = (static_cast<float>(msg.getPitchWheelValue()) - 8192.0f) / 8192.0f * 2.0f;
                targetFrequency = juce::jlimit(20.0f, 1000.0f, baseFrequency * std::pow(2.0f, pitchBendSemitones / 12.0f));
            }
            ++event;
        }

        if (slideMs <= 0.0f) currentFrequency = targetFrequency;
        else {
            const float glideSamples = juce::jmax(1.0f, slideMs * 0.001f * static_cast<float>(currentSampleRate));
            currentFrequency += (targetFrequency - currentFrequency) / glideSamples;
        }
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
        const float sine = std::sin(subPhase);
        float wave = sine;
        if (subWave == 1) wave = (2.0f / juce::MathConstants<float>::pi) * std::asin(sine);
        else if (subWave == 2) wave = sine >= 0.0f ? 1.0f : -1.0f;
        else if (subWave == 3) wave = subPhase / juce::MathConstants<float>::pi - 1.0f;
        const float sub = wave;
        float in = (body * 2.8f + sub * subMix * 0.72f) * amplitude;
        if (wahAmount > 0.001f) {
            wahPhase += juce::MathConstants<float>::twoPi * 1.7f / static_cast<float>(currentSampleRate);
            if (wahPhase >= juce::MathConstants<float>::twoPi) wahPhase -= juce::MathConstants<float>::twoPi;
            const float sweep = 0.5f + 0.5f * std::sin(wahPhase);
            wahFilter.setCutoffFrequency(350.0f + sweep * 2850.0f * wahAmount);
            in = in * (1.0f - wahAmount) + wahFilter.processSample(0, in) * wahAmount * 2.2f;
        }
        const float gain = 1.0f + drive * 18.0f;
        const float out = std::tanh(in * gain) / std::tanh(gain);
        left[i] = right[i] = out * 0.85f;
    }
}

juce::AudioProcessorEditor* BassBoxxDigitalOneAudioProcessor::createEditor() { return new BassBoxxDigitalOneAudioProcessorEditor(*this); }
void BassBoxxDigitalOneAudioProcessor::getStateInformation(juce::MemoryBlock& dest) {
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, dest);
}
void BassBoxxDigitalOneAudioProcessor::setStateInformation(const void* data, int size) {
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, size));
    if (xml && xml->hasTagName(parameters.state.getType())) parameters.replaceState(juce::ValueTree::fromXml(*xml));
}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new BassBoxxDigitalOneAudioProcessor(); }
