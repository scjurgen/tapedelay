#pragma once
/*
 * AUTO GENERATED,
 * NOT A GOOD IDEA TO CHANGE STUFF HERE
 * Keep the file readonly
 */

#include "TapedelayProcessor.h"

#include "UiElements.h"

//==============================================================================
class AudioPluginAudioProcessorEditor : public juce::AudioProcessorEditor, juce::Timer
{
  public:
    explicit AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
        : AudioProcessorEditor(&p)
        , processorRef(p)
        , valueTreeState(vts)
        , backgroundApp(juce::Colour(Constants::Colors::bg_App))
    {
        setLookAndFeel(&m_laf);
        initWidgets();
        setResizable(true, true);
        setResizeLimits(Constants::InitJuce::WindowWidth, Constants::InitJuce::WindowHeight, 4000, 3000);
        setSize(Constants::InitJuce::WindowWidth, Constants::InitJuce::WindowHeight);
        startTimerHz(Constants::InitJuce::TimerHertz);
    }

    ~AudioPluginAudioProcessorEditor() override
    {
        setLookAndFeel(nullptr);
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(backgroundApp);
    }
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"
    void resized() override
    {
        auto area = getLocalBounds().reduced(static_cast<int>(Constants::Margins::big));

        // auto generated
        // const juce::FlexItem::Margin knobMargin = juce::FlexItem::Margin(Constants::Margins::small);
        const juce::FlexItem::Margin knobMarginSmall = juce::FlexItem::Margin(Constants::Margins::medium);

        std::vector<juce::Rectangle<int>> areas(6);
        const auto colWidth = area.getWidth() / 13;
        const auto rowHeight = area.getHeight() / 6;
        areas[0] = area.removeFromLeft(colWidth * 1).reduced(Constants::Margins::small);
        auto keepArea = area;
        areas[1] = area.removeFromTop(rowHeight * 1).reduced(Constants::Margins::small);
        areas[2] = area.removeFromTop(rowHeight * 1).reduced(Constants::Margins::small);
        areas[3] = area.removeFromTop(rowHeight * 1).reduced(Constants::Margins::small);
        areas[4] = area.removeFromTop(rowHeight * 1).reduced(Constants::Margins::small);
        areas[5] = area.reduced(Constants::Margins::small);

        {
            juce::FlexBox box;
            box.flexWrap = juce::FlexBox::Wrap::noWrap;
            box.flexDirection = juce::FlexBox::Direction::column;
            box.justifyContent = juce::FlexBox::JustifyContent::spaceAround;
            box.items.add(juce::FlexItem(levelGauge).withHeight(500).withMargin(knobMarginSmall));
            box.items.add(juce::FlexItem(cpuGauge).withHeight(200).withMargin(knobMarginSmall));
            box.performLayout(areas[0].toFloat());
        }
        {
            juce::FlexBox box;
            box.flexWrap = juce::FlexBox::Wrap::noWrap;
            box.flexDirection = juce::FlexBox::Direction::row;
            box.justifyContent = juce::FlexBox::JustifyContent::spaceAround;
            box.items.add(juce::FlexItem(levelDial).withFlex(1).withMargin(knobMarginSmall));
            box.items.add(juce::FlexItem(tapeSpeedDial).withFlex(1).withMargin(knobMarginSmall));
            box.items.add(juce::FlexItem(saturationDial).withFlex(1).withMargin(knobMarginSmall));
            box.performLayout(areas[1].toFloat());
        }
        {
            juce::FlexBox box;
            box.flexWrap = juce::FlexBox::Wrap::noWrap;
            box.flexDirection = juce::FlexBox::Direction::row;
            box.justifyContent = juce::FlexBox::JustifyContent::spaceAround;
            box.items.add(juce::FlexItem(hysteresisDial).withFlex(1).withMargin(knobMarginSmall));
            box.items.add(juce::FlexItem(noiseFloorDial).withFlex(1).withMargin(knobMarginSmall));
            box.items.add(juce::FlexItem(noiseDistributionDial).withFlex(1).withMargin(knobMarginSmall));
            box.performLayout(areas[2].toFloat());
        }
        {
            juce::FlexBox box;
            box.flexWrap = juce::FlexBox::Wrap::noWrap;
            box.flexDirection = juce::FlexBox::Direction::row;
            box.justifyContent = juce::FlexBox::JustifyContent::spaceAround;
            box.items.add(juce::FlexItem(delayTime1Dial).withFlex(1).withMargin(knobMarginSmall));
            box.items.add(juce::FlexItem(delayLevel1Dial).withFlex(1).withMargin(knobMarginSmall));
            box.items.add(juce::FlexItem(feedback1Dial).withFlex(1).withMargin(knobMarginSmall));
            box.items.add(juce::FlexItem(delayTime2Dial).withFlex(1).withMargin(knobMarginSmall));
            box.items.add(juce::FlexItem(delayLevel2Dial).withFlex(1).withMargin(knobMarginSmall));
            box.items.add(juce::FlexItem(feedback2Dial).withFlex(1).withMargin(knobMarginSmall));
            box.performLayout(areas[3].toFloat());
        }
        {
            juce::FlexBox box;
            box.flexWrap = juce::FlexBox::Wrap::noWrap;
            box.flexDirection = juce::FlexBox::Direction::row;
            box.justifyContent = juce::FlexBox::JustifyContent::spaceAround;
            box.items.add(juce::FlexItem(delayTime3Dial).withFlex(1).withMargin(knobMarginSmall));
            box.items.add(juce::FlexItem(delayLevel3Dial).withFlex(1).withMargin(knobMarginSmall));
            box.items.add(juce::FlexItem(feedback3Dial).withFlex(1).withMargin(knobMarginSmall));
            box.items.add(juce::FlexItem(delayTime4Dial).withFlex(1).withMargin(knobMarginSmall));
            box.items.add(juce::FlexItem(delayLevel4Dial).withFlex(1).withMargin(knobMarginSmall));
            box.items.add(juce::FlexItem(feedback4Dial).withFlex(1).withMargin(knobMarginSmall));
            box.performLayout(areas[4].toFloat());
        }
        {
            juce::FlexBox box;
            box.flexWrap = juce::FlexBox::Wrap::noWrap;
            box.flexDirection = juce::FlexBox::Direction::row;
            box.justifyContent = juce::FlexBox::JustifyContent::spaceAround;
            box.items.add(juce::FlexItem(spectrogramGauge).withFlex(1).withMargin(knobMarginSmall));
            box.items.add(juce::FlexItem(signalGauge).withFlex(1).withMargin(knobMarginSmall));
            box.performLayout(areas[5].toFloat());
        }
    }
#pragma GCC diagnostic pop

    void timerCallback() override
    {
        cpuGauge.update(processorRef.getCpuLoad());
        levelGauge.update(processorRef.getInputDbLoad(), processorRef.getOutputDbLoad());
        spectrogramGauge.update(processorRef.getSpectrogram());
        signalGauge.update(processorRef.getWaveDataToShow());
    }

    void initWidgets()
    {
        addAndMakeVisible(levelDial);
        levelDial.reset(valueTreeState, "level");
        levelDial.setLabelText(juce::String::fromUTF8("Level"));
        addAndMakeVisible(tapeSpeedDial);
        tapeSpeedDial.reset(valueTreeState, "tapeSpeed");
        tapeSpeedDial.setLabelText(juce::String::fromUTF8("Tape speed"));
        addAndMakeVisible(hysteresisDial);
        hysteresisDial.reset(valueTreeState, "hysteresis");
        hysteresisDial.setLabelText(juce::String::fromUTF8("Hysteresis"));
        addAndMakeVisible(saturationDial);
        saturationDial.reset(valueTreeState, "saturation");
        saturationDial.setLabelText(juce::String::fromUTF8("Saturation"));
        addAndMakeVisible(noiseFloorDial);
        noiseFloorDial.reset(valueTreeState, "noiseFloor");
        noiseFloorDial.setLabelText(juce::String::fromUTF8("Noise floor"));
        addAndMakeVisible(noiseDistributionDial);
        noiseDistributionDial.reset(valueTreeState, "noiseDistribution");
        noiseDistributionDial.setLabelText(juce::String::fromUTF8("Noise distribution"));
        addAndMakeVisible(delayTime1Dial);
        delayTime1Dial.reset(valueTreeState, "delayTime1");
        delayTime1Dial.setLabelText(juce::String::fromUTF8("Delay time 1"));
        addAndMakeVisible(delayLevel1Dial);
        delayLevel1Dial.reset(valueTreeState, "delayLevel1");
        delayLevel1Dial.setLabelText(juce::String::fromUTF8("Delay level 1"));
        addAndMakeVisible(feedback1Dial);
        feedback1Dial.reset(valueTreeState, "feedback1");
        feedback1Dial.setLabelText(juce::String::fromUTF8("Feedback 1"));
        addAndMakeVisible(delayTime2Dial);
        delayTime2Dial.reset(valueTreeState, "delayTime2");
        delayTime2Dial.setLabelText(juce::String::fromUTF8("Delta time 2"));
        addAndMakeVisible(delayLevel2Dial);
        delayLevel2Dial.reset(valueTreeState, "delayLevel2");
        delayLevel2Dial.setLabelText(juce::String::fromUTF8("Delay level 2"));
        addAndMakeVisible(feedback2Dial);
        feedback2Dial.reset(valueTreeState, "feedback2");
        feedback2Dial.setLabelText(juce::String::fromUTF8("Feedback 2"));
        addAndMakeVisible(delayTime3Dial);
        delayTime3Dial.reset(valueTreeState, "delayTime3");
        delayTime3Dial.setLabelText(juce::String::fromUTF8("Delta time 3"));
        addAndMakeVisible(delayLevel3Dial);
        delayLevel3Dial.reset(valueTreeState, "delayLevel3");
        delayLevel3Dial.setLabelText(juce::String::fromUTF8("Delay level 3"));
        addAndMakeVisible(feedback3Dial);
        feedback3Dial.reset(valueTreeState, "feedback3");
        feedback3Dial.setLabelText(juce::String::fromUTF8("Feedback 3"));
        addAndMakeVisible(delayTime4Dial);
        delayTime4Dial.reset(valueTreeState, "delayTime4");
        delayTime4Dial.setLabelText(juce::String::fromUTF8("SOS time"));
        addAndMakeVisible(delayLevel4Dial);
        delayLevel4Dial.reset(valueTreeState, "delayLevel4");
        delayLevel4Dial.setLabelText(juce::String::fromUTF8("SOS level"));
        addAndMakeVisible(feedback4Dial);
        feedback4Dial.reset(valueTreeState, "feedback4");
        feedback4Dial.setLabelText(juce::String::fromUTF8("SOS Feedback "));
        addAndMakeVisible(cpuGauge);
        cpuGauge.setLabelText(juce::String::fromUTF8("CPU"));
        addAndMakeVisible(levelGauge);
        levelGauge.setLabelText(juce::String::fromUTF8("Level"));
        addAndMakeVisible(spectrogramGauge);
        spectrogramGauge.setLabelText(juce::String::fromUTF8("Spectrogram"));
        addAndMakeVisible(signalGauge);
        signalGauge.setLabelText(juce::String::fromUTF8("Signal"));
    }

  private:
    AudioPluginAudioProcessor& processorRef;
    juce::AudioProcessorValueTreeState& valueTreeState;
    GuiLookAndFeel m_laf;
    juce::Colour backgroundApp;

    CustomRotaryDial levelDial{this};
    CustomRotaryDial tapeSpeedDial{this};
    CustomRotaryDial hysteresisDial{this};
    CustomRotaryDial saturationDial{this};
    CustomRotaryDial noiseFloorDial{this};
    CustomRotaryDial noiseDistributionDial{this};
    CustomRotaryDial delayTime1Dial{this};
    CustomRotaryDial delayLevel1Dial{this};
    CustomRotaryDial feedback1Dial{this};
    CustomRotaryDial delayTime2Dial{this};
    CustomRotaryDial delayLevel2Dial{this};
    CustomRotaryDial feedback2Dial{this};
    CustomRotaryDial delayTime3Dial{this};
    CustomRotaryDial delayLevel3Dial{this};
    CustomRotaryDial feedback3Dial{this};
    CustomRotaryDial delayTime4Dial{this};
    CustomRotaryDial delayLevel4Dial{this};
    CustomRotaryDial feedback4Dial{this};
    CpuGauge cpuGauge{};
    Gauge levelGauge{};
    SpectrogramDisplay spectrogramGauge{};
    WaveformGauge signalGauge{};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessorEditor)
};
