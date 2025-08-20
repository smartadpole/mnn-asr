//
// Created by smart on 2025/8/20.
//

#include "wavfrontend.h"
#include "asrconfig.hpp"
#include <MNN/expr/Expr.hpp>
#include <MNN/expr/ExprCreator.hpp>
#include <audio/audio.hpp>

MNN::Express::VARP WavFrontend::apply_cmvn(MNN::Express::VARP samples)
{
    auto mean = MNN::Express::_Const(mean_.data(), {static_cast<int>(mean_.size())});
    auto var = MNN::Express::_Const(var_.data(), {static_cast<int>(mean_.size())});
    samples = (samples + mean) * var;
    return samples;
}

WavFrontend::WavFrontend(std::shared_ptr<SR::AsrConfig> config): config_(config)
{
    mean_ = config->mean();
    var_ = config->var();
}

MNN::Express::VARP WavFrontend::apply_lfr(MNN::Express::VARP samples)
{
    auto dim = samples->getInfo()->dim;
    int row = dim[0];
    int padding_len = (lfr_m_ - 1) / 2;
    int t_lfr = DIV_UP(row, lfr_n_);
    std::vector<int> lfr_regions = {
        // region 0
        0, // src offset
        1, 0, 1, // src strides
        0, // dst offset
        1, num_bins_, 1, // dst strides
        1, padding_len, num_bins_, // dst sizes
        // region 1
        0, // src offset
        1, num_bins_, 1, // src strides
        padding_len * num_bins_, // dst offset
        1, num_bins_, 1, // dst strides
        1, lfr_m_ - padding_len, num_bins_, // dst sizes
        // region 2
        (lfr_n_ - padding_len) * num_bins_, // src offset
        lfr_n_ * num_bins_, num_bins_, 1, // src strides
        lfr_m_ * num_bins_, // dst offset
        lfr_m_ * num_bins_, num_bins_, 1, // dst strides
        t_lfr, lfr_m_, num_bins_ // dst sizes
    };
    samples = MNN::Express::_Raster({samples, samples, samples}, lfr_regions, {1, t_lfr, lfr_m_ * num_bins_});
    return samples;
}

MNN::Express::VARP WavFrontend::extract_feat(MNN::Express::VARP waveforms)
{
    waveforms = waveforms * MNN::Express::_Scalar<float>(32768);
    auto feature = MNN::AUDIO::fbank(waveforms);
    feature = apply_lfr(feature);
    feature = apply_cmvn(feature);
    return feature;
}
