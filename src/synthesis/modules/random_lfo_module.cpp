#include "random_lfo_module.h"
#include "random_lfo.h"

namespace vital {

    /**
     * @brief Constructs the RandomLfoModule and adds the internal RandomLfo processor.
     *
     * @param prefix A string prefix for LFO parameters.
     * @param beats_per_second A pointer to an Output containing the tempo in beats per second.
     */
    RandomLfoModule::RandomLfoModule(const std::string& prefix, const Output* beats_per_second) :
            SynthModule(kNumInputs, 1), prefix_(prefix), beats_per_second_(beats_per_second) {
        lfo_ = new RandomLfo();
        addProcessor(lfo_);
    }

    /**
     * @brief Initializes the RandomLfoModule by creating parameter controls and connecting them to the LFO.
     *
     * This includes:
     * - Frequency control (with tempo sync option).
     * - Style selection to determine the character of the random LFO.
     * - Stereo parameter for stereo spread.
     * - Sync type parameter to determine how the LFO behaves in relation to the host tempo.
     */
    void RandomLfoModule::init() {
        Output* free_frequency = createPolyModControl(prefix_ + "_frequency");
        Value* style = createBaseControl(prefix_ + "_style");
        Value* stereo = createBaseControl(prefix_ + "_stereo");
        Value* sync_type = createBaseControl(prefix_ + "_sync_type");

        Output* frequency = createTempoSyncSwitch(prefix_, free_frequency->owner, beats_per_second_, true, input(kMidi));
        lfo_->useInput(input(kNoteTrigger), RandomLfo::kReset);
        lfo_->useOutput(output());
        lfo_->plug(frequency, RandomLfo::kFrequency);
        lfo_->plug(style, RandomLfo::kStyle);
        lfo_->plug(stereo, RandomLfo::kStereo);
        lfo_->plug(sync_type, RandomLfo::kSync);
    }

    /**
     * @brief Adjusts the internal LFO timing to the given playback time.
     *
     * @param seconds The playback time in seconds.
     */
    void RandomLfoModule::correctToTime(double seconds) {
        lfo_->correctToTime(seconds);
    }
} // namespace vital
