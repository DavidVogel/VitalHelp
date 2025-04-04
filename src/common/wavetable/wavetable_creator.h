#pragma once

#include "JuceHeader.h"
#include "wave_frame.h"
#include "wavetable_group.h"
#include "file_source.h"
#include "json/json.h"
#include "wavetable.h"

using json = nlohmann::json;

class LineGenerator;

/**
 * @brief A class responsible for creating complete wavetables from groups of wavetable components.
 *
 * WavetableCreator combines multiple WavetableGroups (each containing sources and modifiers) to produce
 * a final Wavetable object. It supports:
 * - Managing multiple groups of components (sources/modifiers).
 * - Rendering the entire wavetable by blending the outputs of all groups.
 * - Loading wavetables from audio files and applying different load styles (splicing, vocoding, pitched analysis).
 * - Initializing predefined wave shapes or line-based generators.
 * - Normalizing and DC-removing the final table if desired.
 *
 * The creator can serialize its state to JSON and restore it, integrating with preset/loading systems.
 */
class WavetableCreator {
public:
    /**
     * @enum AudioFileLoadStyle
     * @brief Defines how audio files are interpreted when loading into a wavetable.
     */
    enum AudioFileLoadStyle {
        kNone,
        kWavetableSplice, ///< Slice the audio into segments mapping onto wavetable frames.
        kVocoded,         ///< Apply vocoder-like analysis to extract fundamental cycles.
        kTtwt,            ///< Similar to vocoded but optimized for TTWT (text-to-wavetable).
        kPitched,         ///< Attempts to extract pitch-based cycles from the audio.
        kNumDragLoadStyles
    };

    /**
     * @brief Constructs a WavetableCreator associated with a given Wavetable object.
     *
     * By default, full normalization and DC removal are enabled.
     *
     * @param wavetable A pointer to the Wavetable to be created or modified.
     */
    WavetableCreator(vital::Wavetable* wavetable) : wavetable_(wavetable),
                                                    full_normalize_(true), remove_all_dc_(true) { }

    int getGroupIndex(WavetableGroup* group);
    void addGroup(WavetableGroup* group) { groups_.push_back(std::unique_ptr<WavetableGroup>(group)); }
    void removeGroup(int index);
    void moveUp(int index);
    void moveDown(int index);

    /**
     * @brief Gets the total number of WavetableGroups.
     *
     * @return The number of groups.
     */
    int numGroups() const { return static_cast<int>(groups_.size()); }

    /**
     * @brief Retrieves a WavetableGroup by index.
     *
     * @param index The index of the desired group.
     * @return A pointer to the WavetableGroup.
     */
    WavetableGroup* getGroup(int index) const { return groups_[index].get(); }

    float render(int position);
    void render();
    void postRender(float max_span);
    void renderToBuffer(float* buffer, int num_frames, int frame_size);
    void init();
    void clear();
    void loadDefaultCreator();

    void initPredefinedWaves();
    void initFromAudioFile(const float* audio_buffer, int num_samples, int sample_rate,
                           AudioFileLoadStyle load_style, FileSource::FadeStyle fade_style);

    /**
     * @brief Sets the name of the wavetable.
     *
     * @param name The name to assign.
     */
    void setName(const std::string& name) { wavetable_->setName(name); }

    /**
     * @brief Sets the author of the wavetable.
     *
     * @param author The author name.
     */
    void setAuthor(const std::string& author) { wavetable_->setAuthor(author); }

    /**
     * @brief Records the path of the last file loaded to create this wavetable.
     *
     * @param path The file path.
     */
    void setFileLoaded(const std::string& path) { last_file_loaded_ = path; }

    /**
     * @brief Gets the name of the wavetable.
     *
     * @return The wavetable's name.
     */
    std::string getName() const { return wavetable_->getName(); }

    /**
     * @brief Gets the author of the wavetable.
     *
     * @return The author's name.
     */
    std::string getAuthor() const { return wavetable_->getAuthor(); }

    /**
     * @brief Gets the last loaded file path.
     *
     * @return The last file path used for loading the wavetable.
     */
    std::string getLastFileLoaded() { return last_file_loaded_; }

    /**
     * @brief Checks if a given JSON data represents a valid wavetable creator state.
     *
     * @param data The JSON object to validate.
     * @return True if valid, false otherwise.
     */
    static bool isValidJson(json data);

    json updateJson(json data);
    json stateToJson();
    void jsonToState(json data);

    /**
     * @brief Gets the internal wavetable object being created.
     *
     * @return A pointer to the wavetable.
     */
    vital::Wavetable* getWavetable() { return wavetable_; }

protected:
    void initFromSplicedAudioFile(const float* audio_buffer, int num_samples, int sample_rate,
                                  FileSource::FadeStyle fade_style);
    void initFromVocodedAudioFile(const float* audio_buffer, int num_samples, int sample_rate, bool ttwt);
    void initFromPitchedAudioFile(const float* audio_buffer, int num_samples, int sample_rate);
    void initFromLineGenerator(LineGenerator* line_generator);

    vital::WaveFrame compute_frame_combine_;
    vital::WaveFrame compute_frame_;
    std::vector<std::unique_ptr<WavetableGroup>> groups_;

    std::string last_file_loaded_;
    vital::Wavetable* wavetable_;
    bool full_normalize_;
    bool remove_all_dc_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WavetableCreator)
};
