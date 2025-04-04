#pragma once

#include "JuceHeader.h"
#include "synth_types.h"
#include <set>

namespace vital {
    class SoundEngine;
}

/**
 * @class SynthComputerKeyboard
 * @brief Provides a computer-keyboard-based MIDI input mechanism for the Vital standalone application.
 *
 * SynthComputerKeyboard maps certain keys on a computer keyboard to MIDI note on/off events, allowing users
 * to play notes without a hardware MIDI controller. It also supports shifting the keyboard layout
 * up and down octaves via dedicated keys.
 *
 * This class implements JUCE's KeyListener interface to receive and respond to keyboard events.
 */
class SynthComputerKeyboard : public vital::StringLayout, public KeyListener {
public:
    /**
     * @brief The MIDI channel used for the computer keyboard input.
     */
    static constexpr int kKeyboardMidiChannel = 1;

    /**
     * @brief Deleted default constructor to ensure that a SoundEngine and MidiKeyboardState are provided.
     */
    SynthComputerKeyboard() = delete;

    /**
     * @brief Constructs a SynthComputerKeyboard tied to a given Vital SoundEngine and MidiKeyboardState.
     *
     * @param synth A pointer to the Vital::SoundEngine, used to handle corrections in time and audio state.
     * @param keyboard_state A pointer to a MidiKeyboardState that tracks the active note on/off states.
     */
    SynthComputerKeyboard(vital::SoundEngine* synth, MidiKeyboardState* keyboard_state);

    /**
     * @brief Destructor. Cleans up any resources.
     */
    ~SynthComputerKeyboard();

    /**
     * @brief Changes the base offset of the computer keyboard notes.
     *
     * For example, shifting the keyboard up or down one octave changes which MIDI notes the keys trigger.
     *
     * @param new_offset The new keyboard offset in semitones (MIDI note numbers).
     */
    void changeKeyboardOffset(int new_offset);

    /**
     * @brief Called when a key is pressed.
     *
     * @param key The KeyPress representing the pressed key.
     * @param origin The Component that originated the event.
     * @return True if the event was handled; false otherwise.
     */
    bool keyPressed(const KeyPress &key, Component *origin) override;

    /**
     * @brief Called when a key state changes (pressed or released).
     *
     * This function detects when keys mapped to notes are pressed or released, and sends note-on or
     * note-off messages to the synthesizer accordingly.
     *
     * @param isKeyDown True if a key is currently pressed; false if released.
     * @param origin The Component that originated the event.
     * @return True if the event was handled; false otherwise.
     */
    bool keyStateChanged(bool isKeyDown, Component *origin) override;

private:
    vital::SoundEngine* synth_;           ///< Pointer to the Vital sound engine.
    MidiKeyboardState* keyboard_state_;   ///< Pointer to the MidiKeyboardState for tracking note states.
    std::set<char> keys_pressed_;         ///< A set of keys currently pressed.
    int computer_keyboard_offset_;         ///< The current offset (in MIDI notes) for the computer keyboard.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SynthComputerKeyboard)
};
