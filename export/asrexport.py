import os
import sys
import json
import time
import yaml
import base64
import argparse
import functools
import traceback
import numpy as np
from pathlib import Path
from yaspin import yaspin
from funasr import AutoModel

RESET = "\033[0m"
GREEN = "\033[32;1m"
YELLOW = "\033[33;4m"

def spinner_run(text='Processing...', hide=False):
    def decorator(func):
        @functools.wraps(func)
        def wrapper(*args, **kwargs):
            with yaspin(text=text, color="cyan") as spinner:
                start = time.time()
                try:
                    if hide: spinner.hide()
                    result = func(*args, **kwargs)
                    if hide: spinner.show()
                except Exception as e:
                    spinner.fail("💥 Failed")
                    traceback.print_exc()
                    exit(1)
                end = time.time()
                during = f'[{end-start:05.2f} s]'.replace('[0', '[ ')
                padding = ' ' * (64 - len(spinner.text))
                spinner.text = f'{spinner.text}{padding}{GREEN}{during}{RESET}'
                spinner.ok("✅ Done")
                return result
        return wrapper
    return decorator

class Paraformer:

    def __init__(self, args):
        self.model_path = args.path
        self.dst_path = os.path.abspath(args.dst_path)
        if not os.path.exists(self.dst_path):
            os.makedirs(self.dst_path)
        if os.path.exists(args.mnnconvert):
            self.mnnconvert = args.mnnconvert
        else:
            self.mnnconvert = None

    def convert(self, onnx_path, mnn_path):
        convert_args = [
            '',
            '-f',
            'ONNX',
            '--modelFile',
            str(onnx_path),
            '--MNNModel',
            str(mnn_path),
            # '--transformerFuse',
        ]
        sfd = os.dup(1)
        log_fp = open('./.export.log', "a")
        log_fd = log_fp.fileno()
        # mnnconvert ... > .export.log
        os.dup2(log_fd, 1)
        try:
            sys.argv = convert_args
            sys.argc = len(convert_args)
            if self.mnnconvert is None:
                from MNN.tools import mnnconvert
                mnnconvert.main()
            else:
                convert_args[0] = self.mnnconvert
                cmd = ' '.join(convert_args)
                message = os.popen(cmd).read()
                print(message)
            sys.argv = []
        finally:
            os.dup2(sfd, 1)
            os.close(log_fd)

    @spinner_run("export onnx")
    def export_onnx(self):
        model = AutoModel(model=self.model_path, disable_update=True)
        onnx_path = model.export(quantize=False)
        return onnx_path

    @spinner_run("convert onnx to mnn")
    def convert_to_mnn(self, encoder_model_file, decoder_model_file):
        encoder_path = f'{self.dst_path}/encoder.mnn'
        decoder_path = f'{self.dst_path}/decoder.mnn'

        self.convert(encoder_model_file, encoder_path)
        self.convert(decoder_model_file, decoder_path)

        print(f'{GREEN}[SAVED]{RESET} {encoder_path}')
        print(f'{GREEN}[SAVED]{RESET} {decoder_path}')
        return self.dst_path

    def export_model(self):
        onnx_path = self.export_onnx()
        encoder_model_file = os.path.join(onnx_path, "model.onnx")
        decoder_model_file = os.path.join(onnx_path, "decoder.onnx")
        self.convert_to_mnn(encoder_model_file, decoder_model_file)

    def load_cmvn(self):
        cmvn_file = os.path.join(self.model_path, "am.mvn")
        with open(cmvn_file, "r", encoding="utf-8") as f:
            lines = f.readlines()

        means_list = []
        vars_list = []
        for i in range(len(lines)):
            line_item = lines[i].split()
            if line_item[0] == "<AddShift>":
                line_item = lines[i + 1].split()
                if line_item[0] == "<LearnRateCoef>":
                    add_shift_line = line_item[3 : (len(line_item) - 1)]
                    means_list = list(add_shift_line)
                    continue
            elif line_item[0] == "<Rescale>":
                line_item = lines[i + 1].split()
                if line_item[0] == "<LearnRateCoef>":
                    rescale_line = line_item[3 : (len(line_item) - 1)]
                    vars_list = list(rescale_line)
                    continue

        means = np.array(means_list).astype(np.float32).tolist()
        vars = np.array(vars_list).astype(np.float32).tolist()
        return means, vars

    @spinner_run("export config")
    def export_config(self):
        asr_config = {}
        # load config from `config.yaml`
        config_file = os.path.join(self.model_path, "config.yaml")
        if not Path(config_file).exists():
            raise FileExistsError(f"The {config_file} does not exist.")
        with open(str(config_file), "rb") as f:
            data = yaml.load(f, Loader=yaml.Loader)
            # model
            asr_config['encoder_output_size'] = data["encoder_conf"]["output_size"]
            asr_config['fsmn_layer'] = data["decoder_conf"]["num_blocks"]
            asr_config['fsmn_lorder'] = data["decoder_conf"]["kernel_size"] - 1
            asr_config['fsmn_dims'] = data["encoder_conf"]["output_size"]
            asr_config['feats_dims'] = data["frontend_conf"]["n_mels"] * data["frontend_conf"]["lfr_m"]
            # predictor
            asr_config['cif_threshold'] = data['predictor_conf']['threshold']
            asr_config['tail_threshold'] = data['predictor_conf']['tail_threshold']
            # frontend
            asr_config['samp_freq'] = data["frontend_conf"]["fs"]
            asr_config['window_type'] = data["frontend_conf"]["window"]
            asr_config['frame_shift_ms'] = data["frontend_conf"]["frame_shift"]
            asr_config['frame_length_ms'] = data["frontend_conf"]["frame_length"]
            asr_config['num_bins'] = data["frontend_conf"]["n_mels"]
            # asr_config['dither'] = data["frontend_conf"]["dither"]
            asr_config['lfr_m'] = data["frontend_conf"]["lfr_m"]
            asr_config['lfr_n'] = data["frontend_conf"]["lfr_n"]

        mean, var = self.load_cmvn()
        asr_config['mean'] = mean
        asr_config['var'] = var
        asr_config['chunk_size'] = [5, 10, 5]

        asr_config_path = f'{self.dst_path}/asr_config.json'
        config_path = f'{self.dst_path}/config.json'

        with open(asr_config_path, 'w', encoding='utf-8') as f:
            json.dump(asr_config, f, ensure_ascii=False, indent=4)

        with open(config_path, 'w', encoding='utf-8') as f:
            config = {
                "encoder_model": f"encoder.mnn",
                "decoder_model": f"decoder.mnn",
                "backend_type": "cpu",
                "thread_num": 4,
                "precision": "low",
                "memory": "low"
            }
            json.dump(config, f, ensure_ascii=False, indent=4)

        print(f'{GREEN}[SAVED]{RESET} {config_path}')

    @spinner_run("export tokenizer")
    def export_tokenizer(self):
        # TOKENIZER MAGIC NUMBER
        MAGIC_NUMBER = 430
        # TOKENIZER TYPE
        SENTENCEPIECE = 0; TIKTOIKEN = 1; BERT = 2; HUGGINGFACE = 3
        def write_line(fp, *args):
            for arg in args:
                for token in arg:
                    fp.write(str(token) + ' ')
            fp.write('\n')
        def write_header(fp, type, stop_ids, speicals, prefix = []):
            fp.write(f'{MAGIC_NUMBER} {type}\n')
            fp.write(f'{len(speicals)} {len(stop_ids)} {len(prefix)}\n')
            write_line(fp, speicals, stop_ids, prefix)
        token_list = os.path.join(self.model_path, "tokens.json")
        with open(token_list, "r", encoding="utf-8") as f:
            vocab_list = json.load(f)

        file_path = os.path.join(self.dst_path, "tokenizer.txt")
        with open(file_path, "w", encoding="utf8") as fp:
                write_header(fp, TIKTOIKEN, [], [], [])
                fp.write(f'{len(vocab_list)}\n')
                for v in vocab_list:
                    line = base64.b64encode(v.encode('utf-8')).decode("utf8") + "\n"
                    fp.write(line)

        print(f'{GREEN}[SAVED]{RESET} {file_path}')

    def export(self):
        self.export_model()
        self.export_config()
        self.export_tokenizer()
        print(f'{GREEN}[SUCCESS]{RESET} export model to {YELLOW}{self.dst_path}{RESET}')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='asr_exporter', formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument('--path', type=str, required=True, help='path of model.')
    parser.add_argument('--dst_path', type=str, default='./model', help='export onnx/mnn model to path, defaut is `./model`.')
    parser.add_argument('--mnnconvert', type=str, default='../../../build/MNNConvert', help='local mnnconvert path, if invalid, using pymnn.')
    args = parser.parse_args()
    paraformer = Paraformer(args)
    paraformer.export()