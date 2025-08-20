//
//  asr.cpp
//
//  Created by MNN on 2024/10/31.
//  ZhaodeWang
//

#include "asr.hpp"
#include <audio/audio.hpp>
#include <cmath>
#include <complex>
#include <random>
#include "utils/utils.h"
#include "utils/timer.h"

#include "asrconfig.hpp"
#include "tokenizer.hpp"
#include "wavfrontend.h"

// #define USE_CPU


struct OnlineCache
{
    int start_idx = 0;
    bool is_final = false;
    bool last_chunk = false;
    std::vector<int> chunk_size;
    MNN::Express::VARP cif_hidden;
    MNN::Express::VARP cif_alphas;
    MNN::Express::VARP feats;
    std::vector<MNN::Express::VARP> decoder_fsmn;
    std::vector<int> tokens;
};

namespace SR
{
    template <typename T>
    static inline MNN::Express::VARP _var(std::vector<T> vec, const std::vector<int>& dims)
    {
        return MNN::Express::_Const(vec.data(), dims, MNN::Express::NHWC, halide_type_of<T>());
    }

    static inline MNN::Express::VARP _zeros(const std::vector<int>& dims)
    {
        std::vector<float> data(std::accumulate(dims.begin(), dims.end(), 1, std::multiplies<int>()), 0);
        return MNN::Express::_Const(data.data(), dims, MNN::Express::NCHW, halide_type_of<float>());
    }
}


static void dump_impl(const float* signal, size_t size, int row = 0)
{
    if (row)
    {
        constexpr int lines = 3;
        int col = size / row;
        printf("# %d, %d: [\n", row, col);
        for (int i = 0; i < lines; i++)
        {
            for (int j = 0; j < 3; j++)
            {
                printf("%f, ", signal[i * col + j]);
            }
            printf("..., ");
            for (int j = col - 3; j < col; j++)
            {
                printf("%f, ", signal[i * col + j]);
            }
            printf("\n");
        }
        printf("..., \n");
        for (int i = row - lines; i < row; i++)
        {
            for (int j = 0; j < 3; j++)
            {
                printf("%f, ", signal[i * col + j]);
            }
            printf("..., ");
            for (int j = col - 3; j < col; j++)
            {
                printf("%f, ", signal[i * col + j]);
            }
            printf("\n");
        }
        printf("]\n");
    }
    else
    {
        printf("# %lu: [", size);
        for (int i = 0; i < 3; i++)
        {
            printf("%f, ", signal[i]);
        }
        printf("..., ");
        for (int i = size - 3; i < size; i++)
        {
            printf("%f, ", signal[i]);
        }
        printf("]\n");
    }
}

static void dump(const std::vector<float>& signal, int row = 0)
{
    dump_impl(signal.data(), signal.size(), row);
}

void dump_var(MNN::Express::VARP var)
{
    auto dims = var->getInfo()->dim;
    bool isfloat = true;
    printf("{\ndtype = ");
    if (var->getInfo()->type == halide_type_of<float>())
    {
        printf("float");
        isfloat = true;
    }
    else if (var->getInfo()->type == halide_type_of<int>())
    {
        printf("int");
        isfloat = false;
    }
    printf("\nformat = %d\n", var->getInfo()->order);
    printf("\ndims = [");
    for (int i = 0; i < dims.size(); i++)
    {
        printf("%d ", dims[i]);
    }
    printf("]\n");

    if (isfloat)
    {
        if ((dims.size() > 2 && dims[1] > 1 && dims[2] > 1) || (dims.size() == 2 && dims[0] > 1 && dims[1] > 1))
        {
            int row = dims[dims.size() - 2];
            if (dims.size() > 2 && dims[0] > 1)
            {
                row *= dims[0];
            }
            dump_impl(var->readMap<float>(), var->getInfo()->size, row);
        }
        else
        {
            printf("data = [");
            auto total = var->getInfo()->size;
            if (total > 32)
            {
                for (int i = 0; i < 5; i++)
                {
                    printf("%f ", var->readMap<float>()[i]);
                }
                printf("..., ");
                for (int i = total - 5; i < total; i++)
                {
                    printf("%f ", var->readMap<float>()[i]);
                }
            }
            else
            {
                for (int i = 0; i < total; i++)
                {
                    printf("%f ", var->readMap<float>()[i]);
                }
            }
            printf("]\n}\n");
        }
    }
    else
    {
        printf("data = [");
        int size = var->getInfo()->size > 10 ? 10 : var->getInfo()->size;
        for (int i = 0; i < size; i++)
        {
            printf("%d ", var->readMap<int>()[i]);
        }
        printf("]\n}\n");
    }
}


MNN::Express::VARP SR::Asr::position_encoding(MNN::Express::VARP samples)
{
    auto ptr = (float*)samples->readMap<float>();
    auto dims = samples->getInfo()->dim;
    int length = dims[1];
    int feat_dims = dims[2];
    constexpr float neglog_timescale = -0.03301197265941284;
    for (int i = 0; i < length; i++)
    {
        int offset = i + 1 + cache_->start_idx;
        for (int j = 0; j < feat_dims / 2; j++)
        {
            float inv_timescale = offset * std::exp(j * neglog_timescale);
            ptr[i * feat_dims + j] += std::sin(inv_timescale);
            ptr[i * feat_dims + j + feat_dims / 2] += std::cos(inv_timescale);
        }
    }
    cache_->start_idx += length;
    return samples;
}

void SR::Asr::init_cache(int batch_size)
{
    cache_.reset(new OnlineCache);
    cache_->start_idx = 0;
    cache_->is_final = false;
    cache_->last_chunk = false;
    cache_->chunk_size = chunk_size_;
    cache_->cif_hidden = SR::_zeros({batch_size, 1, config_->encoder_output_size()});
    cache_->cif_alphas = SR::_zeros({batch_size, 1});
    cache_->feats = SR::_zeros({batch_size, chunk_size_[0] + chunk_size_[2], feats_dims_});
    for (int i = 0; i < config_->fsmn_layer(); i++)
    {
        cache_->decoder_fsmn.emplace_back(SR::_zeros({
            batch_size, config_->fsmn_dims(), config_->fsmn_lorder()
        }));
    }
}

MNN::Express::VARP SR::Asr::add_overlap_chunk(MNN::Express::VARP feats)
{
    if (!cache_) return feats;
    feats = MNN::Express::_Concat({cache_->feats, feats}, 1);
    if (cache_->is_final)
    {
        cache_->feats = MNN::Express::_Slice(feats, SR::_var<int>({0, -chunk_size_[0], 0}, {3}),
                                             SR::_var<int>({-1, -1, -1}, {3}));
        if (!cache_->last_chunk)
        {
            int padding_length = std::accumulate(chunk_size_.begin(), chunk_size_.end(), 0) - feats->getInfo()->
                dim[1];
            feats = MNN::Express::_Pad(feats, SR::_var<int>({0, 0, 0, padding_length, 0, 0}, {3, 2}));
        }
    }
    else
    {
        cache_->feats = MNN::Express::_Slice(
            feats, SR::_var<int>({0, -(chunk_size_[0] + chunk_size_[2]), 0}, {3}),
            SR::_var<int>({-1, -1, -1}, {3}));
    }
    return feats;
}

MNN::Express::VARPS SR::Asr::cif_search(MNN::Express::VARP hidden, MNN::Express::VARP alphas)
{
    auto chunk_alpha_ptr = const_cast<float*>(alphas->readMap<float>());
    for (int i = 0; i < alphas->getInfo()->size; i++)
    {
        if (i < chunk_size_[0] || i >= chunk_size_[0] + chunk_size_[1])
        {
            chunk_alpha_ptr[i] = 0.f;
        }
    }
    if (cache_->last_chunk)
    {
        int hidden_size = hidden->getInfo()->dim[2];
        auto tail_hidden = SR::_zeros({1, 1, hidden_size});
        auto tail_alphas = SR::_var<float>({config_->tail_threshold()}, {1, 1});
        hidden = MNN::Express::_Concat({cache_->cif_hidden, hidden, tail_hidden}, 1);
        alphas = MNN::Express::_Concat({cache_->cif_alphas, alphas, tail_alphas}, 1);
    }
    else
    {
        hidden = MNN::Express::_Concat({cache_->cif_hidden, hidden}, 1);
        alphas = MNN::Express::_Concat({cache_->cif_alphas, alphas}, 1);
    }
    auto alpha_ptr = alphas->readMap<float>();

    auto dims = hidden->getInfo()->dim;
    int batch_size = dims[0], len_time = dims[1], hidden_size = dims[2];
    auto frames = SR::_zeros({hidden_size});
    float cif_threshold = config_->cif_threshold();
    float integrate = 0.f;
    std::vector<MNN::Express::VARP> list_frame;
    for (int t = 0; t < len_time; t++)
    {
        float alpha = alpha_ptr[t];
        auto hidden_t = MNN::Express::_GatherV2(hidden, SR::_var<int>({t}, {1}),
                                                MNN::Express::_Scalar<int>(1));
        if (alpha + integrate < cif_threshold)
        {
            integrate += alpha;
            frames = frames + MNN::Express::_Scalar<float>(alpha) * hidden_t;
        }
        else
        {
            frames = frames + MNN::Express::_Scalar<float>(cif_threshold - integrate) * hidden_t;
            list_frame.push_back(frames);
            integrate += alpha;
            integrate -= cif_threshold;
            frames = MNN::Express::_Scalar<float>(integrate) * hidden_t;
        }
    }
    // update cache
    cache_->cif_alphas = SR::_var<float>({integrate}, {1, 1});
    cache_->cif_hidden = integrate > 0.f ? (frames / MNN::Express::_Scalar<float>(integrate)) : frames;
    return list_frame;
}


std::string SR::Asr::decode(MNN::Express::VARP logits)
{
    int token_num = logits->getInfo()->dim[1];
    auto token_ptr = _ArgMax(logits, -1)->readMap<int>();
    std::string text;
    for (int i = 0; i < token_num; i++)
    {
        int token = token_ptr[i];
        if (tokenizer_->is_special(token))
        {
            continue;
        }
        cache_->tokens.push_back(token);
        auto symbol = tokenizer_->decode(token);
        // end with '@@'
        if (symbol.size() > 2 && symbol.back() == '@' && symbol[symbol.size() - 2] == '@')
        {
            symbol = std::string(symbol.data(), symbol.size() - 2);
        }
        else
        {
            if (reinterpret_cast<const uint8_t*>(symbol.c_str())[0] < 0x80)
            {
                symbol.append(" ");
            }
        }
        text.append(symbol);
    }
    return text;
}

std::string SR::Asr::infer(MNN::Express::VARP feats)
{
    auto enc_len = MNN::Express::_Input({1}, MNN::Express::NCHW, halide_type_of<int>());
    enc_len->writeMap<int>()[0] = feats->getInfo()->dim[1];
    auto encoder_outputs = modules_[0]->onForward({feats, enc_len});
    auto alphas = encoder_outputs[0];
    auto enc = encoder_outputs[1];
    enc_len = encoder_outputs[2];
    auto acoustic_embeds_list = cif_search(enc, alphas);
    if (acoustic_embeds_list.empty())
    {
        return "";
    }
    auto acoustic_embeds = MNN::Express::_Concat(acoustic_embeds_list, 1);
    int acoustic_embeds_len = static_cast<int>(acoustic_embeds_list.size());
    MNN::Express::VARPS decocder_inputs{
        enc, enc_len, acoustic_embeds, SR::_var<int>({acoustic_embeds_len}, {1})
    };
    for (auto fsmn : cache_->decoder_fsmn)
    {
        decocder_inputs.push_back(fsmn);
    }
    auto decoder_outputs = modules_[1]->onForward(decocder_inputs);

    auto logits = decoder_outputs[0];
    for (int i = 0; i < config_->fsmn_layer(); i++)
    {
        cache_->decoder_fsmn[i] = decoder_outputs[2 + i];
    }
    auto text = decode(logits);
    // printf("%s", text.c_str());
    return text;
}

// std::string Asr::recognize(std::vector<float>& waveforms) {
std::string SR::Asr::recognize(MNN::Express::VARP waveforms)
{
    Timer timer;
    size_t wave_length = waveforms->getInfo()->size;
    if (wave_length < 16 * 60 && cache_->is_final)
    {
        cache_->last_chunk = true;
        return infer(cache_->feats);
    }
    // auto t1 = std::chrono::system_clock::now();
    auto feats = frontend_->extract_feat(waveforms);
    // std::cout << "feats time: " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - t1).count() << std::endl;

    feats = feats * MNN::Express::_Scalar<float>(std::sqrt(config_->encoder_output_size()));
    feats = position_encoding(feats);
    if (cache_->is_final)
    {
        auto dims = feats->getInfo()->dim;
        if (dims[1] + chunk_size_[2] <= chunk_size_[1])
        {
            cache_->last_chunk = true;
            feats = add_overlap_chunk(feats);
        }
        else
        {
            // first chunk
            auto feats1 = feats;
            if (dims[1] > chunk_size_[1])
            {
                feats1 = MNN::Express::_Slice(feats, SR::_var<int>({0, 0, 0}, {3}),
                                              SR::_var<int>({-1, chunk_size_[1], -1}, {3}));
            }
            auto feats_chunk1 = add_overlap_chunk(feats1);
            auto res1 = infer(feats_chunk1);
            // last chunk
            cache_->last_chunk = true;
            auto feat2 = feats;
            int start = dims[1] + chunk_size_[2] - chunk_size_[1];
            if (start != 0)
            {
                feat2 = MNN::Express::_Slice(feats, SR::_var<int>({0, -start, 0}, {3}),
                                             SR::_var<int>({-1, -1, -1}, {3}));
            }
            auto feats_chunk2 = add_overlap_chunk(feat2);
            auto res2 = infer(feats_chunk2);
            return res1 + res2;
        }
    }
    else
    {
        feats = add_overlap_chunk(feats);
    }

    DEBUG_PRINT(timer.TimingStr("preprocess"));
    std::string result = infer(feats);

    DEBUG_PRINT(timer.TimingStr("recognize"));
    return result;
}

void SR::Asr::online_recognize(const std::string& wav_file)
{
    Timer timer, timer_total;
    LOG_PRINT("load wav file from: " + wav_file);
    bool is_ok = false;
#if 0
    std::vector<float> speech = read_wave(wav_file, &sample_rate, &is_ok);
    if (!is_ok) {
        fprintf(stderr, "Failed to read '%s'\n", wav_file.c_str());
        return;
    }
    float mean_val = std::accumulate(speech.begin(), speech.end(), 0.f) / speech.size();
    std::cout << "### wav file: " << wav_file << ", mean_val: " << mean_val << std::endl;
    auto speech_length = speech.size();
#else
    auto audio_file = MNN::AUDIO::load(wav_file);
    auto speech = audio_file.first;
    int sample_rate = audio_file.second;
    auto speech_length = speech->getInfo()->size;
    auto speech_ptr = speech->readMap<int>();
    int start = 0;
    int end = speech_length - 1;
    int frame_length = speech_length / sample_rate; // second

    LOG_PRINT("audio length: " + std::to_string(frame_length) + "s, sample rate: " + std::to_string(sample_rate) + "Hz");

    while (start < speech_length)
    {
        if (speech_ptr[start] != 0)
        {
            break;
        }
        start++;
    }

    while (end >= 0)
    {
        if (speech_ptr[end] != 0)
        {
            break;
        }
        end--;
    }
    speech_length = end - start + 1;
#endif
    int chunk_size = chunk_size_[1] * 960;
    int steps = DIV_UP(speech_length, chunk_size);
    init_cache();
    std::string total = "";
    for (int i = 0; i < steps; i++)
    {
        int deal_size = chunk_size;
        if (i == steps - 1)
        {
            cache_->is_final = true;
            deal_size = speech_length - i * chunk_size;
        }
        // std::vector<float> chunk(speech.begin() + i * chunk_size, speech.begin() + i * chunk_size + deal_size);
        auto chunk = MNN::Express::_Slice(speech, SR::_var<int>({i * chunk_size + start}, {1}),
                                          SR::_var<int>({deal_size}, {1}));
        DEBUG_PRINT(timer.TimingStr("preprocess"));
        auto res = recognize(chunk);
        DEBUG_PRINT("preds: " + res);
        total += res;
        timer.TimingStr("");
    }
    LOG_PRINT(total);
    TIMING(timer_total.TimingStr("whole recognize"));
}

SR::Asr* SR::Asr::createASR(const std::string& config_path)
{
    std::shared_ptr<SR::AsrConfig> config(new SR::AsrConfig(config_path));
    return new Asr(config);
}

SR::Asr::~Asr()
{
}

void SR::Asr::load()
{
    Timer timer, timer_total;
    // 检查配置文件中的文件是否存在
    std::vector<std::pair<std::string, std::string>> files_to_check = {
        {"encoder_model", config_->encoder_model()},
        {"decoder_model", config_->decoder_model()},
        {"tokenizer_file", config_->tokenizer_file()}
    };

    for (const auto& [config_key, file_path] : files_to_check)
    {
        std::ifstream file_check(file_path);
        if (!file_check.is_open())
        {
            ERROR_PRINT("Error: File not found for config '" + config_key + "': " + file_path);
            ERROR_PRINT("Please check if the file exists and has correct permissions.");
            return;
        }
        file_check.close();
        INFO_PRINT("✓ Found file: " + file_path);
    }

    // 检查配置值是否有效
    if (config_->feats_dims() <= 0)
    {
        ERROR_PRINT("Error: feats_dims not configured properly or is invalid: " + config_->feats_dims());
        return;
    }

    if (config_->chunk_size().empty())
    {
        ERROR_PRINT("Error: chunk_size not configured properly or is empty");
        return;
    }

    if (config_->fsmn_layer() <= 0)
    {
        ERROR_PRINT("Error: fsmn_layer not configured properly or is invalid: " + config_->fsmn_layer());
        return;
    }

    INFO_PRINT("✓ Configuration validation passed");

    feats_dims_ = config_->feats_dims();
    chunk_size_ = config_->chunk_size();
    frontend_.reset(new WavFrontend(config_));

    // 创建tokenizer前再次检查文件
    LOG_PRINT("Loading tokenizer from: " + config_->tokenizer_file());
    tokenizer_.reset(Tokenizer::createTokenizer(config_->tokenizer_file()));
    if (!tokenizer_)
    {
        ERROR_PRINT("Error: Failed to create tokenizer from: " + config_->tokenizer_file());
        return;
    }
    INFO_PRINT("✓ Tokenizer loaded successfully");

    {
        MNN::ScheduleConfig config;
        MNN::BackendConfig config_backend;
        // if (MNN::BackendConfig::isOpenCLAvailable())
#ifdef USE_GPU
        config.type = MNN_FORWARD_CUDA;
        config_backend.power = MNN::BackendConfig::Power_Normal;
#else
        config.type = MNN_FORWARD_CPU;
        config_backend.power = MNN::BackendConfig::Power_Low;
#endif
        // config.type = MNN_FORWARD_VULKAN ;
        config.numThread = 4;
        config.backendConfig = &config_backend;

        runtime_manager_.reset(MNN::Express::Executor::RuntimeManager::createRuntimeManager(config));
        runtime_manager_->setHint(MNN::Interpreter::MEM_ALLOCATOR_TYPE, 0);
        runtime_manager_->setHint(MNN::Interpreter::DYNAMIC_QUANT_OPTIONS, 1);
    }

    modules_.resize(2);
    MNN::Express::Module::Config module_config;
    module_config.shapeMutable = true;
    module_config.rearrange = true;

    std::vector<std::string> encoder_inputs{"speech", "enc_len"};
    std::vector<std::string> encoder_outputs{"alphas", "enc", "enc_len"};
    std::vector<std::string> decoder_inputs{"enc", "enc_len", "acoustic_embeds", "acoustic_embeds_len"};
    std::vector<std::string> decoder_outputs{"logits", "sample_ids"};

    for (int i = 0; i < config_->fsmn_layer(); i++)
    {
        decoder_inputs.emplace_back("in_cache_" + std::to_string(i));
        decoder_outputs.emplace_back("out_cache_" + std::to_string(i));
    }
    DEBUG_PRINT(timer.TimingStr("check model"));

    // 加载encoder模型
    LOG_PRINT("Loading encoder model from: " + config_->encoder_model());
    modules_[0].reset(MNN::Express::Module::load(encoder_inputs, encoder_outputs, config_->encoder_model().c_str(),
                                                 runtime_manager_, &module_config));
    if (!modules_[0])
    {
        ERROR_PRINT("Error: Failed to load encoder model from: " + config_->encoder_model());
        return;
    }
    INFO_PRINT("✓ Encoder model loaded successfully");
    DEBUG_PRINT(timer.TimingStr("load encoder model"));

    // 加载decoder模型
    std::cout << "Loading decoder model from: " << config_->decoder_model() << std::endl;
    modules_[1].reset(MNN::Express::Module::load(decoder_inputs, decoder_outputs, config_->decoder_model().c_str(),
                                                 runtime_manager_, &module_config));
    if (!modules_[1])
    {
        ERROR_PRINT("Error: Failed to load decoder model from: " + config_->decoder_model());
        return;
    }

    INFO_PRINT("✓ Decoder model loaded successfully");
    INFO_PRINT("✓ All models and components loaded successfully!");
    DEBUG_PRINT(timer.TimingStr("load decoder model"));
    TIMING(timer_total.TimingStr("whole load model"));
}
