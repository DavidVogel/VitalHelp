/// @file preset_selector.h
/// @brief Declares the PresetSelector class which provides a UI component for selecting presets.
#pragma once

#include "JuceHeader.h"
#include "synth_section.h"

/// @class PresetSelector
/// @brief A UI component for selecting presets within the synthesizer.
///
/// The PresetSelector class displays the current preset name and provides buttons to cycle through
/// presets. It supports an optional text-only mode and notifies registered listeners when the user
/// interacts with the component.
class PresetSelector : public SynthSection {
  public:
    /// Default ratio of the font height relative to the component's height.
    static constexpr float kDefaultFontHeightRatio = 0.63f;

    /// @class Listener
    /// @brief Interface for objects that want to be notified of PresetSelector events.
    ///
    /// Classes implementing this interface can be registered to the PresetSelector to receive
    /// callbacks when navigation buttons are clicked or when text is clicked.
    class Listener {
      public:
        /// Virtual destructor for proper cleanup.
        virtual ~Listener() { }

        /// Called when the "previous" button is clicked.
        virtual void prevClicked() = 0;

        /// Called when the "next" button is clicked.
        virtual void nextClicked() = 0;

        /// Called when the text area receives a mouse-up event.
        /// @param e The mouse event data.
        virtual void textMouseUp(const MouseEvent& e) { }

        /// Called when the text area receives a mouse-down event.
        /// @param e The mouse event data.
        virtual void textMouseDown(const MouseEvent& e) { }
    };

    /// Constructor.
    PresetSelector();

    /// Destructor.
    ~PresetSelector();

    /// Paints the background of the PresetSelector.
    /// @param g The Graphics context used for painting.
    void paintBackground(Graphics& g) override;

    /// Resizes and lays out the child components.
    void resized() override;

    /// Handles mouse-down events on the PresetSelector.
    /// @param e The mouse event data.
    void mouseDown(const MouseEvent& e) override;

    /// Handles mouse-up events on the PresetSelector.
    /// @param e The mouse event data.
    void mouseUp(const MouseEvent& e) override;

    /// Handles button click events for previous/next preset buttons.
    /// @param clicked_button The button that was clicked.
    void buttonClicked(Button* clicked_button) override;

    /// Called when the mouse enters the PresetSelector area.
    /// @param e The mouse event data.
    void mouseEnter(const MouseEvent& e) override { hover_ = true; }

    /// Called when the mouse leaves the PresetSelector area.
    /// @param e The mouse event data.
    void mouseExit(const MouseEvent& e) override { hover_ = false; }

    /// Sets the displayed text in the PresetSelector.
    /// @param text The new text to display.
    void setText(String text);

    /// Sets the displayed text by combining three separate strings.
    /// @param left The left portion of the text.
    /// @param center The center portion of the text.
    /// @param right The right portion of the text.
    void setText(String left, String center, String right);

    /// Retrieves the displayed text.
    /// @return The currently displayed text.
    String getText() { return text_->getText(); }

    /// Sets the ratio of font height relative to the component's height.
    /// @param ratio The font height ratio.
    void setFontRatio(float ratio) { font_height_ratio_ = ratio; }

    /// Sets the amount of rounding applied to corners.
    /// @param round_amount The rounding amount.
    void setRoundAmount(float round_amount) { round_amount_ = round_amount; }

    /// Adds a listener to receive events from this PresetSelector.
    /// @param listener The listener to add.
    void addListener(Listener* listener) { listeners_.push_back(listener); }

    /// Programmatically simulate a click on the "previous" preset button.
    void clickPrev();

    /// Programmatically simulate a click on the "next" preset button.
    void clickNext();

    /// Sets whether this selector uses a text component layout instead of a default layout.
    /// @param text_component If true, uses text component layout.
    void setTextComponent(bool text_component) { text_component_ = text_component; }

  private:
    /// Helper method to handle text mouse-down events.
    /// @param e The mouse event data.
    void textMouseDown(const MouseEvent& e);

    /// Helper method to handle text mouse-up events.
    /// @param e The mouse event data.
    void textMouseUp(const MouseEvent& e);

    std::vector<Listener*> listeners_;        ///< Registered listeners.
    float font_height_ratio_;                 ///< Ratio for font height.
    float round_amount_;                      ///< Rounding amount for corners.
    bool hover_;                              ///< Indicates mouse hover state.
    bool text_component_;                     ///< Indicates if using text component layout.

    std::unique_ptr<PlainTextComponent> text_;       ///< Displayed text component.
    std::unique_ptr<OpenGlShapeButton> prev_preset_; ///< Previous preset button.
    std::unique_ptr<OpenGlShapeButton> next_preset_; ///< Next preset button.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetSelector)
};

