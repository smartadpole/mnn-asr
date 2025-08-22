//
//  mnn_asr_api.hpp
//  MNN ASR API
//
//  Created by MNN on 2024/10/31.
//  ZhaodeWang
//

#ifndef MNN_ASR_API_HPP
#define MNN_ASR_API_HPP

#include <string>
#include <memory>
#include <vector>

#ifdef _WIN32
#ifdef MNN_ASR_EXPORTS
        #define MNN_ASR_API __declspec(dllexport)
#else
        #define MNN_ASR_API __declspec(dllimport)
#endif
#else
#define MNN_ASR_API __attribute__((visibility("default")))
#endif

namespace asl
{
    struct Config
    {
        std::string model_path; // 模型路径
        std::string config_path; // 配置文件路径
        std::string tokenizer_path; // 分词器路径
        int sample_rate = 16000; // 采样率
        int num_channels = 1; // 声道数
        bool use_gpu = false; // 是否使用GPU
        int batch_size = 1; // 批处理大小
        float chunk_size = 0.5f; // 分块大小（秒）
        bool enable_vad = false; // 是否启用VAD
        float vad_threshold = 0.5f; // VAD阈值
    };

    struct Result
    {
        std::string text; // 识别的文本
        float confidence; // 置信度
        double start_time; // 开始时间（秒）
        double end_time; // 结束时间（秒）
        bool is_final; // 是否为最终结果
    };

    // ASR 引擎类
    class MNN_ASR_API SpeechEngine
    {
    public:
        // 析构函数
        virtual ~SpeechEngine();

        // 初始化引擎
        virtual bool Init(const Config& config) = 0;

        // 检查是否已初始化
        virtual bool IsInitialized() const = 0;

        // 离线识别音频文件
        virtual Result Recognize(const std::string& audio_file_path) = 0;

        // 离线识别音频数据
        virtual Result Recognize(const std::vector<float>& audio_data, int sample_rate = 16000) = 0;

        // 重置在线识别状态
        virtual void ResetStream() = 0;

        // 获取引擎信息
        virtual std::string GetEngineInfo() const = 0;

        // 获取配置信息
        virtual Config GetConfig() const = 0;

    protected:
        SpeechEngine()
        {
        }

    public:
        SpeechEngine(const SpeechEngine&) = delete;
        SpeechEngine& operator=(const SpeechEngine&) = delete;
    };


    // 创建 ASR 引擎实例
    std::shared_ptr<SpeechEngine> Create();

    // 工具函数
    namespace Utils
    {
        // 检查音频文件格式是否支持
        MNN_ASR_API bool IsAudioFileSupported(const std::string& file_path);

        // 获取音频文件信息
        MNN_ASR_API bool GetAudioInfo(const std::string& file_path, int& sample_rate, int& channels, int& duration_ms);

        // 音频格式转换（如果需要）
        MNN_ASR_API bool ConvertAudioFormat(const std::string& input_path, const std::string& output_path,
                                            int target_sample_rate = 16000, int target_channels = 1);
    }
} // namespace MNN_ASR

#endif // MNN_ASR_API_HPP
