//
//  mnn_asr_api.cpp
//  MNN ASR API Implementation
//
//  Created by MNN on 2024/10/31.
//  ZhaodeWang
//

#include "reconstruct_speech.hpp"
#include "asr.hpp"
#include "asrconfig.hpp"
#include "tokenizer.hpp"
#include "wavfrontend.h"
#include "utils/utils.h"
#include "utils/timer.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <memory>
#include "reconstruct_speech_impl.h"

using namespace asl;


SpeechEngine::~SpeechEngine() = default;

std::shared_ptr<SpeechEngine> asl::Create()
{
    return std::shared_ptr<SpeechEngine>(static_cast<SpeechEngine*>(new SpeechEngineImpl()));
}

// 工具函数实现

bool asl::Utils::IsAudioFileSupported(const std::string& file_path)
{
    std::string extension = file_path.substr(file_path.find_last_of(".") + 1);
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

    return (extension == "wav" || extension == "mp3" || extension == "flac" ||
        extension == "m4a" || extension == "aac" || extension == "ogg");
}

bool asl::Utils::GetAudioInfo(const std::string& file_path, int& sample_rate, int& channels, int& duration_ms)
{
    // 这里需要实现音频文件信息读取
    // 可以使用现有的音频库或者系统调用

    // 临时实现，返回默认值
    sample_rate = 16000;
    channels = 1;
    duration_ms = 0;

    return true;
}

bool asl::Utils::ConvertAudioFormat(const std::string& input_path, const std::string& output_path,
                                    int target_sample_rate, int target_channels)
{
    // 这里需要实现音频格式转换
    // 可以使用 ffmpeg 或其他音频处理库

    // 临时实现
    std::cout << "Converting " << input_path << " to " << output_path << std::endl;
    std::cout << "Target: " << target_sample_rate << " Hz, " << target_channels << " channels" << std::endl;

    return true;
}
