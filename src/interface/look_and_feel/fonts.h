#pragma once

#include "JuceHeader.h"

/**
 * @class Fonts
 * @brief A singleton class providing access to a set of custom fonts.
 *
 * This class loads and stores various typefaces used throughout the UI.
 * It provides a consistent source for fonts, including regular, light,
 * title, and monospace variants.
 */
class Fonts {
public:
    /** @brief Destructor. */
    virtual ~Fonts() { }

    /**
     * @brief Returns a reference to the proportional regular font.
     * @return A reference to the Font object.
     */
    Font& proportional_regular() { return proportional_regular_; }

    /**
     * @brief Returns a reference to the proportional light font.
     * @return A reference to the Font object.
     */
    Font& proportional_light() { return proportional_light_; }

    /**
     * @brief Returns a reference to the proportional title (light) font.
     * @return A reference to the Font object.
     */
    Font& proportional_title() { return proportional_title_; }

    /**
     * @brief Returns a reference to the proportional title (regular) font.
     * @return A reference to the Font object.
     */
    Font& proportional_title_regular() { return proportional_title_regular_; }

    /**
     * @brief Returns a reference to the monospace font.
     * @return A reference to the Font object.
     */
    Font& monospace() { return monospace_; }

    /**
     * @brief Gets the singleton instance of the Fonts class.
     * @return Pointer to the singleton Fonts instance.
     */
    static Fonts* instance() {
        static Fonts instance;
        return &instance;
    }

private:
    /**
     * @brief Constructs the Fonts singleton, loading fonts from binary data.
     */
    Fonts();

    Font proportional_regular_;       ///< The proportional regular font.
    Font proportional_light_;         ///< The proportional light font.
    Font proportional_title_;         ///< The proportional title (light) font.
    Font proportional_title_regular_; ///< The proportional title (regular) font.
    Font monospace_;                  ///< A monospace font.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Fonts)
};
