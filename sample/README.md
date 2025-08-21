# MNN ASR API 使用说明

本文档介绍如何使用 MNN ASR API 进行音频识别。

## 概述

MNN ASR API 是一个封装了 MNN 语音识别功能的 C++ 接口，提供了简洁易用的 API 来进行音频识别。主要特性包括：

- 离线音频文件识别
- 流式音频识别
- 音频数据识别
- 支持多种音频格式
- 可配置的识别参数
- 跨平台支持

## 文件结构

```
sample/
├── asr_sample.cpp          # C++ 示例程序
├── asr_python_sample.py    # Python 示例程序（参考）
└── README.md               # 本文档
```

## 编译

### 前置要求

- CMake 3.5+
- C++11 兼容的编译器
- MNN 库
- 音频处理库

### 编译步骤

1. 设置 MNN 环境变量：
```bash
export MNN_DIR=/path/to/mnn
```

2. 创建构建目录：
```bash
mkdir build && cd build
```

3. 配置和编译：
```bash
cmake ..
make -j4
```

4. 编译示例程序：
```bash
make asr_sample
```

## 使用方法

### C++ 示例程序

#### 运行演示模式
```bash
./asr_sample --demo
```

#### 识别音频文件
```bash
./asr_sample <config_path> <audio_file>
```

例如：
```bash
./asr_sample ../export/paraformer/config.json ../resource/audio.wav
```

### Python 示例程序

#### 运行演示模式
```bash
python3 asr_python_sample.py --demo
```

#### 识别音频文件
```bash
python3 asr_python_sample.py <config_path> <audio_file>
```

## API 接口

### 主要类

#### AsrConfig
配置结构体，包含以下参数：

- `config_path`: 配置文件路径
- `sample_rate`: 采样率（默认 16000 Hz）
- `num_channels`: 声道数（默认 1）
- `use_gpu`: 是否使用 GPU
- `batch_size`: 批处理大小
- `chunk_size`: 分块大小（秒）
- `enable_vad`: 是否启用 VAD
- `vad_threshold`: VAD 阈值

#### AsrEngine
主要的 ASR 引擎类，提供以下方法：

- `create(config)`: 创建引擎实例
- `initialize()`: 初始化引擎
- `recognize_file(audio_file)`: 识别音频文件
- `recognize_audio(audio_data, sample_rate)`: 识别音频数据
- `recognize_stream(audio_chunk, sample_rate)`: 流式识别
- `reset_stream()`: 重置流式识别状态
- `get_engine_info()`: 获取引擎信息

#### AsrResult
识别结果结构体，包含：

- `text`: 识别的文本
- `confidence`: 置信度
- `start_time`: 开始时间
- `end_time`: 结束时间
- `is_final`: 是否为最终结果

### 工具函数

#### Utils 命名空间

- `is_audio_file_supported(file_path)`: 检查音频格式是否支持
- `get_audio_info(file_path, sample_rate, channels, duration_ms)`: 获取音频文件信息
- `convert_audio_format(input_path, output_path, target_sample_rate, target_channels)`: 音频格式转换

## 代码示例

### 基本使用

```cpp
#include "mnn_asr_api.hpp"

// 创建配置
MNN_ASR::AsrConfig config;
config.config_path = "config.json";
config.sample_rate = 16000;
config.num_channels = 1;

// 创建引擎
auto asr_engine = MNN_ASR::AsrEngine::create(config);

// 初始化
if (asr_engine->initialize()) {
    // 识别音频文件
    auto result = asr_engine->recognize_file("audio.wav");
    std::cout << "识别结果: " << result.text << std::endl;
}
```

### 流式识别

```cpp
// 流式识别
std::vector<float> audio_chunk(8000, 0.1f);  // 0.5秒音频数据
auto result = asr_engine->recognize_stream(audio_chunk, 16000);

if (result.is_final) {
    std::cout << "最终结果: " << result.text << std::endl;
} else {
    std::cout << "部分结果: " << result.text << std::endl;
}

// 重置状态
asr_engine->reset_stream();
```

## 支持的音频格式

- WAV
- MP3
- FLAC
- M4A
- AAC
- OGG

## 性能优化建议

1. **GPU 加速**: 如果可用，启用 GPU 加速可以显著提升性能
2. **批处理**: 对于多个音频文件，使用批处理模式
3. **分块大小**: 根据实际需求调整分块大小
4. **内存管理**: 对于长音频，注意内存使用情况

## 错误处理

API 使用异常安全的设计，主要错误情况包括：

- 配置文件不存在或格式错误
- 音频文件不支持或损坏
- 模型加载失败
- 内存不足
- GPU 相关错误

建议在使用时添加适当的错误处理代码。

## 故障排除

### 常见问题

1. **编译错误**: 检查 MNN 库路径和版本兼容性
2. **运行时错误**: 确认配置文件和模型文件路径正确
3. **性能问题**: 检查是否启用了 GPU 加速
4. **内存问题**: 调整批处理大小和分块大小

### 调试模式

编译时启用调试信息：
```bash
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
```

## 许可证

本项目遵循与 MNN 相同的许可证条款。

## 贡献

欢迎提交 Issue 和 Pull Request 来改进这个 API。

## 联系方式

如有问题，请通过以下方式联系：

- 提交 GitHub Issue
- 发送邮件到项目维护者
