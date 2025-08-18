#pragma once

#include "EffectBase.h"

#include "Audio/AudioBuffer.h"
#include "Samplerate/VariReader.h"
#include "Samplerate/sinc_4_128.h"
#include <cassert>
#include <cstdint>
#include <cmath>
#include <functional>


template <std::size_t NRelays = 8>
class TapePreisachHysteresis
{
  public:
    explicit TapePreisachHysteresis(float minLevel = -1.0f, float maxLevel = 1.0f, float spread = 0.8f)
    {
        // Evenly distribute relay thresholds with some spread for asymmetry/natural tape feel
        for (std::size_t i = 0; i < NRelays; ++i)
        {
            float pos = float(i) / float(NRelays - 1);
            m_thresholdsUp[i] = lerp(minLevel, maxLevel * spread, pos);
            m_thresholdsDown[i] = lerp(minLevel * spread, maxLevel, pos);
        }
    }

    float processSample(float x) noexcept
    {
        // Apply each relay Schmitt trigger and accumulate their (±1) states:
        float sum = 0.0f;
        for (std::size_t i = 0; i < NRelays; ++i)
        {
            // Up/Down thresholds for this relay
            auto& st = m_states[i];
            if (x > m_thresholdsUp[i])
                st = 1.0f;
            else if (x < m_thresholdsDown[i])
                st = -1.0f;
            // otherwise, state stays ("memory")
            sum += st;
        }
        // Normalize by relay count, shape output (soft clip)
        return std::tanh((sum / static_cast<float>(NRelays)) * m_drive);
    }

    void reset() noexcept
    {
        m_states.fill(0.0f);
    }

    void setDrive(float d) noexcept
    {
        m_drive = d;
    }

  private:
    static constexpr float lerp(float a, float b, float t) noexcept
    {
        return a + (b - a) * t;
    }

    std::array<float, NRelays> m_states{};
    std::array<float, NRelays> m_thresholdsUp{};
    std::array<float, NRelays> m_thresholdsDown{};
    float m_drive{1.4f}; // tweak for saturation flavor
};

class TapeHysteresis
{
  public:
    float processSample(float x) noexcept
    {
        // memory coefficient (0=full memory, 1=no memory)
        constexpr float memory = 0.9f;
        m_state = memory * m_state + (1.0f - memory) * x;
        return std::tanh(m_state * drive); // shape/distort as needed
    }

  private:
    float m_state{0.0f};
    float drive{1.5f}; // Make parameterizable for flavor
};

class Tape
{
  public:
};

template <size_t BlockSize>
class TapeDelay final : public EffectBase
{
  public:
    TapeDelay(const float sampleRate)
        : EffectBase(sampleRate)
        , m_sr(sampleRate, std::make_shared<AbacDsp::SincFilter>(AbacDsp::init_4_128))
    {
        m_sr.setRatio(1.f);
        m_visualWavedata.resize(6000);
    }
    void setLevel(const float value)
    {
        m_level = std::pow(10.f, value / 20.f);
    }
    void setTapeLoopTime(const float value)
    {
        m_tapeLoopTime = value;
    }
    void setTapeSpeed(const float value)
    {
        m_tapeSpeed = value;
        m_sr.setRatio(m_tapeSpeed);
    }
    void setFeedback(const float value)
    {
        m_feedback = value;
    }
    void setHysteresis(const float value)
    {
        m_hysteresis = value;
    }

    void setSosTime(const float value)
    {
        m_sosTime = value;
    }
    void setSaturation(const float value)
    {
        m_saturation = value;
    }
    void setNoiseFloor(const float value)
    {
        m_noiseFloor = value;
    }
    void setNoiseDistribution(const float value)
    {
        m_noiseDistribution = value;
    }
    void setDelayTime1(const float value)
    {
        m_delayTime1 = value;
    }
    void setDelayLevel1(const float value)
    {
        m_delayLevel1 = std::pow(10.f, value / 20.f);
    }
    void setDelayTime2(const float value)
    {
        m_delayTime2 = value;
    }
    void setDelayLevel2(const float value)
    {
        m_delayLevel2 = std::pow(10.f, value / 20.f);
    }
    void setDelayTime3(const float value)
    {
        m_delayTime3 = value;
    }
    void setDelayLevel3(const float value)
    {
        m_delayLevel3 = std::pow(10.f, value / 20.f);
    }
    void setDelayTime4(const float value)
    {
        m_delayTime4 = value;
    }
    void setDelayLevel4(const float value)
    {
        m_delayLevel4 = std::pow(10.f, value / 20.f);
    }

    void processBlock(const AudioBuffer<2, BlockSize>& in, AudioBuffer<2, BlockSize>& out)
    {
        std::array<float, BlockSize * 2> m_tmp{};
        for (size_t i = 0; i < BlockSize; ++i)
        {
            m_tmp[i * 2 + 0] = in(i, 0) * m_level;
            m_tmp[i * 2 + 1] = in(i, 1) * m_level / 2;
        }
        m_sr.process(m_tmp.data(), BlockSize);
        m_sr.readBlock(m_tmp.data(), BlockSize);
        for (size_t i = 0; i < BlockSize; ++i)
        {
            out(i, 0) = m_tmp[i * 2 + 0];
            out(i, 1) = m_tmp[i * 2 + 1];
        }

        m_visualWavedata[m_currentSample] = out(0, 0) * 0.5f;
        m_currentSample++;
        if (m_currentSample >= m_visualWavedata.size())
        {
            m_currentSample = 0;
        }
    }
    const std::vector<float>& visualizeWaveData()
    {
        m_preparedWavedata.resize(m_visualWavedata.size());
        m_preparedWavedata = m_visualWavedata;
        return m_preparedWavedata;
    }

  private:
    float m_level{1.f};
    float m_tapeLoopTime{10};
    float m_tapeSpeed{1.f};
    float m_feedback{0.f};
    float m_hysteresis{0.f};
    float m_sosTime{0.f};
    float m_saturation{0.f};
    float m_noiseFloor{0.f};
    float m_noiseDistribution{0.f};
    float m_delayTime1{200.f};
    float m_delayLevel1{1.f};
    float m_delayTime2{50.f};
    float m_delayLevel2{0.5f};
    float m_delayTime3{50.f};
    float m_delayLevel3{0.25f};
    float m_delayTime4{50.f};
    float m_delayLevel4{0.125};

    AbacDsp::VariReader<48000 * 2, 2> m_sr;
    std::vector<float> m_visualWavedata;
    std::vector<float> m_preparedWavedata;
    size_t m_currentSample = 0;
};