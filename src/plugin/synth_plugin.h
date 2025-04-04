/*
Summary:

SynthPlugin is the core plugin class handling parameter management, saving/loading state, preparing audio processing, and communicating with the host. It integrates the synth engine, GUI interface, and handles parameter automation through ValueBridge instances. It also ensures that parameter changes and preset loading are communicated effectively to the GUI and host.
 */

#pragma once

#include "JuceHeader.h"

#include "synth_base.h"
#include "value_bridge.h"

class ValueBridge;

/**
 * @class SynthPlugin
 * @brief An AudioProcessor implementation for a synthesizer plugin.
 *
 * SynthPlugin integrates a SynthBase for synthesis engine management and
 * JUCE's AudioProcessor for plugin-host communication. It manages parameters,
 * state loading/saving, and interaction with the GUI. SynthPlugin also
 * interacts with ValueBridge objects to synchronize parameter changes
 * between the synth engine and the host.
 */
class SynthPlugin : public SynthBase, public AudioProcessor, public ValueBridge::Listener {
  public:
    static constexpr int kSetProgramWaitMilliseconds = 500; ///< Wait time after setting a program.

    /**
     * @brief Constructs the SynthPlugin.
     *
     * Registers parameters, sets up ValueBridges, and initializes the bypass parameter.
     */
    SynthPlugin();

    /**
     * @brief Destructor, cleans up allocated resources.
     */
    virtual ~SynthPlugin();

    /**
     * @brief Returns the GUI interface for this plugin.
     * @return Pointer to SynthGuiInterface if the editor is active, otherwise nullptr.
     */
    SynthGuiInterface* getGuiInterface() override;

    /**
     * @brief Begins a parameter change gesture for the specified parameter.
     * @param name The parameter's name.
     */
    void beginChangeGesture(const std::string& name) override;

    /**
     * @brief Ends a parameter change gesture for the specified parameter.
     * @param name The parameter's name.
     */
    void endChangeGesture(const std::string& name) override;

    /**
     * @brief Sets a parameter value and notifies the host.
     * @param name The parameter's name.
     * @param value The new parameter value to set.
     */
    void setValueNotifyHost(const std::string& name, vital::mono_float value) override;

    /**
     * @brief Returns the CriticalSection for thread synchronization.
     * @return A reference to the callback lock CriticalSection.
     */
    const CriticalSection& getCriticalSection() override;

    /**
     * @brief Pauses or resumes audio processing.
     * @param pause True to suspend processing, false to resume.
     */
    void pauseProcessing(bool pause) override;

    /**
     * @brief Prepare the plugin to play with the given sample rate and buffer size.
     * @param sample_rate The audio sample rate.
     * @param buffer_size The buffer size in samples.
     */
    void prepareToPlay(double sample_rate, int buffer_size) override;

    /**
     * @brief Release any resources allocated for playback.
     */
    void releaseResources() override;

    /**
     * @brief Process audio and MIDI data each block.
     * @param buffer The audio buffer to process.
     * @param midi_messages The MIDI events to process.
     */
    void processBlock(AudioSampleBuffer&, MidiBuffer&) override;

    /**
     * @brief Creates and returns the AudioProcessorEditor for this plugin.
     * @return A pointer to the newly created editor component.
     */
    AudioProcessorEditor* createEditor() override;

    /**
     * @brief Checks if the plugin has an editor.
     * @return True, since this plugin provides a custom editor.
     */
    bool hasEditor() const override;

    /**
     * @brief Returns the name of the plugin.
     * @return The plugin's name as a JUCE String.
     */
    const String getName() const override;

    /**
     * @brief Returns whether the plugin supports MPE (MIDI Polyphonic Expression).
     * @return True if MPE is supported.
     */
    bool supportsMPE() const override { return true; }

    /**
     * @brief Returns the input channel name.
     * @param channel_index The zero-based index of the input channel.
     */
    const String getInputChannelName(int channel_index) const override;

    /**
     * @brief Returns the output channel name.
     * @param channel_index The zero-based index of the output channel.
     */
    const String getOutputChannelName(int channel_index) const override;

    /**
     * @brief Checks if the given input channel forms a stereo pair.
     * @param index The input channel index.
     * @return True if it forms a stereo pair.
     */
    bool isInputChannelStereoPair(int index) const override;

    /**
     * @brief Checks if the given output channel forms a stereo pair.
     * @param index The output channel index.
     * @return True if it forms a stereo pair.
     */
    bool isOutputChannelStereoPair(int index) const override;

    /**
     * @brief Checks if the plugin accepts MIDI input.
     * @return True if it accepts MIDI input.
     */
    bool acceptsMidi() const override;

    /**
     * @brief Checks if the plugin produces MIDI output.
     * @return True if it produces MIDI output.
     */
    bool producesMidi() const override;

    /**
     * @brief Checks if silence in leads to silence out.
     * @return False for this plugin.
     */
    bool silenceInProducesSilenceOut() const override;

    /**
     * @brief Gets the plugin's tail length in seconds.
     * @return Tail length in seconds.
     */
    double getTailLengthSeconds() const override;

    /**
     * @brief Returns the number of programs (only one here).
     */
    int getNumPrograms() override { return 1; }

    /**
     * @brief Returns the current program index (always 0 here).
     */
    int getCurrentProgram() override { return 0; }

    /**
     * @brief Sets the current program (does nothing here).
     * @param index The program index to set.
     */
    void setCurrentProgram(int index) override { }

    /**
     * @brief Gets the name of a program.
     * @param index The program index.
     * @return The program name.
     */
    const String getProgramName(int index) override;

    /**
     * @brief Changes the name of a program (not implemented).
     * @param index The program index.
     * @param new_name The new program name.
     */
    void changeProgramName(int index, const String& new_name) override { }

    /**
     * @brief Saves the plugin state to a memory block.
     * @param destData The memory block to write state data.
     */
    void getStateInformation(MemoryBlock& destData) override;

    /**
     * @brief Restores the plugin state from a memory block.
     * @param data Pointer to the state data.
     * @param size_in_bytes Size of the state data in bytes.
     */
    void setStateInformation(const void* data, int size_in_bytes) override;

    /**
     * @brief Returns the bypass parameter for hosts that support bypass.
     * @return A pointer to the bypass parameter.
     */
    AudioProcessorParameter* getBypassParameter() const override { return bypass_parameter_; }

    /**
     * @brief Called when a parameter changes externally via a ValueBridge.
     * @param name The name of the changed parameter.
     * @param value The new parameter value.
     */
    void parameterChanged(std::string name, vital::mono_float value) override;

  private:
    ValueBridge* bypass_parameter_;      ///< Pointer to the bypass parameter bridge.
    double last_seconds_time_;           ///< Tracks the last processed time in seconds.
    AudioPlayHead::CurrentPositionInfo position_info_; ///< Stores current host position information.

    std::map<std::string, ValueBridge*> bridge_lookup_; ///< Lookup for parameter name to ValueBridge objects.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SynthPlugin)
};

