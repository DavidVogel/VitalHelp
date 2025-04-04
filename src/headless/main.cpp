#include "JuceHeader.h"
#include "load_save.h"
#include "tuning.h"
#include "synth_base.h"

/**
 * @brief Retrieves the value of a command-line argument following a given flag.
 *
 * Searches through the command-line arguments for the specified short or long flag. If found,
 * returns the next argument as the value. Otherwise, returns an empty string.
 *
 * @param argc The number of command-line arguments.
 * @param argv The array of command-line argument strings.
 * @param flag The short flag (e.g. "-o").
 * @param full_flag The long flag (e.g. "--output").
 * @return The value string following the given flag, or "" if not found.
 */
String getArgumentValue(int argc, const char* argv[], const String& flag, const String& full_flag) {
    for (int i = 0; i < argc - 1; ++i) {
        std::string arg = argv[i];
        if (arg == flag || arg == full_flag)
            return argv[i + 1];
    }

    return "";
}

/**
 * @brief Checks if a particular flag is present in the command-line arguments.
 *
 * @param argc The number of command-line arguments.
 * @param argv The array of command-line argument strings.
 * @param flag The short flag to check.
 * @param full_flag The long flag to check.
 * @return True if the flag is found, false otherwise.
 */
bool hasFlag(int argc, const char* argv[], const String& flag, const String& full_flag) {
    for (int i = 0; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == flag || arg == full_flag)
            return true;
    }

    return false;
}

/**
 * @brief Retrieves the desired length in seconds for rendering output audio from the command line.
 *
 * If not specified, defaults to 5.0 seconds. Caps at a maximum of 15.0 seconds.
 *
 * @param argc The number of arguments.
 * @param argv The argument vector.
 * @return The render length in seconds.
 */
float getRenderLength(int argc, const char* argv[]) {
    static constexpr float kDefaultRenderLength = 5.0f;
    static constexpr float kMaxRenderLength = 15.0f;

    String string_length = getArgumentValue(argc, argv, "-l", "--length");
    float length = kDefaultRenderLength;
    if (string_length.isEmpty())
        return kDefaultRenderLength;

    float float_val = string_length.getFloatValue();
    if (float_val > 0.0f)
        length = std::min(float_val, kMaxRenderLength);

    return length;
}

/**
 * @brief Parses the command line for desired MIDI note inputs to render.
 *
 * Uses note names or numeric values. If none provided, defaults to MIDI note 48.
 *
 * @param argc The number of arguments.
 * @param argv The argument vector.
 * @return A vector of MIDI notes to use during rendering.
 */
std::vector<int> getRenderMidiNotes(int argc, const char* argv[]) {
    static constexpr int kDefaultMidiNote = 48;

    String string_midi = getArgumentValue(argc, argv, "-m", "--midi");
    std::vector<int> midi_notes;
    if (!string_midi.isEmpty()) {
        StringArray midi_tokens;
        midi_tokens.addTokens(string_midi, ",", "");

        for (const String& midi_token : midi_tokens) {
            int midi = Tuning::noteToMidiKey(midi_token);
            if (midi >= 0)
                midi_notes.push_back(midi);
        }
    }

    if (midi_notes.empty())
        midi_notes.push_back(kDefaultMidiNote);

    return midi_notes;
}

/**
 * @brief Retrieves the BPM (Beats Per Minute) from command-line arguments.
 *
 * Defaults to 120 BPM. Enforces minimum 5.0 BPM and max 900.0 BPM.
 *
 * @param argc The number of arguments.
 * @param argv The argument vector.
 * @return The BPM to use when rendering.
 */
float getRenderBpm(int argc, const char* argv[]) {
    static constexpr float kDefaultBpm = 120.0f;
    static constexpr float kMinBpm = 5.0f;
    static constexpr float kMaxBpm = 900.0f;

    String string_length = getArgumentValue(argc, argv, "-b", "--bpm");
    float bpm = kDefaultBpm;
    if (string_length.isEmpty())
        return kDefaultBpm;

    bpm = std::min(string_length.getFloatValue(), kMaxBpm);
    return std::max(bpm, kMinBpm);
}

/**
 * @brief Renders the loaded synth configuration to an audio file if output is specified.
 *
 * Checks command-line flags for output path, whether to render images, length, BPM, and MIDI notes,
 * and uses HeadlessSynth::renderAudioToFile to produce the result.
 *
 * @param headless_synth The synth to render from.
 * @param argc The number of arguments.
 * @param argv The argument vector.
 */
void doRenderToFile(HeadlessSynth& headless_synth, int argc, const char* argv[]) {
    String string_output_file = getArgumentValue(argc, argv, "-o", "--output");
    bool render_images = hasFlag(argc, argv, "-i", "--render-images");

    if (string_output_file.isEmpty())
        return;

    if (!string_output_file.startsWith("/"))
        string_output_file = "./" + string_output_file;

    File output_file(string_output_file);
    if (!output_file.hasWriteAccess()) {
        std::cout << "Error: Don't have permission to write output file." << newLine;
        return;
    }

    float length = getRenderLength(argc, argv);
    float bpm = getRenderBpm(argc, argv);
    std::vector<int> midi_notes = getRenderMidiNotes(argc, argv);

    headless_synth.renderAudioToFile(output_file, length, bpm, midi_notes, render_images);
}

/**
 * @brief Attempts to load a file from the command line into the HeadlessSynth.
 *
 * If the argument is a file path, tries to load it as a preset or wavetable. If successful, returns true.
 *
 * @param synth The HeadlessSynth instance.
 * @param command_line The file path or command.
 * @return True if loaded successfully, false otherwise.
 */
bool loadFromCommandLine(HeadlessSynth& synth, const String& command_line) {
    String file_path = command_line;
    if (file_path[0] == '"' && file_path[file_path.length() - 1] == '"')
        file_path = command_line.substring(1, command_line.length() - 1);
    File file = File::getCurrentWorkingDirectory().getChildFile(file_path);
    if (!file.exists())
        return false;

    std::string error;
    synth.loadFromFile(file, error);
    return true;
}

/**
 * @brief The main entry point for the headless tool.
 *
 * Processes command-line arguments to optionally load a preset or wavetable and render it to an audio file.
 *
 * Usage:
 * - Provide a file path to load as a preset/wavetable.
 * - Use flags like -o/--output for output file, -l/--length for render length, -m/--midi for MIDI notes,
 *   -b/--bpm for BPM, and -i/--render-images to produce image frames.
 *
 * @param argc The number of arguments.
 * @param argv The argument vector.
 * @return Returns 0 on success.
 */
int main(int argc, const char* argv[]) {
    HeadlessSynth headless_synth;

    bool last_arg_was_option = false;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg != "" && arg[0] != '-' && !last_arg_was_option && loadFromCommandLine(headless_synth, arg))
            break;

        last_arg_was_option = arg[0] == '-' && arg != "--headless";
    }

    doRenderToFile(headless_synth, argc, argv);
}
