//
//  asr_sample.cpp
//  MNN ASR Sample Program
//
//  Created by MNN on 2024/10/31.
//  ZhaodeWang
//

#include "../src/mnn_asr_api.hpp"
#include <iostream>
#include <string>
#include <chrono>
#include <thread>

void print_usage() {
    std::cout << "MNN ASR Sample Program\n";
    std::cout << "Usage:\n";
    std::cout << "  " << "asr_sample <config_path> <audio_file>\n";
    std::cout << "  " << "asr_sample --demo\n";
    std::cout << "\n";
    std::cout << "Examples:\n";
    std::cout << "  " << "asr_sample config.json audio.wav\n";
    std::cout << "  " << "asr_sample --demo\n";
}

void run_demo() {
    std::cout << "=== MNN ASR Demo Mode ===\n\n";
    
    // 创建配置
    MNN_ASR::AsrConfig config;
    config.config_path = "../export/paraformer/config.json";  // 默认配置文件路径
    config.sample_rate = 16000;
    config.num_channels = 1;
    config.use_gpu = false;
    config.batch_size = 1;
    config.chunk_size = 0.5f;
    config.enable_vad = false;
    
    std::cout << "Creating ASR engine with config:\n";
    std::cout << "  Config path: " << config.config_path << "\n";
    std::cout << "  Sample rate: " << config.sample_rate << " Hz\n";
    std::cout << "  Channels: " << config.num_channels << "\n";
    std::cout << "  GPU enabled: " << (config.use_gpu ? "Yes" : "No") << "\n";
    std::cout << "  Chunk size: " << config.chunk_size << "s\n\n";
    
    try {
        // 创建 ASR 引擎
        auto asr_engine = MNN_ASR::AsrEngine::create(config);
        if (!asr_engine) {
            std::cerr << "Failed to create ASR engine\n";
            return;
        }
        
        std::cout << "ASR engine created successfully\n";
        
        // 初始化引擎
        std::cout << "Initializing ASR engine...\n";
        auto start_time = std::chrono::high_resolution_clock::now();
        
        if (!asr_engine->initialize()) {
            std::cerr << "Failed to initialize ASR engine\n";
            return;
        }
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        
        std::cout << "ASR engine initialized successfully in " << duration.count() << " ms\n\n";
        
        // 显示引擎信息
        std::cout << "Engine Info:\n";
        std::cout << asr_engine->get_engine_info() << "\n";
        
        // 测试音频文件识别
        std::string test_audio = "../resource/audio.wav";
        if (MNN_ASR::Utils::is_audio_file_supported(test_audio)) {
            std::cout << "Testing audio file recognition: " << test_audio << "\n";
            
            // 获取音频文件信息
            int sample_rate, channels, duration_ms;
            if (MNN_ASR::Utils::get_audio_info(test_audio, sample_rate, channels, duration_ms)) {
                std::cout << "  Audio info: " << sample_rate << " Hz, " << channels << " channels, " 
                         << duration_ms << " ms\n";
            }
            
            // 执行识别
            start_time = std::chrono::high_resolution_clock::now();
            auto result = asr_engine->recognize_file(test_audio);
            end_time = std::chrono::high_resolution_clock::now();
            auto recognition_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
            
            std::cout << "  Recognition result:\n";
            std::cout << "    Text: " << result.text << "\n";
            std::cout << "    Confidence: " << result.confidence << "\n";
            std::cout << "    Time: " << recognition_time.count() << " ms\n\n";
            
        } else {
            std::cout << "Test audio file not supported: " << test_audio << "\n\n";
        }
        
        // 测试流式识别
        std::cout << "Testing stream recognition...\n";
        
        // 模拟音频数据块
        std::vector<float> audio_chunk(8000, 0.1f);  // 0.5秒的音频数据 @ 16kHz
        
        for (int i = 0; i < 3; ++i) {
            std::cout << "  Processing chunk " << (i + 1) << "...\n";
            
            auto stream_result = asr_engine->recognize_stream(audio_chunk, 16000);
            std::cout << "    Partial result: " << stream_result.text << "\n";
            std::cout << "    Confidence: " << stream_result.confidence << "\n";
            std::cout << "    Is final: " << (stream_result.is_final ? "Yes" : "No") << "\n";
            
            // 模拟处理延迟
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        // 重置流式识别状态
        asr_engine->reset_stream();
        std::cout << "  Stream recognition reset\n\n";
        
        // 测试音频数据识别
        std::cout << "Testing audio data recognition...\n";
        std::vector<float> test_audio_data(16000, 0.05f);  // 1秒的测试音频数据
        
        start_time = std::chrono::high_resolution_clock::now();
        auto data_result = asr_engine->recognize_audio(test_audio_data, 16000);
        end_time = std::chrono::high_resolution_clock::now();
        auto data_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        
        std::cout << "  Data recognition result:\n";
        std::cout << "    Text: " << data_result.text << "\n";
        std::cout << "    Confidence: " << data_result.confidence << "\n";
        std::cout << "    Time: " << data_time.count() << " ms\n\n";
        
        std::cout << "=== Demo completed successfully ===\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Demo error: " << e.what() << "\n";
    }
}

void run_file_recognition(const std::string& config_path, const std::string& audio_file) {
    std::cout << "=== MNN ASR File Recognition ===\n\n";
    
    // 创建配置
    MNN_ASR::AsrConfig config;
    config.config_path = config_path;
    config.sample_rate = 16000;
    config.num_channels = 1;
    config.use_gpu = false;
    config.batch_size = 1;
    config.chunk_size = 0.5f;
    config.enable_vad = false;
    
    std::cout << "Config: " << config_path << "\n";
    std::cout << "Audio file: " << audio_file << "\n\n";
    
    try {
        // 创建 ASR 引擎
        auto asr_engine = MNN_ASR::AsrEngine::create(config);
        if (!asr_engine) {
            std::cerr << "Failed to create ASR engine\n";
            return;
        }
        
        // 初始化引擎
        std::cout << "Initializing ASR engine...\n";
        if (!asr_engine->initialize()) {
            std::cerr << "Failed to initialize ASR engine\n";
            return;
        }
        std::cout << "ASR engine initialized successfully\n\n";
        
        // 检查音频文件
        if (!MNN_ASR::Utils::is_audio_file_supported(audio_file)) {
            std::cerr << "Unsupported audio file format: " << audio_file << "\n";
            return;
        }
        
        // 获取音频文件信息
        int sample_rate, channels, duration_ms;
        if (MNN_ASR::Utils::get_audio_info(audio_file, sample_rate, channels, duration_ms)) {
            std::cout << "Audio file info:\n";
            std::cout << "  Sample rate: " << sample_rate << " Hz\n";
            std::cout << "  Channels: " << channels << "\n";
            std::cout << "  Duration: " << duration_ms << " ms\n\n";
        }
        
        // 执行识别
        std::cout << "Starting recognition...\n";
        auto start_time = std::chrono::high_resolution_clock::now();
        
        auto result = asr_engine->recognize_file(audio_file);
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto recognition_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        
        // 显示结果
        std::cout << "\nRecognition completed in " << recognition_time.count() << " ms\n";
        std::cout << "Result:\n";
        std::cout << "  Text: " << result.text << "\n";
        std::cout << "  Confidence: " << result.confidence << "\n";
        std::cout << "  Start time: " << result.start_time << "s\n";
        std::cout << "  End time: " << result.end_time << "s\n";
        std::cout << "  Is final: " << (result.is_final ? "Yes" : "No") << "\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Recognition error: " << e.what() << "\n";
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage();
        return 1;
    }
    
    std::string first_arg = argv[1];
    
    if (first_arg == "--demo" || first_arg == "-d") {
        run_demo();
    } else if (argc >= 3) {
        std::string config_path = argv[1];
        std::string audio_file = argv[2];
        run_file_recognition(config_path, audio_file);
    } else {
        print_usage();
        return 1;
    }
    
    return 0;
}
