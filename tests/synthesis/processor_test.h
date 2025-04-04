/**
 * @file processor_test.h
 * @brief Declares the ProcessorTest class, which provides a testing framework for Vital's Processor objects.
 */

#pragma once

#include "JuceHeader.h"
#include <set>

namespace vital {
    class Processor;
} // namespace vital

/**
 * @class ProcessorTest
 * @brief A base test class for verifying the stability, correctness, and finite outputs of Processor objects.
 *
 * The ProcessorTest class provides a framework to run input bounds tests on various Processor instances.
 * It ensures that under extreme input conditions (e.g. very large or very negative values, random noise),
 * the processor still outputs finite values without instability.
 */
class ProcessorTest : public UnitTest {
public:
    /**
     * @brief Constructs a ProcessorTest with the specified test name.
     * @param name The name of the test.
     */
    ProcessorTest(String name) : UnitTest(name, "Processor") { }

    /**
     * @brief Runs a standardized input bounds test on a Processor with default settings.
     *
     * This test sets input values to zero, very high, very negative, and random values to ensure the processor remains stable.
     * @param processor A pointer to the Processor under test.
     */
    void runInputBoundsTest(vital::Processor* processor);

    /**
     * @brief Runs an input bounds test on a Processor, optionally leaving certain inputs unchanged and ignoring certain outputs.
     *
     * @param processor A pointer to the Processor under test.
     * @param leave_inputs A set of input indices that should be left unchanged and not overridden by the test conditions.
     * @param ignore_outputs A set of output indices to ignore when verifying output finiteness. Useful when certain outputs are not relevant.
     */
    void runInputBoundsTest(vital::Processor* processor, std::set<int> leave_inputs, std::set<int> ignore_outputs);

    /**
     * @brief Processes the processor multiple times and checks if the output remains finite.
     *
     * This method repeatedly processes the Processor's buffer and verifies that all relevant outputs remain finite.
     * @param processor A pointer to the Processor under test.
     * @param ignore_outputs A set of output indices to ignore when checking for finite values.
     */
    void processAndCheckFinite(vital::Processor* processor, const std::set<int>& ignore_outputs);
};
