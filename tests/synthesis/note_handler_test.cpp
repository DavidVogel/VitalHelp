/**
 * @file note_handler_test.cpp
 * @brief Implements the NoteHandlerTest class, verifying note handling functionality in the SoundEngine.
 */

#include "note_handler_test.h"
#include "sound_engine.h"

void NoteHandlerTest::processAndExpectFinite(vital::SoundEngine* engine) {
    // Process one block of audio and verify output is finite.
    engine->process(vital::kMaxBufferSize);

    vital::Output* output = engine->output();
    expect(vital::utils::isFinite(output->buffer, output->buffer_size),
           "Output buffer contains non-finite values.");
}

void NoteHandlerTest::processAndExpectQuiet(vital::SoundEngine* engine) {
    // Process one block of audio and verify output is quiet.
    engine->process(vital::kMaxBufferSize);

    vital::Output* output = engine->output();
    expect(vital::utils::rms(reinterpret_cast<float*>(output->buffer), output->buffer_size) < 0.001f,
           "Output buffer is not quiet.");
}

void NoteHandlerTest::runTest() {
    vital::SoundEngine engine;
    // Set a zero release time for immediate silence after note offs.
    engine.getControls()["env_1_release"]->set(0.0f);

    beginTest("No Notes");
    processAndExpectQuiet(&engine);

    beginTest("One Note On");
    engine.noteOn(60, 1.0f, 10, 0);
    processAndExpectFinite(&engine);
    processAndExpectFinite(&engine);

    beginTest("One Note Off");
    engine.noteOff(60, 20, 0, 0);
    processAndExpectFinite(&engine);
    processAndExpectQuiet(&engine);

    beginTest("Three Notes On");
    engine.noteOn(61, 1.0f, 10, 0);
    engine.noteOn(62, 1.0f, vital::kMaxBufferSize - 1, 0);
    engine.noteOn(63, 1.0f, vital::kMaxBufferSize - 1, 0);
    processAndExpectFinite(&engine);
    processAndExpectFinite(&engine);

    beginTest("Three Notes Off");
    engine.noteOff(61, 0, 0, 0);
    engine.noteOff(62, 0, 0, 0);
    engine.noteOff(63, 0, 0, 0);
    processAndExpectFinite(&engine);
    processAndExpectQuiet(&engine);

    beginTest("Four Notes On");
    engine.noteOn(61, 1.0f, 0, 0);
    engine.noteOn(62, 1.0f, 0, 0);
    engine.noteOn(63, 1.0f, 0, 0);
    engine.noteOn(64, 1.0f, 0, 0);
    processAndExpectFinite(&engine);
    processAndExpectFinite(&engine);

    beginTest("Four Notes Off");
    engine.noteOff(64, 0, 0, 0);
    engine.noteOff(61, 0, 0, 0);
    engine.noteOff(62, 0, 0, 0);
    engine.noteOff(63, 0, 0, 0);
    processAndExpectFinite(&engine);
    processAndExpectQuiet(&engine);
}

// Registers the test instance so it will be automatically discovered and run.
static NoteHandlerTest note_handler_test;
