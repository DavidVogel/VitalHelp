/**
 * @file note_handler_test.h
 * @brief Declares the NoteHandlerTest class, which tests the note handling logic within the SoundEngine.
 */

#pragma once

#include "JuceHeader.h"

namespace vital {
    class SoundEngine;
} // namespace vital

/**
 * @class NoteHandlerTest
 * @brief A test class for verifying the correctness of note handling in the SoundEngine.
 *
 * This test checks that notes can be turned on and off, and that the engine's output behaves as expected:
 * producing sound when notes are active and remaining quiet when no notes are sounding. It also ensures
 * that the output remains finite and stable under various scenarios.
 */
class NoteHandlerTest : public UnitTest {
public:
    /**
     * @brief Constructs a new NoteHandlerTest with the specified test name.
     */
    NoteHandlerTest() : UnitTest("Note Handler") { }

    /**
     * @brief Processes one block of audio with the given engine and checks that the output is finite.
     * @param engine The SoundEngine to process and verify.
     */
    void processAndExpectFinite(vital::SoundEngine* engine);

    /**
     * @brief Processes one block of audio with the given engine and checks that the output is effectively quiet.
     * @param engine The SoundEngine to process and verify.
     */
    void processAndExpectQuiet(vital::SoundEngine* engine);

    /**
     * @brief Runs all note handler tests, verifying note on/off behavior and ensuring finite, stable output.
     */
    void runTest() override;
};
