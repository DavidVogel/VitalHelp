#pragma once

#include "synth_module.h"
#include "producers_module.h"
#include "voice_handler.h"
#include "wavetable.h"
#include "synth_types.h"
#include "line_generator.h"

#include <vector>

namespace vital {

  // Forward declarations
  class AudioRateEnvelope;
  class FiltersModule;
  class LegatoFilter;
  class LineMap;
  class LfoModule;
  class EnvelopeModule;
  class RandomLfoModule;
  class TriggerRandom;

  /**
   * @class EffectsModulationHandler
   * @brief A VoiceHandler extension managing various modulation sources for effects processing.
   *
   * The EffectsModulationHandler provides and orchestrates multiple modulation sources,
   * such as LFOs, envelopes, random modulators, and user-defined macros, intended for use
   * within the effects processing chain. It handles note on/off events, synchronizes modulation
   * sources, and manages the reading and writing of polyphonic modulation data.
   */
  class EffectsModulationHandler : public VoiceHandler {
    public:
      /**
       * @brief Constructs an EffectsModulationHandler given a beats-per-second output reference.
       *
       * @param beats_per_second Pointer to an Output that tracks the tempo in beats per second.
       */
      EffectsModulationHandler(Output* beats_per_second);

      /** @brief Default destructor. Cleans up allocated resources. */
      virtual ~EffectsModulationHandler() { }

      /**
       * @brief Creates a clone of this Processor. (Not implemented for EffectsModulationHandler).
       * @return Returns nullptr.
       */
      virtual Processor* clone() const override { VITAL_ASSERT(false); return nullptr; }

      /**
       * @brief Initializes internal processors and modulation sources.
       *
       * Adds submodules (LFO, envelopes, random modulators, etc.) and configures modulation
       * connections and parameter readouts.
       */
      void init() override;

      /**
       * @brief Prepares the object for destruction by removing all modulation processors.
       */
      void prepareDestroy();

      /**
       * @brief Processes audio/midi data for a block of samples.
       *
       * Updates modulation sources, handles note triggers/retriggers, and sets up
       * status outputs for the active voices.
       *
       * @param num_samples The number of samples to process.
       */
      void process(int num_samples) override;

      /**
       * @brief Handles note-on events, triggering envelopes and other modulation if needed.
       *
       * @param note     MIDI note number.
       * @param velocity Note-on velocity.
       * @param sample   The sample index where this note-on occurs.
       * @param channel  The MIDI channel.
       */
      void noteOn(int note, mono_float velocity, int sample, int channel) override;

      /**
       * @brief Handles note-off events, ending envelopes and other modulation if needed.
       *
       * @param note    MIDI note number.
       * @param lift    The note-off velocity or release velocity.
       * @param sample  The sample index where this note-off occurs.
       * @param channel The MIDI channel.
       */
      void noteOff(int note, mono_float lift, int sample, int channel) override;

      /**
       * @brief Indicates whether an Output should accumulate (sum) multiple voices.
       *
       * In this context, the function always returns `false` to avoid accumulation.
       *
       * @param output Pointer to the Output in question.
       * @return False, indicating no accumulation is needed.
       */
      bool shouldAccumulate(Output* output) override { return false;  }

      /**
       * @brief Synchronizes internal modulator phases to a given absolute time.
       *
       * Used to place LFOs or other time-based modulators at a specific location in their cycle.
       *
       * @param seconds The absolute time in seconds.
       */
      void correctToTime(double seconds) override;

      /**
       * @brief Disables all modulation sources that are not strictly necessary.
       *
       * Typically used to reduce CPU overhead when certain modulators are not in use.
       */
      void disableUnnecessaryModSources();

      /**
       * @brief Disables a specific modulation source by name.
       *
       * @param source The string identifier of the source to disable.
       */
      void disableModSource(const std::string& source);

      /**
       * @brief Returns a reference to the polyphonic modulation output map.
       *
       * @return Reference to the output map storing polyphonic modulations.
       */
      output_map& getPolyModulations() override;

      /**
       * @brief Returns a reference to the modulation connection bank.
       *
       * @return Reference to the ModulationConnectionBank that stores modulation connections.
       */
      ModulationConnectionBank& getModulationBank() { return modulation_bank_; }

      /**
       * @brief Retrieves a pointer to one of the internal LFO line generators.
       *
       * @param index Zero-based index of the LFO line generator.
       * @return Pointer to the requested LineGenerator.
       */
      LineGenerator* getLfoSource(int index) { return &lfo_sources_[index]; }

      /**
       * @brief Retrieves the direct output used for sub-mixing signals.
       *
       * @return Pointer to the direct output (accumulated output).
       */
      Output* getDirectOutput() { return getAccumulatedOutput(sub_direct_output_->output()); }

      /**
       * @brief Provides access to a note-retriggered output, triggered on each note-on event.
       *
       * @return Pointer to the Output representing note retrigger events.
       */
      Output* note_retrigger() { return &note_retriggered_; }

      /**
       * @brief Returns a pointer to the MIDI offset output used in pitch computations.
       *
       * @return Pointer to the Output representing MIDI pitch offset.
       */
      Output* midi_offset_output() { return midi_offset_output_; }

    private:
      /**
       * @brief Creates the articulation path, handling note pitch, portamento, and pitch bending.
       */
      void createArticulation();

      /**
       * @brief Sets up all internal modulators (LFOs, envelopes, random).
       */
      void createModulators();

      /**
       * @brief Initializes and plugs in FiltersModule objects using the provided keytrack output.
       *
       * @param keytrack Pointer to the output indicating MIDI note-based keytrack information.
       */
      void createFilters(Output* keytrack);

      /**
       * @brief Sets up readouts for polyphonic modulations, storing them for GUI or debugging.
       */
      void setupPolyModulationReadouts();

      /**
       * @brief Bank of modulation connections (processors) that handle routing from sources to targets.
       */
      ModulationConnectionBank modulation_bank_;

      /**
       * @brief Pointer to an output representing the tempo in beats per second.
       */
      Output* beats_per_second_;

      /**
       * @brief Processor computing note pitch relative to a reference (MIDI center).
       */
      Processor* note_from_reference_;

      /**
       * @brief Output for the MIDI pitch offset (used in pitch computations).
       */
      Output* midi_offset_output_;

      /**
       * @brief Adds local pitch bend to MIDI note pitch for final pitch computations.
       */
      Processor* bent_midi_;

      /**
       * @brief Processor tracking the current pitch (with portamento) for voice-based usage.
       */
      Processor* current_midi_note_;

      /**
       * @brief Pointer to the filters module handling the filter creation and management.
       */
      FiltersModule* filters_module_;

      /**
       * @brief Array of LfoModule pointers, one for each LFO slot.
       */
      LfoModule* lfos_[kNumLfos];

      /**
       * @brief Array of EnvelopeModule pointers, one for each envelope slot.
       */
      EnvelopeModule* envelopes_[kNumEnvelopes];

      /**
       * @brief An Output triggered each time a note-on event occurs, if retrigger is needed.
       */
      Output note_retriggered_;

      /**
       * @brief Array of line generators for LFO waveforms.
       */
      LineGenerator lfo_sources_[kNumLfos];

      /**
       * @brief Random trigger generator, producing a random value on each retrigger event.
       */
      TriggerRandom* random_;

      /**
       * @brief Array of random LFO modules, each producing a smoothly varying random signal.
       */
      RandomLfoModule* random_lfos_[kNumRandomLfos];

      /**
       * @brief @c LineMap pointers for mapping MIDI note, velocity, aftertouch, etc. to internal values.
       */
      LineMap* note_mapping_;
      LineMap* velocity_mapping_;
      LineMap* aftertouch_mapping_;
      LineMap* slide_mapping_;
      LineMap* lift_mapping_;
      LineMap* mod_wheel_mapping_;
      LineMap* pitch_wheel_mapping_;

      /**
       * @brief A constant representing stereo factor (generally for stereo-based modulations).
       */
      cr::Value* stereo_;

      /**
       * @brief Multiplier used to convert MIDI note values to a [0..1] range (note percentage).
       */
      cr::Multiply* note_percentage_;

      /**
       * @brief Internal multiplier used to build a direct output signal chain.
       */
      Multiply* output_;
      Multiply* sub_direct_output_;

      /**
       * @brief Map of polyphonic readouts used by the synthesizer for tracking modulations.
       */
      output_map poly_readouts_;

      JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EffectsModulationHandler)
  };
} // namespace vital
