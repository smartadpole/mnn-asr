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

namespace MNN_ASR {

// 前向声明
class AsrEngine;

// ASR 配置结构体
struct AsrConfig {
    std::string model_path;           // 模型路径
    std::string config_path;          // 配置文件路径
    std::string tokenizer_path;       // 分词器路径
    int sample_rate = 16000;          // 采样率
    int num_channels = 1;             // 声道数
    bool use_gpu = false;             // 是否使用GPU
    int batch_size = 1;               // 批处理大小
    float chunk_size = 0.5f;          // 分块大小（秒）
    bool enable_vad = false;          // 是否启用VAD
    float vad_threshold = 0.5f;       // VAD阈值
};

// ASR 识别结果结构体
struct AsrResult {
    std::string text;                 // 识别的文本
    float confidence;                 // 置信度
    double start_time;                // 开始时间（秒）
    double end_time;                  // 结束时间（秒）
    bool is_final;                    // 是否为最终结果
};

// ASR 引擎类
class MNN_ASR_API AsrEngine {
public:
    // 创建 ASR 引擎实例
    static std::shared_ptr<AsrEngine> create(const AsrConfig& config);
    
    // 析构函数
    virtual ~AsrEngine();
    
    // 初始化引擎
    bool initialize();
    
    // 检查是否已初始化
    bool is_initialized() const;
    
    // 离线识别音频文件
    AsrResult recognize_file(const std::string& audio_file_path);
    
    // 离线识别音频数据
    AsrResult recognize_audio(const std::vector<float>& audio_data, int sample_rate = 16000);
    
    // 在线识别音频数据（流式）
    AsrResult recognize_stream(const std::vector<float>& audio_chunk, int sample_rate = 16000);
    
    // 重置在线识别状态
    void reset_stream();
    
    // 获取引擎信息
    std::string get_engine_info() const;
    
    // 获取配置信息
    AsrConfig get_config() const;

private:
    AsrEngine(const AsrConfig& config);
    AsrEngine(const AsrEngine&) = delete;
    AsrEngine& operator=(const AsrEngine&) = delete;
    
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

// 工具函数
namespace Utils {
    // 检查音频文件格式是否支持
    MNN_ASR_API bool is_audio_file_supported(const std::string& file_path);
    
    // 获取音频文件信息
    MNN_ASR_API bool get_audio_info(const std::string& file_path, int& sample_rate, int& channels, int& duration_ms);
    
    // 音频格式转换（如果需要）
    MNN_ASR_API bool convert_audio_format(const std::string& input_path, const std::string& output_path, 
                                         int target_sample_rate = 16000, int target_channels = 1);
}

} // namespace MNN_ASR

#endif // MNN_ASR_API_HPP
