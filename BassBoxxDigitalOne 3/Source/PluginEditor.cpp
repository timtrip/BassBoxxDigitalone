#include "PluginEditor.h"

BassBoxxDigitalOneAudioProcessorEditor::BassBoxxDigitalOneAudioProcessorEditor(BassBoxxDigitalOneAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p) {
    setSize(760, 490);
    title.setText("BASS BOXX", juce::dontSendNotification);
    title.setFont(juce::Font(juce::FontOptions(30.0f, juce::Font::bold)));
    title.setColour(juce::Label::textColourId, juce::Colour(0xffffcc55));
    addAndMakeVisible(title);
    subtitle.setText("DIGITAL ONE  |  ONE STRING. YOUR WHOLE LOW END.", juce::dontSendNotification);
    subtitle.setFont(juce::Font(juce::FontOptions(13.0f, juce::Font::bold)));
    subtitle.setColour(juce::Label::textColourId, juce::Colour(0xffaaa9a5));
    addAndMakeVisible(subtitle);
    styleSlider(pickup, pickupLabel, "PICKUP");
    styleSlider(sub, subLabel, "SUB");
    styleSlider(drive, driveLabel, "DRIVE");
    styleSlider(pluck, pluckLabel, "PLUCK");
    styleSlider(slide, slideLabel, "SLIDE");
    styleSlider(wah, wahLabel, "WAH");
    pickupAttach = std::make_unique<Attachment>(processor.parameters, "pickup", pickup);
    subAttach = std::make_unique<Attachment>(processor.parameters, "sub", sub);
    driveAttach = std::make_unique<Attachment>(processor.parameters, "drive", drive);
    pluckAttach = std::make_unique<Attachment>(processor.parameters, "pluck", pluck);
    slideAttach = std::make_unique<Attachment>(processor.parameters, "slide", slide);
    wahAttach = std::make_unique<Attachment>(processor.parameters, "wah", wah);
    subWave.addItem("Sine", 1);
    subWave.addItem("Triangle", 2);
    subWave.addItem("Square", 3);
    subWave.addItem("Saw", 4);
    subWave.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff202124));
    subWave.setColour(juce::ComboBox::textColourId, juce::Colours::white);
    subWave.setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff444449));
    addAndMakeVisible(subWave);
    subWaveLabel.setText("SUB WAVE", juce::dontSendNotification);
    subWaveLabel.setFont(juce::Font(juce::FontOptions(12.0f, juce::Font::bold)));
    subWaveLabel.setColour(juce::Label::textColourId, juce::Colour(0xffd1d0cb));
    addAndMakeVisible(subWaveLabel);
    subWaveAttach = std::make_unique<ComboAttachment>(processor.parameters, "subWave", subWave);
}

void BassBoxxDigitalOneAudioProcessorEditor::styleSlider(juce::Slider& s, juce::Label& l, const juce::String& name) {
    s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 72, 20);
    s.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xffffbd45));
    s.setColour(juce::Slider::thumbColourId, juce::Colour(0xffffe1a0));
    s.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    s.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff202124));
    s.setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0xff444449));
    addAndMakeVisible(s);
    l.setText(name, juce::dontSendNotification);
    l.setJustificationType(juce::Justification::centred);
    l.setFont(juce::Font(juce::FontOptions(12.0f, juce::Font::bold)));
    l.setColour(juce::Label::textColourId, juce::Colour(0xffd1d0cb));
    addAndMakeVisible(l);
}

void BassBoxxDigitalOneAudioProcessorEditor::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xff111214));
    auto area = getLocalBounds().toFloat();
    g.setColour(juce::Colour(0xff1b1c1f));
    g.fillRoundedRectangle(area.reduced(18.0f).withTop(92.0f), 16.0f);
    // A single oversized string with a movable pickup marker.
    const float y = 225.0f;
    g.setColour(juce::Colour(0xff34353a));
    g.fillRoundedRectangle(58.0f, y - 12.0f, 644.0f, 24.0f, 12.0f);
    g.setColour(juce::Colour(0xffa6a7aa));
    g.fillRoundedRectangle(58.0f, y - 3.0f, 644.0f, 6.0f, 3.0f);
    const float pos = 58.0f + processor.parameters.getRawParameterValue("pickup")->load() * 644.0f;
    g.setColour(juce::Colour(0xffffbd45));
    g.fillRoundedRectangle(pos - 4.0f, y - 33.0f, 8.0f, 66.0f, 4.0f);
    g.setColour(juce::Colour(0xff8c8d91));
    for (int i = 1; i <= 11; ++i) {
        const float x = 58.0f + i * 52.0f;
        g.drawLine(x, y - 18.0f, x, y + 18.0f, 1.4f);
    }
    g.setColour(juce::Colour(0xff85868b));
    g.setFont(12.0f);
    g.drawFittedText("STRING", 58, 264, 90, 20, juce::Justification::left, 1);
    g.drawFittedText("BRIDGE", 610, 264, 90, 20, juce::Justification::right, 1);
    g.setColour(juce::Colour(0xff303136));
    g.drawRoundedRectangle(area.reduced(18.0f).withTop(92.0f), 16.0f, 1.0f);
}

void BassBoxxDigitalOneAudioProcessorEditor::resized() {
    title.setBounds(34, 20, 360, 42);
    subtitle.setBounds(36, 59, 360, 24);
    subWaveLabel.setBounds(558, 112, 115, 20);
    subWave.setBounds(558, 134, 145, 30);
    const int y = 330;
    juce::Slider* sliders[] = { &pickup, &sub, &drive, &pluck, &slide, &wah };
    juce::Label* labels[] = { &pickupLabel, &subLabel, &driveLabel, &pluckLabel, &slideLabel, &wahLabel };
    for (int i = 0; i < 6; ++i) {
        const int x = 8 + i * 124;
        labels[i]->setBounds(x, y, 120, 22);
        sliders[i]->setBounds(x + 4, y + 22, 112, 116);
    }
}

void BassBoxxDigitalOneAudioProcessorEditor::mouseDown(const juce::MouseEvent& event) {
    if (event.position.y >= 190.0f && event.position.y <= 260.0f)
        setPickupFromMouse(event.position);
}

void BassBoxxDigitalOneAudioProcessorEditor::mouseDrag(const juce::MouseEvent& event) {
    setPickupFromMouse(event.position);
}

void BassBoxxDigitalOneAudioProcessorEditor::setPickupFromMouse(juce::Point<float> position) {
    auto* parameter = processor.parameters.getParameter("pickup");
    if (parameter == nullptr) return;

    const float actualValue = juce::jlimit(0.05f, 0.95f, (position.x - 58.0f) / 644.0f);
    parameter->setValueNotifyingHost(parameter->convertTo0to1(actualValue));
    repaint();
}
