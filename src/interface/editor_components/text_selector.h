/// @file text_selector.h
/// @brief Declares the TextSelector class and PaintPatternSelector class for selecting text-based options and displaying patterns.

#pragma once

#include "JuceHeader.h"
#include "synth_slider.h"

/// @class TextSelector
/// @brief A specialized SynthSlider that displays a popup menu of text options.
///
/// The TextSelector uses the slider value as an index into a string lookup table.
/// When clicked, it shows a popup menu for the user to select a textual option.
class TextSelector : public SynthSlider {
public:
    /// Constructor.
    /// @param name The name of the TextSelector.
    TextSelector(String name);

    /// Handles mouse-down events. If not a right-click context menu, shows a popup with text options.
    /// @param e The mouse event.
    void mouseDown(const MouseEvent& e) override;

    /// Handles mouse-up events. Specifically handles context-menu logic.
    /// @param e The mouse event.
    void mouseUp(const MouseEvent& e) override;

    /// Determines whether a popup should be shown on hover.
    /// @return False, as the TextSelector only shows popup on click.
    bool shouldShowPopup() override { return false; }

    /// Sets an alternate lookup table for longer text strings.
    /// @param lookup The alternate (long) lookup array of strings.
    void setLongStringLookup(const std::string* lookup) { long_lookup_ = lookup; }

protected:
    const std::string* long_lookup_; ///< Optional alternate lookup table for longer strings.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TextSelector)
};

/// @class PaintPatternSelector
/// @brief A specialized TextSelector that draws a visual pattern instead of text.
///
/// The PaintPatternSelector uses the slider value as an index to determine a paint pattern
/// and draws it. This can visually represent waveforms or other patterns based on the selected index.
class PaintPatternSelector : public TextSelector {
public:
    /// Constructor.
    /// @param name The name of the PaintPatternSelector.
    PaintPatternSelector(String name) : TextSelector(name), padding_(0) { }

    /// Paints the pattern corresponding to the current slider value.
    /// @param g The graphics context.
    void paint(Graphics& g) override;

    /// Sets the padding around the drawn pattern.
    /// @param padding The padding in pixels.
    void setPadding(int padding) { padding_ = padding; }

private:
    int padding_; ///< The padding around the drawn pattern.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PaintPatternSelector)
};
