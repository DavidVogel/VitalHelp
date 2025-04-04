#include "voice_section_test.h"
#include "voice_section.h"

void VoiceSectionTest::runTest() {
    // Initializes the synth engine environment for testing.
    createSynthEngine();

    // Acquires a MessageManager lock required for certain UI-related operations.
    MessageManager::getInstance();
    std::unique_ptr<MessageManagerLock> lock = std::make_unique<MessageManagerLock>();

    // Creates a VoiceSection UI component named "Voice".
    VoiceSection voice_section("Voice");

    // Releases the lock before performing stress tests.
    lock.reset();

    // Executes random stress tests on the voice section to ensure stability and correctness.
    runStressRandomTest(&voice_section);

    // Cleans up the synth engine after tests complete.
    deleteSynthEngine();
}

// Registers and runs the VoiceSectionTest as part of the test suite.
static VoiceSectionTest voice_section_test;
