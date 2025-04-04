#pragma once

#include "JuceHeader.h"
#include "common.h"

#include <string>
#include <map>

#if !defined(JUCE_AUDIO_DEVICES_H_INCLUDED)
// Stubs for JUCE classes if JUCE_AUDIO_DEVICES is not included.

class MidiInput { };

class MidiInputCallback {
public:
    virtual ~MidiInputCallback() { }
    virtual void handleIncomingMidiMessage(MidiInput *source, const MidiMessage &midi_message) { }
};

class MidiMessageCollector {
public:
    void reset(int sample_rate) { }
    void removeNextBlockOfMessages(MidiBuffer& buffer, int num_samples) { }
    void addMessageToQueue(const MidiMessage &midi_message) { }
};
#endif

class SynthBase;

namespace vital {
    class SoundEngine;
    struct ValueDetails;
}

/**
 * @brief The MidiManager class handles all incoming MIDI messages and directs them to the synthesizer engine.
 *
 * This class interfaces with both the JUCE MIDI subsystem and the Vital `SoundEngine`. It can:
 * - Process note on/off, pitch bend, mod wheel, aftertouch, and other MIDI control messages.
 * - Handle MPE (MIDI Polyphonic Expression) messages if enabled.
 * - Support MIDI learn functionality, allowing MIDI controls to be dynamically mapped to synth parameters.
 * - Relay MIDI-driven preset changes and gather MIDI input from external sources.
 */
class MidiManager : public MidiInputCallback {
public:
    /**
     * @brief A nested type defining a map from MIDI controls to a map of parameter names and their ValueDetails.
     *
     * The outer map key (int) corresponds to a MIDI control number, and the inner map relates parameter names (strings)
     * to pointers of their associated ValueDetails struct.
     */
    typedef std::map<int, std::map<std::string, const vital::ValueDetails*>> midi_map;

    /**
     * @brief Enum representing main categories of MIDI events (status bytes).
     */
    enum MidiMainType {
        kNoteOff = 0x80,         ///< MIDI Note Off event
        kNoteOn = 0x90,          ///< MIDI Note On event
        kAftertouch = 0xa0,      ///< MIDI Polyphonic Aftertouch
        kController = 0xb0,      ///< MIDI Control Change event
        kProgramChange = 0xc0,   ///< MIDI Program Change
        kChannelPressure = 0xd0, ///< MIDI Channel Pressure (monophonic aftertouch)
        kPitchWheel = 0xe0,      ///< MIDI Pitch Wheel event
    };

    /**
     * @brief Enum representing secondary MIDI controller values (MIDI CC numbers).
     */
    enum MidiSecondaryType {
        kBankSelect = 0x00,
        kModWheel = 0x01,
        kFolderSelect = 0x20,
        kSustainPedal = 0x40,
        kSostenutoPedal = 0x42,
        kSoftPedalOn = 0x43,
        kSlide = 0x4a,
        kLsbPressure = 0x66,
        kLsbSlide = 0x6a,
        kAllSoundsOff = 0x78,
        kAllControllersOff = 0x79,
        kAllNotesOff = 0x7b,
    };

    /**
     * @brief An interface for classes that listen to MIDI-driven parameter changes.
     *
     * Implement this listener if you need to respond to MIDI values changing synth parameters,
     * pitch wheel movements, mod wheel updates, or preset changes triggered via MIDI.
     */
    class Listener {
    public:
        virtual ~Listener() { }
        /**
         * @brief Called when a parameter value changes due to a MIDI control message.
         *
         * @param name The parameter name.
         * @param value The new parameter value (normalized or absolute depending on parameter).
         */
        virtual void valueChangedThroughMidi(const std::string& name, vital::mono_float value) = 0;

        /**
         * @brief Called when the pitch wheel changes.
         *
         * @param value The new pitch wheel value in a normalized range (-1.0 to 1.0).
         */
        virtual void pitchWheelMidiChanged(vital::mono_float value) = 0;

        /**
         * @brief Called when the mod wheel changes.
         *
         * @param value The new mod wheel value (0.0 to 1.0).
         */
        virtual void modWheelMidiChanged(vital::mono_float value) = 0;

        /**
         * @brief Called when a preset change is triggered via MIDI.
         *
         * @param preset The preset file that has been changed to.
         */
        virtual void presetChangedThroughMidi(File preset) = 0;
    };

    /**
     * @brief Constructs a MidiManager to handle MIDI events for a given SynthBase instance.
     *
     * @param synth A pointer to the SynthBase to receive MIDI events.
     * @param keyboard_state A pointer to a MidiKeyboardState for GUI keyboard handling.
     * @param gui_state A map of GUI-related state strings.
     * @param listener A listener for MIDI-driven parameter changes and preset loads, if any.
     */
    MidiManager(SynthBase* synth, MidiKeyboardState* keyboard_state,
                std::map<std::string, String>* gui_state, Listener* listener = nullptr);

    /**
     * @brief Destructor.
     */
    virtual ~MidiManager();

    /**
     * @brief Arms the MIDI learn functionality for a given parameter.
     *
     * The next MIDI control message will map that parameter to the corresponding MIDI control.
     *
     * @param name The parameter name to arm for MIDI learn.
     */
    void armMidiLearn(std::string name);

    /**
     * @brief Cancels the MIDI learn operation without assigning any mapping.
     */
    void cancelMidiLearn();

    /**
     * @brief Clears an existing MIDI mapping for a given parameter.
     *
     * @param name The parameter name whose MIDI mapping will be cleared.
     */
    void clearMidiLearn(const std::string& name);

    /**
     * @brief Handles a direct MIDI input value and applies MIDI learn if armed.
     *
     * If MIDI learn is armed, this maps the armed parameter to the given MIDI control.
     * Otherwise, it updates any parameters already mapped to that MIDI control.
     *
     * @param control The MIDI control number.
     * @param value The MIDI value, generally 0-127.
     */
    void midiInput(int control, vital::mono_float value);

    /**
     * @brief Processes a given MIDI message and forwards it to the synth engine.
     *
     * This handles note on/off, control changes, pitch bend, aftertouch, etc.
     *
     * @param midi_message The incoming MIDI message.
     * @param sample_position The sample frame position for timing (optional).
     */
    void processMidiMessage(const MidiMessage &midi_message, int sample_position = 0);

    /**
     * @brief Checks if a parameter is already mapped to a MIDI control.
     *
     * @param name The parameter name.
     * @return True if the parameter is MIDI-mapped, false otherwise.
     */
    bool isMidiMapped(const std::string& name) const;

    /**
     * @brief Sets the audio sample rate for MIDI message timing.
     *
     * @param sample_rate The sample rate in Hz.
     */
    void setSampleRate(double sample_rate);

    /**
     * @brief Removes the next block of MIDI messages, filling a MidiBuffer.
     *
     * This is typically called every audio block to retrieve pending MIDI events.
     *
     * @param buffer A reference to the MidiBuffer to fill.
     * @param num_samples The number of audio samples in the current buffer.
     */
    void removeNextBlockOfMessages(MidiBuffer& buffer, int num_samples);

    /**
     * @brief Replaces keyboard messages in the given MIDI buffer.
     *
     * This allows on-screen keyboard events to be integrated into the MIDI processing flow.
     *
     * @param buffer The MidiBuffer to process.
     * @param num_samples The number of samples in the current audio block.
     */
    void replaceKeyboardMessages(MidiBuffer& buffer, int num_samples);

    /**
     * @brief Processes an 'All Notes Off' MIDI message.
     *
     * @param midi_message The MIDI message triggering the event.
     * @param sample_position The timing sample position.
     * @param channel The MIDI channel (0-based) affected.
     */
    void processAllNotesOff(const MidiMessage& midi_message, int sample_position, int channel);

    /**
     * @brief Processes an 'All Sounds Off' MIDI message, turning off all voices immediately.
     */
    void processAllSoundsOff();

    /**
     * @brief Processes a sustain pedal MIDI message.
     *
     * @param midi_message The MIDI sustain message.
     * @param sample_position The timing sample position.
     * @param channel The MIDI channel affected.
     */
    void processSustain(const MidiMessage& midi_message, int sample_position, int channel);

    /**
     * @brief Processes a sostenuto pedal MIDI message.
     *
     * @param midi_message The MIDI sostenuto message.
     * @param sample_position The timing sample position.
     * @param channel The MIDI channel affected.
     */
    void processSostenuto(const MidiMessage& midi_message, int sample_position, int channel);

    /**
     * @brief Processes a pitch bend MIDI message.
     *
     * @param midi_message The pitch bend message.
     * @param sample_position The timing sample position.
     * @param channel The MIDI channel affected.
     */
    void processPitchBend(const MidiMessage& midi_message, int sample_position, int channel);

    /**
     * @brief Processes channel pressure (aftertouch) MIDI messages.
     *
     * @param midi_message The aftertouch message.
     * @param sample_position The timing sample position.
     * @param channel The MIDI channel affected.
     */
    void processPressure(const MidiMessage& midi_message, int sample_position, int channel);

    /**
     * @brief Processes slide messages (MPE CC events) translating them to appropriate modulation.
     *
     * @param midi_message The slide-related CC message.
     * @param sample_position The timing sample position.
     * @param channel The MIDI channel affected.
     */
    void processSlide(const MidiMessage& midi_message, int sample_position, int channel);

    /**
     * @brief Checks if the given channel is the MPE master channel for the lower zone.
     *
     * @param channel The 0-based MIDI channel.
     * @return True if it is the MPE lower zone master channel.
     */
    bool isMpeChannelMasterLowerZone(int channel);

    /**
     * @brief Checks if the given channel is the MPE master channel for the upper zone.
     *
     * @param channel The 0-based MIDI channel.
     * @return True if it is the MPE upper zone master channel.
     */
    bool isMpeChannelMasterUpperZone(int channel);

    /// @brief Returns the start channel index (0-based) of the MPE lower zone.
    force_inline int lowerZoneStartChannel() { return mpe_zone_layout_.getLowerZone().getFirstMemberChannel() - 1; }
    /// @brief Returns the start channel index (0-based) of the MPE upper zone.
    force_inline int upperZoneStartChannel() { return mpe_zone_layout_.getUpperZone().getLastMemberChannel() - 1; }
    /// @brief Returns the end channel index (0-based) of the MPE lower zone.
    force_inline int lowerZoneEndChannel() { return mpe_zone_layout_.getLowerZone().getLastMemberChannel() - 1; }
    /// @brief Returns the end channel index (0-based) of the MPE upper zone.
    force_inline int upperZoneEndChannel() { return mpe_zone_layout_.getUpperZone().getFirstMemberChannel() - 1; }
    /// @brief Returns the master channel index (0-based) of the MPE lower zone.
    force_inline int lowerMasterChannel() { return mpe_zone_layout_.getLowerZone().getMasterChannel() - 1; }
    /// @brief Returns the master channel index (0-based) of the MPE upper zone.
    force_inline int upperMasterChannel() { return mpe_zone_layout_.getUpperZone().getMasterChannel() - 1; }

    /**
     * @brief Enables or disables MPE mode for MIDI processing.
     *
     * @param enabled True to enable MPE, false to disable.
     */
    void setMpeEnabled(bool enabled) { mpe_enabled_ = enabled; }

    /**
     * @brief Returns the current MIDI learn map.
     *
     * @return The current midi_map structure.
     */
    midi_map getMidiLearnMap() { return midi_learn_map_; }

    /**
     * @brief Sets the current MIDI learn map, overriding existing mappings.
     *
     * @param midi_learn_map A midi_map of control-to-parameter assignments.
     */
    void setMidiLearnMap(const midi_map& midi_learn_map) { midi_learn_map_ = midi_learn_map; }

    /**
     * @brief Handles incoming MIDI messages from a MidiInput source (MidiInputCallback interface).
     *
     * Adds messages to the MidiMessageCollector queue for processing.
     *
     * @param source The MidiInput source that provided the message.
     * @param midi_message The incoming MIDI message.
     */
    void handleIncomingMidiMessage(MidiInput *source, const MidiMessage &midi_message) override;

    /**
     * @brief A callback message class that notifies listeners of preset changes through MIDI.
     *
     * When dispatched, it calls the listener's presetChangedThroughMidi method.
     */
    struct PresetLoadedCallback : public CallbackMessage {
        /**
         * @brief Constructs a PresetLoadedCallback.
         *
         * @param lis The listener to notify.
         * @param pre The preset file that was loaded.
         */
        PresetLoadedCallback(Listener* lis, File pre) : listener(lis), preset(std::move(pre)) { }

        /**
         * @brief The callback invoked when the message is delivered.
         */
        void messageCallback() override {
            if (listener)
                listener->presetChangedThroughMidi(preset);
        }

        Listener* listener; ///< The listener to notify.
        File preset;         ///< The preset file that was loaded.
    };

protected:
    /**
     * @brief Parses and processes an MPE-related MIDI message.
     *
     * @param message The MIDI message to process.
     */
    void readMpeMessage(const MidiMessage& message);

    SynthBase* synth_;                       ///< Associated synthesizer base instance.
    vital::SoundEngine* engine_;             ///< The sound engine for handling audio events.
    MidiKeyboardState* keyboard_state_;       ///< The keyboard state for on-screen keyboard integration.
    MidiMessageCollector midi_collector_;     ///< Collects incoming MIDI messages.
    std::map<std::string, String>* gui_state_;///< Holds GUI-related state info.
    Listener* listener_;                      ///< Listener for MIDI-driven parameter changes and preset loads.
    int current_bank_;                        ///< Tracks the current bank for preset selection.
    int current_folder_;                      ///< Tracks the current folder for preset selection.
    int current_preset_;                      ///< Tracks the current preset within a folder.

    const vital::ValueDetails* armed_value_;  ///< Parameter armed for MIDI learn.
    midi_map midi_learn_map_;                 ///< Mapping of MIDI controls to synth parameters.

    int msb_pressure_values_[vital::kNumMidiChannels]; ///< MSB values for channel pressure.
    int lsb_pressure_values_[vital::kNumMidiChannels]; ///< LSB values for channel pressure.
    int msb_slide_values_[vital::kNumMidiChannels];    ///< MSB values for slide control.
    int lsb_slide_values_[vital::kNumMidiChannels];    ///< LSB values for slide control.

    bool mpe_enabled_;                        ///< True if MPE mode is enabled.
    MPEZoneLayout mpe_zone_layout_;           ///< MPE zone layout object.
    MidiRPNDetector rpn_detector_;            ///< Detects and processes MIDI RPN/NRPN messages.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiManager)
};
