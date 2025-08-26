#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "impl/GenericImpl.h"
#include "impl/TapeDelay.h"

TEST(Tapedelaytest, balance)
{
    constexpr size_t BlockSize{16};
    TapeDelay<BlockSize> sut{48000};
    sut.setDelayLevel1(1.f);
    sut.setDelayTime1(10.f);
    AudioBuffer<2, BlockSize> in{};
    AudioBuffer<2, BlockSize> out{};
    in(0, 0) = 1;
    in(0, 1) = 1;
    for (size_t slice = 0; slice < 100; ++slice)
    {
        sut.processBlock(in, out);
        for (size_t j = 0; j < BlockSize; ++j)
        {
            if (out(j, 0) != 0.f || out(j, 1) != 0.f)
            {
                std::cout << slice << "." << j << "\t" << out(j, 0) << "\t" << out(j, 1) << std::endl;
            }
        }
        in(0, 0) = 0;
        in(0, 1) = 0;
    }
}
