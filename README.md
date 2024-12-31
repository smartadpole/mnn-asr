![mnn-asr](resource/logo.png)

# mnn-asr
[![License](https://img.shields.io/github/license/wangzhaode/mnn-asr)](LICENSE)

## 模型导出
```sh
cd export
# 下载模型
modelscope download --model iic/speech_paraformer-large_asr_nat-zh-cn-16k-common-vocab8404-online --local_dir paraformer
# 安装依赖
pip install -r requirements.txt
# 导出模型
python asrexport.py --path ./paraformer
```

## 编译
```sh
mkdir buildc
cd build
make -j16
```

## 测试
```sh
./asr_demo ../export/model/config.json ../resource/audio.wav
```