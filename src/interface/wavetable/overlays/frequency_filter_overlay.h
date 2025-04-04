#pragma once

#include "JuceHeader.h"
#include "wavetable_component_overlay.h"
#include "frequency_filter_modifier.h"

class TextSelector;

/**
 * @class FrequencyFilterOverlay
 * @brief Overlay UI for editing a frequency filter modifier in a wavetable.
 *
 * FrequencyFilterOverlay provides controls for a FrequencyFilterModifier which
 * applies frequency-based transformations to a wavetable. Users can select
 * the filter style, cutoff, and shape parameters, as well as enable normalization.
 */
class FrequencyFilterOverlay : public WavetableComponentOverlay {
  public:
    /**
     * @brief Constructs the FrequencyFilterOverlay and sets up UI components.
     */
    FrequencyFilterOverlay();

    /**
     * @brief Called when a keyframe in the wavetable is selected.
     *
     * If the selected keyframe belongs to the associated frequency_modifier_,
     * the overlay updates its UI to reflect the keyframe's parameters.
     *
     * @param keyframe The selected keyframe, or nullptr if none is selected.
     */
    virtual void frameSelected(WavetableKeyframe* keyframe) override;

    /**
     * @brief Called when a keyframe is dragged, but this overlay does not handle frame dragging.
     *
     * @param keyframe The dragged keyframe.
     * @param position The new position of the keyframe.
     */
    virtual void frameDragged(WavetableKeyframe* keyframe, int position) override { }

    /**
     * @brief Sets the editing bounds for placing UI components.
     *
     * @param bounds The area in which to place this overlay's UI.
     */
    virtual void setEditBounds(Rectangle<int> bounds) override;

    /**
     * @brief Handles setting frequency amplitude bounds, not actively used in this overlay.
     *
     * @param bounds The frequency amplitude bounds.
     * @return true if handled successfully.
     */
    virtual bool setFrequencyAmplitudeBounds(Rectangle<int> bounds) override;

    /**
     * @brief Called when a slider value in this overlay changes.
     *
     * Handles updating the frequency filter modifier with new parameter values.
     *
     * @param moved_slider The slider that changed.
     */
    void sliderValueChanged(Slider* moved_slider) override;

    /**
     * @brief Called when the user finishes interacting with a slider.
     *
     * Used to notify that changes are complete and to finalize state.
     *
     * @param moved_slider The slider that finished interaction.
     */
    void sliderDragEnded(Slider* moved_slider) override;

    /**
     * @brief Called when a button in this overlay is clicked.
     *
     * Currently handles toggling normalization.
     *
     * @param clicked_button The clicked button.
     */
    void buttonClicked(Button* clicked_button) override;

    /**
     * @brief Sets the FrequencyFilterModifier that this overlay edits.
     *
     * @param frequency_modifier The frequency filter modifier instance.
     */
    void setFilterModifier(FrequencyFilterModifier* frequency_modifier) {
        frequency_modifier_ = frequency_modifier;
        current_frame_ = nullptr;
    }

protected:
    FrequencyFilterModifier* frequency_modifier_; ///< The frequency filter modifier being edited.
    FrequencyFilterModifier::FrequencyFilterModifierKeyframe* current_frame_; ///< Current keyframe being edited.

    std::unique_ptr<SynthSlider> cutoff_;   ///< Slider to adjust cutoff frequency.
    std::unique_ptr<SynthSlider> shape_;    ///< Slider to adjust filter shape.
    std::unique_ptr<OpenGlToggleButton> normalize_; ///< Toggle to normalize output.
    std::unique_ptr<TextSelector> style_;   ///< Selector for choosing filter style.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FrequencyFilterOverlay)
};
