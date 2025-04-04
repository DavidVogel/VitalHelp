/// @file tempo_selector.h
/// @brief Declares the TempoSelector class, a specialized slider for selecting tempo-related modes.

#pragma once

#include "JuceHeader.h"
#include "synth_slider.h"

/// @class TempoSelector
/// @brief A slider component that allows selection between different tempo modes (seconds, tempo, dotted, triplet, keytrack).
///
/// The TempoSelector integrates with other sliders (e.g., a free-slider for seconds mode, tempo-slider for tempo modes,
/// and keytrack sliders for keytrack mode) to show/hide them depending on the selected mode.
class TempoSelector : public SynthSlider {
public:
    /// @enum MenuId
    /// @brief Identifiers for the different tempo modes.
    enum MenuId {
        kSeconds,       ///< Seconds mode.
        kTempo,         ///< Regular tempo mode.
        kTempoDotted,   ///< Dotted tempo mode.
        kTempoTriplet,  ///< Triplet tempo mode.
        kKeytrack       ///< Keytrack mode.
    };

    /// Constructor.
    /// @param name The name of the TempoSelector.
    TempoSelector(String name);

    /// Handles mouse-down events. Shows a popup menu for selecting the tempo mode if right-clicked.
    /// @param e The mouse event.
    void mouseDown(const MouseEvent& e) override;

    /// Handles mouse-up events.
    /// @param e The mouse event.
    void mouseUp(const MouseEvent& e) override;

    /// Paints the current tempo mode icon (clock, note, etc.).
    /// @param g The Graphics context.
    void paint(Graphics& g) override;

    /// Called when the slider value changes (the mode changes). Shows or hides linked sliders accordingly.
    void valueChanged() override;

    /// Sets the slider to be shown when in "Seconds" mode.
    /// @param slider The slider to show when mode is kSeconds.
    void setFreeSlider(Slider* slider);

    /// Sets the slider to be shown when in a tempo-based mode (not seconds or keytrack).
    /// @param slider The slider to show when in tempo-based modes.
    void setTempoSlider(Slider* slider);

    /// Sets the transpose slider to be shown when in keytrack mode.
    /// @param slider The transpose slider for keytrack mode.
    void setKeytrackTransposeSlider(Slider* slider);

    /// Sets the tune slider to be shown when in keytrack mode.
    /// @param slider The tune slider for keytrack mode.
    void setKeytrackTuneSlider(Slider* slider);

    /// Checks if the current mode is keytrack.
    /// @return True if the mode is keytrack, false otherwise.
    bool isKeytrack() const { return getValue() + 1 == kKeytrack; }

private:
    Slider* free_slider_;              ///< The slider for free mode (seconds).
    Slider* tempo_slider_;             ///< The slider for tempo mode.
    Slider* keytrack_transpose_slider_;///< The slider for keytrack transpose.
    Slider* keytrack_tune_slider_;     ///< The slider for keytrack tuning.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TempoSelector)
};
