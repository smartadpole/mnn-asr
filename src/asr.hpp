//
//  asr.hpp
//
//  Created by MNN on 2024/10/31.
//  ZhaodeWang
//

#ifndef ASR_hpp
#define ASR_hpp

#include <vector>
#include <memory>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <streambuf>
#include <functional>
#include <unordered_map>

#include <MNN/expr/Expr.hpp>
#include <MNN/expr/Module.hpp>
#include <MNN/expr/MathOp.hpp>
#include <MNN/expr/NeuralNetWorkOp.hpp>


class WavFrontend;
class OnlineCache;

namespace SR
{
class AsrConfig;
class Tokenizer;

class MNN_PUBLIC Asr {
public:
    static Asr* createASR(const std::string& config_path);
    Asr(std::shared_ptr<AsrConfig> config) : config_(config), cache_(nullptr) {}
    virtual ~Asr();
    void load();
    std::string recognize(MNN::Express::VARP speech);
    void online_recognize(const std::string& wav_file);
private:
    void init_cache(int batch_size = 1);
    MNN::Express::VARP add_overlap_chunk(MNN::Express::VARP feats);
    MNN::Express::VARP position_encoding(MNN::Express::VARP sample);
    MNN::Express::VARPS cif_search(MNN::Express::VARP enc, MNN::Express::VARP alpha);
    std::string decode(MNN::Express::VARP logits);
    std::string infer(MNN::Express::VARP feats);
private:
    std::shared_ptr<AsrConfig> config_;
    std::shared_ptr<Tokenizer> tokenizer_;
    std::shared_ptr<WavFrontend> frontend_;
    std::shared_ptr<MNN::Express::Executor::RuntimeManager> runtime_manager_;
    std::vector<std::shared_ptr<MNN::Express::Module>> modules_;
    std::shared_ptr<OnlineCache> cache_;
    int feats_dims_;
    std::vector<int> chunk_size_;
};
}

#endif // ASR_hpp
