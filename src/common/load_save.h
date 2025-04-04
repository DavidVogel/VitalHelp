#pragma once

#include "JuceHeader.h"
#include "json/json.h"

#include <map>
#include <set>
#include <string>

using json = nlohmann::json;

namespace vital {
    class StringLayout;
}

class MidiManager;
class SynthBase;

/**
 * @brief A utility class for loading and saving configuration, preset, and state data for the Vital synthesizer.
 *
 * This class provides a comprehensive set of static methods for:
 * - Converting between data formats (e.g., float PCM data and integer PCM).
 * - Reading and writing synth states, including parameters, wavetables, samples, LFOs, modulations, and more.
 * - Managing user directories, favorites, installed packs, and additional folders.
 * - Updating old configuration/preset states to newer versions.
 * - Handling checks for updates, authentication, window size, and other global settings.
 */
class LoadSave {
public:
    /**
     * @brief A helper class for sorting files in ascending order based on their names.
     */
    class FileSorterAscending {
    public:
        FileSorterAscending() { }

        /**
         * @brief Compares two files by their full path names in a case-insensitive, natural order.
         *
         * @param a The first file to compare.
         * @param b The second file to compare.
         * @return An integer less than, equal to, or greater than zero depending on the lexicographical order.
         */
        static int compareElements(File a, File b) {
            return a.getFullPathName().toLowerCase().compareNatural(b.getFullPathName().toLowerCase());
        }

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FileSorterAscending)
    };

    /**
     * @brief Enumeration of preset styles that categorize sounds.
     */
    enum PresetStyle {
        kBass,
        kLead,
        kKeys,
        kPad,
        kPercussion,
        kSequence,
        kExperimental,
        kSfx,
        kTemplate,
        kNumPresetStyles
    };

    /// Maximum length of preset comments.
    static const int kMaxCommentLength = 500;

    /// Various folder and directory name constants.
    static const std::string kUserDirectoryName;
    static const std::string kPresetFolderName;
    static const std::string kWavetableFolderName;
    static const std::string kSkinFolderName;
    static const std::string kSampleFolderName;
    static const std::string kLfoFolderName;
    static const std::string kAdditionalWavetableFoldersName;
    static const std::string kAdditionalSampleFoldersName;

    /**
     * @brief Converts a float buffer field in JSON to PCM format in base64.
     *
     * @param data The JSON object containing the field.
     * @param field The name of the field to convert.
     */
    static void convertBufferToPcm(json& data, const std::string& field);

    /**
     * @brief Converts a PCM buffer field in JSON to a float buffer in base64.
     *
     * @param data The JSON object containing the field.
     * @param field The name of the field to convert.
     */
    static void convertPcmToFloatBuffer(json& data, const std::string& field);

    /**
     * @brief Converts the state of a given SynthBase to JSON.
     *
     * Captures all synth settings, modulations, wavetables, LFO sources, etc.
     *
     * @param synth A pointer to the SynthBase instance.
     * @param critical_section A reference to a CriticalSection for thread safety.
     * @return A JSON object representing the current synth state.
     */
    static json stateToJson(SynthBase* synth, const CriticalSection& critical_section);

    /**
     * @brief Loads controls from the given JSON data into a SynthBase instance.
     *
     * This sets each control parameter in the synth to the values found in the JSON object.
     *
     * @param synth A pointer to the SynthBase instance to modify.
     * @param data The JSON object containing control data.
     */
    static void loadControls(SynthBase* synth, const json& data);

    /**
     * @brief Loads modulation connections from JSON data into a SynthBase instance.
     *
     * Rebuilds modulation sources and destinations, as well as line mappings.
     *
     * @param synth A pointer to the SynthBase instance.
     * @param modulations A JSON array of modulation definitions.
     */
    static void loadModulations(SynthBase* synth, const json& modulations);

    /**
     * @brief Loads a sample configuration into the SynthBase from a JSON object.
     *
     * @param synth A pointer to the SynthBase.
     * @param sample A JSON object representing the sample state.
     */
    static void loadSample(SynthBase* synth, const json& sample);

    /**
     * @brief Loads wavetable configurations into a SynthBase from a JSON array.
     *
     * @param synth A pointer to the SynthBase.
     * @param wavetables A JSON array of wavetable definitions.
     */
    static void loadWavetables(SynthBase* synth, const json& wavetables);

    /**
     * @brief Loads LFO states (line shapes) from a JSON array into a SynthBase.
     *
     * @param synth A pointer to the SynthBase.
     * @param lfos A JSON array of LFO states.
     */
    static void loadLfos(SynthBase* synth, const json& lfos);

    /**
     * @brief Extracts and stores basic preset info (name, author, comments, style, macros) from JSON data.
     *
     * @param save_info A map where info is stored as key-value pairs.
     * @param data The JSON data containing preset info.
     */
    static void loadSaveState(std::map<std::string, String>& save_info, json data);

    /**
     * @brief Initializes save_info with default values for preset information.
     *
     * @param save_info The map to initialize.
     */
    static void initSaveInfo(std::map<std::string, String>& save_info);

    /**
     * @brief Updates a JSON state from an older version of Vital's format to the current version.
     *
     * Applies various transformations to parameter names, ranges, or modulation destinations to maintain compatibility.
     *
     * @param state The original JSON state.
     * @return The updated JSON state.
     */
    static json updateFromOldVersion(json state);

    /**
     * @brief Loads a JSON state into the given SynthBase, applying older version updates if necessary.
     *
     * @param synth A pointer to the SynthBase.
     * @param save_info A map to store extracted preset information.
     * @param state The JSON state to load.
     * @return True if successful, false if the version is incompatible.
     */
    static bool jsonToState(SynthBase* synth, std::map<std::string, String>& save_info, json state);

    /**
     * @brief Extracts the author's name from a given preset file.
     *
     * @param file The preset File to examine.
     * @return The author's name if found, or an empty String otherwise.
     */
    static String getAuthorFromFile(const File& file);

    /**
     * @brief Extracts the style from a given preset file.
     *
     * @param file The preset File to examine.
     * @return The preset style if found, or an empty String otherwise.
     */
    static String getStyleFromFile(const File& file);

    /**
     * @brief Extracts the author name from a JSON object representing a preset/state.
     *
     * @param file A JSON object containing the author's field.
     * @return The author's name as a std::string.
     */
    static std::string getAuthor(json file);

    /**
     * @brief Extracts the license information from a JSON state, if present.
     *
     * @param state The JSON object potentially containing a "license" key.
     * @return The license string if found, or an empty string otherwise.
     */
    static std::string getLicense(json state);

    /**
     * @brief Retrieves the main configuration file path for Vital.
     *
     * @return A File object representing the configuration file.
     */
    static File getConfigFile();

    /**
     * @brief Writes a crash log to a file in the data directory.
     *
     * @param crash_log The crash log text.
     */
    static void writeCrashLog(String crash_log);

    /**
     * @brief Appends an error message to an error log file.
     *
     * @param error_log The error message to write.
     */
    static void writeErrorLog(String error_log);

    /**
     * @brief Parses and returns the main configuration JSON.
     *
     * @return A JSON object representing the current configuration.
     */
    static json getConfigJson();

    /**
     * @brief Retrieves the file storing the user's favorites.
     *
     * @return A File object representing the favorites file.
     */
    static File getFavoritesFile();

    /**
     * @brief Retrieves the file specifying the default skin.
     *
     * @return A File object representing the default skin file.
     */
    static File getDefaultSkin();

    /**
     * @brief Parses and returns the favorites JSON data.
     *
     * @return A JSON object of favorites.
     */
    static json getFavoritesJson();

    /**
     * @brief Adds a new favorite preset or item to the favorites file.
     *
     * @param new_favorite The file to be added as a favorite.
     */
    static void addFavorite(const File& new_favorite);

    /**
     * @brief Removes a favorite item from the favorites.
     *
     * @param old_favorite The file to remove from favorites.
     */
    static void removeFavorite(const File& old_favorite);

    /**
     * @brief Retrieves all favorites as a set of string paths.
     *
     * @return A set of full path names representing favorites.
     */
    static std::set<std::string> getFavorites();

    /**
     * @brief Checks if a data directory is properly configured (exists and has packs.json).
     *
     * @return True if a valid data directory is found, false otherwise.
     */
    static bool hasDataDirectory();

    /**
     * @brief Retrieves the file listing available packs.
     *
     * @return A File object representing the available packs file.
     */
    static File getAvailablePacksFile();

    /**
     * @brief Parses and returns JSON data about available packs.
     *
     * @return A JSON object listing available packs.
     */
    static json getAvailablePacks();

    /**
     * @brief Retrieves the file that stores information about installed packs.
     *
     * @return A File object representing the installed packs file.
     */
    static File getInstalledPacksFile();

    /**
     * @brief Returns a JSON list of installed packs.
     *
     * @return A JSON object of installed packs.
     */
    static json getInstalledPacks();

    /**
     * @brief Saves the given JSON pack configuration to the installed packs file.
     *
     * @param packs A JSON object representing the current installed packs.
     */
    static void saveInstalledPacks(const json& packs);

    /**
     * @brief Marks a pack as installed by ID in the packs file.
     *
     * @param id The integer ID of the pack.
     */
    static void markPackInstalled(int id);

    /**
     * @brief Marks a pack as installed by name in the packs file.
     *
     * @param name The name of the pack.
     */
    static void markPackInstalled(const std::string& name);

    /**
     * @brief Saves the given directory as the data directory in the configuration.
     *
     * @param data_directory The File representing the chosen data directory.
     */
    static void saveDataDirectory(const File& data_directory);

    /**
     * @brief Checks if Vital is fully installed (data directory present).
     *
     * @return True if installed, false otherwise.
     */
    static bool isInstalled();

    /**
     * @brief Checks if Vital was upgraded from a previous version.
     *
     * @return True if the saved version is older than the current build.
     */
    static bool wasUpgraded();

    /**
     * @brief Checks if this build of Vital has expired.
     *
     * @return True if expired, false if still within allowed usage days.
     */
    static bool isExpired();

    /**
     * @brief Checks if this build of Vital includes an expiration mechanism.
     *
     * @return True if it expires after a certain number of days, otherwise false.
     */
    static bool doesExpire();

    /**
     * @brief Returns the number of days remaining until expiration.
     *
     * @return Days left before expiration, or a negative number if expired.
     */
    static int getDaysToExpire();

    /**
     * @brief Checks if Vital should perform update checks.
     *
     * @return True if update checks are enabled, false otherwise.
     */
    static bool shouldCheckForUpdates();

    /**
     * @brief Checks if Vital should operate in offline mode.
     *
     * @return True if offline mode is enabled, false otherwise.
     */
    static bool shouldWorkOffline();

    /**
     * @brief Retrieves the currently loaded skin name.
     *
     * @return The name of the loaded skin, or an empty string if none is set.
     */
    static std::string getLoadedSkin();

    /**
     * @brief Determines if widget animations are enabled.
     *
     * @return True if animations are enabled, false otherwise.
     */
    static bool shouldAnimateWidgets();

    /**
     * @brief Determines if frequencies should be displayed in Hz.
     *
     * @return True if frequencies are displayed in Hz.
     */
    static bool displayHzFrequency();

    /**
     * @brief Checks if the user is authenticated.
     *
     * @return True if authenticated, false otherwise.
     */
    static bool authenticated();

    /**
     * @brief Retrieves the current oversampling amount.
     *
     * @return The oversampling factor as an integer.
     */
    static int getOversamplingAmount();

    /**
     * @brief Loads the saved window size scaling factor.
     *
     * @return The window size scaling factor (>= 0.25f).
     */
    static float loadWindowSize();

    /**
     * @brief Loads the saved synth version string.
     *
     * @return The version string, or "0.0.0" if none is available.
     */
    static String loadVersion();

    /**
     * @brief Loads the saved content version string.
     *
     * @return The content version string, or "0.0" if not set.
     */
    static String loadContentVersion();

    /**
     * @brief Saves a given JSON object to the configuration file.
     *
     * @param config_state The JSON object to save.
     */
    static void saveJsonToConfig(json config_state);

    /**
     * @brief Saves a JSON object of favorites to the favorites file.
     *
     * @param favorites_json The JSON object of favorites.
     */
    static void saveJsonToFavorites(json favorites_json);

    /**
     * @brief Saves the provided author name to the config.
     *
     * @param author The author's name.
     */
    static void saveAuthor(std::string author);

    /**
     * @brief Saves a preferred Text-To-Wavetable (TTWT) language to the config.
     *
     * @param language The language code as a string.
     */
    static void savePreferredTTWTLanguage(std::string language);

    /**
     * @brief Saves layout configuration (keyboard layout and octave controls).
     *
     * @param layout A pointer to a StringLayout representing the computer keyboard layout.
     */
    static void saveLayoutConfig(vital::StringLayout* layout);

    /**
     * @brief Saves the current synth version to the config file.
     */
    static void saveVersionConfig();

    /**
     * @brief Saves the current content version to the config file.
     *
     * @param version The new content version string.
     */
    static void saveContentVersion(std::string version);

    /**
     * @brief Saves the user's preference regarding update checks.
     *
     * @param check_for_updates True if updates should be checked, false otherwise.
     */
    static void saveUpdateCheckConfig(bool check_for_updates);

    /**
     * @brief Saves the user's preference for working offline.
     *
     * @param work_offline True to work offline, false otherwise.
     */
    static void saveWorkOffline(bool work_offline);

    /**
     * @brief Saves the currently loaded skin name to the config.
     *
     * @param name The skin name as a string.
     */
    static void saveLoadedSkin(const std::string& name);

    /**
     * @brief Saves the widget animation preference.
     *
     * @param animate_widgets True if widgets should animate, false otherwise.
     */
    static void saveAnimateWidgets(bool animate_widgets);

    /**
     * @brief Saves the preference to display frequency in Hz.
     *
     * @param display_hz True if displaying in Hz, false otherwise.
     */
    static void saveDisplayHzFrequency(bool display_hz);

    /**
     * @brief Saves the user's authentication status.
     *
     * @param authenticated True if authenticated, false otherwise.
     */
    static void saveAuthenticated(bool authenticated);

    /**
     * @brief Saves the window size scaling factor.
     *
     * @param window_size The scaling factor to save.
     */
    static void saveWindowSize(float window_size);

    /**
     * @brief Saves MIDI mapping configuration.
     *
     * @param midi_manager A pointer to the MidiManager from which to extract mappings.
     */
    static void saveMidiMapConfig(MidiManager* midi_manager);

    /**
     * @brief Loads configuration data into a MidiManager and optional StringLayout.
     *
     * @param midi_manager A pointer to the MidiManager for loading MIDI maps.
     * @param layout An optional StringLayout pointer to set keyboard layouts.
     */
    static void loadConfig(MidiManager* midi_manager, vital::StringLayout* layout = nullptr);

    /**
     * @brief Retrieves the saved computer keyboard layout for playing notes.
     *
     * @return A wide-string representing the keyboard layout.
     */
    static std::wstring getComputerKeyboardLayout();

    /**
     * @brief Returns the preferred Text-To-Wavetable language, if set.
     *
     * @return The language code as a string.
     */
    static std::string getPreferredTTWTLanguage();

    /**
     * @brief Retrieves the saved author name from the config.
     *
     * @return The author name.
     */
    static std::string getAuthor();

    /**
     * @brief Retrieves the keys used for octave shifts on the computer keyboard layout.
     *
     * @return A pair of wchar_t representing the down and up keys.
     */
    static std::pair<wchar_t, wchar_t> getComputerKeyboardOctaveControls();

    /**
     * @brief Saves additional folder paths for presets, wavetables, or samples.
     *
     * @param name The category name (e.g., "wavetable_folders").
     * @param folders A vector of folder paths as strings.
     */
    static void saveAdditionalFolders(const std::string& name, std::vector<std::string> folders);

    /**
     * @brief Retrieves a list of additional folder paths for a given category.
     *
     * @param name The category name (e.g., "wavetable_folders").
     * @return A vector of folder paths as strings.
     */
    static std::vector<std::string> getAdditionalFolders(const std::string& name);

    /**
     * @brief Gets the current data directory from the config.
     *
     * @return A File object representing the data directory.
     */
    static File getDataDirectory();

    /**
     * @brief Retrieves directories of a given folder name under the data directory structure.
     *
     * @param folder_name The name of the folder (e.g., "Presets").
     * @return A vector of File objects pointing to directories.
     */
    static std::vector<File> getDirectories(const String& folder_name);

    /**
     * @brief Gets directories that should contain presets.
     *
     * @return A vector of File objects representing preset directories.
     */
    static std::vector<File> getPresetDirectories();

    /**
     * @brief Gets directories that should contain wavetables.
     *
     * @return A vector of File objects representing wavetable directories.
     */
    static std::vector<File> getWavetableDirectories();

    /**
     * @brief Gets directories that should contain skins.
     *
     * @return A vector of File objects representing skin directories.
     */
    static std::vector<File> getSkinDirectories();

    /**
     * @brief Gets directories that should contain samples.
     *
     * @return A vector of File objects representing sample directories.
     */
    static std::vector<File> getSampleDirectories();

    /**
     * @brief Gets directories that should contain LFO shapes.
     *
     * @return A vector of File objects representing LFO directories.
     */
    static std::vector<File> getLfoDirectories();

    /**
     * @brief Retrieves the user directory inside the data directory.
     *
     * @return A File object representing the user directory.
     */
    static File getUserDirectory();

    /**
     * @brief Retrieves the user's preset directory.
     *
     * @return A File object representing the user preset directory.
     */
    static File getUserPresetDirectory();

    /**
     * @brief Retrieves the user's wavetable directory.
     *
     * @return A File object representing the user wavetable directory.
     */
    static File getUserWavetableDirectory();

    /**
     * @brief Retrieves the user's skin directory.
     *
     * @return A File object representing the user skin directory.
     */
    static File getUserSkinDirectory();

    /**
     * @brief Retrieves the user's sample directory.
     *
     * @return A File object representing the user sample directory.
     */
    static File getUserSampleDirectory();

    /**
     * @brief Retrieves the user's LFO directory.
     *
     * @return A File object representing the user LFO directory.
     */
    static File getUserLfoDirectory();

    /**
     * @brief Scans a set of directories for files matching certain extensions.
     *
     * @param files An Array<File> to store the results.
     * @param extensions A string specifying the file extensions to look for.
     * @param directories A vector of File objects representing directories to search.
     */
    static void getAllFilesOfTypeInDirectories(Array<File>& files, const String& extensions,
                                               const std::vector<File>& directories);

    /**
     * @brief Retrieves all preset files from preset directories.
     *
     * @param presets An Array<File> to store the found preset files.
     */
    static void getAllPresets(Array<File>& presets);

    /**
     * @brief Retrieves all wavetables from wavetable directories.
     *
     * @param wavetables An Array<File> to store the found wavetables.
     */
    static void getAllWavetables(Array<File>& wavetables);

    /**
     * @brief Retrieves all skins from skin directories.
     *
     * @param skins An Array<File> to store the found skins.
     */
    static void getAllSkins(Array<File>& skins);

    /**
     * @brief Retrieves all LFO shapes from LFO directories.
     *
     * @param lfos An Array<File> to store the found LFO files.
     */
    static void getAllLfos(Array<File>& lfos);

    /**
     * @brief Retrieves all samples (wav files) from sample directories.
     *
     * @param samples An Array<File> to store the found sample files.
     */
    static void getAllSamples(Array<File>& samples);

    /**
     * @brief Retrieves all user presets (from data and user directories).
     *
     * @param presets An Array<File> to store the found user presets.
     */
    static void getAllUserPresets(Array<File>& presets);

    /**
     * @brief Retrieves all user wavetables (from data and user directories).
     *
     * @param wavetables An Array<File> to store the found user wavetables.
     */
    static void getAllUserWavetables(Array<File>& wavetables);

    /**
     * @brief Retrieves all user LFO shapes (from data and user directories).
     *
     * @param lfos An Array<File> to store the found user LFO files.
     */
    static void getAllUserLfos(Array<File>& lfos);

    /**
     * @brief Retrieves all user samples (from data and user directories).
     *
     * @param samples An Array<File> to store the found user samples.
     */
    static void getAllUserSamples(Array<File>& samples);

    /**
     * @brief Compares two feature version strings (ignoring patch-level differences).
     *
     * @param a The first version string.
     * @param b The second version string.
     * @return A comparison integer: < 0 if a < b, 0 if equal, > 0 if a > b.
     */
    static int compareFeatureVersionStrings(String a, String b);

    /**
     * @brief Compares two version strings.
     *
     * @param a The first version string.
     * @param b The second version string.
     * @return A comparison integer: < 0 if a < b, 0 if equal, > 0 if a > b.
     */
    static int compareVersionStrings(String a, String b);

    /**
     * @brief Given a directory name and extensions, returns a file shifted by some offset from the current file.
     *
     * Used for navigating presets or wavetables by stepping forward/backward.
     *
     * @param directory_name The directory name to search.
     * @param extensions The file extensions to consider.
     * @param additional_folders_name The config key for extra directories.
     * @param current_file The current file.
     * @param shift The integer offset to move (e.g., +1 for next file, -1 for previous).
     * @return The shifted File, or a default empty File if none found.
     */
    static File getShiftedFile(const String directory_name, const String& extensions,
                               const std::string& additional_folders_name, const File& current_file, int shift);

private:
    LoadSave() { }
};
