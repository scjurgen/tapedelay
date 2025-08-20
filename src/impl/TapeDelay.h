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
    static constexpr size_t NumChannels{2};
    static constexpr size_t NumReadHeads{4};
    static constexpr float TapeNormSpeed{7.5f};

    explicit TapeDelay(const float sampleRate)
        : EffectBase(sampleRate)
        , m_sampleRate(sampleRate)
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
        m_tapeSpeed = value / TapeNormSpeed;
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

    void setDelayTime(const size_t hdIdx, const float value)
    {
        m_delayTime[hdIdx] = value;
        m_sr.setReadHead(hdIdx, m_delayTime[hdIdx] / 1000 * m_sampleRate);
    }
    void setDelayTime1(const float value)
    {
        setDelayTime(0, value);
    }
    void setDelayTime2(const float value)
    {
        setDelayTime(1, value);
    }
    void setDelayTime3(const float value)
    {
        setDelayTime(2, value);
    }

    void setDelayTime4(const float value)
    {
        setDelayTime(3, value);
    }

    void setDelayLevel(const size_t hdIdx, const float value)
    {
        m_delayLevel[hdIdx] = std::pow(10.f, value / 20.f);
    }

    void setDelayLevel1(const float value)
    {
        setDelayLevel(0, value);
    }

    void setDelayLevel2(const float value)
    {
        setDelayLevel(1, value);
    }

    void setDelayLevel3(const float value)
    {
        setDelayLevel(2, value);
    }
    void setDelayLevel4(const float value)
    {
        setDelayLevel(3, value);
    }

    void setFeedback(const size_t hdIdx, const float value)
    {
        m_feedbackFactor[hdIdx] = value;
    }

    void setFeedback1(const float value)
    {
        setFeedback(0, value);
    }
    void setFeedback2(const float value)
    {
        setFeedback(1, value);
    }
    void setFeedback3(const float value)
    {
        setFeedback(2, value);
    }
    void setFeedback4(const float value)
    {
        setFeedback(3, value);
    }

    void processBlock(const AudioBuffer<2, BlockSize>& in, AudioBuffer<2, BlockSize>& out)
    {
        std::array<float, BlockSize * 2> m_tmp{};
        for (size_t i = 0; i < BlockSize; ++i)
        {
            for (size_t c = 0; c < NumChannels; ++c)
            {
                m_tmp[i * NumChannels + c] = in(i, c) * m_level + m_feedbackTmp[i * NumChannels + c];
            }
        }
        m_sr.process(m_tmp.data(), BlockSize);
        std::ranges::fill(m_tmp, 0.f);
        std::ranges::fill(m_feedbackTmp, 0.f);
        for (size_t hdIdx = 0; hdIdx < NumReadHeads; ++hdIdx)
        {
            std::array<float, BlockSize * NumChannels> m_delay{};
            m_sr.readBlock(hdIdx, m_delay.data(), BlockSize);
            for (size_t i = 0, idx = 0; i < BlockSize; ++i, idx += 2)
            {
                for (size_t c = 0; c < NumChannels; ++c)
                {
                    m_feedbackTmp[idx + c] += m_feedbackFactor[hdIdx] * m_delay[idx + c];
                    m_tmp[idx + c] += m_delayLevel[hdIdx] * m_delay[idx + c];
                }
            }
        }
        for (size_t i = 0; i < BlockSize; ++i)
        {
            for (size_t c = 0; c < NumChannels; ++c)
            {
                out(i, c) = m_tmp[i * NumChannels + c] + in(i, c);
            }
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
    std::array<float, NumReadHeads> m_delayTime{200.f, 50.f, 50.f, 50.f};
    std::array<float, NumReadHeads> m_delayLevel{1.f, 0.5f, 0.25f, 0.125f};
    std::array<float, BlockSize * 2> m_feedbackTmp{};
    std::array<float, NumReadHeads> m_feedbackFactor{};

    AbacDsp::VariReader<48000 * 60, 2> m_sr;
    std::vector<float> m_visualWavedata;
    std::vector<float> m_preparedWavedata;
    size_t m_currentSample = 0;
};
