#pragma once

#include "circular_queue.h"
#include "synth_module.h"
#include "note_handler.h"

class LineGenerator;
class Tuning;

namespace vital {
    class Decimator;
    class PeakMeter;
    class Sample;
    class ReorderableEffectChain;
    class StereoMemory;
    class SynthVoiceHandler;
    class SynthLfo;
    class Value;
    class ValueSwitch;
    class WaveFrame;
    class Wavetable;

    /**
     * @class SoundEngine
     * @brief The main synthesis engine that handles voices, modulation, effects, and other
     *        top-level audio processing components for the Vital synthesizer.
     *
     * The SoundEngine is responsible for managing voices, applying modulation,
     * handling note on/off events, applying oversampling, and routing audio through
     * the effects chain. It integrates with a SynthVoiceHandler to manage polyphony,
     * and it uses a ReorderableEffectChain to process the final audio output.
     */
    class SoundEngine : public SynthModule, public NoteHandler {
    public:
        /**
         * @brief The default oversampling amount for the engine.
         */
        static constexpr int kDefaultOversamplingAmount = 2;

        /**
         * @brief The default sample rate for the engine.
         */
        static constexpr int kDefaultSampleRate = 44100;

        /**
         * @brief Constructs a new SoundEngine instance.
         */
        SoundEngine();

        /**
         * @brief Destroys the SoundEngine instance.
         */
        virtual ~SoundEngine();

        /**
         * @brief Initializes the SoundEngine, setting up controls, voices, and effects.
         *
         * Called internally upon construction to set up default parameters
         * and processing modules.
         */
        void init() override;

        /**
         * @brief Processes a block of samples through the voice handler and effects chain.
         * @param num_samples The number of samples to process.
         */
        void process(int num_samples) override;

        /**
         * @brief Corrects internal timing to a given absolute time in seconds.
         * @param seconds The time in seconds to correct to.
         */
        void correctToTime(double seconds) override;

        /**
         * @brief Gets the number of currently pressed notes.
         * @return The number of pressed notes.
         */
        int getNumPressedNotes();

        /**
         * @brief Connects a modulation source to its destination.
         * @param change The modulation change descriptor.
         */
        void connectModulation(const modulation_change& change);

        /**
         * @brief Disconnects a previously connected modulation source from its destination.
         * @param change The modulation change descriptor.
         */
        void disconnectModulation(const modulation_change& change);

        /**
         * @brief Gets the number of currently active voices.
         * @return The number of active voices.
         */
        int getNumActiveVoices();

        /**
         * @brief Gets the modulation connection bank for this engine.
         * @return A reference to the ModulationConnectionBank.
         */
        ModulationConnectionBank& getModulationBank();

        /**
         * @brief Gets the last active note played.
         * @return The last active note as a floating-point value.
         */
        mono_float getLastActiveNote() const;

        /**
         * @brief Sets the tuning to use for pitch calculations.
         * @param tuning A pointer to the Tuning object to apply.
         */
        void setTuning(const Tuning* tuning);

        /**
         * @brief Turns off all sounds immediately.
         */
        void allSoundsOff() override;

        /**
         * @brief Sends all voices a note-off command at a given sample.
         * @param sample The sample index to apply the note-off command.
         */
        void allNotesOff(int sample) override;

        /**
         * @brief Sends all voices on a given channel a note-off command.
         * @param sample The sample index at which to apply the note-off command.
         * @param channel The channel from which to remove active notes.
         */
        void allNotesOff(int sample, int channel) override;

        /**
         * @brief Sends all voices in a range of channels a note-off command.
         * @param sample The sample index at which to apply the note-offs.
         * @param from_channel The first channel in the range.
         * @param to_channel The last channel in the range.
         */
        void allNotesOffRange(int sample, int from_channel, int to_channel);

        /**
         * @brief Triggers a note-on event.
         * @param note The MIDI note number.
         * @param velocity The note velocity.
         * @param sample The sample index at which the note-on occurs.
         * @param channel The channel on which the note is triggered.
         */
        void noteOn(int note, mono_float velocity, int sample, int channel) override;

        /**
         * @brief Triggers a note-off event.
         * @param note The MIDI note number.
         * @param lift The note release velocity or lift value.
         * @param sample The sample index at which the note-off occurs.
         * @param channel The channel on which the note is being released.
         */
        void noteOff(int note, mono_float lift, int sample, int channel) override;

        /**
         * @brief Sets the modulation wheel value for a given channel.
         * @param value The modulation wheel value.
         * @param channel The channel to set the mod wheel on.
         */
        void setModWheel(mono_float value, int channel);

        /**
         * @brief Sets the modulation wheel value for all channels.
         * @param value The modulation wheel value.
         */
        void setModWheelAllChannels(mono_float value);

        /**
         * @brief Sets the pitch wheel value for a given channel.
         * @param value The pitch wheel value.
         * @param channel The channel to set the pitch wheel on.
         */
        void setPitchWheel(mono_float value, int channel);

        /**
         * @brief Sets the pitch wheel value for a range of channels.
         * @param value The pitch wheel value.
         * @param from_channel The first channel in the range.
         * @param to_channel The last channel in the range.
         */
        void setZonedPitchWheel(mono_float value, int from_channel, int to_channel);

        /**
         * @brief Disables any modulation sources that are not required.
         */
        void disableUnnecessaryModSources();

        /**
         * @brief Enables a modulation source by name.
         * @param source The name of the modulation source to enable.
         */
        void enableModSource(const std::string& source);

        /**
         * @brief Disables a modulation source by name.
         * @param source The name of the modulation source to disable.
         */
        void disableModSource(const std::string& source);

        /**
         * @brief Checks if a modulation source is enabled.
         * @param source The name of the modulation source.
         * @return True if the source is enabled, false otherwise.
         */
        bool isModSourceEnabled(const std::string& source);

        /**
         * @brief Gets a pointer to the stereo memory used by the equalizer.
         * @return A pointer to the StereoMemory object.
         */
        const StereoMemory* getEqualizerMemory();

        /**
         * @brief Sets the BPM (beats per minute) value for the engine.
         * @param bpm The new BPM value.
         */
        void setBpm(mono_float bpm);

        /**
         * @brief Sets note-based aftertouch values.
         * @param note The MIDI note number.
         * @param value The aftertouch value.
         * @param sample The sample index at which this occurs.
         * @param channel The channel on which the aftertouch is applied.
         */
        void setAftertouch(mono_float note, mono_float value, int sample, int channel);

        /**
         * @brief Sets aftertouch values for an entire channel.
         * @param channel The channel to set aftertouch on.
         * @param value The aftertouch value.
         * @param sample The sample index at which this occurs.
         */
        void setChannelAftertouch(int channel, mono_float value, int sample);

        /**
         * @brief Sets aftertouch values for a range of channels.
         * @param from_channel The first channel in the range.
         * @param to_channel The last channel in the range.
         * @param value The aftertouch value.
         * @param sample The sample index at which this occurs.
         */
        void setChannelRangeAftertouch(int from_channel, int to_channel, mono_float value, int sample);

        /**
         * @brief Sets channel-based slide (MPE/MIDI CC glides).
         * @param channel The channel on which to set the slide value.
         * @param value The slide value.
         * @param sample The sample index at which this occurs.
         */
        void setChannelSlide(int channel, mono_float value, int sample);

        /**
         * @brief Sets slide values for a range of channels.
         * @param from_channel The first channel in the range.
         * @param to_channel The last channel in the range.
         * @param value The slide value.
         * @param sample The sample index at which this occurs.
         */
        void setChannelRangeSlide(int from_channel, int to_channel, mono_float value, int sample);

        /**
         * @brief Gets a pointer to a wavetable by index.
         * @param index The index of the wavetable.
         * @return A pointer to the Wavetable object.
         */
        Wavetable* getWavetable(int index);

        /**
         * @brief Gets a pointer to the Sample object.
         * @return A pointer to the Sample object used by the voice handler.
         */
        Sample* getSample();

        /**
         * @brief Gets a pointer to an LFO source by index.
         * @param index The index of the LFO source.
         * @return A pointer to the LineGenerator (LFO source).
         */
        LineGenerator* getLfoSource(int index);

        /**
         * @brief Turns sustain on for a given channel.
         * @param channel The channel to sustain.
         */
        void sustainOn(int channel);

        /**
         * @brief Turns sustain off for a given channel at a specific sample.
         * @param sample The sample index at which to turn sustain off.
         * @param channel The channel on which to turn sustain off.
         */
        void sustainOff(int sample, int channel);

        /**
         * @brief Turns sostenuto on for a given channel.
         * @param channel The channel to apply sostenuto.
         */
        void sostenutoOn(int channel);

        /**
         * @brief Turns sostenuto off for a given channel at a specific sample.
         * @param sample The sample index at which to turn sostenuto off.
         * @param channel The channel on which to turn sostenuto off.
         */
        void sostenutoOff(int sample, int channel);

        /**
         * @brief Turns sustain on for a range of channels.
         * @param from_channel The first channel in the range.
         * @param to_channel The last channel in the range.
         */
        void sustainOnRange(int from_channel, int to_channel);

        /**
         * @brief Turns sustain off for a range of channels at a given sample.
         * @param sample The sample index at which to turn sustain off.
         * @param from_channel The first channel in the range.
         * @param to_channel The last channel in the range.
         */
        void sustainOffRange(int sample, int from_channel, int to_channel);

        /**
         * @brief Turns sostenuto on for a range of channels.
         * @param from_channel The first channel in the range.
         * @param to_channel The last channel in the range.
         */
        void sostenutoOnRange(int from_channel, int to_channel);

        /**
         * @brief Turns sostenuto off for a range of channels at a given sample.
         * @param sample The sample index at which to turn sostenuto off.
         * @param from_channel The first channel in the range.
         * @param to_channel The last channel in the range.
         */
        void sostenutoOffRange(int sample, int from_channel, int to_channel);

        /**
         * @brief Gets the current oversampling amount.
         * @return The current oversampling amount.
         */
        force_inline int getOversamplingAmount() const { return last_oversampling_amount_; }

        /**
         * @brief Checks if the oversampling settings have changed and updates accordingly.
         */
        void checkOversampling();

    private:
        /**
         * @brief Sets the oversampling amount and adjusts the sample rate accordingly.
         * @param oversampling_amount The new oversampling amount.
         * @param sample_rate The current sample rate.
         */
        void setOversamplingAmount(int oversampling_amount, int sample_rate);

        SynthVoiceHandler* voice_handler_;             ///< Manages the synthesis voices.
        ReorderableEffectChain* effect_chain_;         ///< Handles the chain of audio effects.
        Add* output_total_;                            ///< Combines effect and voice output.

        int last_oversampling_amount_;                 ///< The last applied oversampling amount.
        int last_sample_rate_;                         ///< The last known sample rate.
        Value* oversampling_;                          ///< Oversampling parameter Value.
        Value* bps_;                                   ///< Beats per second parameter.
        Value* legato_;                                ///< Legato parameter Value.
        Decimator* decimator_;                         ///< A decimator for final audio output.
        PeakMeter* peak_meter_;                        ///< Measures peak levels of the output.

        CircularQueue<Processor*> modulation_processors_; ///< Queue of active modulation processors.

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SoundEngine)
    };
} // namespace vital
