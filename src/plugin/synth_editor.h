/*
Summary:

SynthEditor is the main UI class for a synth plugin. It displays a FullInterface GUI, applies scaling, ensures an aspect ratio, and updates the UI in response to plugin state changes. It manages reading user preferences (e.g., animation, window size) and resizing behavior.
*/

#pragma once

#include "JuceHeader.h"
#include "border_bounds_constrainer.h"
#include "synth_plugin.h"
#include "full_interface.h"
#include "synth_gui_interface.h"

/**
 * @class SynthEditor
 * @brief The main editor component for the SynthPlugin audio processor.
 *
 * SynthEditor is an AudioProcessorEditor that integrates a SynthGuiInterface,
 * providing the user interface for the plugin. It handles resizing,
 * aspect ratio constraints, scaling, and updates to the GUI as the underlying
 * synthesizer changes.
 */
class SynthEditor : public AudioProcessorEditor, public SynthGuiInterface {
public:
    /**
     * @brief Constructs the SynthEditor.
     * @param synth Reference to the SynthPlugin instance associated with this editor.
     *
     * Initializes the look and feel, sets up the main GUI (FullInterface),
     * applies animation settings, and configures resizing constraints and scaling.
     */
    SynthEditor(SynthPlugin& synth);

    /**
     * @brief Paints the editor background.
     * @param g The graphics context used for rendering.
     */
    void paint(Graphics&) override { }

    /**
     * @brief Called when the editor is resized.
     *
     * Adjusts the GUI bounds to match the new editor size.
     */
    void resized() override;

    /**
     * @brief Sets a new scale factor for the GUI.
     * @param newScale The new scaling factor to apply.
     *
     * Adjusts the interface scaling and triggers a background re-render.
     */
    void setScaleFactor(float newScale) override;

    /**
     * @brief Forces a full GUI update.
     *
     * Updates all GUI elements and notifies the host that the display might have changed.
     */
    void updateFullGui() override;

private:
    SynthPlugin& synth_;             ///< Reference to the associated SynthPlugin.
    bool was_animating_;             ///< Stores whether animations were previously enabled.
    BorderBoundsConstrainer constrainer_;  ///< Constrainer ensuring aspect ratio and sizing.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SynthEditor)
};
