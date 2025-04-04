/**
 * @file modulation_stress_test.h
 * @brief Declares the ModulationStressTest class, which conducts stress tests on modulations within the SoundEngine.
 */

#pragma once

#include "JuceHeader.h"
#include "processor.h"
#include "sound_engine.h"

/**
 * @class ModulationStressTest
 * @brief A stress test class that connects a large number of modulations within the SoundEngine
 * and ensures that the engine remains stable, producing finite (non-NaN and non-infinite) values.
 *
 * This test exercises various modulation sources and destinations, repeatedly connecting and
 * disconnecting them, applying large modulation amounts, and verifying engine output correctness
 * after processing.
 */
class ModulationStressTest : public UnitTest {
public:
    /**
     * @brief Constructs a ModulationStressTest with the specified test name and category.
     */
    ModulationStressTest() : UnitTest("Modulations", "Stress") { }

    /**
     * @brief Runs the modulation stress tests, including both "allModulations" and "randomModulations" tests.
     */
    void runTest() override;

    /**
     * @brief Connects a wide variety of modulation sources and destinations, then disconnects them,
     * ensuring the engine remains stable and produces finite values.
     */
    void allModulations();

    /**
     * @brief Randomly connects and disconnects modulation sources and destinations to ensure the engine can handle
     * unpredictable and extreme modulation scenarios without instability.
     */
    void randomModulations();

    /**
     * @brief Processes audio through the given processor multiple times and checks that the output remains finite.
     * @param processor The processor through which to run and validate audio data.
     */
    void processAndCheckFinite(vital::Processor* processor);
};
