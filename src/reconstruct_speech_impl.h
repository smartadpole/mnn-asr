//
// Created by smart on 2025/8/22.
//

#ifndef RECONSTRUCT_SPEECH_INNER_H
#define RECONSTRUCT_SPEECH_INNER_H
#include "reconstruct_speech.hpp"
#include "asr.hpp"
#include "asrconfig.hpp"

namespace SR
{
    class Asr;
}

class SpeechEngineImpl : public asl::SpeechEngine
{
public:
    SpeechEngineImpl();

    ~SpeechEngineImpl() override;

    bool Init(const asl::Config& config) override;

    bool IsInitialized() const override;

    asl::Result Recognize(const std::string& audio_file_path) override;

    asl::Result Recognize(const std::vector<float>& audio_data, int sample_rate) override;

    void ResetStream() override;

    std::string GetEngineInfo() const override;

    asl::Config GetConfig() const override;

private:
    asl::Config config_;
    std::unique_ptr<SR::Asr> asr_;
    bool is_initialized_;
};


#endif //RECONSTRUCT_SPEECH_INNER_H
