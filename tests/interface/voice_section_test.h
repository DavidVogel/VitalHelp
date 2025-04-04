#pragma once

#include "interface_test.h"

/**
 * @brief A test class for verifying the functionality of the VoiceSection UI component.
 *
 * The VoiceSectionTest class inherits from InterfaceTest and implements tests specifically
 * for the VoiceSection UI element within the Vital synthesizer. It confirms that the
 * VoiceSection behaves as expected under various conditions and random stress tests.
 */
class VoiceSectionTest : public InterfaceTest {
public:
    /**
     * @brief Constructs a VoiceSectionTest.
     *
     * Sets the test name to "Voice Section".
     */
    VoiceSectionTest() : InterfaceTest("Voice Section") { }

    /**
     * @brief Runs the voice section test.
     *
     * Creates a synth engine, acquires a MessageManagerLock, and executes a series of stress
     * tests against the VoiceSection UI component. Then, it releases the lock and cleans up.
     */
    void runTest() override;
};
