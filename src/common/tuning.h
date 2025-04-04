/*
Summary:
The Tuning class provides a flexible microtonal tuning system for Vital. It can load multiple file formats, define custom scales, and set reference notes and frequencies. The code handles reading and interpreting different tuning standards (Scala, Tun), as well as mapping notes via keyboard mapping files. This allows Vital users to easily explore alternate tunings beyond standard equal temperament.
 */

#pragma once

#include "JuceHeader.h"

#include "common.h"
#include "json/json.h"

using json = nlohmann::json;

/**
 * @brief A class for managing microtonal tunings and custom pitch mappings in Vital.
 *
 * The Tuning class allows loading and applying scales, .scl (Scala) files, .tun files,
 * and keyboard mapping files (.kbm) to adjust how MIDI notes are mapped to frequencies.
 * It supports setting a reference frequency, defining custom scales, and converting
 * MIDI note numbers to pitch values according to the chosen tuning.
 */
class Tuning {
public:
    /**
     * @brief The total size of the internal tuning table.
     *
     * The table extends from -vital::kMidiSize to +vital::kMidiSize, creating a large range
     * for indexing notes around a central reference.
     */
    static constexpr int kTuningSize = 2 * vital::kMidiSize;

    /**
     * @brief The center index of the tuning table.
     *
     * This is the "middle" reference point used when indexing tuning values by MIDI note offsets.
     */
    static constexpr int kTuningCenter = vital::kMidiSize;

    /**
     * @brief Creates a Tuning object from a given file.
     *
     * Supported files include Scala (.scl), .tun, and .kbm files. The tuning data is loaded
     * and parsed into a form usable by the synthesizer.
     *
     * @param file The file to load the tuning from.
     * @return A Tuning object representing the loaded tuning.
     */
    static Tuning getTuningForFile(File file);

    /**
     * @brief Returns a string listing all supported tuning file extensions.
     *
     * E.g., "*.scl;*.kbm;*.tun"
     *
     * @return A string of semicolon-separated file extensions.
     */
    static String allFileExtensions();

    /**
     * @brief Converts a note name (e.g. "A4") to a MIDI key number.
     *
     * @param note The note name string.
     * @return The MIDI note number or -1 if invalid.
     */
    static int noteToMidiKey(const String& note);

    /**
     * @brief Constructs a Tuning object representing default (12-tone equal temperament) tuning.
     */
    Tuning();

    /**
     * @brief Constructs a Tuning object from a given file, loading that tuning.
     *
     * @param file The tuning file.
     */
    Tuning(File file);

    /**
     * @brief Loads a custom scale from a vector of offsets, specified in semitones or transposition units.
     *
     * @param scale A vector of floats representing scale steps.
     */
    void loadScale(std::vector<float> scale);

    /**
     * @brief Loads a tuning from a given file, automatically detecting its format (.scl, .kbm, .tun).
     *
     * @param file The tuning file.
     */
    void loadFile(File file);

    /**
     * @brief Sets a constant tuning, making all notes map to the same pitch offset.
     *
     * Useful as a fallback if scale loading fails.
     *
     * @param note The constant pitch offset to apply.
     */
    void setConstantTuning(float note);

    /**
     * @brief Resets the tuning to the default 12-tone equal temperament with a standard reference pitch.
     */
    void setDefaultTuning();

    /**
     * @brief Converts a MIDI note number to a pitch offset based on the current tuning.
     *
     * @param note The MIDI note number.
     * @return A mono_float representing the pitch offset (in semitones).
     */
    vital::mono_float convertMidiNote(int note) const;

    /**
     * @brief Sets the starting MIDI note for the scale mapping.
     *
     * @param start_midi_note The MIDI note to start the scale at.
     */
    void setStartMidiNote(int start_midi_note) { scale_start_midi_note_ = start_midi_note; }

    /**
     * @brief Sets the reference MIDI note number around which the tuning is centered.
     *
     * @param reference_note The reference MIDI note.
     */
    void setReferenceNote(int reference_note) { reference_midi_note_ = reference_note; }

    /**
     * @brief Sets the reference frequency used for calculating pitches.
     *
     * @param frequency The reference frequency in Hz.
     */
    void setReferenceFrequency(float frequency);

    /**
     * @brief Sets the reference note and frequency pair.
     *
     * @param midi_note The reference MIDI note.
     * @param frequency The frequency to assign to this MIDI note.
     */
    void setReferenceNoteFrequency(int midi_note, float frequency);

    /**
     * @brief Sets the reference ratio, defining a pitch offset in ratio form.
     *
     * @param ratio The ratio used to set the reference note offset.
     */
    void setReferenceRatio(float ratio);

    /**
     * @brief Gets the name of the current tuning, combining tuning and mapping names if both exist.
     *
     * @return A string representing the tuning name.
     */
    std::string getName() const {
        if (mapping_name_.empty())
            return tuning_name_;
        if (tuning_name_.empty())
            return mapping_name_;
        return tuning_name_ + " / " + mapping_name_;
    }

    /**
     * @brief Sets a custom name for the current tuning, clearing any mapping name.
     *
     * @param name The new tuning name.
     */
    void setName(const std::string& name) {
        mapping_name_.clear();
        tuning_name_ = name;
    }

    /**
     * @brief Checks if the tuning is the default equal temperament.
     *
     * @return True if default, false otherwise.
     */
    bool isDefault() const { return default_; }

    /**
     * @brief Saves the current tuning state into a JSON object.
     *
     * @return A JSON object representing the tuning state.
     */
    json stateToJson() const;

    /**
     * @brief Restores the tuning state from a JSON object.
     *
     * @param data The JSON object containing tuning state data.
     */
    void jsonToState(const json& data);

    /**
     * @brief Loads a Scala (.scl) file from a set of lines.
     *
     * This helper allows loading Scala format from already read lines.
     *
     * @param scala_lines The lines of the Scala file.
     */
    void loadScalaFile(const StringArray& scala_lines);

private:
    void loadScalaFile(File scala_file);
    void loadKeyboardMapFile(File kbm_file);
    void loadTunFile(File tun_file);

    int scale_start_midi_note_;
    float reference_midi_note_;
    std::vector<float> scale_;
    std::vector<int> keyboard_mapping_;
    vital::mono_float tuning_[kTuningSize];
    std::string tuning_name_;
    std::string mapping_name_;
    bool default_;

    JUCE_LEAK_DETECTOR(Tuning)
};
