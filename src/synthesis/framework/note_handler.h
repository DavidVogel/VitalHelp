#pragma once

#include "common.h"

namespace vital {

    /**
     * @class NoteHandler
     * @brief An interface for handling MIDI note events within a synthesizer or audio system.
     *
     * The NoteHandler interface provides methods to respond to note-on and note-off events from MIDI,
     * as well as commands to release all active notes and silence all sounds. Classes implementing this
     * interface can manage voice allocation, envelope triggering, and other note-related behaviors
     * in response to incoming MIDI data.
     */
    class NoteHandler {
    public:
        /**
         * @brief Virtual destructor.
         */
        virtual ~NoteHandler() { }

        /**
         * @brief Immediately turns off all sounding notes and stops all sound production.
         */
        virtual void allSoundsOff() = 0;

        /**
         * @brief Turns off all currently active notes, optionally specifying a sample index for timing.
         *
         * @param sample The sample index at which to turn off notes.
         */
        virtual void allNotesOff(int sample) = 0;

        /**
         * @brief Turns off all currently active notes on a specific MIDI channel, optionally specifying a sample index.
         *
         * @param sample The sample index at which to turn off notes.
         * @param channel The MIDI channel for which to turn off notes.
         */
        virtual void allNotesOff(int sample, int channel) = 0;

        /**
         * @brief Handles a MIDI note-on event, starting a note with a specified velocity and timing.
         *
         * @param note The MIDI note number (0-127).
         * @param velocity The velocity of the note-on event (0.0 to 1.0).
         * @param sample The sample index at which the note-on occurs.
         * @param channel The MIDI channel on which the note-on occurred.
         */
        virtual void noteOn(int note, mono_float velocity, int sample, int channel) = 0;

        /**
         * @brief Handles a MIDI note-off event, releasing a currently active note.
         *
         * @param note The MIDI note number (0-127).
         * @param lift The release velocity (0.0 to 1.0).
         * @param sample The sample index at which the note-off occurs.
         * @param channel The MIDI channel on which the note-off occurred.
         */
        virtual void noteOff(int note, mono_float lift, int sample, int channel) = 0;
    };
} // namespace vital
