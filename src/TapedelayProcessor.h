#pragma once
/*
 * AUTO GENERATED,
 * NOT A GOOD IDEA TO CHANGE STUFF HERE
 * Keep the file readonly
 */

#include "Analysis/EnvelopeFollower.h"
#include "Analysis/Spectrogram.h"

#include "Audio/FixedSizeProcessor.h"

#include "UiElements.h"

#include "impl/TapeDelay.h"

#include <juce_audio_processors/juce_audio_processors.h>

class AudioPluginAudioProcessor : public juce::AudioProcessor, public juce::AudioProcessorValueTreeState::Listener
{
  public:
    static constexpr size_t NumSamplesPerBlock = 16;
    AudioPluginAudioProcessor()
        : AudioProcessor(BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
                             .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
                             .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
                             )
        , fixedRunner([this](const AudioBuffer<2, NumSamplesPerBlock>& input,
                             AudioBuffer<2, NumSamplesPerBlock>& output) { pluginRunner->processBlock(input, output); })
        , m_parameters(*this, nullptr, "PARAMETERS", createParameterLayout())
        , m_avgCpu(8, 0)
        , m_head{0}
        , m_runningWindowCpu(8 * 300)
        , m_envInput{AbacDsp::RmsFollower(10000), AbacDsp::RmsFollower(10000)}
        , m_envOutput{AbacDsp::RmsFollower(10000), AbacDsp::RmsFollower(10000)}
        , m_spectrogram{}
    {
        m_parameters.addParameterListener("feedGain", this);
        m_parameters.addParameterListener("tapeSpeed", this);
        m_parameters.addParameterListener("wow", this);
        m_parameters.addParameterListener("hysteresis", this);
        m_parameters.addParameterListener("saturation", this);
        m_parameters.addParameterListener("noiseFloor", this);
        m_parameters.addParameterListener("noiseDistribution", this);
        m_parameters.addParameterListener("delayTime1", this);
        m_parameters.addParameterListener("delayTime2", this);
        m_parameters.addParameterListener("delayTime3", this);
        m_parameters.addParameterListener("delayTime4", this);
        m_parameters.addParameterListener("delayLevel1", this);
        m_parameters.addParameterListener("delayLevel2", this);
        m_parameters.addParameterListener("delayLevel3", this);
        m_parameters.addParameterListener("delayLevel4", this);
        m_parameters.addParameterListener("feedback1", this);
        m_parameters.addParameterListener("feedback2", this);
        m_parameters.addParameterListener("feedback3", this);
        m_parameters.addParameterListener("feedback4", this);
    }
    ~AudioPluginAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override
    {
        pluginRunner = std::make_unique<TapeDelay<NumSamplesPerBlock>>(static_cast<float>(sampleRate));
        m_sampleRate = static_cast<size_t>(sampleRate);
        for (auto* param : getParameters())
        {
            if (auto* p = dynamic_cast<juce::RangedAudioParameter*>(param))
            {
                const auto normalizedValue = p->getValue();
                p->sendValueChangedMessageToListeners(normalizedValue);
            }
        }
        if (m_newState.isValid())
        {
            m_parameters.replaceState(m_newState);
        }

        juce::ignoreUnused(samplesPerBlock);
    }

    void releaseResources() override
    {
        pluginRunner = nullptr;
    }

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override
    {
#if JucePlugin_IsMidiEffect
        juce::ignoreUnused(layouts);
        return true;
#else
        /* This is the place where you check if the layout is supported.
         * In this template code we only support mono or stereo.
         */
        if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
            layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        {
            return false;
        }

        /* This checks if the input layout matches the output layout */
#if !JucePlugin_IsSynth
        if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        {
            return false;
        }
#endif
        return true;
#endif
    }

    juce::AudioProcessorEditor* createEditor() override;

    bool hasEditor() const override
    {
        return true;
    }

    const juce::String getName() const override
    {
        return JucePlugin_Name;
    }

    bool acceptsMidi() const override
    {
#if JucePlugin_WantsMidiInput
        return true;
#else
        return false;
#endif
    }

    bool producesMidi() const override
    {
#if JucePlugin_ProducesMidiOutput
        return true;
#else
        return false;
#endif
    }

    bool isMidiEffect() const override
    {
#if JucePlugin_IsMidiEffect
        return true;
#else
        return false;
#endif
    }

    double getTailLengthSeconds() const override
    {
        return 2.0;
    }

    int getNumPrograms() override
    {
        return 1;
        /* NB: some hosts don't cope very well if you tell them there are 0 programs,
                   so this should be at least 1, even if you're not really implementing programs.
        */
    }

    int getCurrentProgram() override
    {
        return 0;
    }

    void setCurrentProgram(const int index) override
    {
        m_program = index;
    }

    const juce::String getProgramName(const int index) override
    {
        switch (index)
        {
            case 0:
                return {"Program 0"};
            case 1:
                return {"Program 1"};
            case 2:
                return {"Program 2"};
            default:
                return {"Program unknown"};
        }
    }

    void changeProgramName(int index, const juce::String& newName) override
    {
        juce::ignoreUnused(index, newName);
    }

    void getStateInformation(juce::MemoryBlock& destData) override
    {
        auto state = m_parameters.copyState();
        std::unique_ptr<juce::XmlElement> xml(state.createXml());
        if (xml != nullptr)
        {
            copyXmlToBinary(*xml, destData);
        }
    }

    void setStateInformation(const void* data, int sizeInBytes) override
    {
        std::unique_ptr xmlState(getXmlFromBinary(data, sizeInBytes));

        if (xmlState != nullptr)
        {
            if (xmlState->hasTagName(m_parameters.state.getType()))
            {
                m_newState = juce::ValueTree::fromXml(*xmlState);
            }
        }
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-float-conversion"
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
    {
        std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID("feedGain", 1), "Feed", juce::NormalisableRange<float>(-60, 12, 0.1, 1, false), 0,
            juce::String("Feed"), juce::AudioProcessorParameter::genericParameter,
            [](float value, float) { return juce::String(value, 1) + " dB"; }));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID("tapeSpeed", 1), "Tape speed", juce::NormalisableRange<float>(0.5, 60.0, 0.1, 1.0, false),
            7.5, juce::String("Tape speed"), juce::AudioProcessorParameter::genericParameter,
            [](float value, float) { return juce::String(value, 0) + " IPS"; }));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID("wow", 1), "WOW", juce::NormalisableRange<float>(0, 1, 0.01, 1.0, false), 0,
            juce::String("WOW"), juce::AudioProcessorParameter::genericParameter,
            [](float value, float) { return juce::String(value, 0) + " "; }));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID("hysteresis", 1), "Hysteresis", juce::NormalisableRange<float>(0, 1, 0.01, 1.0, false), 0,
            juce::String("Hysteresis"), juce::AudioProcessorParameter::genericParameter,
            [](float value, float) { return juce::String(value, 0) + " "; }));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID("saturation", 1), "Saturation", juce::NormalisableRange<float>(0, 1, 0.01, 1.0, false), 0,
            juce::String("Saturation"), juce::AudioProcessorParameter::genericParameter,
            [](float value, float) { return juce::String(value, 0) + " "; }));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID("noiseFloor", 1), "Noise floor", juce::NormalisableRange<float>(0, 1, 0.01, 1.0, false),
            0, juce::String("Noise floor"), juce::AudioProcessorParameter::genericParameter,
            [](float value, float) { return juce::String(value, 0) + " "; }));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID("noiseDistribution", 1), "Noise distribution",
            juce::NormalisableRange<float>(0, 1, 0.01, 1.0, false), 0, juce::String("Noise distribution"),
            juce::AudioProcessorParameter::genericParameter,
            [](float value, float) { return juce::String(value, 0) + " "; }));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID("delayTime1", 1), "Delay time 1",
            juce::NormalisableRange<float>(0, 1000, 0.1, 0.4, false), 200, juce::String("Delay time 1"),
            juce::AudioProcessorParameter::genericParameter,
            [](float value, float) { return juce::String(value, 0) + " ms"; }));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID("delayTime2", 1), "Delay time 2",
            juce::NormalisableRange<float>(0, 1000, 0.1, 0.4, false), 100, juce::String("Delay time 2"),
            juce::AudioProcessorParameter::genericParameter,
            [](float value, float) { return juce::String(value, 0) + " ms"; }));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID("delayTime3", 1), "Delay time 3",
            juce::NormalisableRange<float>(0, 1000, 0.1, 0.4, false), 100, juce::String("Delay time 3"),
            juce::AudioProcessorParameter::genericParameter,
            [](float value, float) { return juce::String(value, 0) + " ms"; }));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID("delayTime4", 1), "Tape time 4",
            juce::NormalisableRange<float>(1000, 10000, 0.1, 0.4, false), 2000, juce::String("Tape time 4"),
            juce::AudioProcessorParameter::genericParameter,
            [](float value, float) { return juce::String(value, 0) + " ms"; }));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID("delayLevel1", 1), "Delay level 1",
            juce::NormalisableRange<float>(-72, 12, 0.1, 1.0, false), 0, juce::String("Delay level 1"),
            juce::AudioProcessorParameter::genericParameter,
            [](float value, float) { return juce::String(value, 0) + " dB"; }));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID("delayLevel2", 1), "Delay level 2",
            juce::NormalisableRange<float>(-72, 12, 0.1, 1.0, false), -6, juce::String("Delay level 2"),
            juce::AudioProcessorParameter::genericParameter,
            [](float value, float) { return juce::String(value, 0) + " dB"; }));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID("delayLevel3", 1), "Delay level 3",
            juce::NormalisableRange<float>(-72, 12, 0.1, 1.0, false), -12, juce::String("Delay level 3"),
            juce::AudioProcessorParameter::genericParameter,
            [](float value, float) { return juce::String(value, 0) + " dB"; }));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID("delayLevel4", 1), "Delay level 4",
            juce::NormalisableRange<float>(-72, 12, 0.1, 1.0, false), -18, juce::String("Delay level 4"),
            juce::AudioProcessorParameter::genericParameter,
            [](float value, float) { return juce::String(value, 0) + " dB"; }));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID("feedback1", 1), "Feedback 1", juce::NormalisableRange<float>(0, 1, 0.01, 1.0, false), 0,
            juce::String("Feedback 1"), juce::AudioProcessorParameter::genericParameter,
            [](float value, float) { return juce::String(value, 0) + " "; }));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID("feedback2", 1), "Feedback 2", juce::NormalisableRange<float>(0, 1, 0.01, 1.0, false), 0,
            juce::String("Feedback 2"), juce::AudioProcessorParameter::genericParameter,
            [](float value, float) { return juce::String(value, 0) + " "; }));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID("feedback3", 1), "Feedback 3", juce::NormalisableRange<float>(0, 1, 0.01, 1.0, false), 0,
            juce::String("Feedback 3"), juce::AudioProcessorParameter::genericParameter,
            [](float value, float) { return juce::String(value, 0) + " "; }));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID("feedback4", 1), "Feedback 4", juce::NormalisableRange<float>(0, 1, 0.01, 1.0, false), 0,
            juce::String("Feedback 4"), juce::AudioProcessorParameter::genericParameter,
            [](float value, float) { return juce::String(value, 0) + " "; }));

        return {params.begin(), params.end()};
    }
#pragma GCC diagnostic pop

    void parameterChanged(const juce::String& parameterID, float newValue) override
    {
        if (pluginRunner == nullptr)
        {
            return;
        }
        static const std::map<juce::String, std::function<void(AudioPluginAudioProcessor&, float)>> parameterMap{
            {"feedGain", [](const AudioPluginAudioProcessor& p, const float v) { p.pluginRunner->setFeedGain(v); }},
            {"tapeSpeed", [](const AudioPluginAudioProcessor& p, const float v) { p.pluginRunner->setTapeSpeed(v); }},
            {"wow", [](const AudioPluginAudioProcessor& p, const float v) { p.pluginRunner->setWow(v); }},
            {"hysteresis", [](const AudioPluginAudioProcessor& p, const float v) { p.pluginRunner->setHysteresis(v); }},
            {"saturation", [](const AudioPluginAudioProcessor& p, const float v) { p.pluginRunner->setSaturation(v); }},
            {"noiseFloor", [](const AudioPluginAudioProcessor& p, const float v) { p.pluginRunner->setNoiseFloor(v); }},
            {"noiseDistribution",
             [](const AudioPluginAudioProcessor& p, const float v) { p.pluginRunner->setNoiseDistribution(v); }},
            {"delayTime1", [](const AudioPluginAudioProcessor& p, const float v) { p.pluginRunner->setDelayTime1(v); }},
            {"delayTime2", [](const AudioPluginAudioProcessor& p, const float v) { p.pluginRunner->setDelayTime2(v); }},
            {"delayTime3", [](const AudioPluginAudioProcessor& p, const float v) { p.pluginRunner->setDelayTime3(v); }},
            {"delayTime4", [](const AudioPluginAudioProcessor& p, const float v) { p.pluginRunner->setDelayTime4(v); }},
            {"delayLevel1",
             [](const AudioPluginAudioProcessor& p, const float v) { p.pluginRunner->setDelayLevel1(v); }},
            {"delayLevel2",
             [](const AudioPluginAudioProcessor& p, const float v) { p.pluginRunner->setDelayLevel2(v); }},
            {"delayLevel3",
             [](const AudioPluginAudioProcessor& p, const float v) { p.pluginRunner->setDelayLevel3(v); }},
            {"delayLevel4",
             [](const AudioPluginAudioProcessor& p, const float v) { p.pluginRunner->setDelayLevel4(v); }},
            {"feedback1", [](const AudioPluginAudioProcessor& p, const float v) { p.pluginRunner->setFeedback1(v); }},
            {"feedback2", [](const AudioPluginAudioProcessor& p, const float v) { p.pluginRunner->setFeedback2(v); }},
            {"feedback3", [](const AudioPluginAudioProcessor& p, const float v) { p.pluginRunner->setFeedback3(v); }},
            {"feedback4", [](const AudioPluginAudioProcessor& p, const float v) { p.pluginRunner->setFeedback4(v); }},

        };
        if (auto it = parameterMap.find(parameterID); it != parameterMap.end())
        {
            it->second(*this, newValue);
        }
    }
    void computeCpuLoad(std::chrono::nanoseconds elapsed, size_t numSamples)
    {
        samplesProcessed += numSamples;
        elapsedTotalNanoSeconds += static_cast<size_t>(elapsed.count());
        constexpr float secondsPoll = 0.5f;
        if (samplesProcessed > m_sampleRate * secondsPoll)
        {
            const auto pRate = static_cast<float>(100.0 * static_cast<double>(elapsedTotalNanoSeconds) /
                                                  (secondsPoll * 1'000'000'000.0));
            m_runningWindowCpu += static_cast<size_t>(pRate * 100.f);
            m_runningWindowCpu -= m_avgCpu[m_head];
            m_avgCpu[m_head++] = static_cast<size_t>(pRate * 100.f);
            m_head = m_head % m_avgCpu.size();
            m_cpuLoad.store(m_runningWindowCpu * 0.01f / m_avgCpu.size());
            elapsedTotalNanoSeconds = 0;
            samplesProcessed = 0;
        }
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"

    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override
    {
        juce::ScopedNoDenormals noDenormals;
        const auto beginTime = std::chrono::high_resolution_clock::now();

        if (!midiMessages.isEmpty())
        {
            for (const auto& msg : midiMessages)
            {
                pluginRunner->processMidi(msg.data);
            }
        }
        for (int c = 0; c < std::min(2, buffer.getNumChannels()); ++c)
        {
            m_envInput[c].feed(buffer.getReadPointer(c), buffer.getNumSamples());
            m_inputDb[c].store(std::log10(m_envInput[c].getRms()) * 20.f);
        }
        if ((getTotalNumInputChannels() == 2) && (getTotalNumOutputChannels() == 2))
        {
            fixedRunner.processBlock(buffer);
        }
        for (int c = 0; c < std::min(2, buffer.getNumChannels()); ++c)
        {
            m_envOutput[c].feed(buffer.getReadPointer(c), buffer.getNumSamples());
            m_outputDb[c].store(std::log10(m_envOutput[c].getRms()) * 20.f);
        }
        m_spectrogram.processBlock(buffer.getWritePointer(0), buffer.getNumSamples());
        const auto endTime = std::chrono::high_resolution_clock::now();
        computeCpuLoad(std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - beginTime),
                       static_cast<size_t>(buffer.getNumSamples()));
    }

#pragma GCC diagnostic pop

    [[nodiscard]] float getCpuLoad() const
    {
        return m_cpuLoad.load();
    }

    [[nodiscard]] const std::vector<float>& getWaveDataToShow()
    {
        return pluginRunner->visualizeWaveData();
    }
    [[nodiscard]] std::pair<float, float> getInputDbLoad() const
    {
        return {m_inputDb[0].load(), m_inputDb[1].load()};
    }

    [[nodiscard]] std::pair<float, float> getOutputDbLoad() const
    {
        return {m_outputDb[0].load(), m_outputDb[1].load()};
    }
    [[nodiscard]] SpectrumImageSet getSpectrogram() const
    {
        return m_spectrogram.getImageSet();
    }

    float m_maxValue{0.f};
    size_t elapsedTotalNanoSeconds{0};
    size_t samplesProcessed = 0;

  private:
    size_t m_sampleRate{48000};

    static bool isChanged(const float a, const float b)
    {
        return std::abs(a - b) > 1E-8f;
    }

    int m_program{0};
    juce::ValueTree m_newState;

    FixedSizeProcessor<2, NumSamplesPerBlock, juce::AudioBuffer<float>> fixedRunner;
    std::unique_ptr<TapeDelay<NumSamplesPerBlock>> pluginRunner;
    juce::AudioProcessorValueTreeState m_parameters;
    // CPU-Load
    std::atomic<float> m_cpuLoad;
    std::vector<size_t> m_avgCpu;
    size_t m_head{};
    size_t m_runningWindowCpu;
    // VU-Meter
    std::atomic<float> m_inputDb[2];
    std::atomic<float> m_outputDb[2];
    std::array<AbacDsp::RmsFollower, 2> m_envInput;
    std::array<AbacDsp::RmsFollower, 2> m_envOutput;
    SimpleSpectrogram m_spectrogram;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessor)
};
