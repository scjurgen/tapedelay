#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "impl/TapeDelay.h"
#include "AudioFile/AudioFileIO.h"

TEST(Tapedelaytest, failed)
{
    constexpr size_t BlockSize{16};
    TapeDelay<BlockSize> sut{48000};
    sut.setTapeSpeed(.5f);
    AudioBuffer<2, BlockSize> inBuffer;
    AudioBuffer<2, BlockSize> outBuffer;
    std::vector<float> results[2];
    float phase = 0.f;
    float phaseAdvance = 1000.f / 48000.f * 3.1415926535 * 2;

    for (size_t i = 0; i < 10000; ++i)
    {
        for (size_t j = 0; j < BlockSize; ++j)
        {
            inBuffer(j, 0) = sin(phase);
            inBuffer(j, 1) = cos(phase);
            phase += phaseAdvance;
        }
        sut.processBlock(inBuffer, outBuffer);
        for (size_t j = 0; j < BlockSize; ++j)
        {
            results[0].push_back(outBuffer(j, 0));
            results[1].push_back(outBuffer(j, 1));
        }
    }
    auto res = AudioUtility::FileOut::ToAudioFile(results[0], results[1], 48000);
    res.save("/tmp/test.wav");
    sut.writeBufferToAudio();
}
