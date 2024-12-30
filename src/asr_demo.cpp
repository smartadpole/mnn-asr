//
//  asr_demo.cpp
//
//  Created by MNN on 2024/10/31.
//  ZhaodeWang
//

#include "asr.hpp"

using namespace MNN::Transformer;

int main(int argc, const char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " config.json test.wav" << std::endl;
        return 0;
    }

    std::string config_path = argv[1];
    std::unique_ptr<Asr> asr(Asr::createASR(config_path));
    std::string wav_file = argv[2];
    asr->load();
    asr->online_recognize(wav_file);
    return 0;
}