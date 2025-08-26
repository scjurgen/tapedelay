#pragma once

// https://www.youtube.com/watch?v=vaG5tVnpkwc

#include "EffectBase.h"

#include "Audio/AudioBuffer.h"


#include "Delays/VariSpeedTapeDelay.h"
#include "Filters/OnePoleFilter.h"
#include "Filters/PinkFilter.h"
#include "NonLinear/SimpleHysteresis.h"
#include "Samplerate/sinc_4_128.h"
#include "Samplerate/sinc_11_128.h"
#include "Samplerate/sinc_69_768.h"

#include <cassert>
#include <cstdint>
#include <cmath>
#include <functional>


/*

Practical DSP approach:

Generate white noise (Gaussian/distributed random).

Shape it with a bandpass (e.g., HPF at 500 Hz, LPF at 8-10 kHz)
and/or a bell boost centered at 2-3 kHz.

Optionally, modulate or dither its gain slightly to avoid a static sound.

band-limited noise 500 Hz–10 kHz and peaking it gently around 2–3 kHz.
Reference known tape emulation plugins and spectral plots for fine-tuning
the most authentic sound
*/


template <size_t BlockSize>
class TapeDelay final : public EffectBase
{
  public:
    static constexpr size_t NumChannels{2};
    static constexpr size_t NumReadHeads{4};
    static constexpr float TapeNormSpeed{7.5f};
    using OnePoleLP = AbacDsp::OnePoleFilter<AbacDsp::OnePoleFilterCharacteristic::LowPass>;
    using OnePoleHP = AbacDsp::OnePoleFilter<AbacDsp::OnePoleFilterCharacteristic::HighPass>;
    explicit TapeDelay(const float sampleRate)
        : EffectBase(sampleRate)
        , m_sampleRate(sampleRate)
        , m_lpIn{OnePoleLP(sampleRate), OnePoleLP(sampleRate)}
        , m_hpIn{OnePoleHP(sampleRate), OnePoleHP(sampleRate)}
        , m_lpOut{OnePoleLP(sampleRate), OnePoleLP(sampleRate)}
        , m_hpOut{OnePoleHP(sampleRate), OnePoleHP(sampleRate)}
        , m_sr(sampleRate, std::make_shared<AbacDsp::SincFilter>(AbacDsp::init_4_128))
        , m_hysteresis{SimpleHysteresis(sampleRate), SimpleHysteresis(sampleRate)}
    {
        for (auto& f : m_hpIn)
        {
            f.setCutoff(30.f);
        }
        for (auto& f : m_lpIn)
        {
            f.setCutoff(12000.f);
        }
        for (auto& f : m_hpOut)
        {
            f.setCutoff(20.f);
        }
        for (auto& f : m_lpOut)
        {
            f.setCutoff(18000.f);
        }
        m_sr.setRatio(1.f);
        m_visualWavedata.resize(6000);
    }


    void setFeedGain(const float value)
    {
        m_feedGain = std::pow(10.f, value / 20.f);
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

    void setWow(const float value)
    {
        m_wowFactor = value;
    }

    void setFeedback(const float value)
    {
        m_feedback = value;
    }

    void setHysteresis(const float v)
    {
        m_hysteresis[0].setFrequencyResponse(18000 - v * 9000, 18000 - v * 4500);
        m_hysteresis[1].setFrequencyResponse(18000 - v * 9000, 18000 - v * 4500);
    }

    void setSosTime(const float value)
    {
        m_sosTime = value;
    }

    void setSaturation(const float value)
    {
        m_saturation = std::pow(10.f, value);
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

    void processBlock(const AudioBuffer<NumChannels, BlockSize>& in, AudioBuffer<NumChannels, BlockSize>& out)
    {
        std::array<float, BlockSize * NumChannels> m_tmp{};
        for (size_t i = 0; i < BlockSize; ++i)
        {
            for (size_t c = 0; c < NumChannels; ++c)
            {
                auto v = in(i, c) * m_feedGain + m_feedbackTmp[i * NumChannels + c];
                v = m_hpIn[c].step(v);
                v = m_lpIn[c].step(v);
                v *= m_saturation;
                v = std::tanh(v);
                v = m_hysteresis[c].step(v);
                m_tmp[i * NumChannels + c] =
                    v + m_tapeFeedback[i * NumChannels + c] * m_feedbackFactor[NumReadHeads - 1];
            }
        }

        m_sr.process(m_tmp.data(), BlockSize);
        std::ranges::fill(m_tmp, 0.f);
        std::ranges::fill(m_feedbackTmp, 0.f);
        for (size_t hdIdx = 0; hdIdx < NumReadHeads - 1; ++hdIdx)
        {
            std::array<float, BlockSize * NumChannels> m_delay{};
            m_sr.readBlock(hdIdx, m_delay.data(), BlockSize);
            for (size_t i = 0, idx = 0; i < BlockSize; ++i, idx += NumChannels)
            {
                for (size_t c = 0; c < NumChannels; ++c)
                {
                    m_feedbackTmp[idx + c] += m_feedbackFactor[hdIdx] * m_delay[idx + c];
                    m_tmp[idx + c] += m_delayLevel[hdIdx] * m_delay[idx + c];
                }
            }
        }
        m_sr.readBlock(NumReadHeads - 1, m_tapeFeedback.data(), BlockSize);

        for (size_t i = 0, idx = 0; i < BlockSize; ++i, idx += NumChannels)
        {
            for (size_t c = 0; c < NumChannels; ++c)
            {
                m_feedbackTmp[idx + c] /= m_saturation;
            }
        }
        for (size_t i = 0; i < BlockSize; ++i)
        {
            for (size_t c = 0; c < NumChannels; ++c)
            {
                float v = m_tmp[i * NumChannels + c];
                v = m_hpOut[c].step(v);
                v = m_lpOut[c].step(v);
                out(i, c) = v + in(i, c);
            }
        }

        m_visualWavedata[m_currentSample] = (out(0, 0) + out(0, 1)) * 0.5f;
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
    // speed depending lowpass
    static float updateLowpassCoeff(const float sampleRate, const float baseCutoffHz, const float advance)
    {
        // Scale cutoff proportionally to speed
        float cutoff = baseCutoffHz * advance; // Linear relation
        cutoff = std::clamp(cutoff, 100.0f, sampleRate * 0.45f);
        return cutoff;
    }


    float m_sampleRate;
    float m_feedGain{1.f};
    float m_tapeLoopTime{10};
    float m_tapeSpeed{1.f};
    float m_wowFactor{0.f};
    float m_feedback{0.f};
    float m_sosTime{0.f};
    float m_saturation{1.f};
    float m_noiseFloor{0.f};
    float m_noiseDistribution{0.f};
    float m_hold{0.f};
    std::array<float, NumReadHeads> m_delayTime{200.f, 50.f, 50.f, 50.f};
    std::array<float, NumReadHeads> m_delayLevel{1.f, 0.5f, 0.25f, 0.125f};
    std::array<float, BlockSize * NumChannels> m_feedbackTmp{};
    std::array<float, BlockSize * NumChannels> m_tapeFeedback{};
    std::array<float, NumReadHeads> m_feedbackFactor{};

    std::array<OnePoleLP, NumChannels> m_lpIn;
    std::array<OnePoleHP, NumChannels> m_hpIn;
    std::array<OnePoleLP, NumChannels> m_lpOut;
    std::array<OnePoleHP, NumChannels> m_hpOut;

    AbacDsp::VariSpeedTapeDelay<48000 * 20, 2> m_sr;
    std::vector<float> m_visualWavedata;
    std::vector<float> m_preparedWavedata;
    size_t m_currentSample = 0;
    std::array<PinkFilter, NumChannels> m_pink;
    std::array<SimpleHysteresis, NumChannels> m_hysteresis;
};
