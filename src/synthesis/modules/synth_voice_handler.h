#pragma once

#include "synth_module.h"
#include "producers_module.h"
#include "voice_handler.h"
#include "wavetable.h"
#include "synth_types.h"
#include "line_generator.h"

#include <vector>

namespace vital {
    class AudioRateEnvelope;
    class FiltersModule;
    class LegatoFilter;
    class LineMap;
    class LfoModule;
    class EnvelopeModule;
    class RandomLfoModule;
    class TriggerRandom;

    /**
     * @brief Manages per-voice processing of audio signals within the Vital synthesizer.
     *
     * The SynthVoiceHandler is responsible for coordinating the various signal sources
     * (oscillators, sampler, LFOs, envelopes, random LFOs, and filters) at the voice level.
     * It handles note on/off events, voice allocation, pitch and modulation routing,
     * and provides readouts of modulation and status outputs for use in the synthesizer’s
     * modulation system and UI feedback.
     */
    class SynthVoiceHandler : public VoiceHandler {
    public:
        /**
         * @brief Constructs a SynthVoiceHandler with a given beats-per-second reference.
         *
         * @param beats_per_second An Output providing the current tempo in beats per second.
         */
        SynthVoiceHandler(Output* beats_per_second);

        /**
         * @brief Destructor.
         */
        virtual ~SynthVoiceHandler() { }

        /**
         * @brief Unsupported clone operation.
         *
         * @return Always returns nullptr and asserts.
         */
        virtual Processor* clone() const override { VITAL_ASSERT(false); return nullptr; }

        /**
         * @brief Initializes the voice handler, creating and configuring all internal modules.
         */
        void init() override;

        /**
         * @brief Prepares the voice handler for destruction, removing processors as needed.
         */
        void prepareDestroy();

        /**
         * @brief Processes all active voices for a given number of samples.
         *
         * @param num_samples The number of samples to process.
         */
        void process(int num_samples) override;

        /**
         * @brief Handles a note-on event, starting or retriggering voices as appropriate.
         *
         * @param note The MIDI note number.
         * @param velocity The note-on velocity.
         * @param sample The sample index at which the note-on occurs.
         * @param channel The MIDI channel of the note event.
         */
        void noteOn(int note, mono_float velocity, int sample, int channel) override;

        /**
         * @brief Handles a note-off event, releasing voices or retriggering them in legato contexts.
         *
         * @param note The MIDI note number.
         * @param lift The release velocity (lift) value.
         * @param sample The sample index at which the note-off occurs.
         * @param channel The MIDI channel of the note event.
         */
        void noteOff(int note, mono_float lift, int sample, int channel) override;

        /**
         * @brief Determines if the given output should be accumulated across voices.
         *
         * @param output The output to check.
         * @return True if it should be accumulated, false otherwise.
         */
        bool shouldAccumulate(Output* output) override;

        /**
         * @brief Corrects time-dependent parameters to a given playback time.
         *
         * Ensures that LFOs and random LFOs align with the given time.
         *
         * @param seconds The playback time in seconds.
         */
        void correctToTime(double seconds) override;

        /**
         * @brief Disables all unnecessary modulation sources for efficiency.
         *
         * Typically done when certain sources are not connected and need not run.
         */
        void disableUnnecessaryModSources();

        /**
         * @brief Disables a specific modulation source by name.
         *
         * @param source The name of the modulation source (e.g., "env_2", "lfo_1").
         */
        void disableModSource(const std::string& source);

        /**
         * @brief Retrieves a map of all polyphonic modulations.
         *
         * @return A reference to an output_map of polyphonic modulation outputs.
         */
        output_map& getPolyModulations() override;

        /**
         * @brief Retrieves the modulation connection bank.
         *
         * @return A reference to the ModulationConnectionBank handling all modulation connections.
         */
        ModulationConnectionBank& getModulationBank() { return modulation_bank_; }

        /**
         * @brief Gets a wavetable from the producers module.
         *
         * @param index The oscillator index.
         * @return A pointer to the requested Wavetable.
         */
        Wavetable* getWavetable(int index) { return producers_->getWavetable(index); }

        /**
         * @brief Gets the current sample from the producers module's sampler.
         *
         * @return A pointer to the current Sample in use.
         */
        Sample* getSample() { return producers_->getSample(); }

        /**
         * @brief Retrieves the LFO source LineGenerator at a given index.
         *
         * @param index The LFO index.
         * @return A pointer to the LineGenerator for that LFO.
         */
        LineGenerator* getLfoSource(int index) { return &lfo_sources_[index]; }

        /**
         * @brief Gets the direct output (bypassing filters/effects) for accumulation.
         *
         * @return An Output pointer for the direct audio signal.
         */
        Output* getDirectOutput() { return getAccumulatedOutput(direct_output_->output()); }

        /**
         * @brief Retrieves the output that triggers when a note is retriggered.
         *
         * @return A pointer to the Output representing note retriggers.
         */
        Output* note_retrigger() { return &note_retriggered_; }

        /**
         * @brief Retrieves the MIDI offset output.
         *
         * @return A pointer to the Output representing the MIDI offset.
         */
        Output* midi_offset_output() { return midi_offset_output_; }

        /**
         * @brief Enables a modulation connection processor, making its modulation active.
         *
         * @param processor The ModulationConnectionProcessor to enable.
         */
        void enableModulationConnection(ModulationConnectionProcessor* processor);

        /**
         * @brief Disables a modulation connection processor, removing it from active modulation.
         *
         * @param processor The ModulationConnectionProcessor to disable.
         */
        void disableModulationConnection(ModulationConnectionProcessor* processor);

        /**
         * @brief Gets the queue of currently enabled modulation connection processors.
         *
         * @return A reference to a CircularQueue of ModulationConnectionProcessor*.
         */
        CircularQueue<ModulationConnectionProcessor*>& enabledModulationConnection() {
            return enabled_modulation_processors_;
        }

    private:
        /**
         * @brief Creates modules and connections related to note articulation (portamento, pitch bend).
         */
        void createNoteArticulation();

        /**
         * @brief Creates and sets up the producers module (oscillators, sampler).
         */
        void createProducers();

        /**
         * @brief Creates LFOs, envelopes, random LFOs, and other modulation sources.
         */
        void createModulators();

        /**
         * @brief Creates the final voice output stage, including amplitude handling.
         */
        void createVoiceOutput();

        /**
         * @brief Creates and configures filter modules and related routing.
         *
         * @param keytrack An Output providing key-tracking information to the filters.
         */
        void createFilters(Output* keytrack);

        /**
         * @brief Sets up polyphonic modulation readouts, registering them as control-rate outputs.
         */
        void setupPolyModulationReadouts();

        ModulationConnectionBank modulation_bank_;  /**< Bank of all modulation connections. */
        CircularQueue<ModulationConnectionProcessor*> enabled_modulation_processors_; /**< Active modulation processors. */

        ProducersModule* producers_; /**< The producers module (oscillators + sampler). */
        Output* beats_per_second_;   /**< Provides tempo in beats per second. */

        Processor* note_from_reference_; /**< Processor adjusting MIDI note to a reference point. */
        Output* midi_offset_output_;     /**< MIDI offset output, processed from note and pitch bend data. */
        Processor* bent_midi_;           /**< Processor for pitch-bent MIDI note values. */
        Processor* current_midi_note_;   /**< Handles portamento and final MIDI note calculation. */
        EnvelopeModule* amplitude_envelope_; /**< The amplitude envelope (env_1). */
        Processor* amplitude_;           /**< Controls the final amplitude of the voice. */
        Processor* pitch_wheel_;         /**< Processor handling pitch wheel modulation. */

        FiltersModule* filters_module_;  /**< The filters module processing routed oscillator/sample outputs. */

        LfoModule* lfos_[kNumLfos];           /**< LFO modules. */
        EnvelopeModule* envelopes_[kNumEnvelopes]; /**< Envelope modules. */

        Output note_retriggered_; /**< Output that triggers on note retrigger events. */

        LineGenerator lfo_sources_[kNumLfos]; /**< Line generators for initializing LFO waveforms. */

        TriggerRandom* random_;                /**< A trigger-based random source. */
        RandomLfoModule* random_lfos_[kNumRandomLfos]; /**< Random LFO modules. */

        LineMap* note_mapping_;       /**< Maps note values. */
        LineMap* velocity_mapping_;   /**< Maps velocity values. */
        LineMap* aftertouch_mapping_; /**< Maps aftertouch values. */
        LineMap* slide_mapping_;      /**< Maps slide values. */
        LineMap* lift_mapping_;       /**< Maps lift values. */
        LineMap* mod_wheel_mapping_;  /**< Maps mod wheel values. */
        LineMap* pitch_wheel_mapping_;/**< Maps pitch wheel values. */

        cr::Value* stereo_;               /**< A fixed stereo value output. */
        cr::Multiply* note_percentage_;   /**< Converts MIDI notes into a 0-1 percentage scale. */

        Multiply* output_;        /**< Final output multiplication (voice sum * amplitude). */
        Multiply* direct_output_; /**< Direct output bypassing filters. */
        Output num_voices_;       /**< Tracks number of active voices. */

        output_map poly_readouts_;  /**< Map of polyphonic modulation readouts. */
        poly_mask last_active_voice_mask_; /**< Mask of currently active voices. */

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SynthVoiceHandler)
    };
} // namespace vital
