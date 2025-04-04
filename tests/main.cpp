/**
 * @file main.cpp
 * @brief Entry point for running Vital's unit tests, either all at once or in a targeted manner.
 *
 * This file defines a main function that:
 * - Can run all tests if no arguments are provided.
 * - Can run only non-graphical tests if arguments are provided.
 * - Provides helper functions for running individual tests, categories of tests, and verifying their results.
 *
 * It also includes some placeholder code for re-branding wav files (not directly related to testing),
 * and logic for reading metadata from wav files.
 */

#include "JuceHeader.h"
#include "interface/full_interface_test.h"

/**
 * @class SynthTestRunner
 * @brief A custom UnitTestRunner that logs messages to stdout.
 */
class SynthTestRunner : public UnitTestRunner {
protected:
    /**
     * @brief Logs a message from the test runner to stdout.
     * @param message The message to log.
     */
    void logMessage(const String& message) override {
        std::cout << message.toStdString() << std::endl;
    }
};

/**
 * @brief Converts a chunk name to a little-endian integer for comparison.
 * @param chunk_name A 4-character chunk name.
 * @return An integer representation of the chunk name.
 */
force_inline int chunkNameToData(const char* chunk_name) {
    return static_cast<int>(ByteOrder::littleEndianInt(chunk_name));
}

/**
 * @brief Reads a custom "clm " chunk from a WAV file and returns a string representing the data.
 *
 * The WAV file structure is navigated to find a chunk with the label "clm ". If found,
 * its data is read and converted to a string. The returned string is truncated to 27 characters.
 *
 * @param input_stream The InputStream to read data from.
 * @return A substring of the "clm " chunk data if found, otherwise an empty string.
 */
String getWavetableDataString(InputStream* input_stream) {
    static constexpr int kDataLength = 27;
    int first_chunk = input_stream->readInt();
    if (first_chunk != chunkNameToData("RIFF"))
        return "";

    int length = input_stream->readInt();
    int data_end = static_cast<int>(input_stream->getPosition()) + length;

    if (input_stream->readInt() != chunkNameToData("WAVE"))
        return "";

    while (!input_stream->isExhausted() && input_stream->getPosition() < data_end) {
        int chunk_label = input_stream->readInt();
        int chunk_length = input_stream->readInt();

        if (chunk_label == chunkNameToData("clm ")) {
            MemoryBlock memory_block;
            input_stream->readIntoMemoryBlock(memory_block, chunk_length);
            return memory_block.toString().substring(0, kDataLength);
        }

        input_stream->setPosition(input_stream->getPosition() + chunk_length);
    }

    return "";
}

/**
 * @brief Rebrands all WAV files in a specified directory by reading their data and adding a "[Matt Tytel]" tag.
 *
 * This function is currently not directly related to testing. It scans a directory for WAV files, extracts
 * custom metadata using getWavetableDataString(), reads audio data, and writes out a new WAV file with
 * updated metadata and a fixed sample rate of 88200 Hz.
 */
void rebrandAllWavs() {
    static constexpr int kWavetableSampleRate = 88200;

    File directory("D:\\dev\\PurchasedWavetables");
    if (!directory.exists())
        return;

    Array<File> wavs;
    directory.findChildFiles(wavs, File::findFiles, true, "*.wav");
    AudioFormatManager format_manager;
    format_manager.registerBasicFormats();

    File converted_directory = directory.getChildFile("Converted");
    converted_directory.createDirectory();
    for (File& file : wavs) {
        FileInputStream* input_stream = new FileInputStream(file);
        String clm_data = getWavetableDataString(input_stream) + "[Matt Tytel]";
        input_stream->setPosition(0);
        std::unique_ptr<AudioFormatReader> format_reader(
                format_manager.createReaderFor(std::unique_ptr<InputStream>(input_stream)));
        if (format_reader == nullptr)
            continue;

        AudioSampleBuffer sample_buffer;
        int num_samples = static_cast<int>(format_reader->lengthInSamples);
        sample_buffer.setSize(1, num_samples);
        format_reader->read(&sample_buffer, 0, num_samples, 0, true, false);

        File output_file = converted_directory.getChildFile(file.getFileName());
        std::unique_ptr<FileOutputStream> file_stream = output_file.createOutputStream();
        WavAudioFormat wav_format;
        StringPairArray meta_data;
        meta_data.set("clm ", clm_data);
        std::unique_ptr<AudioFormatWriter> writer(wav_format.createWriterFor(file_stream.get(), kWavetableSampleRate,
                                                                             1, 16, meta_data, 0));

        const float* channel = sample_buffer.getReadPointer(0);
        writer->writeFromFloatArrays(&channel, 1, num_samples);
        writer->flush();
        file_stream->flush();
    }
}

/**
 * @brief Checks all test results and returns 0 if successful, -1 if any test failed.
 * @param test_runner The test runner containing the results.
 * @return 0 if all tests passed, -1 if any test failed.
 */
int getResult(UnitTestRunner& test_runner) {
    for (int i = 0; i < test_runner.getNumResults(); ++i) {
        if (test_runner.getResult(i)->failures)
            return -1;
    }

    return 0;
}

/**
 * @brief Runs a single test and returns the result.
 * @param test A pointer to the UnitTest to run.
 * @return 0 if the test passed, -1 if it failed.
 */
int runSingleTest(UnitTest* test) {
    SynthTestRunner test_runner;
    test_runner.setAssertOnFailure(true);

    Array<UnitTest*> tests;
    tests.add(test);
    test_runner.runTests(tests);
    return getResult(test_runner);
}

/**
 * @brief Runs a single test identified by category and name.
 * @param category The category the test belongs to.
 * @param name The name of the test.
 * @return 0 if the test was found and passed, -1 if it failed or not found.
 */
int runSingleTest(String category, String name) {
    StringArray categories = UnitTest::getAllCategories();
    for (const String& category_name : categories) {
        if (category_name == category) {
            Array<UnitTest*> tests = UnitTest::getTestsInCategory(category);

            for (UnitTest* test : tests) {
                if (test->getName() == name)
                    return runSingleTest(test);
            }
        }
    }
    return -1;
}

/**
 * @brief Runs all non-graphical tests (i.e., all categories except "Interface").
 * @return 0 if all tests passed, -1 if any test failed.
 */
int runNonGraphicalTests() {
    SynthTestRunner test_runner;
    test_runner.setAssertOnFailure(true);

    StringArray categories = UnitTest::getAllCategories();
    for (const String& category : categories) {
        if (category != "Interface")
            test_runner.runTestsInCategory(category);

        for (int i = 0; i < test_runner.getNumResults(); ++i) {
            if (test_runner.getResult(i)->failures)
                return -1;
        }
    }

    return getResult(test_runner);
}

/**
 * @brief Runs all tests (graphical and non-graphical).
 * @return 0 if all tests passed, -1 if any test failed.
 */
int runAllTests() {
    SynthTestRunner test_runner;
    test_runner.setAssertOnFailure(true);
    test_runner.runAllTests();

    return getResult(test_runner);
}

/**
 * @brief Decides which tests to run based on command line arguments.
 * @param argc Number of command line arguments.
 * @return 0 if the selected tests passed, -1 otherwise.
 */
int runTests(int argc) {
    if (argc > 1)
        return runNonGraphicalTests();

    return runAllTests();
}

/**
 * @brief The main entry point of the test runner application.
 *
 * Running the program with no arguments runs all tests. Providing arguments runs only non-graphical tests.
 *
 * @param argc The number of command line arguments.
 * @param argv The command line arguments.
 * @return 0 if tests passed, -1 if tests failed.
 */
int main(int argc, char* argv[]) {
    int result = runTests(argc);

    DeletedAtShutdown::deleteAll();
    MessageManager::deleteInstance();
    return result;
}
