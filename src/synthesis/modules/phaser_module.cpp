#include "phaser_module.h"
#include "phaser.h"

namespace vital {

    /**
     * @brief Constructs the phaser module.
     *
     * @param beats_per_second An output providing beats per second for tempo sync.
     */
    PhaserModule::PhaserModule(const Output* beats_per_second) :
            SynthModule(0, kNumOutputs), beats_per_second_(beats_per_second), phaser_(nullptr) {
    }

    /**
     * @brief Destructor for PhaserModule.
     */
    PhaserModule::~PhaserModule() {
    }

    /**
     * @brief Initializes the phaser module.
     *
     * Sets up the internal Phaser processor, creates modulation controls and plugs them into the phaser.
     */
    void PhaserModule::init() {
        phaser_ = new Phaser();
        // Connect internal Phaser outputs to module outputs.
        phaser_->useOutput(output(kAudioOutput), Phaser::kAudioOutput);
        phaser_->useOutput(output(kCutoffOutput), Phaser::kCutoffOutput);

        // Add the phaser to idle processors, so it will be processed even if not explicitly triggered.
        addIdleProcessor(phaser_);

        // Create parameter controls for the phaser.
        Output* phaser_free_frequency = createMonoModControl("phaser_frequency");
        Output* phaser_frequency = createTempoSyncSwitch("phaser", phaser_free_frequency->owner,
                                                         beats_per_second_, false);
        Output* phaser_feedback = createMonoModControl("phaser_feedback");
        Output* phaser_wet = createMonoModControl("phaser_dry_wet");
        Output* phaser_center = createMonoModControl("phaser_center", true, true);
        Output* phaser_mod_depth = createMonoModControl("phaser_mod_depth");
        Output* phaser_phase_offset = createMonoModControl("phaser_phase_offset");
        Output* phaser_blend = createMonoModControl("phaser_blend");

        // Plug the created controls into the Phaser parameters.
        phaser_->plug(phaser_frequency, Phaser::kRate);
        phaser_->plug(phaser_wet, Phaser::kMix);
        phaser_->plug(phaser_feedback, Phaser::kFeedbackGain);
        phaser_->plug(phaser_center, Phaser::kCenter);
        phaser_->plug(phaser_mod_depth, Phaser::kModDepth);
        phaser_->plug(phaser_phase_offset, Phaser::kPhaseOffset);
        phaser_->plug(phaser_blend, Phaser::kBlend);

        // Initialize the phaser after plugging in parameters.
        phaser_->init();

        // Complete the initialization of the SynthModule base class.
        SynthModule::init();
    }

    /**
     * @brief Performs a hard reset of the phaser.
     *
     * Resets internal modulation state and filters so the phaser starts fresh.
     */
    void PhaserModule::hardReset() {
        phaser_->hardReset();
    }

    /**
     * @brief Enables or disables the phaser module.
     *
     * When enabled, it starts processing audio. On enabling, it performs a hard reset.
     *
     * @param enable True to enable processing, false to disable.
     */
    void PhaserModule::enable(bool enable) {
        SynthModule::enable(enable);
        process(1); // Process a single sample to update state.
        if (enable)
            phaser_->hardReset();
    }

    /**
     * @brief Corrects the phaser's time-based parameters.
     *
     * Adjusts the phaser's internal timing state to match a specified playback time.
     *
     * @param seconds The playback time in seconds.
     */
    void PhaserModule::correctToTime(double seconds) {
        SynthModule::correctToTime(seconds);
        phaser_->correctToTime(seconds);
    }

    /**
     * @brief Sets the sample rate of the phaser.
     *
     * The phaser updates its internal parameters and processes accordingly at the new sample rate.
     *
     * @param sample_rate The sample rate in Hz.
     */
    void PhaserModule::setSampleRate(int sample_rate) {
        SynthModule::setSampleRate(sample_rate);
        phaser_->setSampleRate(sample_rate);
    }

    /**
     * @brief Processes an input audio buffer through the phaser.
     *
     * First, the SynthModule processes the block, then the internal phaser applies its effect.
     *
     * @param audio_in Pointer to the input audio buffer.
     * @param num_samples Number of samples to process.
     */
    void PhaserModule::processWithInput(const poly_float* audio_in, int num_samples) {
        SynthModule::process(num_samples);
        phaser_->processWithInput(audio_in, num_samples);
    }
} // namespace vital
