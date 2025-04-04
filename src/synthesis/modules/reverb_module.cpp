#include "reverb_module.h"
#include "reverb.h"

namespace vital {

    /**
     * @brief Constructs a ReverbModule.
     *
     * Initializes the module with no inputs and a single output channel.
     */
    ReverbModule::ReverbModule() : SynthModule(0, 1), reverb_(nullptr) { }

    /**
     * @brief Destructor for ReverbModule.
     */
    ReverbModule::~ReverbModule() {
    }

    /**
     * @brief Initializes the reverb by creating parameter controls and plugging them into the Reverb processor.
     *
     * The controls include decay time, pre and post filtering (low/high shelf), size, delay, wet mix,
     * and chorus modulation parameters. After connecting these parameters, the reverb is ready for use.
     */
    void ReverbModule::init() {
        reverb_ = new Reverb();
        reverb_->useOutput(output());
        addIdleProcessor(reverb_);

        // Create controls for reverb parameters.
        Output* reverb_decay_time = createMonoModControl("reverb_decay_time");
        Output* reverb_pre_low_cutoff = createMonoModControl("reverb_pre_low_cutoff");
        Output* reverb_pre_high_cutoff = createMonoModControl("reverb_pre_high_cutoff");
        Output* reverb_low_shelf_cutoff = createMonoModControl("reverb_low_shelf_cutoff");
        Output* reverb_low_shelf_gain = createMonoModControl("reverb_low_shelf_gain");
        Output* reverb_high_shelf_cutoff = createMonoModControl("reverb_high_shelf_cutoff");
        Output* reverb_high_shelf_gain = createMonoModControl("reverb_high_shelf_gain");
        Output* reverb_chorus_amount = createMonoModControl("reverb_chorus_amount");
        Output* reverb_chorus_frequency = createMonoModControl("reverb_chorus_frequency");
        Output* reverb_size = createMonoModControl("reverb_size");
        Output* reverb_delay = createMonoModControl("reverb_delay");
        Output* reverb_wet = createMonoModControl("reverb_dry_wet");

        // Connect parameter controls to the Reverb processor.
        reverb_->plug(reverb_decay_time, Reverb::kDecayTime);
        reverb_->plug(reverb_pre_low_cutoff, Reverb::kPreLowCutoff);
        reverb_->plug(reverb_pre_high_cutoff, Reverb::kPreHighCutoff);
        reverb_->plug(reverb_low_shelf_cutoff, Reverb::kLowCutoff);
        reverb_->plug(reverb_low_shelf_gain, Reverb::kLowGain);
        reverb_->plug(reverb_high_shelf_cutoff, Reverb::kHighCutoff);
        reverb_->plug(reverb_high_shelf_gain, Reverb::kHighGain);
        reverb_->plug(reverb_chorus_amount, Reverb::kChorusAmount);
        reverb_->plug(reverb_chorus_frequency, Reverb::kChorusFrequency);
        reverb_->plug(reverb_delay, Reverb::kDelay);
        reverb_->plug(reverb_size, Reverb::kSize);
        reverb_->plug(reverb_wet, Reverb::kWet);

        SynthModule::init();
    }

    /**
     * @brief Resets the reverb's internal buffers and state.
     */
    void ReverbModule::hardReset() {
        reverb_->hardReset();
    }

    /**
     * @brief Enables or disables the reverb processing.
     *
     * If disabled, a hard reset of the reverb is performed. If enabled, reverb processing resumes after a single process call.
     *
     * @param enable True to enable the reverb effect, false to disable.
     */
    void ReverbModule::enable(bool enable) {
        SynthModule::enable(enable);
        process(1);  // Process a single sample to update state if enabling
        if (!enable)
            reverb_->hardReset();
    }

    /**
     * @brief Sets the reverb's internal sample rate.
     *
     * Adjusts internal parameters that depend on the sampling frequency (e.g., delay times).
     *
     * @param sample_rate The new sample rate in Hz.
     */
    void ReverbModule::setSampleRate(int sample_rate) {
        SynthModule::setSampleRate(sample_rate);
        reverb_->setSampleRate(sample_rate);
    }

    /**
     * @brief Processes an input audio buffer through the reverb effect.
     *
     * @param audio_in Pointer to the input audio samples.
     * @param num_samples Number of samples to process.
     */
    void ReverbModule::processWithInput(const poly_float* audio_in, int num_samples) {
        SynthModule::process(num_samples);
        reverb_->processWithInput(audio_in, num_samples);
    }
} // namespace vital
