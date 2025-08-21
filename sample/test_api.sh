#!/bin/bash

# MNN ASR API 测试脚本
# 用于验证 API 功能是否正常工作

set -e

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 打印带颜色的消息
print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# 检查文件是否存在
check_file() {
    if [ -f "$1" ]; then
        print_success "文件存在: $1"
        return 0
    else
        print_error "文件不存在: $1"
        return 1
    fi
}

# 检查目录是否存在
check_directory() {
    if [ -d "$1" ]; then
        print_success "目录存在: $1"
        return 0
    else
        print_error "目录不存在: $1"
        return 1
    fi
}

# 检查可执行文件
check_executable() {
    if [ -x "$1" ]; then
        print_success "可执行文件存在: $1"
        return 0
    else
        print_error "可执行文件不存在或不可执行: $1"
        return 1
    fi
}

# 运行测试
run_test() {
    local test_name="$1"
    local test_command="$2"
    
    print_info "运行测试: $test_name"
    print_info "命令: $test_command"
    
    if eval "$test_command"; then
        print_success "测试通过: $test_name"
        return 0
    else
        print_error "测试失败: $test_name"
        return 1
    fi
}

# 主函数
main() {
    echo "=========================================="
    echo "        MNN ASR API 测试脚本"
    echo "=========================================="
    echo ""
    
    # 检查构建目录
    print_info "检查构建目录..."
    if [ ! -d "../build" ]; then
        print_warning "构建目录不存在，请先运行 'make build'"
        echo ""
        print_info "创建构建目录并构建项目..."
        cd .. && make build && cd sample
    fi
    
    # 检查必要的文件
    print_info "检查必要文件..."
    
    local build_dir="../build"
    local config_file="../export/paraformer/config.json"
    local audio_file="../resource/audio.wav"
    
    check_directory "$build_dir"
    check_file "$config_file"
    check_file "$audio_file"
    
    # 检查可执行文件
    print_info "检查可执行文件..."
    check_executable "$build_dir/asr_sample"
    check_executable "$build_dir/asr_demo"
    
    echo ""
    print_info "开始运行测试..."
    echo ""
    
    local test_passed=0
    local test_total=0
    
    # 测试 1: 运行演示模式
    test_total=$((test_total + 1))
    if run_test "演示模式" "$build_dir/asr_sample --demo"; then
        test_passed=$((test_passed + 1))
    fi
    
    echo ""
    
    # 测试 2: 音频文件识别
    test_total=$((test_total + 1))
    if run_test "音频文件识别" "$build_dir/asr_sample $config_file $audio_file"; then
        test_passed=$((test_passed + 1))
    fi
    
    echo ""
    
    # 测试 3: 原始演示程序
    test_total=$((test_total + 1))
    if run_test "原始演示程序" "$build_dir/asr_demo $config_file $audio_file"; then
        test_passed=$((test_passed + 1))
    fi
    
    echo ""
    echo "=========================================="
    echo "              测试结果"
    echo "=========================================="
    echo "总测试数: $test_total"
    echo "通过测试: $test_passed"
    echo "失败测试: $((test_total - test_passed))"
    
    if [ $test_passed -eq $test_total ]; then
        print_success "所有测试通过！"
        exit 0
    else
        print_error "部分测试失败！"
        exit 1
    fi
}

# 错误处理
trap 'print_error "脚本执行出错，退出码: $?"; exit 1' ERR

# 运行主函数
main "$@"
