#pragma once

#include "JuceHeader.h"

#include <climits>

/**
 * @brief A helper class for handling drag-and-drop of audio files into a JUCE component.
 *
 * AudioFileDropSource integrates with JUCE's FileDragAndDropTarget to respond when a user drags
 * and drops an audio file over a component. It:
 * - Registers basic audio formats with an AudioFormatManager.
 * - Checks if a dropped file is a supported audio format.
 * - Notifies registered listeners when an audio file has been dropped and loaded.
 *
 * Subclasses must implement audioFileLoaded(const File& file) to define custom loading behavior,
 * and can add listeners that also receive audio file load notifications.
 */
class AudioFileDropSource : public FileDragAndDropTarget {
public:
    /**
     * @brief A listener interface for classes interested in receiving audio file load events.
     *
     * Implementing classes receive a callback when an audio file is dropped and recognized by this source.
     */
    class Listener {
    public:
        virtual ~Listener() { }

        /**
         * @brief Called when an audio file is successfully dropped and recognized.
         *
         * @param file The audio file that was loaded.
         */
        virtual void audioFileLoaded(const File& file) = 0;
    };

    /**
     * @brief Constructs an AudioFileDropSource and registers basic audio formats.
     */
    AudioFileDropSource() {
        format_manager_.registerBasicFormats();
    }

    /**
     * @brief Checks if the drag operation includes exactly one file and if it matches supported audio formats.
     *
     * @param files The list of files being dragged.
     * @return True if there's a single file and it matches a known audio format, false otherwise.
     */
    bool isInterestedInFileDrag(const StringArray& files) override {
        if (files.size() != 1)
            return false;

        String file = files[0];
        StringArray wildcards;
        wildcards.addTokens(getExtensions(), ";", "\"");
        for (const String& wildcard : wildcards) {
            if (file.matchesWildcard(wildcard, true))
                return true;
        }
        return false;
    }

    /**
     * @brief Called when files are dropped onto the component.
     *
     * If at least one file is dropped, it calls audioFileLoaded with the first file and notifies all listeners.
     *
     * @param files The array of dropped files.
     * @param x The x-coordinate of the drop (unused).
     * @param y The y-coordinate of the drop (unused).
     */
    void filesDropped(const StringArray& files, int x, int y) override {
        if (files.size() == 0)
            return;

        File file(files[0]);
        audioFileLoaded(file);
        for (Listener* listener : listeners_)
            listener->audioFileLoaded(file);
    }

    /**
     * @brief Called internally when a recognized audio file is dropped. Must be implemented by subclasses.
     *
     * @param file The dropped audio file.
     */
    virtual void audioFileLoaded(const File& file) = 0;

    /**
     * @brief Adds a listener to receive audio file load notifications.
     *
     * @param listener The listener to add.
     */
    void addListener(Listener* listener) { listeners_.push_back(listener); }

    /**
     * @brief Gets a wildcard pattern representing all supported audio formats.
     *
     * @return A string containing wildcard patterns for all recognized audio formats (e.g. "*.wav;*.aif").
     */
    String getExtensions() { return format_manager_.getWildcardForAllFormats(); }

    /**
     * @brief Provides access to the underlying AudioFormatManager.
     *
     * @return A reference to the AudioFormatManager.
     */
    AudioFormatManager& formatManager() { return format_manager_; }

protected:
    AudioFormatManager format_manager_; ///< Manages and recognizes different audio file formats.

private:
    std::vector<Listener*> listeners_; ///< Registered listeners for file load events.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioFileDropSource)
};
