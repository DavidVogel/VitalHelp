/**
 * @file engine_launch_test.cpp
 * @brief Implements the EngineLaunchTest class, performing stress tests on the SoundEngine initialization and usage.
 */

#include "engine_launch_test.h"
#include "sound_engine.h"

namespace {
    /// Number of SoundEngine instances to create and test.
    constexpr int kNumRuns = 10;
}

void EngineLaunchTest::launchTest() {
    beginTest("Launch Test");

    // Create multiple SoundEngine instances in an array to test repeated launches.
    vital::SoundEngine engines[kNumRuns];

    for (int i = 0; i < kNumRuns; ++i) {
        vital::SoundEngine& engine = engines[i];

        // Test basic operations: stopping notes, checking output validity.
        engine.allNotesOff(0);
        expect(vital::utils::isFinite(engine.output()->buffer[0]), "Output is not finite after allNotesOff.");

        // Trigger notes and process audio, verifying finite output.
        engine.noteOn(60, 1.0f, 0, 0);
        engine.process(vital::kMaxBufferSize);
        engine.noteOn(62, 1.0f, 0, 0);
        engine.process(vital::kMaxBufferSize);
        engine.noteOn(64, 1.0f, 0, 0);
        expect(vital::utils::isFinite(engine.output()->buffer[0]), "Output is not finite after multiple note-ons.");

        // More notes and processing.
        engine.noteOn(65, 1.0f, 0, 0);
        engine.process(vital::kMaxBufferSize);
        engine.process(vital::kMaxBufferSize);
        expect(vital::utils::isFinite(engine.output()->buffer[0]), "Output is not finite after extended processing.");

        // Release notes and process again.
        engine.noteOff(64, 1.0f, 0, 0);
        engine.noteOff(65, 1.0f, 0, 0);
        engine.noteOff(62, 1.0f, 0, 0);
        engine.noteOff(60, 1.0f, 0, 0);
        engine.process(vital::kMaxBufferSize);
        expect(vital::utils::isFinite(engine.output()->buffer[0]), "Output is not finite after note-offs.");

        // Additional processing to ensure no issues after notes are released.
        engine.process(vital::kMaxBufferSize);
        engine.process(vital::kMaxBufferSize);
        engine.process(vital::kMaxBufferSize);
        engine.process(vital::kMaxBufferSize);
        engine.process(vital::kMaxBufferSize);
        expect(vital::utils::isFinite(engine.output()->buffer[0]), "Output became non-finite after long processing.");
    }
}

void EngineLaunchTest::runTest() {
    // Execute the launch test routine.
    launchTest();
}

// Registers the test so that it runs automatically.
static EngineLaunchTest engine_launch_test;
