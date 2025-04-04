#pragma once

#include "JuceHeader.h"
#include "concurrentqueue/concurrentqueue.h"
#include "line_generator.h"
#include "synth_constants.h"
#include "synth_types.h"
#include "midi_manager.h"
#include "tuning.h"
#include "wavetable_creator.h"

#include <set>
#include <string>

namespace vital {
    class SoundEngine;
    struct Output;
    class StatusOutput;
    class StereoMemory;
    class Sample;
    class WaveFrame;
    class Wavetable;
}

class SynthGuiInterface;

/**
 * @brief A base class providing foundational functionality for the Vital synthesizer’s engine, UI integration,
 *        modulation system, MIDI mapping, and preset management.
 *
 * The SynthBase class:
 * - Integrates a SoundEngine for audio generation and modulation.
 * - Manages MIDI input through a MidiManager.
 * - Maintains a set of controls (parameters) and handles parameter changes from GUI, MIDI, or code.
 * - Supports loading/saving presets, wavetables, and other stateful elements.
 * - Provides a queue-based system for thread-safe modulation and parameter changes.
 * - Offers methods to render audio to disk, load tunings, and initiate default or user presets.
 *
 * The class can be integrated with a GUI through a SynthGuiInterface or used headlessly (see HeadlessSynth).
 * Subclasses must implement certain virtual methods to integrate with an audio processing environment.
 */
class SynthBase : public MidiManager::Listener {
public:
    /// Minimum and maximum note values considered for output window display or related processing.
    static constexpr float kOutputWindowMinNote = 16.0f;
    static constexpr float kOutputWindowMaxNote = 128.0f;

    /**
     * @brief Constructs a SynthBase, initializing the sound engine, MIDI manager, wavetables, and settings.
     */
    SynthBase();

    /**
     * @brief Destroys the SynthBase, cleaning up resources such as sound engine and memory buffers.
     */
    virtual ~SynthBase();

    /**
     * @brief Updates the value of a control parameter.
     *
     * @param name The name of the parameter.
     * @param value The new value.
     */
    void valueChanged(const std::string& name, vital::mono_float value);

    /**
     * @brief Handles parameter changes triggered through MIDI mappings.
     *
     * @param name The name of the parameter.
     * @param value The new parameter value.
     */
    void valueChangedThroughMidi(const std::string& name, vital::mono_float value) override;

    /**
     * @brief Called when the pitch wheel value changes via MIDI.
     *
     * @param value The new pitch wheel value.
     */
    void pitchWheelMidiChanged(vital::mono_float value) override;

    /**
     * @brief Called when the mod wheel value changes via MIDI.
     *
     * @param value The new mod wheel value.
     */
    void modWheelMidiChanged(vital::mono_float value) override;

    /**
     * @brief Called when the pitch wheel changes via the GUI or another external source.
     *
     * @param value The new pitch wheel value.
     */
    void pitchWheelGuiChanged(vital::mono_float value);

    /**
     * @brief Called when the mod wheel changes via the GUI or another external source.
     *
     * @param value The new mod wheel value.
     */
    void modWheelGuiChanged(vital::mono_float value);

    /**
     * @brief Called when a preset is changed through MIDI (e.g., program change messages).
     *
     * @param preset The new preset file.
     */
    void presetChangedThroughMidi(File preset) override;

    /**
     * @brief Handles external (non-GUI, non-MIDI) value changes to parameters.
     *
     * @param name The parameter name.
     * @param value The new value.
     */
    void valueChangedExternal(const std::string& name, vital::mono_float value);

    /**
     * @brief Handles internal value changes, updating the parameter and optionally notifying the host.
     *
     * @param name The parameter name.
     * @param value The new parameter value.
     */
    void valueChangedInternal(const std::string& name, vital::mono_float value);

    /**
     * @brief Connects a modulation source to a destination parameter.
     *
     * @param source The modulation source name.
     * @param destination The parameter name to modulate.
     * @return True if a new connection was created, false if it was already existing.
     */
    bool connectModulation(const std::string& source, const std::string& destination);

    /**
     * @brief Connects a modulation using a pre-existing ModulationConnection object.
     *
     * @param connection A pointer to the ModulationConnection object to be connected.
     */
    void connectModulation(vital::ModulationConnection* connection);

    /**
     * @brief Disconnects a modulation source from a destination parameter.
     *
     * @param source The modulation source name.
     * @param destination The parameter name.
     */
    void disconnectModulation(const std::string& source, const std::string& destination);

    /**
     * @brief Disconnects a modulation given a ModulationConnection object.
     *
     * @param connection The ModulationConnection to disconnect.
     */
    void disconnectModulation(vital::ModulationConnection* connection);

    /**
     * @brief Clears all modulation connections.
     */
    void clearModulations();

    /**
     * @brief Forces a modulation source to remain active even if not currently connected.
     *
     * @param source The modulation source.
     * @param force True to keep it active, false to allow disabling.
     */
    void forceShowModulation(const std::string& source, bool force);

    /**
     * @brief Checks if a modulation source is currently enabled.
     *
     * @param source The modulation source name.
     * @return True if enabled, false otherwise.
     */
    bool isModSourceEnabled(const std::string& source);

    /**
     * @brief Counts how many modulations target a given parameter.
     *
     * @param destination The parameter name.
     * @return The number of modulations.
     */
    int getNumModulations(const std::string& destination);

    /**
     * @brief Gets the index of a modulation connection given its source and destination.
     *
     * @param source The modulation source.
     * @param destination The parameter destination.
     * @return The index or -1 if not found.
     */
    int getConnectionIndex(const std::string& source, const std::string& destination);

    /**
     * @brief Gets all current modulation connections as a CircularQueue.
     *
     * @return A queue of pointers to ModulationConnection objects.
     */
    vital::CircularQueue<vital::ModulationConnection*> getModulationConnections() { return mod_connections_; }

    /**
     * @brief Returns all modulation connections from a particular source.
     *
     * @param source The modulation source.
     * @return A vector of ModulationConnection pointers.
     */
    std::vector<vital::ModulationConnection*> getSourceConnections(const std::string& source);

    /**
     * @brief Checks if a given modulation source has any active connections.
     *
     * @param source The modulation source.
     * @return True if the source is connected, false otherwise.
     */
    bool isSourceConnected(const std::string& source);

    /**
     * @brief Returns all modulation connections targeting a given destination parameter.
     *
     * @param destination The parameter name.
     * @return A vector of ModulationConnection pointers.
     */
    std::vector<vital::ModulationConnection*> getDestinationConnections(const std::string& destination);

    /**
     * @brief Retrieves a status output by name.
     *
     * @param name The status output name.
     * @return A pointer to the StatusOutput, or nullptr if not found.
     */
    const vital::StatusOutput* getStatusOutput(const std::string& name);

    /**
     * @brief Gets a wavetable object from the engine.
     *
     * @param index The oscillator index.
     * @return A pointer to the Wavetable.
     */
    vital::Wavetable* getWavetable(int index);

    /**
     * @brief Gets a WavetableCreator for a given oscillator index.
     *
     * @param index The oscillator index.
     * @return A pointer to the WavetableCreator.
     */
    WavetableCreator* getWavetableCreator(int index);

    /**
     * @brief Gets the Sample object used by the engine.
     *
     * @return A pointer to the Sample.
     */
    vital::Sample* getSample();

    /**
     * @brief Retrieves an LFO source by index.
     *
     * @param index The LFO index.
     * @return A pointer to the LFO’s LineGenerator object.
     */
    LineGenerator* getLfoSource(int index);

    /**
     * @brief Retrieves the current audio sample rate used by the engine.
     *
     * @return The sample rate in Hz.
     */
    int getSampleRate();

    /**
     * @brief Initializes the engine to a default state, clearing modulations and resetting parameters.
     */
    void initEngine();

    /**
     * @brief Loads a tuning file into the synthesizer’s tuning system.
     *
     * @param file The File representing the tuning.
     */
    void loadTuningFile(const File& file);

    /**
     * @brief Loads an "init" preset, resetting Vital to its default initial state.
     */
    void loadInitPreset();

    /**
     * @brief Attempts to load a preset from a file.
     *
     * @param preset The preset file.
     * @param error A string to store error messages if loading fails.
     * @return True if successful, false otherwise.
     */
    bool loadFromFile(File preset, std::string& error);

    /**
     * @brief Renders audio to a WAV file for a given duration and note sequence.
     *
     * @param file The output WAV file.
     * @param seconds The duration in seconds.
     * @param bpm The tempo in beats per minute.
     * @param notes A vector of MIDI notes to play.
     * @param render_images Whether to render oscilloscope images alongside.
     */
    void renderAudioToFile(File file, float seconds, float bpm, std::vector<int> notes, bool render_images);

    /**
     * @brief Renders audio for the purpose of resynthesis into a provided buffer.
     *
     * @param data Pointer to a float array to store samples.
     * @param samples The number of samples to render.
     * @param note The MIDI note to play for resynthesis.
     */
    void renderAudioForResynthesis(float* data, int samples, int note);

    /**
     * @brief Saves the current preset state to the specified file.
     *
     * @param preset The file to save to (the extension is enforced).
     * @return True if the save was successful, false otherwise.
     */
    bool saveToFile(File preset);

    /**
     * @brief Saves the current preset state to the active file (if any).
     *
     * @return True if successful, false otherwise.
     */
    bool saveToActiveFile();

    /**
     * @brief Clears the currently active preset file, meaning changes are not saved to a previously loaded file.
     */
    void clearActiveFile() { active_file_ = File(); }

    /**
     * @brief Gets the currently active preset file.
     *
     * @return The active File object.
     */
    File getActiveFile() { return active_file_; }

    /**
     * @brief Enables or disables MPE (MIDI Polyphonic Expression) mode.
     *
     * @param enabled True to enable MPE, false to disable.
     */
    void setMpeEnabled(bool enabled);

    /**
     * @brief Called when a parameter change gesture begins. Typically not implemented in this base class.
     *
     * @param name The parameter name.
     */
    virtual void beginChangeGesture(const std::string& name) { }

    /**
     * @brief Called when a parameter change gesture ends. Typically not implemented in this base class.
     *
     * @param name The parameter name.
     */
    virtual void endChangeGesture(const std::string& name) { }

    /**
     * @brief Called when a parameter changes to notify a potential host environment. Typically not implemented here.
     *
     * @param name The parameter name.
     * @param value The new parameter value.
     */
    virtual void setValueNotifyHost(const std::string& name, vital::mono_float value) { }

    /**
     * @brief Arms the given parameter name for MIDI learn, associating the next received MIDI control with it.
     *
     * @param name The parameter name.
     */
    void armMidiLearn(const std::string& name);

    /**
     * @brief Cancels a previously armed MIDI learn operation.
     */
    void cancelMidiLearn();

    /**
     * @brief Clears the MIDI mapping for a given parameter name.
     *
     * @param name The parameter to clear from MIDI mappings.
     */
    void clearMidiLearn(const std::string& name);

    /**
     * @brief Checks if a given parameter name is MIDI mapped.
     *
     * @param name The parameter name.
     * @return True if mapped, false otherwise.
     */
    bool isMidiMapped(const std::string& name);

    /**
     * @brief Sets the author metadata of the current preset.
     *
     * @param author The author's name.
     */
    void setAuthor(const String& author);

    /**
     * @brief Sets the comments or description metadata of the current preset.
     *
     * @param comments The comments string.
     */
    void setComments(const String& comments);

    /**
     * @brief Sets the style metadata of the current preset (e.g., bass, lead, etc.).
     *
     * @param style The style description string.
     */
    void setStyle(const String& style);

    /**
     * @brief Sets the preset name.
     *
     * @param preset_name The preset name as a String.
     */
    void setPresetName(const String& preset_name);

    /**
     * @brief Sets the name of a macro control by index.
     *
     * @param index The macro index (0-based).
     * @param macro_name The macro name as a String.
     */
    void setMacroName(int index, const String& macro_name);

    /**
     * @brief Gets the author of the current preset.
     *
     * @return The author’s name as a String.
     */
    String getAuthor();

    /**
     * @brief Gets the comments for the current preset.
     *
     * @return The comments string.
     */
    String getComments();

    /**
     * @brief Gets the style of the current preset.
     *
     * @return The style string.
     */
    String getStyle();

    /**
     * @brief Gets the current preset’s name.
     *
     * @return The preset name.
     */
    String getPresetName();

    /**
     * @brief Gets the name of a macro control by index.
     *
     * @param index The macro index (0-based).
     * @return The macro name.
     */
    String getMacroName(int index);

    /**
     * @brief Provides access to the controls map (parameter name to Parameter pointer).
     *
     * @return A reference to the control_map.
     */
    vital::control_map& getControls() { return controls_; }

    /**
     * @brief Returns a pointer to the SoundEngine used for audio and modulation processing.
     *
     * @return A pointer to the SoundEngine.
     */
    vital::SoundEngine* getEngine() { return engine_.get(); }

    /**
     * @brief Retrieves the keyboard state (for processing MIDI note events from a virtual keyboard).
     *
     * @return A pointer to the MidiKeyboardState.
     */
    MidiKeyboardState* getKeyboardState() { return keyboard_state_.get(); }

    /**
     * @brief Retrieves the oscilloscope memory for visualization of audio output waveforms.
     *
     * @return A pointer to an array of poly_float samples representing the oscilloscope memory.
     */
    const vital::poly_float* getOscilloscopeMemory() { return oscilloscope_memory_; }

    /**
     * @brief Retrieves stereo memory holding recent audio output samples for visualization.
     *
     * @return A pointer to the StereoMemory object.
     */
    const vital::StereoMemory* getAudioMemory() { return audio_memory_.get(); }

    /**
     * @brief Retrieves memory used for equalizer visualization, if available.
     *
     * @return A pointer to the StereoMemory for the equalizer, or nullptr if not available.
     */
    const vital::StereoMemory* getEqualizerMemory();

    /**
     * @brief Retrieves the ModulationConnectionBank managing all modulation connections.
     *
     * @return A reference to the ModulationConnectionBank.
     */
    vital::ModulationConnectionBank& getModulationBank();

    /**
     * @brief Notifies that oversampling settings have changed, reinitializing the engine if needed.
     */
    void notifyOversamplingChanged();

    /**
     * @brief Checks and updates oversampling settings. May be called if parameters affecting oversampling change.
     */
    void checkOversampling();

    /**
     * @brief Provides access to the synth’s internal CriticalSection for thread safety.
     *
     * Must be implemented by subclasses.
     *
     * @return A reference to the CriticalSection.
     */
    virtual const CriticalSection& getCriticalSection() = 0;

    /**
     * @brief Pauses or resumes audio processing.
     *
     * When paused, processing locks the critical section to avoid concurrency issues.
     *
     * @param pause True to pause, false to resume.
     */
    virtual void pauseProcessing(bool pause) = 0;

    /**
     * @brief Returns a pointer to the synth's Tuning object.
     *
     * @return A pointer to the Tuning.
     */
    Tuning* getTuning() { return &tuning_; }

    /**
     * @brief A callback message used when values change, allowing asynchronous updates to GUI or host.
     */
    struct ValueChangedCallback : public CallbackMessage {
        ValueChangedCallback(std::shared_ptr<SynthBase*> listener, std::string name, vital::mono_float val) :
                listener(listener), control_name(std::move(name)), value(val) { }

        void messageCallback() override;

        std::weak_ptr<SynthBase*> listener;
        std::string control_name;
        vital::mono_float value;
    };

protected:
    /**
     * @brief Creates a modulation_change structure for a given connection, preparing it for engine operations.
     *
     * @param connection A pointer to a ModulationConnection.
     * @return A modulation_change object describing the connection.
     */
    vital::modulation_change createModulationChange(vital::ModulationConnection* connection);

    /**
     * @brief Checks if a modulation_change is invalid (e.g., creates a loop).
     *
     * @param change The modulation_change object to check.
     * @return True if invalid, false otherwise.
     */
    bool isInvalidConnection(const vital::modulation_change& change);

    /**
     * @brief Retrieves the GUI interface if available, for updating controls or notifications.
     *
     * @return A pointer to a SynthGuiInterface, or nullptr if headless.
     */
    virtual SynthGuiInterface* getGuiInterface() = 0;

    /**
     * @brief Serializes the current synth state into a JSON object.
     *
     * @return A JSON object representing the synth’s state.
     */
    json saveToJson();

    /**
     * @brief Deserializes and applies the synth state from a JSON object.
     *
     * @param state The JSON object to load.
     * @return True if successful, false if incompatible.
     */
    bool loadFromJson(const json& state);

    /**
     * @brief Finds a ModulationConnection by source and destination names.
     *
     * @param source The modulation source name.
     * @param destination The parameter destination name.
     * @return A pointer to the ModulationConnection, or nullptr if not found.
     */
    vital::ModulationConnection* getConnection(const std::string& source, const std::string& destination);

    /**
     * @brief Attempts to dequeue a modulation_change for processing.
     *
     * @param change A reference to store the dequeued change.
     * @return True if a change was dequeued, false otherwise.
     */
    inline bool getNextModulationChange(vital::modulation_change& change) {
        return modulation_change_queue_.try_dequeue_non_interleaved(change);
    }

    /**
     * @brief Clears the modulation change queue, removing all pending changes.
     */
    inline void clearModulationQueue() {
        vital::modulation_change change;
        while (modulation_change_queue_.try_dequeue_non_interleaved(change))
            ;
    }

    /**
     * @brief Processes audio into the given buffer. May be overridden or extended by subclasses.
     *
     * @param buffer Pointer to the output AudioSampleBuffer.
     * @param channels Number of output channels.
     * @param samples Number of samples to process.
     * @param offset Sample offset in the output buffer.
     */
    void processAudio(AudioSampleBuffer* buffer, int channels, int samples, int offset);

    /**
     * @brief Processes audio with an input buffer, often used for sidechain or feedback loops.
     *
     * @param buffer The output buffer.
     * @param input_buffer A pointer to the input poly_float buffer.
     * @param channels Number of output channels.
     * @param samples Number of samples to process.
     * @param offset Sample offset in the output buffer.
     */
    void processAudioWithInput(AudioSampleBuffer* buffer, const vital::poly_float* input_buffer,
                               int channels, int samples, int offset);

    /**
     * @brief Writes audio data from the engine to the output buffer.
     *
     * @param buffer The output AudioSampleBuffer.
     * @param channels Number of output channels.
     * @param samples Number of samples to write.
     * @param offset The sample offset in the output buffer.
     */
    void writeAudio(AudioSampleBuffer* buffer, int channels, int samples, int offset);

    /**
     * @brief Processes MIDI messages from a MidiBuffer, applying them to the engine’s sound generation.
     *
     * @param buffer The MidiBuffer containing MIDI messages.
     * @param start_sample The starting sample to process MIDI for.
     * @param end_sample The ending sample (0 means process all).
     */
    void processMidi(MidiBuffer& buffer, int start_sample = 0, int end_sample = 0);

    /**
     * @brief Processes keyboard events from a MidiBuffer, integrating them with the MidiKeyboardState.
     *
     * @param buffer The MidiBuffer with keyboard messages.
     * @param num_samples The number of samples to process.
     */
    void processKeyboardEvents(MidiBuffer& buffer, int num_samples);

    /**
     * @brief Processes pending modulation changes and updates the engine accordingly.
     */
    void processModulationChanges();

    /**
     * @brief Updates the oscilloscope memory with the latest audio samples.
     *
     * @param samples Number of samples processed.
     * @param audio Pointer to the audio sample data.
     */
    void updateMemoryOutput(int samples, const vital::poly_float* audio);

    std::unique_ptr<vital::SoundEngine> engine_;
    std::unique_ptr<MidiManager> midi_manager_;
    std::unique_ptr<MidiKeyboardState> keyboard_state_;

    std::unique_ptr<WavetableCreator> wavetable_creators_[vital::kNumOscillators];
    std::shared_ptr<SynthBase*> self_reference_;

    File active_file_;
    vital::poly_float oscilloscope_memory_[2 * vital::kOscilloscopeMemoryResolution];
    vital::poly_float oscilloscope_memory_write_[2 * vital::kOscilloscopeMemoryResolution];
    std::unique_ptr<vital::StereoMemory> audio_memory_;
    vital::mono_float last_played_note_;
    int last_num_pressed_;
    vital::mono_float memory_reset_period_;
    vital::mono_float memory_input_offset_;
    int memory_index_;
    bool expired_;

    std::map<std::string, String> save_info_;
    vital::control_map controls_;
    vital::CircularQueue<vital::ModulationConnection*> mod_connections_;
    moodycamel::ConcurrentQueue<vital::control_change> value_change_queue_;
    moodycamel::ConcurrentQueue<vital::modulation_change> modulation_change_queue_;
    Tuning tuning_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SynthBase)
};

/**
 * @brief A headless subclass of SynthBase that provides a critical section and pausing capability.
 *
 * The HeadlessSynth can operate without a GUI, useful in environments like CLI tools,
 * render farms, or automated test rigs. It uses a CriticalSection to manage audio processing safely.
 */
class HeadlessSynth : public SynthBase {
public:
    const CriticalSection& getCriticalSection() override {
        return critical_section_;
    }

    void pauseProcessing(bool pause) override {
        if (pause)
            critical_section_.enter();
        else
            critical_section_.exit();
    }

protected:
    SynthGuiInterface* getGuiInterface() override { return nullptr; }

private:
    CriticalSection critical_section_;
};
