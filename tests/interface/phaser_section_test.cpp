/**
 * @file phaser_section_test.cpp
 * @brief Implements the PhaserSectionTest class, performing stress tests on the PhaserSection UI component.
 */

#include "phaser_section_test.h"
#include "phaser_section.h"

void PhaserSectionTest::runTest() {
    // Create and configure the synth engine required by the PhaserSection.
    vital::SoundEngine* engine = createSynthEngine();

    // Acquire a MessageManagerLock for safe UI operations.
    MessageManager::getInstance();
    std::unique_ptr<MessageManagerLock> lock = std::make_unique<MessageManagerLock>();

    // Create the PhaserSection component, passing in mono modulations from the engine.
    PhaserSection phaser_section("Phaser", engine->getMonoModulations());

    // Release the lock before running stress tests to allow UI processing.
    lock.reset();

    // Perform stress tests with random input on the phaser_section
    // to ensure it behaves correctly and robustly.
    runStressRandomTest(&phaser_section);

    // Clean up the synth engine once tests are complete.
    deleteSynthEngine();
}

// Instantiate a static test instance that registers and runs this test.
static PhaserSectionTest phaser_section_test;
