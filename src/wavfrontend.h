//
// Created by smart on 2025/8/20.
//

#ifndef WAVFRONTEND_H
#define WAVFRONTEND_H

#include <memory>
#include <vector>
#include <MNN/expr/Expr.hpp>

#include "asr.hpp"


#define DIV_UP(a, b) (((a) + (b) - 1) / (b))

namespace SR
{
    class AsrConfig;
}

class WavFrontend
{
public:
    WavFrontend(std::shared_ptr<SR::AsrConfig> config);
    ~WavFrontend() = default;
    MNN::Express::VARP apply_lfr(MNN::Express::VARP samples);
    MNN::Express::VARP apply_cmvn(MNN::Express::VARP samples);
    MNN::Express::VARP extract_feat(MNN::Express::VARP samples);

private:
    std::shared_ptr<SR::AsrConfig> config_;
    std::vector<float> mean_;
    std::vector<float> var_;
    float dither_ = 1.0;
    int frame_length_ms_ = 25;
    int frame_shift_ms_ = 10;
    int sampling_rate = 16000;
    float preemphasis_coefficient = 0.97;
    int num_bins_ = 80;
    int lfr_m_ = 7;
    int lfr_n_ = 6;
    int feats_dims_ = 560;
};


#endif //WAVFRONTEND_H
