#include "compressor_module.h"
#include "compressor.h"

namespace vital {

    CompressorModule::CompressorModule() : SynthModule(0, kNumOutputs), compressor_(nullptr) { }

    CompressorModule::~CompressorModule() {
        // Destructor ensures proper cleanup of the compressor_ pointer if necessary.
    }

    void CompressorModule::init() {
        /**
         * @brief Initializes the multiband compressor and connects its outputs and parameters.
         *
         * This sets up the internal MultibandCompressor, registers it as a processor,
         * and creates modulation controls for attack, release, gains, thresholds, ratios,
         * and mix. It then plugs these controls into the compressor's parameters.
         */
        compressor_ = new MultibandCompressor();
        compressor_->useOutput(output(kAudio), MultibandCompressor::kAudio);
        compressor_->useOutput(output(kLowInputMeanSquared), MultibandCompressor::kLowInputMeanSquared);
        compressor_->useOutput(output(kBandInputMeanSquared), MultibandCompressor::kBandInputMeanSquared);
        compressor_->useOutput(output(kHighInputMeanSquared), MultibandCompressor::kHighInputMeanSquared);
        compressor_->useOutput(output(kLowOutputMeanSquared), MultibandCompressor::kLowOutputMeanSquared);
        compressor_->useOutput(output(kBandOutputMeanSquared), MultibandCompressor::kBandOutputMeanSquared);
        compressor_->useOutput(output(kHighOutputMeanSquared), MultibandCompressor::kHighOutputMeanSquared);
        addIdleProcessor(compressor_);

        Output* compressor_attack = createMonoModControl("compressor_attack");
        Output* compressor_release = createMonoModControl("compressor_release");
        Output* compressor_low_gain = createMonoModControl("compressor_low_gain");
        Output* compressor_band_gain = createMonoModControl("compressor_band_gain");
        Output* compressor_high_gain = createMonoModControl("compressor_high_gain");

        Value* compressor_enabled_bands = createBaseControl("compressor_enabled_bands");

        Value* compressor_low_upper_ratio = createBaseControl("compressor_low_upper_ratio");
        Value* compressor_band_upper_ratio = createBaseControl("compressor_band_upper_ratio");
        Value* compressor_high_upper_ratio = createBaseControl("compressor_high_upper_ratio");
        Value* compressor_low_lower_ratio = createBaseControl("compressor_low_lower_ratio");
        Value* compressor_band_lower_ratio = createBaseControl("compressor_band_lower_ratio");
        Value* compressor_high_lower_ratio = createBaseControl("compressor_high_lower_ratio");

        Value* compressor_low_upper_threshold = createBaseControl("compressor_low_upper_threshold");
        Value* compressor_band_upper_threshold = createBaseControl("compressor_band_upper_threshold");
        Value* compressor_high_upper_threshold = createBaseControl("compressor_high_upper_threshold");
        Value* compressor_low_lower_threshold = createBaseControl("compressor_low_lower_threshold");
        Value* compressor_band_lower_threshold = createBaseControl("compressor_band_lower_threshold");
        Value* compressor_high_lower_threshold = createBaseControl("compressor_high_lower_threshold");

        Output* compressor_mix = createMonoModControl("compressor_mix");

        // Plugging each parameter into the compressor.
        compressor_->plug(compressor_mix, MultibandCompressor::kMix);
        compressor_->plug(compressor_attack, MultibandCompressor::kAttack);
        compressor_->plug(compressor_release, MultibandCompressor::kRelease);
        compressor_->plug(compressor_low_gain, MultibandCompressor::kLowOutputGain);
        compressor_->plug(compressor_band_gain, MultibandCompressor::kBandOutputGain);
        compressor_->plug(compressor_high_gain, MultibandCompressor::kHighOutputGain);
        compressor_->plug(compressor_enabled_bands, MultibandCompressor::kEnabledBands);

        compressor_->plug(compressor_low_upper_ratio, MultibandCompressor::kLowUpperRatio);
        compressor_->plug(compressor_band_upper_ratio, MultibandCompressor::kBandUpperRatio);
        compressor_->plug(compressor_high_upper_ratio, MultibandCompressor::kHighUpperRatio);
        compressor_->plug(compressor_low_lower_ratio, MultibandCompressor::kLowLowerRatio);
        compressor_->plug(compressor_band_lower_ratio, MultibandCompressor::kBandLowerRatio);
        compressor_->plug(compressor_high_lower_ratio, MultibandCompressor::kHighLowerRatio);

        compressor_->plug(compressor_low_upper_threshold, MultibandCompressor::kLowUpperThreshold);
        compressor_->plug(compressor_band_upper_threshold, MultibandCompressor::kBandUpperThreshold);
        compressor_->plug(compressor_high_upper_threshold, MultibandCompressor::kHighUpperThreshold);
        compressor_->plug(compressor_low_lower_threshold, MultibandCompressor::kLowLowerThreshold);
        compressor_->plug(compressor_band_lower_threshold, MultibandCompressor::kBandLowerThreshold);
        compressor_->plug(compressor_high_lower_threshold, MultibandCompressor::kHighLowerThreshold);

        SynthModule::init();
    }

    void CompressorModule::setSampleRate(int sample_rate) {
        /**
         * @brief Updates the sample rate of the compressor and notifies the MultibandCompressor so
         * time-based parameters (attack, release) remain correct.
         */
        SynthModule::setSampleRate(sample_rate);
        compressor_->setSampleRate(sample_rate);
    }

    void CompressorModule::processWithInput(const poly_float* audio_in, int num_samples) {
        /**
         * @brief Processes the given input audio through the multiband compressor.
         *
         * The method first updates internal parameters by calling SynthModule::process(),
         * then passes the input samples to the compressor for processing.
         */
        SynthModule::process(num_samples);
        compressor_->processWithInput(audio_in, num_samples);
    }

    void CompressorModule::enable(bool enable) {
        /**
         * @brief Enables or disables the compressor. When disabled, it resets the internal state.
         */
        SynthModule::enable(enable);
        if (!enable)
            compressor_->reset(constants::kFullMask);
    }

    void CompressorModule::hardReset() {
        /**
         * @brief Performs a hard reset on the compressor, clearing all internal buffers and states.
         */
        compressor_->reset(constants::kFullMask);
    }
} // namespace vital
