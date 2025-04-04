#pragma once

#include "circular_queue.h"
#include "synth_module.h"
#include "note_handler.h"
#include "tuning.h"

class LineGenerator;

namespace vital {
  // Forward declarations
  class PeakMeter;
  class ReorderableEffectChain;
  class EffectsModulationHandler;
  class Sample;
  class StereoMemory;
  class SynthLfo;
  class Upsampler;
  class Value;
  class ValueSwitch;
  class Wavetable;

  /**
   * @class SoundEngine
   * @brief Core class responsible for handling note events, oversampling, and the main effects chain.
   *
   * The SoundEngine manages the top-level module architecture for Vital's effect
   * and modulation engines, connecting note handling, oversampling, effect chaining,
   * and modulation routing. It derives from SynthModule for the base module framework
   * and NoteHandler for note-level logic.
   */
  class SoundEngine : public SynthModule, public NoteHandler {
    public:
      /**
       * @brief Default oversampling factor.
       */
      static constexpr int kDefaultOversamplingAmount = 2;

      /**
       * @brief Default sample rate used before explicit configuration.
       */
      static constexpr int kDefaultSampleRate = 44100;

      /**
       * @brief Constructs a SoundEngine and initializes internal state and controls.
       */
      SoundEngine();

      /**
       * @brief Destroys the SoundEngine, cleaning up submodules and modulation resources.
       */
      virtual ~SoundEngine();

      /**
       * @brief Initializes the engine by creating base controls and submodules (effects chain, etc.).
       */
      void init() override;

      /**
       * @brief Processes a block of samples using the current state of the SoundEngine.
       *
       * Renders any oversampling, effect chaining, and modulation updates.
       *
       * @param audio_in    A buffer of poly_float audio data to be processed (unused in many contexts).
       * @param num_samples Number of samples in the buffer.
       */
      void processWithInput(const poly_float* audio_in, int num_samples) override;

      /**
       * @brief Synchronizes all time-based modules to a given time, e.g. for timeline-based automation.
       *
       * @param seconds The absolute time in seconds.
       */
      void correctToTime(double seconds) override;

      /**
       * @brief Returns the number of currently pressed notes across all channels.
       *
       * @return The count of pressed notes.
       */
      int getNumPressedNotes();

      /**
       * @brief Connects a modulation source to a destination via the given modulation_change structure.
       *
       * @param change A struct describing the source, destination, and scaling parameters.
       */
      void connectModulation(const modulation_change& change);

      /**
       * @brief Disconnects a previously established modulation route.
       *
       * @param change A struct describing the source, destination, and parameters for removal.
       */
      void disconnectModulation(const modulation_change& change);

      /**
       * @brief Returns the number of active voices in the engine.
       *
       * @return The count of active voices.
       */
      int getNumActiveVoices();

      /**
       * @brief Provides access to the internal ModulationConnectionBank for managing mod routes.
       *
       * @return A reference to the ModulationConnectionBank.
       */
      ModulationConnectionBank& getModulationBank();

      /**
       * @brief Retrieves the most recently active note number.
       *
       * @return The last note number played (as a float).
       */
      mono_float getLastActiveNote() const;

      /**
       * @brief Sets a custom Tuning object for note-to-frequency mapping.
       *
       * @param tuning Pointer to a Tuning instance defining frequency ratios.
       */
      void setTuning(const Tuning* tuning);

      /**
       * @brief Configures the engine’s oversampling factor and sample rate.
       *
       * @param oversampling_amount The new oversampling factor (e.g., 2, 4).
       * @param sample_rate         The current audio sample rate in Hz.
       */
      void setOversamplingAmount(int oversampling_amount, int sample_rate);

      /**
       * @brief Stops and resets all currently playing notes and any lingering states in the effect chain.
       */
      void allSoundsOff() override;

      /**
       * @brief Turns off all notes on all channels at the specified sample index.
       *
       * @param sample The sample index at which to stop notes.
       */
      void allNotesOff(int sample) override;

      /**
       * @brief Turns off all notes on a specific channel at a given sample.
       *
       * @param sample  The sample index to stop notes.
       * @param channel The MIDI channel.
       */
      void allNotesOff(int sample, int channel) override;

      /**
       * @brief Turns off all notes within a range of channels.
       *
       * @param sample       The sample index at which to stop notes.
       * @param from_channel The first channel in the range.
       * @param to_channel   The last channel in the range (inclusive).
       */
      void allNotesOffRange(int sample, int from_channel, int to_channel);

      /**
       * @brief Handles a note-on event, triggering the engine’s voice handling.
       *
       * @param note     MIDI note number.
       * @param velocity Note-on velocity.
       * @param sample   The sample index where this note-on occurs.
       * @param channel  The MIDI channel.
       */
      void noteOn(int note, mono_float velocity, int sample, int channel) override;

      /**
       * @brief Handles a note-off event, releasing the voice allocated to the note.
       *
       * @param note    MIDI note number.
       * @param lift    Note-off velocity or lift value.
       * @param sample  The sample index where this note-off occurs.
       * @param channel The MIDI channel.
       */
      void noteOff(int note, mono_float lift, int sample, int channel) override;

      /**
       * @brief Sets the mod wheel value for a given MIDI channel.
       *
       * @param value   Mod wheel value in range [0..1].
       * @param channel The MIDI channel.
       */
      void setModWheel(mono_float value, int channel);

      /**
       * @brief Sets the mod wheel value for all MIDI channels.
       *
       * @param value The mod wheel value in range [0..1].
       */
      void setModWheelAllChannels(mono_float value);

      /**
       * @brief Sets the pitch wheel value for a specified MIDI channel.
       *
       * @param value   The pitch bend value in range [-1..1].
       * @param channel The MIDI channel.
       */
      void setPitchWheel(mono_float value, int channel);

      /**
       * @brief Applies a pitch wheel value to a range of MIDI channels.
       *
       * @param value        The pitch bend value in range [-1..1].
       * @param from_channel The first channel in the range.
       * @param to_channel   The last channel in the range (inclusive).
       */
      void setZonedPitchWheel(mono_float value, int from_channel, int to_channel);

      /**
       * @brief Disables unnecessary modulation sources in the engine to save CPU.
       */
      void disableUnnecessaryModSources();

      /**
       * @brief Enables a specific named modulation source by making its owner active.
       *
       * @param source The name of the modulation source.
       */
      void enableModSource(const std::string& source);

      /**
       * @brief Disables a specific named modulation source.
       *
       * @param source The name of the modulation source to disable.
       */
      void disableModSource(const std::string& source);

      /**
       * @brief Checks if a given named modulation source is currently enabled.
       *
       * @param source The name of the source to check.
       * @return True if enabled; otherwise false.
       */
      bool isModSourceEnabled(const std::string& source);

      /**
       * @brief Retrieves the internal equalizer memory (used for certain effect processing).
       *
       * @return Pointer to a StereoMemory buffer for the equalizer effect.
       */
      const StereoMemory* getEqualizerMemory();

      /**
       * @brief Updates the engine’s tempo in BPM, converting to beats per second for internal usage.
       *
       * @param bpm The new tempo in beats per minute.
       */
      void setBpm(mono_float bpm);

      /**
       * @brief Sets polyphonic aftertouch for a specific note on a given channel.
       *
       * @param note    MIDI note number.
       * @param value   Aftertouch value in [0..1].
       * @param sample  The sample index.
       * @param channel The MIDI channel.
       */
      void setAftertouch(mono_float note, mono_float value, int sample, int channel);

      /**
       * @brief Sets channel-wide aftertouch on a given channel.
       *
       * @param channel The MIDI channel.
       * @param value   Aftertouch value in [0..1].
       * @param sample  The sample index.
       */
      void setChannelAftertouch(int channel, mono_float value, int sample);

      /**
       * @brief Applies aftertouch to all channels in a specified range.
       *
       * @param from_channel The first channel in the range.
       * @param to_channel   The last channel in the range (inclusive).
       * @param value        Aftertouch value in [0..1].
       * @param sample       The sample index.
       */
      void setChannelRangeAftertouch(int from_channel, int to_channel, mono_float value, int sample);

      /**
       * @brief Sets channel slide (e.g., an expression or glide parameter) on a specific channel.
       *
       * @param channel The MIDI channel.
       * @param value   The slide value in [0..1].
       * @param sample  The sample index.
       */
      void setChannelSlide(int channel, mono_float value, int sample);

      /**
       * @brief Applies channel slide to all channels in a specified range.
       *
       * @param from_channel The first channel in the range.
       * @param to_channel   The last channel in the range (inclusive).
       * @param value        The slide value in [0..1].
       * @param sample       The sample index.
       */
      void setChannelRangeSlide(int from_channel, int to_channel, mono_float value, int sample);

      /**
       * @brief Retrieves a pointer to a Wavetable by index (not implemented here).
       *
       * @param index The wavetable index.
       * @return Pointer to the Wavetable or nullptr if not implemented.
       */
      Wavetable* getWavetable(int index);

      /**
       * @brief Retrieves a pointer to a Sample object (not implemented here).
       *
       * @return Pointer to the Sample or nullptr if not implemented.
       */
      Sample* getSample();

      /**
       * @brief Retrieves a pointer to the LFO source line generator for the given index.
       *
       * @param index The LFO source index.
       * @return Pointer to the corresponding LineGenerator.
       */
      LineGenerator* getLfoSource(int index);

      /**
       * @brief Engages sustain on a single MIDI channel (holding all pressed notes).
       *
       * @param channel The MIDI channel to sustain.
       */
      void sustainOn(int channel);

      /**
       * @brief Disengages sustain on a single MIDI channel, releasing notes if their keys are up.
       *
       * @param sample  The sample index at which sustain is released.
       * @param channel The MIDI channel.
       */
      void sustainOff(int sample, int channel);

      /**
       * @brief Engages sostenuto on a single MIDI channel (holding only currently pressed notes).
       *
       * @param channel The MIDI channel to engage sostenuto.
       */
      void sostenutoOn(int channel);

      /**
       * @brief Disengages sostenuto on a single MIDI channel.
       *
       * @param sample  The sample index at which sostenuto is released.
       * @param channel The MIDI channel.
       */
      void sostenutoOff(int sample, int channel);

      /**
       * @brief Engages sustain on a range of MIDI channels.
       *
       * @param from_channel The first channel in the range.
       * @param to_channel   The last channel in the range (inclusive).
       */
      void sustainOnRange(int from_channel, int to_channel);

      /**
       * @brief Disengages sustain on a range of MIDI channels.
       *
       * @param sample       The sample index at which sustain is released.
       * @param from_channel The first channel in the range.
       * @param to_channel   The last channel in the range (inclusive).
       */
      void sustainOffRange(int sample, int from_channel, int to_channel);

      /**
       * @brief Engages sostenuto on a range of MIDI channels.
       *
       * @param from_channel The first channel in the range.
       * @param to_channel   The last channel in the range (inclusive).
       */
      void sostenutoOnRange(int from_channel, int to_channel);

      /**
       * @brief Disengages sostenuto on a range of MIDI channels.
       *
       * @param sample       The sample index at which sostenuto is released.
       * @param from_channel The first channel in the range.
       * @param to_channel   The last channel in the range (inclusive).
       */
      void sostenutoOffRange(int sample, int from_channel, int to_channel);

      /**
       * @brief Retrieves the current oversampling factor (e.g., 1, 2, 4).
       *
       * @return The current oversampling amount.
       */
      force_inline int getOversamplingAmount() const { return last_oversampling_amount_; }

      /**
       * @brief Checks if the oversampling setting or sample rate has changed, and reconfigures if needed.
       */
      void checkOversampling();

    private:
      /**
       * @brief Pointer to the EffectsModulationHandler that orchestrates modulation sources and voices.
       */
      EffectsModulationHandler* modulation_handler_;

      /**
       * @brief Pointer to an Upsampler that handles oversampling operations.
       */
      Upsampler* upsampler_;

      /**
       * @brief Pointer to a ReorderableEffectChain managing the user’s effect chain.
       */
      ReorderableEffectChain* effect_chain_;

      /**
       * @brief Tracks the last oversampling amount set, to detect changes.
       */
      int last_oversampling_amount_;

      /**
       * @brief Tracks the last known sample rate, to detect changes.
       */
      int last_sample_rate_;

      /**
       * @brief Base control for oversampling amount.
       */
      Value* oversampling_;

      /**
       * @brief Base control for beats-per-second (converted from BPM).
       */
      Value* bps_;

      /**
       * @brief Base control for legato mode switching (affects voice handling).
       */
      Value* legato_;

      /**
       * @brief Pointer to a PeakMeter for measuring output amplitude.
       */
      PeakMeter* peak_meter_;

      /**
       * @brief Tracks active modulation processors for quick enabling/disabling.
       */
      CircularQueue<Processor*> modulation_processors_;

      JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SoundEngine)
  };
} // namespace vital
