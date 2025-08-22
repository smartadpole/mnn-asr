//
// Created by smart on 2025/8/22.
//

#include "reconstruct_speech_impl.h"


SpeechEngineImpl::SpeechEngineImpl(): is_initialized_(false)
{
}

SpeechEngineImpl::~SpeechEngineImpl()
{
}


bool SpeechEngineImpl::Init(const asl::Config& config)
{
    try
    {
        // 创建 ASR 配置
        auto asr_config = std::make_shared<SR::AsrConfig>();

        // 读取配置文件
        if (!config_.config_path.empty())
        {
            if (false) //(!asr_config->load_from_file(config_.config_path))
            {
                std::cerr << "Failed to load config file: " << config_.config_path << std::endl;
                return false;
            }
        }

        // 创建 ASR 实例
        asr_ = std::unique_ptr<SR::Asr>(SR::Asr::createASR(config_.config_path));
        if (!asr_)
        {
            std::cerr << "Failed to create ASR instance" << std::endl;
            return false;
        }

        // 加载模型
        asr_->load();

        is_initialized_ = true;
        return true;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Initialization error: " << e.what() << std::endl;
        return false;
    }
}

bool SpeechEngineImpl::IsInitialized() const
{
    return is_initialized_ && asr_ != nullptr;
}

asl::Result SpeechEngineImpl::Recognize(const std::string& audio_file_path)
{
    asl::Result result;
    result.text = "";
    result.confidence = 0.0f;
    result.start_time = 0.0;
    result.end_time = 0.0;
    result.is_final = true;

    if (!IsInitialized())
    {
        std::cerr << "ASR engine not initialized" << std::endl;
        return result;
    }

    try
    {
        // 使用现有的离线识别功能
        asr_->online_recognize(audio_file_path);

        // 注意：这里需要根据实际的 ASR 实现来获取结果
        // 由于现有的实现可能没有返回识别结果，我们需要修改或扩展它
        result.text = "Audio recognized successfully"; // 临时占位符
        result.confidence = 0.8f;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Recognition error: " << e.what() << std::endl;
    }

    return result;
}

asl::Result SpeechEngineImpl::Recognize(const std::vector<float>& audio_data, int sample_rate)
{
    asl::Result result;
    result.text = "";
    result.confidence = 0.0f;
    result.start_time = 0.0;
    result.end_time = 0.0;
    result.is_final = true;

    if (!IsInitialized())
    {
        std::cerr << "ASR engine not initialized" << std::endl;
        return result;
    }

    try
    {
        // 将音频数据转换为 MNN 的 VARP 格式
        // 这里需要根据实际的 MNN 接口来实现
        // MNN::Express::VARP speech = convert_audio_to_varp(audio_data, sample_rate);
        // result.text = asr_->recognize(speech);

        result.text = "Audio data recognized successfully"; // 临时占位符
        result.confidence = 0.8f;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Recognition error: " << e.what() << std::endl;
    }

    return result;
}

void SpeechEngineImpl::ResetStream()
{
    // 重置流式识别状态
    // 这里需要根据实际的实现来重置相关状态
}

std::string SpeechEngineImpl::GetEngineInfo() const
{
    std::stringstream ss;
    ss << "MNN ASR Engine v1.0\n";
    ss << "Initialized: " << (is_initialized_ ? "Yes" : "No") << "\n";
    ss << "Config path: " << config_.config_path << "\n";
    ss << "Sample rate: " << config_.sample_rate << " Hz\n";
    ss << "Channels: " << config_.num_channels << "\n";
    ss << "GPU enabled: " << (config_.use_gpu ? "Yes" : "No") << "\n";
    return ss.str();
}

asl::Config SpeechEngineImpl::GetConfig() const
{
    return config_;
}