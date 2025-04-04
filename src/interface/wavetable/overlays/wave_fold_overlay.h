#pragma once

#include "JuceHeader.h"
#include "wavetable_component_overlay.h"
#include "wave_fold_modifier.h"

/**
 * @class WaveFoldOverlay
 * @brief An overlay for controlling a WaveFoldModifier in the wavetable editor.
 *
 * This overlay provides a single rotary control that allows adjusting the amount of wave folding
 * (multiplying the waveform amplitude) applied by a WaveFoldModifier instance to a wavetable frame.
 */
class WaveFoldOverlay : public WavetableComponentOverlay {
public:
    /**
     * @brief Constructor.
     *
     * Initializes the overlay with a rotary slider for controlling the wave folding amount.
     */
    WaveFoldOverlay();

    /**
     * @brief Called when a new frame is selected in the wavetable editor.
     * @param keyframe The newly selected WavetableKeyframe or nullptr if none.
     *
     * If the frame belongs to the associated WaveFoldModifier, updates the slider to the frame's stored value.
     */
    virtual void frameSelected(WavetableKeyframe* keyframe) override;

    /**
     * @brief Called when a frame is dragged, but this overlay does not act on frame drag events.
     * @param keyframe The dragged WavetableKeyframe.
     * @param position The new position of the frame.
     */
    virtual void frameDragged(WavetableKeyframe* keyframe, int position) override { }

    /**
     * @brief Sets the bounds of the editing area in the overlay.
     * @param bounds The rectangular area representing the editing UI space.
     *
     * Positions and resizes the rotary slider control within the provided bounds.
     */
    virtual void setEditBounds(Rectangle<int> bounds) override;

    /**
     * @brief Handles changes to the slider's value.
     * @param moved_slider The slider that changed.
     *
     * Updates the selected keyframe's wave fold boost value and notifies that changes have occurred.
     */
    void sliderValueChanged(Slider* moved_slider) override;

    /**
     * @brief Called when the user finishes interacting with the slider.
     * @param moved_slider The slider whose interaction ended.
     *
     * Notifies that the user has finished making changes so they can be committed or recorded.
     */
    void sliderDragEnded(Slider* moved_slider) override;

    /**
     * @brief Sets the WaveFoldModifier to be controlled by this overlay.
     * @param wave_fold_modifier Pointer to the WaveFoldModifier instance.
     */
    void setWaveFoldModifier(WaveFoldModifier* wave_fold_modifier) {
        wave_fold_modifier_ = wave_fold_modifier;
        current_frame_ = nullptr;
    }

protected:
    WaveFoldModifier* wave_fold_modifier_; ///< The associated WaveFoldModifier instance.
    WaveFoldModifier::WaveFoldModifierKeyframe* current_frame_; ///< Currently selected frame data.

    std::unique_ptr<SynthSlider> wave_fold_amount_; ///< Slider controlling the wave folding amount.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveFoldOverlay)
};
