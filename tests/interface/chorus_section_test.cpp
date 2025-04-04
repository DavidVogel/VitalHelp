/**
 * @file chorus_section_test.cpp
 * @brief Implements the ChorusSectionTest class, running stress tests on the ChorusSection UI component.
 */

#include "chorus_section_test.h"
#include "chorus_section.h"

void ChorusSectionTest::runTest() {
    // Create a synth engine to be used by the ChorusSection.
    vital::SoundEngine* engine = createSynthEngine();

    // Acquire a MessageManagerLock to safely create and manipulate UI components.
    MessageManager::getInstance();
    std::unique_ptr<MessageManagerLock> lock = std::make_unique<MessageManagerLock>();

    // Create the ChorusSection component, passing the engine's mono modulations.
    ChorusSection chorus_section("Chorus", engine->getMonoModulations());

    // Release the lock before running tests, allowing other threads to process messages.
    lock.reset();

    // Run stress tests that provide random input to the chorus_section,
    // ensuring it behaves correctly under various conditions.
    runStressRandomTest(&chorus_section);

    // Clean up the engine after tests complete.
    deleteSynthEngine();
}

// Instantiate a static test instance for the automated test framework.
static ChorusSectionTest chorus_section_test;
