#pragma once

#include "JuceHeader.h"
#include "wavetable_component_overlay.h"
#include "wave_window_editor.h"
#include "wave_window_modifier.h"

/**
 * @class WaveWindowOverlay
 * @brief An overlay interface component for modifying a windowed section of a wavetable.
 *
 * WaveWindowOverlay provides controls to edit a "window" applied to a wavetable's waveform.
 * It allows setting left/right positions of the window and choosing a window shape. The overlay
 * interacts with a WaveWindowModifier and a WaveWindowEditor to visually and interactively
 * manipulate the window parameters for the currently selected keyframe.
 */
class WaveWindowOverlay : public WavetableComponentOverlay,
                          public WaveWindowEditor::Listener {
public:
    /**
     * @brief Constructs a WaveWindowOverlay with default UI elements.
     *
     * Initializes the window shape selector, left/right position sliders,
     * and the underlying WaveWindowEditor for interactive editing.
     */
    WaveWindowOverlay();

    /**
     * @brief Called when a new keyframe is selected.
     *
     * If the keyframe belongs to the associated WaveWindowModifier, the overlay updates
     * the editor and sliders to reflect that keyframe's window parameters.
     *
     * @param keyframe The newly selected keyframe or nullptr if none selected.
     */
    virtual void frameSelected(WavetableKeyframe* keyframe) override;

    /**
     * @brief Called when a selected keyframe is dragged (not used here).
     *
     * @param keyframe The dragged keyframe.
     * @param position The new position index (unused).
     */
    virtual void frameDragged(WavetableKeyframe* keyframe, int position) override { }

    /**
     * @brief Sets the edit bounds for the controls in this overlay.
     *
     * Positions the window shape selector and left/right sliders within the given rectangle.
     *
     * @param bounds The rectangle area to place the editor controls.
     */
    virtual void setEditBounds(Rectangle<int> bounds) override;

    /**
     * @brief Sets the time domain editor's bounds.
     *
     * Positions the WaveWindowEditor within the given rectangle.
     *
     * @param bounds The rectangle area for the time domain editor.
     * @return True if handled, false otherwise.
     */
    virtual bool setTimeDomainBounds(Rectangle<int> bounds) override;

    /**
     * @brief Called when the window editor notifies a window change.
     *
     * Updates the current keyframe's left/right window positions based on the editor changes.
     *
     * @param left True if the left side changed, false if the right side changed.
     * @param mouse_up True if this was triggered by mouse release.
     */
    void windowChanged(bool left, bool mouse_up) override;

    /**
     * @brief Called when a slider's value changes.
     *
     * Updates the wave window shape or position parameters based on user input.
     *
     * @param moved_slider The slider that changed.
     */
    void sliderValueChanged(Slider* moved_slider) override;

    /**
     * @brief Called when a slider drag operation ends.
     *
     * Finalizes changes and marks them as complete for undo/redo.
     *
     * @param moved_slider The slider that was dragged.
     */
    void sliderDragEnded(Slider* moved_slider) override;

    /**
     * @brief Sets the WaveWindowModifier associated with this overlay.
     *
     * @param wave_window_modifier The WaveWindowModifier to control.
     */
    void setWaveWindowModifier(WaveWindowModifier* wave_window_modifier) {
        wave_window_modifier_ = wave_window_modifier;
        current_frame_ = nullptr;
    }

protected:
    WaveWindowModifier* wave_window_modifier_; ///< The associated WaveWindowModifier.
    WaveWindowModifier::WaveWindowModifierKeyframe* current_frame_; ///< The currently active keyframe.
    std::unique_ptr<WaveWindowEditor> editor_; ///< Editor for adjusting the window parameters.
    std::unique_ptr<TextSelector> window_shape_; ///< Selector for window shape type.
    std::unique_ptr<SynthSlider> left_position_; ///< Slider for the left window position.
    std::unique_ptr<SynthSlider> right_position_; ///< Slider for the right window position.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveWindowOverlay)
};
