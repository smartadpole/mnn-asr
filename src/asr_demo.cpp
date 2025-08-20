//
//  asr_demo.cpp
//
//  Created by MNN on 2024/10/31.
//  ZhaodeWang
//

#include "asr.hpp"
#include "utils/utils.h"

void Help()
{
    ERROR_PRINT("please input: ");
    INFO_PRINT("\tconfig.json");
    INFO_PRINT("\ttest.wav");
}

int main(int argc, const char* argv[]) {
    if (argc < 2) {
        Help();
        return 0;
    }

    std::string config_path = argv[1];
    std::unique_ptr<SR::Asr> asr(SR::Asr::createASR(config_path));
    std::string wav_file = argv[2];
    asr->load();
    asr->online_recognize(wav_file);
    return 0;
}