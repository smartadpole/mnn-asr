# MNN ASR Makefile
# 简化构建过程的 Makefile

.PHONY: all clean build install uninstall help

# 默认目标
all: build

# 构建目录
BUILD_DIR = build
INSTALL_DIR = /usr/local

# 构建目标
build:
	@echo "Building MNN ASR..."
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake .. && make -j$(shell nproc)
	@echo "Build completed!"

# 清理构建文件
clean:
	@echo "Cleaning build files..."
	@rm -rf $(BUILD_DIR)
	@echo "Clean completed!"

# 安装到系统
install: build
	@echo "Installing MNN ASR..."
	@cd $(BUILD_DIR) && sudo make install
	@echo "Install completed!"

# 卸载
uninstall:
	@echo "Uninstalling MNN ASR..."
	@sudo rm -f $(INSTALL_DIR)/lib/libmnn_asr.a
	@sudo rm -f $(INSTALL_DIR)/lib/libmnn_asr.so
	@sudo rm -f $(INSTALL_DIR)/include/mnn_asr/mnn_asr_api.hpp
	@sudo rm -f $(INSTALL_DIR)/share/mnn_asr/README.md
	@echo "Uninstall completed!"

# 运行示例程序
demo: build
	@echo "Running demo..."
	@$(BUILD_DIR)/asr_sample --demo

# 测试音频识别
test: build
	@echo "Testing audio recognition..."
	@$(BUILD_DIR)/asr_sample export/paraformer/config.json resource/audio.wav

# 运行原始演示程序
original: build
	@echo "Running original demo..."
	@$(BUILD_DIR)/asr_demo export/paraformer/config.json resource/audio.wav

# 显示帮助信息
help:
	@echo "MNN ASR Makefile 使用说明:"
	@echo ""
	@echo "可用目标:"
	@echo "  all        - 构建所有目标 (默认)"
	@echo "  build      - 构建项目"
	@echo "  clean      - 清理构建文件"
	@echo "  install    - 安装到系统"
	@echo "  uninstall  - 从系统卸载"
	@echo "  demo       - 运行 API 演示程序"
	@echo "  test       - 测试音频识别功能"
	@echo "  original   - 运行原始演示程序"
	@echo "  help       - 显示此帮助信息"
	@echo ""
	@echo "使用示例:"
	@echo "  make build     # 构建项目"
	@echo "  make demo      # 运行演示"
	@echo "  make install   # 安装到系统"
	@echo "  make clean     # 清理构建文件"

# 显示构建信息
info:
	@echo "MNN ASR 项目信息:"
	@echo "  构建目录: $(BUILD_DIR)"
	@echo "  安装目录: $(INSTALL_DIR)"
	@echo "  可用目标: make help"
