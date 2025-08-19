#pragma once

#include "EffectBase.h"

#include "Audio/AudioBuffer.h"
#include "Samplerate/VariReader.h"
#include "Samplerate/sinc_4_128.h"
#include "Samplerate/sinc_11_128.h"
#include <cassert>
#include <cstdint>
#include <cmath>
#include <functional>


template <size_t BlockSize>
class TapeDelay final : public EffectBase
{
  public:
    explicit TapeDelay(const float sampleRate)
        : EffectBase(sampleRate)
        , m_sampleRate(sampleRate)
        , m_sr(sampleRate, std::make_shared<AbacDsp::SincFilter>(AbacDsp::init_11_128))
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
        m_sr.setReadHead(m_delayTime1 / 1000 * m_sampleRate);
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
        std::array<float, BlockSize * 2> m_delay1{};
        m_sr.readBlock(m_delay1.data(), BlockSize);
        for (size_t i = 0; i < BlockSize; ++i)
        {
            out(i, 0) = m_tmp[i * 2 + 0] + m_delayLevel1 * m_delay1[i * 2 + 0];
            out(i, 1) = m_tmp[i * 2 + 1] + m_delayLevel1 * m_delay1[i * 2 + 1];
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
    float m_sampleRate;
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
    std::array<float, BlockSize * 2> m_feedbackTmp{};


    AbacDsp::VariReader<48000 * 60, 2> m_sr;
    std::vector<float> m_visualWavedata;
    std::vector<float> m_preparedWavedata;
    size_t m_currentSample = 0;
};
