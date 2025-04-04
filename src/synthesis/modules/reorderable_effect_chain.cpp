#include "reorderable_effect_chain.h"

#include "chorus_module.h"
#include "compressor_module.h"
#include "delay_module.h"
#include "distortion_module.h"
#include "equalizer_module.h"
#include "flanger_module.h"
#include "filter_module.h"
#include "phaser_module.h"
#include "reverb_module.h"
#include "synth_strings.h"

namespace vital {

    /**
     * @brief A wrapper module that hosts a filter effect. This is a specialized module for filter effects in the chain.
     *
     * The FilterFxModule encapsulates a FilterModule and provides a processWithInput function for convenience.
     */
    class FilterFxModule : public SynthModule {
    public:
        /**
         * @enum InputIndices
         * @brief Defines the input indices for this filter module.
         */
        enum {
            kAudio,    /**< The input audio signal. */
            kKeytrack, /**< Keytrack input for pitch-based modulation of filter parameters. */
            kNumInputs /**< Total number of inputs. */
        };

        /**
         * @brief Constructs the FilterFxModule.
         *
         * @param keytrack A pointer to the output providing a key-track signal.
         */
        FilterFxModule(const Output* keytrack) : SynthModule(kNumInputs, 1) {
            filter_ = new FilterModule("filter_fx");
            filter_->setCreateOnValue(false);
            filter_->setMono(true);
            addSubmodule(filter_);
            addProcessor(filter_);
            filter_->useOutput(output());
            filter_->plug(&input_, FilterModule::kAudio);
            filter_->plug(keytrack, FilterModule::kKeytrack);
        }

        /**
         * @brief Processes a block of samples from a given input buffer through the filter effect.
         *
         * @param audio_in A pointer to the input buffer containing audio samples.
         * @param num_samples The number of samples to process.
         */
        void processWithInput(const poly_float* audio_in, int num_samples) override {
            utils::copyBuffer(input_.buffer, audio_in, num_samples);
            filter_->process(num_samples);
        }

        /**
         * @brief Sets the oversampling amount for this module.
         *
         * @param oversampling The oversampling factor.
         */
        void setOversampleAmount(int oversampling) override {
            input_.ensureBufferSize(kMaxBufferSize * oversampling);
            SynthModule::setOversampleAmount(oversampling);
        }

    private:
        FilterModule* filter_; /**< The internal filter module. */
        Output input_;          /**< The input buffer for processing. */
    };

    /**
     * @brief Constructs the ReorderableEffectChain and initializes all effects.
     *
     * @param beats_per_second An Output providing tempo in beats per second.
     * @param keytrack An Output providing a keytrack signal.
     */
    ReorderableEffectChain::ReorderableEffectChain(const Output* beats_per_second, const Output* keytrack) :
            vital::SynthModule(kNumInputs, 1),
            equalizer_memory_(nullptr),
            beats_per_second_(beats_per_second),
            keytrack_(keytrack),
            last_order_(0.0f) {
        for (int i = 0; i < constants::kNumEffects; ++i) {
            SynthModule* effect_module = createEffectModule(i);
            VITAL_ASSERT(effect_module);

            addSubmodule(effect_module);
            addProcessor(effect_module);
            effects_on_[i] = createBaseControl(strings::kEffectOrder[i] + "_on");
            effects_[i] = effect_module;
            effect_order_[i] = i;
        }

        // Encode the initial order into a float for easy comparison and storage.
        last_order_ = utils::encodeOrderToFloat(effect_order_, constants::kNumEffects);
    }

    /**
     * @brief Instantiates and returns a new effect module based on the effect index.
     *
     * @param index The index of the effect type to create.
     * @return A pointer to the newly created effect SynthModule.
     */
    SynthModule* ReorderableEffectChain::createEffectModule(int index) {
        switch(index) {
            case constants::kChorus:
                return new ChorusModule(beats_per_second_);
            case constants::kCompressor:
                return new CompressorModule();
            case constants::kDelay:
                return new DelayModule(beats_per_second_);
            case constants::kDistortion:
                return new DistortionModule();
            case constants::kEq: {
                EqualizerModule* eq = new EqualizerModule();
                equalizer_memory_ = eq->getAudioMemory();
                return eq;
            }
            case constants::kFlanger:
                return new FlangerModule(beats_per_second_);
            case constants::kFilterFx:
                return new FilterFxModule(keytrack_);
            case constants::kPhaser:
                return new PhaserModule(beats_per_second_);
            case constants::kReverb:
                return new ReverbModule();
            default:
                return nullptr;
        }
    }

    /**
     * @brief Processes the audio by reading from the main audio input.
     *
     * This method obtains the input audio and passes it to processWithInput.
     *
     * @param num_samples The number of samples to process.
     */
    void ReorderableEffectChain::process(int num_samples) {
        const poly_float* audio_in = input(kAudio)->source->buffer;
        processWithInput(audio_in, num_samples);
    }

    /**
     * @brief Processes the given audio through the effects in the specified order.
     *
     * If the order changes (via the order input), it decodes and updates the effect_order_ array.
     * Then, each effect is enabled/disabled based on its "on" parameter and processed in turn.
     *
     * @param audio_in A pointer to the audio buffer containing input samples.
     * @param num_samples The number of samples to process.
     */
    void ReorderableEffectChain::processWithInput(const poly_float* audio_in, int num_samples) {
        mono_float float_order = std::round(input(kOrder)->at(0)[0]);
        if (float_order != last_order_)
            utils::decodeFloatToOrder(effect_order_, float_order, constants::kNumEffects);
        last_order_ = float_order;

        for (int i = 0; i < constants::kNumEffects; ++i) {
            VITAL_ASSERT(utils::isFinite(audio_in, num_samples));

            int index = effect_order_[i];
            bool on = effects_on_[index]->value();
            bool enabled = effects_[index]->enabled();
            if (on != enabled)
                effects_[index]->enable(on);

            if (on) {
                effects_[index]->processWithInput(audio_in, num_samples);
                audio_in = effects_[index]->output(0)->buffer;
            }
        }

        VITAL_ASSERT(utils::isFinite(audio_in, num_samples));
        utils::copyBuffer(output()->buffer, audio_in, num_samples);
    }

    /**
     * @brief Performs a hard reset of all effects in the chain.
     */
    void ReorderableEffectChain::hardReset() {
        for (int i = 0; i < constants::kNumEffects; ++i)
            effects_[i]->hardReset();
    }

    /**
     * @brief Corrects the time-dependent parameters of all effects to the given time.
     *
     * @param seconds The playback time in seconds.
     */
    void ReorderableEffectChain::correctToTime(double seconds) {
        for (int i = 0; i < constants::kNumEffects; ++i)
            effects_[i]->correctToTime(seconds);
    }
} // namespace vital
