#include "equalizer_module.h"

#include "digital_svf.h"
#include "value.h"

namespace vital {

    EqualizerModule::EqualizerModule() :
            SynthModule(0, 1),
            low_mode_(nullptr), band_mode_(nullptr), high_mode_(nullptr),
            high_pass_(nullptr), low_shelf_(nullptr),
            notch_(nullptr), band_shelf_(nullptr),
            low_pass_(nullptr), high_shelf_(nullptr) {
        audio_memory_ = std::make_shared<vital::StereoMemory>(vital::kAudioMemorySamples);
    }

    void EqualizerModule::init() {
        /**
         * @brief Initializes all filter instances and their parameter connections.
         *
         * The equalizer is composed of three sections (low, mid, high), each can operate in one of two modes
         * based on the respective mode Value. It creates and plugs in various modulation controls
         * for cutoff, resonance, gain, and sets the filter styles (12dB pass, shelving, notch, etc.).
         */

        static const cr::Value kPass(DigitalSvf::k12Db);
        static const cr::Value kNotch(DigitalSvf::kNotchPassSwap);
        static const cr::Value kShelving(DigitalSvf::kShelving);

        high_pass_ = new DigitalSvf();
        low_shelf_ = new DigitalSvf();
        band_shelf_ = new DigitalSvf();
        notch_ = new DigitalSvf();
        low_pass_ = new DigitalSvf();
        high_shelf_ = new DigitalSvf();

        // Basic mode and no drive compensation to ensure clean filter output for EQ.
        high_pass_->setDriveCompensation(false);
        high_pass_->setBasic(true);
        notch_->setDriveCompensation(false);
        notch_->setBasic(true);
        low_pass_->setDriveCompensation(false);
        low_pass_->setBasic(true);

        addIdleProcessor(high_pass_);
        addIdleProcessor(low_shelf_);
        addIdleProcessor(notch_);
        addIdleProcessor(band_shelf_);
        addIdleProcessor(low_pass_);
        addIdleProcessor(high_shelf_);
        low_pass_->useOutput(output()); // Final output from either low_pass_ or high_shelf_.
        high_shelf_->useOutput(output());

        // Mode controls for switching filter types.
        low_mode_ = createBaseControl("eq_low_mode");
        band_mode_ = createBaseControl("eq_band_mode");
        high_mode_ = createBaseControl("eq_high_mode");

        // Parameter controls for cutoff frequencies.
        Output* low_cutoff_midi = createMonoModControl("eq_low_cutoff", true, true);
        Output* band_cutoff_midi = createMonoModControl("eq_band_cutoff", true, true);
        Output* high_cutoff_midi = createMonoModControl("eq_high_cutoff", true, true);

        // Resonance controls for each band.
        Output* low_resonance = createMonoModControl("eq_low_resonance");
        Output* band_resonance = createMonoModControl("eq_band_resonance");
        Output* high_resonance = createMonoModControl("eq_high_resonance");

        // Gain controls for shelf filters.
        Output* low_decibels = createMonoModControl("eq_low_gain");
        Output* band_decibels = createMonoModControl("eq_band_gain");
        Output* high_decibels = createMonoModControl("eq_high_gain");

        // Configure each filter with the appropriate style and parameter mappings.
        high_pass_->plug(&kPass, DigitalSvf::kStyle);
        high_pass_->plug(&constants::kValueTwo, DigitalSvf::kPassBlend);
        high_pass_->plug(low_cutoff_midi, DigitalSvf::kMidiCutoff);
        high_pass_->plug(low_resonance, DigitalSvf::kResonance);

        low_shelf_->plug(&kShelving, DigitalSvf::kStyle);
        low_shelf_->plug(&constants::kValueZero, DigitalSvf::kPassBlend);
        low_shelf_->plug(low_cutoff_midi, DigitalSvf::kMidiCutoff);
        low_shelf_->plug(low_resonance, DigitalSvf::kResonance);
        low_shelf_->plug(low_decibels, DigitalSvf::kGain);

        band_shelf_->plug(&kShelving, DigitalSvf::kStyle);
        band_shelf_->plug(&constants::kValueOne, DigitalSvf::kPassBlend);
        band_shelf_->plug(band_cutoff_midi, DigitalSvf::kMidiCutoff);
        band_shelf_->plug(band_resonance, DigitalSvf::kResonance);
        band_shelf_->plug(band_decibels, DigitalSvf::kGain);

        notch_->plug(&kNotch, DigitalSvf::kStyle);
        notch_->plug(&constants::kValueOne, DigitalSvf::kPassBlend);
        notch_->plug(band_cutoff_midi, DigitalSvf::kMidiCutoff);
        notch_->plug(band_resonance, DigitalSvf::kResonance);

        low_pass_->plug(&kPass, DigitalSvf::kStyle);
        low_pass_->plug(&constants::kValueZero, DigitalSvf::kPassBlend);
        low_pass_->plug(high_cutoff_midi, DigitalSvf::kMidiCutoff);
        low_pass_->plug(high_resonance, DigitalSvf::kResonance);

        high_shelf_->plug(&kShelving, DigitalSvf::kStyle);
        high_shelf_->plug(&constants::kValueTwo, DigitalSvf::kPassBlend);
        high_shelf_->plug(high_cutoff_midi, DigitalSvf::kMidiCutoff);
        high_shelf_->plug(high_resonance, DigitalSvf::kResonance);
        high_shelf_->plug(high_decibels, DigitalSvf::kGain);

        SynthModule::init();
    }

    void EqualizerModule::hardReset() {
        /**
         * @brief Resets all filters to their initial states, clearing internal buffers.
         */
        high_pass_->reset(constants::kFullMask);
        low_shelf_->reset(constants::kFullMask);
        band_shelf_->reset(constants::kFullMask);
        notch_->reset(constants::kFullMask);
        low_pass_->reset(constants::kFullMask);
        high_shelf_->reset(constants::kFullMask);
    }

    void EqualizerModule::enable(bool enable) {
        /**
         * @brief Enables or disables the EQ module.
         *
         * On enable, it resets the filters to ensure a clean start. On disable, it stops further processing.
         */
        SynthModule::enable(enable);
        process(1);
        if (enable) {
            high_pass_->reset(constants::kFullMask);
            low_shelf_->reset(constants::kFullMask);
            band_shelf_->reset(constants::kFullMask);
            notch_->reset(constants::kFullMask);
            low_pass_->reset(constants::kFullMask);
            high_shelf_->reset(constants::kFullMask);
        }
    }

    void EqualizerModule::setSampleRate(int sample_rate) {
        /**
         * @brief Updates the sample rate for all internal filters so time-dependent parameters remain correct.
         */
        high_pass_->setSampleRate(sample_rate);
        low_shelf_->setSampleRate(sample_rate);
        band_shelf_->setSampleRate(sample_rate);
        notch_->setSampleRate(sample_rate);
        low_pass_->setSampleRate(sample_rate);
        high_shelf_->setSampleRate(sample_rate);
    }

    void EqualizerModule::processWithInput(const poly_float* audio_in, int num_samples) {
        /**
         * @brief Processes the input audio through the chosen filters for low, mid, and high bands.
         *
         * The module picks the appropriate filter (e.g., high-pass or low shelf for low band)
         * based on the mode parameters, then processes audio in sequence: low section first, then band section,
         * then high section. The final output samples are stored in both the output buffer and the internal stereo memory.
         */
        SynthModule::process(num_samples);

        Processor* low_processor = low_mode_->value() ? high_pass_ : low_shelf_;
        Processor* band_processor = band_mode_->value() ? notch_ : band_shelf_;
        Processor* high_processor = high_mode_->value() ? low_pass_ : high_shelf_;

        // Chain the processing: low section -> band section -> high section
        low_processor->processWithInput(audio_in, num_samples);
        band_processor->processWithInput(low_processor->output()->buffer, num_samples);
        high_processor->processWithInput(band_processor->output()->buffer, num_samples);

        const poly_float* output_buffer = high_processor->output()->buffer;
        for (int i = 0; i < num_samples; ++i)
            audio_memory_->push(output_buffer[i]);
    }
} // namespace vital
