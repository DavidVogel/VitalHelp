/// @file transpose_quantize.h
/// @brief Declares classes for selecting and applying transpose quantization to notes.

#pragma once

#include "JuceHeader.h"
#include "common.h"
#include "open_gl_image_component.h"
#include "synth_section.h"

/// @class TransposeQuantizeCallOut
/// @brief A callout component that allows selecting which notes are quantized for transposition.
///
/// The TransposeQuantizeCallOut shows an arrangement of notes (like piano keys) and lets
/// the user enable or disable each note for quantization. It also includes a global snap
/// toggle button. When the selection changes, registered listeners are notified.
class TransposeQuantizeCallOut : public SynthSection {
public:
    /// The ratio of the title height to the total height.
    static constexpr float kTitleHeightRatio = 0.2f;
    /// The ratio of the global section height to the total height.
    static constexpr float kGlobalHeightRatio = 0.2f;
    /// The ratio of the title text size to the title height.
    static constexpr float kTitleTextRatio = 0.7f;

    /// @class Listener
    /// @brief Interface for objects that want to be notified when quantization changes.
    class Listener {
    public:
        /// Virtual destructor for proper cleanup.
        virtual ~Listener() { }

        /// Called when the quantization selection or global snap changes.
        virtual void quantizeUpdated() = 0;
    };

    /// Constructor.
    /// @param selected A pointer to an array indicating which notes are selected.
    /// @param global_snap A pointer to a boolean that indicates if global snap is enabled.
    TransposeQuantizeCallOut(bool* selected, bool* global_snap);

    /// Paints the callout, including the title, global snap button, and selected notes.
    /// @param g The graphics context.
    void paint(Graphics& g) override;

    /// Resizes and lays out the components within the callout.
    void resized() override;

    /// Handles mouse events to enable/disable notes.
    void mouseDown(const MouseEvent& e) override;
    void mouseDrag(const MouseEvent& e) override;
    void mouseMove(const MouseEvent& e) override;
    void mouseExit(const MouseEvent& e) override;

    /// Handles button click events, specifically the global snap toggle.
    /// @param clicked_button The button that was clicked.
    void buttonClicked(Button* clicked_button) override;

    /// Adds a listener to be notified when the quantization selection changes.
    /// @param listener The listener to add.
    void addQuantizeListener(Listener* listener) { listeners_.push_back(listener); }

private:
    /// Determines which note is under the mouse position.
    /// @param e The mouse event.
    /// @return The index of the hovered note or -1 if none is hovered.
    int getHoverIndex(const MouseEvent& e);

    /// Notifies all registered listeners that the quantization selection changed.
    void notify() {
        for (Listener* listener : listeners_)
            listener->quantizeUpdated();
    }

    std::vector<Listener*> listeners_;         ///< Listeners for quantization updates.
    Rectangle<float> key_bounds_[vital::kNotesPerOctave]; ///< Bounds of each note region.
    std::unique_ptr<ToggleButton> global_snap_button_;    ///< Button for global snap toggling.
    bool* selected_;     ///< Pointer to an array of selected notes.
    bool* global_snap_;  ///< Pointer to the global snap boolean.
    int hover_index_;    ///< Currently hovered note index.
    bool enabling_;      ///< True if currently enabling notes via drag.
    bool disabling_;     ///< True if currently disabling notes via drag.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TransposeQuantizeCallOut)
};

/// @class TransposeQuantizeButton
/// @brief A button component that shows the current transpose quantization state and opens a callout to edit it.
///
/// The TransposeQuantizeButton displays a set of notes (like keys) and which ones are
/// active in the quantization. When clicked, it launches a TransposeQuantizeCallOut
/// for editing. It can notify its listeners about quantization changes.
class TransposeQuantizeButton : public OpenGlImageComponent, public TransposeQuantizeCallOut::Listener {
public:
    /// @class Listener
    /// @brief Interface for objects that want to be notified of quantization changes.
    class Listener {
    public:
        /// Virtual destructor.
        virtual ~Listener() { }

        /// Called when the quantization state changes.
        virtual void quantizeUpdated() = 0;
    };

    /// Constructor.
    TransposeQuantizeButton();

    /// Paints the notes and their selection state.
    /// @param g The graphics context.
    void paint(Graphics& g) override;

    /// Overrides but does nothing for background painting.
    void paintBackground(Graphics& g) override { }

    /// Handles resizing to compute note layout.
    void resized() override;

    /// Shows the callout for editing quantization when clicked.
    /// @param e The mouse event.
    void mouseDown(const MouseEvent& e) override;

    /// Handles mouse enter event for hover state.
    /// @param e The mouse event.
    void mouseEnter(const MouseEvent& e) override;

    /// Handles mouse exit event for hover state.
    /// @param e The mouse event.
    void mouseExit(const MouseEvent& e) override;

    /// Called when quantization updates occur in the callout.
    void quantizeUpdated() override;

    /// Gets the current quantization value as a bitmask.
    /// @return The quantization state value.
    int getValue();

    /// Sets the quantization value from a bitmask.
    /// @param value The new quantization state value.
    void setValue(int value);

    /// Adds a listener to be notified of quantization changes.
    /// @param listener The listener to add.
    void addQuantizeListener(Listener* listener) { listeners_.push_back(listener); }

private:
    std::vector<Listener*> listeners_;         ///< Listeners to notify about changes.
    bool selected_[vital::kNotesPerOctave];    ///< Which notes are currently selected.
    bool global_snap_;                         ///< If global snap is enabled.
    Rectangle<float> key_bounds_[vital::kNotesPerOctave]; ///< Bounds for each note.
    bool hover_;                               ///< Whether the mouse is hovering over the button.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TransposeQuantizeButton)
};
