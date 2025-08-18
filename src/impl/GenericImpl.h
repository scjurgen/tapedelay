#pragma once

#include "EffectBase.h"
#include "Analysis/Spectrogram.h"
#include "Audio/AudioBuffer.h"

#include <cassert>
#include <cstdint>
#include <cmath>
#include <functional>

template <size_t BlockSize>
class GenericImpl final : public EffectBase
{
  public:
    GenericImpl(const float sampleRate)
        : EffectBase(sampleRate)
    {
        m_visualWavedata.resize(6000);
    }
    void setLevel(const float value)
    {
        m_level = std::pow(10.f, value / 20.f);
    }
    void setTapeSpeed(const float value)
    {
        m_tapeSpeed = value;
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
        for (size_t i = 0; i < BlockSize; ++i)
        {
            out(i, 0) = in(i, 0);
            out(i, 1) = in(i, 1);
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
    float m_level{};
    float m_tapeSpeed{};
    float m_feedback{};
    float m_hysteresis{};
    float m_sosTime{};
    float m_saturation{};
    float m_noiseFloor{};
    float m_noiseDistribution{};
    float m_delayTime1{};
    float m_delayLevel1{};
    float m_delayTime2{};
    float m_delayLevel2{};
    float m_delayTime3{};
    float m_delayLevel3{};
    float m_delayTime4{};
    float m_delayLevel4{};


    std::vector<float> m_visualWavedata;
    std::vector<float> m_preparedWavedata;
    size_t m_currentSample = 0;
};