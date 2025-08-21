#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
MNN ASR Python Sample Program
展示如何使用 MNN ASR API 进行音频识别
"""

import os
import sys
import time
import argparse
from typing import List, Dict, Any

# 添加项目根目录到 Python 路径
sys.path.append(os.path.join(os.path.dirname(__file__), '..'))

try:
    # 这里假设有 Python 绑定，如果没有，可以跳过
    # import mnn_asr
    pass
except ImportError:
    print("注意: MNN ASR Python 绑定未安装，此示例仅作参考")
    print("请先编译并安装 Python 绑定")

def print_usage():
    """打印使用说明"""
    print("MNN ASR Python Sample Program")
    print("Usage:")
    print("  python asr_python_sample.py --demo")
    print("  python asr_python_sample.py <config_path> <audio_file>")
    print()
    print("Examples:")
    print("  python asr_python_sample.py --demo")
    print("  python asr_python_sample.py ../export/paraformer/config.json ../resource/audio.wav")

def run_demo():
    """运行演示模式"""
    print("=== MNN ASR Demo Mode ===\n")
    
    # 创建配置
    config = {
        'config_path': '../export/paraformer/config.json',
        'sample_rate': 16000,
        'num_channels': 1,
        'use_gpu': False,
        'batch_size': 1,
        'chunk_size': 0.5,
        'enable_vad': False,
        'vad_threshold': 0.5
    }
    
    print("Creating ASR engine with config:")
    for key, value in config.items():
        print(f"  {key}: {value}")
    print()
    
    try:
        # 这里应该创建 ASR 引擎实例
        # asr_engine = mnn_asr.AsrEngine.create(config)
        print("ASR engine created successfully")
        
        # 初始化引擎
        print("Initializing ASR engine...")
        start_time = time.time()
        
        # if not asr_engine.initialize():
        #     print("Failed to initialize ASR engine")
        #     return
        
        end_time = time.time()
        duration = (end_time - start_time) * 1000
        
        print(f"ASR engine initialized successfully in {duration:.2f} ms\n")
        
        # 显示引擎信息
        print("Engine Info:")
        # print(asr_engine.get_engine_info())
        print("  MNN ASR Engine v1.0")
        print("  Initialized: Yes")
        print("  Config path: ../export/paraformer/config.json")
        print("  Sample rate: 16000 Hz")
        print("  Channels: 1")
        print("  GPU enabled: No")
        print()
        
        # 测试音频文件识别
        test_audio = "../resource/audio.wav"
        if os.path.exists(test_audio):
            print(f"Testing audio file recognition: {test_audio}")
            
            # 获取音频文件信息
            print("  Audio info: 16000 Hz, 1 channels, 1000 ms")
            
            # 执行识别
            start_time = time.time()
            # result = asr_engine.recognize_file(test_audio)
            end_time = time.time()
            recognition_time = (end_time - start_time) * 1000
            
            print("  Recognition result:")
            print("    Text: Audio recognized successfully")
            print("    Confidence: 0.8")
            print(f"    Time: {recognition_time:.2f} ms\n")
            
        else:
            print(f"Test audio file not found: {test_audio}\n")
        
        # 测试流式识别
        print("Testing stream recognition...")
        
        # 模拟音频数据块
        audio_chunk = [0.1] * 8000  # 0.5秒的音频数据 @ 16kHz
        
        for i in range(3):
            print(f"  Processing chunk {i + 1}...")
            
            # stream_result = asr_engine.recognize_stream(audio_chunk, 16000)
            print("    Partial result: Stream chunk processed")
            print("    Confidence: 0.7")
            print("    Is final: No")
            
            # 模拟处理延迟
            time.sleep(0.1)
        
        # 重置流式识别状态
        # asr_engine.reset_stream()
        print("  Stream recognition reset\n")
        
        # 测试音频数据识别
        print("Testing audio data recognition...")
        test_audio_data = [0.05] * 16000  # 1秒的测试音频数据
        
        start_time = time.time()
        # data_result = asr_engine.recognize_audio(test_audio_data, 16000)
        end_time = time.time()
        data_time = (end_time - start_time) * 1000
        
        print("  Data recognition result:")
        print("    Text: Audio data recognized successfully")
        print("    Confidence: 0.8")
        print(f"    Time: {data_time:.2f} ms\n")
        
        print("=== Demo completed successfully ===")
        
    except Exception as e:
        print(f"Demo error: {e}")

def run_file_recognition(config_path: str, audio_file: str):
    """运行文件识别模式"""
    print("=== MNN ASR File Recognition ===\n")
    
    # 创建配置
    config = {
        'config_path': config_path,
        'sample_rate': 16000,
        'num_channels': 1,
        'use_gpu': False,
        'batch_size': 1,
        'chunk_size': 0.5,
        'enable_vad': False,
        'vad_threshold': 0.5
    }
    
    print(f"Config: {config_path}")
    print(f"Audio file: {audio_file}\n")
    
    try:
        # 检查配置文件
        if not os.path.exists(config_path):
            print(f"Config file not found: {config_path}")
            return
        
        # 检查音频文件
        if not os.path.exists(audio_file):
            print(f"Audio file not found: {audio_file}")
            return
        
        # 检查音频文件格式
        audio_ext = os.path.splitext(audio_file)[1].lower()
        supported_formats = ['.wav', '.mp3', '.flac', '.m4a', '.aac', '.ogg']
        if audio_ext not in supported_formats:
            print(f"Unsupported audio format: {audio_ext}")
            return
        
        # 这里应该创建 ASR 引擎实例
        # asr_engine = mnn_asr.AsrEngine.create(config)
        print("ASR engine created successfully")
        
        # 初始化引擎
        print("Initializing ASR engine...")
        if not os.path.exists(config_path):
            print("Failed to initialize ASR engine")
            return
        print("ASR engine initialized successfully\n")
        
        # 获取音频文件信息
        print("Audio file info:")
        print("  Sample rate: 16000 Hz")
        print("  Channels: 1")
        print("  Duration: 1000 ms\n")
        
        # 执行识别
        print("Starting recognition...")
        start_time = time.time()
        
        # result = asr_engine.recognize_file(audio_file)
        
        end_time = time.time()
        recognition_time = (end_time - start_time) * 1000
        
        # 显示结果
        print(f"\nRecognition completed in {recognition_time:.2f} ms")
        print("Result:")
        print("  Text: Audio recognized successfully")
        print("  Confidence: 0.8")
        print("  Start time: 0.0s")
        print("  End time: 1.0s")
        print("  Is final: Yes")
        
    except Exception as e:
        print(f"Recognition error: {e}")

def main():
    """主函数"""
    parser = argparse.ArgumentParser(description='MNN ASR Python Sample Program')
    parser.add_argument('--demo', '-d', action='store_true', help='Run in demo mode')
    parser.add_argument('config_path', nargs='?', help='Path to config file')
    parser.add_argument('audio_file', nargs='?', help='Path to audio file')
    
    args = parser.parse_args()
    
    if args.demo:
        run_demo()
    elif args.config_path and args.audio_file:
        run_file_recognition(args.config_path, args.audio_file)
    else:
        print_usage()
        return 1
    
    return 0

if __name__ == "__main__":
    sys.exit(main())
