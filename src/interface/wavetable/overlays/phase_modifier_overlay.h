#pragma once

#include "JuceHeader.h"
#include "phase_editor.h"
#include "phase_modifier.h"
#include "wavetable_component_overlay.h"
#include "text_selector.h"

/**
 * @class PhaseModifierOverlay
 * @brief Overlay UI for editing a PhaseModifier's parameters in the wavetable editor.
 *
 * This overlay allows adjusting the phase shift and related parameters of a PhaseModifier keyframe.
 * It provides a phase editor for visually editing the phase shift, a text box for manual phase input,
 * and selectors/knobs for controlling phase style and mix.
 */
class PhaseModifierOverlay : public WavetableComponentOverlay,
                             public PhaseEditor::Listener,
                             public TextEditor::Listener {
public:
    /**
     * @brief Constructor.
     *
     * Initializes the overlay with controls for editing phase parameters.
     */
    PhaseModifierOverlay();

    /**
     * @brief Called when a new frame is selected.
     * @param keyframe The selected WavetableKeyframe or nullptr if none.
     *
     * If the keyframe belongs to the assigned PhaseModifier, the overlay updates its controls to reflect
     * the currently selected phase keyframe parameters.
     */
    virtual void frameSelected(WavetableKeyframe* keyframe) override;

    /**
     * @brief Called when a frame is dragged. Not used in this overlay.
     * @param keyframe The dragged keyframe.
     * @param position The new position of the frame (unused).
     */
    virtual void frameDragged(WavetableKeyframe* keyframe, int position) override { }

    /**
     * @brief Set the bounds of editing controls within the overlay.
     * @param bounds The bounding rectangle.
     *
     * Positions the UI controls (phase style selector, phase text input, mix slider).
     */
    virtual void setEditBounds(Rectangle<int> bounds) override;

    /**
     * @brief Set the bounds for the time-domain waveform display area.
     * @param bounds The bounding rectangle for the time-domain display.
     * @return True if successful, false if not used.
     *
     * Positions the phase editor within the specified time-domain area.
     */
    virtual bool setTimeDomainBounds(Rectangle<int> bounds) override;

    /**
     * @brief Called when the user presses 'Enter' in the phase text editor.
     * @param text_editor The text editor that triggered the event.
     *
     * Updates the phase based on the new user input text and notifies changes.
     */
    virtual void textEditorReturnKeyPressed(TextEditor& text_editor) override;

    /**
     * @brief Called when the phase text editor loses focus.
     * @param text_editor The text editor that triggered the event.
     *
     * Updates the phase based on the current text and notifies changes.
     */
    virtual void textEditorFocusLost(TextEditor& text_editor) override;

    /**
     * @brief Callback from PhaseEditor when the phase has changed.
     * @param phase The new phase value in radians.
     * @param mouse_up True if the mouse button was released at this phase change.
     *
     * Updates the current frame's phase and synchronizes with text input and slider visuals.
     */
    void phaseChanged(float phase, bool mouse_up) override;

    /**
     * @brief Called when a slider value changes.
     * @param moved_slider The slider that changed.
     *
     * Updates the phase style or mix based on the slider's new value.
     */
    void sliderValueChanged(Slider* moved_slider) override;

    /**
     * @brief Called when a slider drag operation ends.
     * @param moved_slider The slider that was dragged.
     *
     * Notifies that a final change occurred.
     */
    void sliderDragEnded(Slider* moved_slider) override;

    /**
     * @brief Assign the PhaseModifier whose frames this overlay should edit.
     * @param phase_modifier The PhaseModifier to control.
     *
     * Resets the current frame to null and updates once a frame is selected.
     */
    void setPhaseModifier(PhaseModifier* phase_modifier) {
        phase_modifier_ = phase_modifier;
        current_frame_ = nullptr;
    }

protected:
    /**
     * @brief Sets the phase from a text string (in degrees).
     * @param phase_string A string representing the phase in degrees.
     */
    void setPhase(String phase_string);

    PhaseModifier* phase_modifier_;                               ///< The assigned PhaseModifier.
    PhaseModifier::PhaseModifierKeyframe* current_frame_;          ///< Currently selected frame from the PhaseModifier.
    std::unique_ptr<PhaseEditor> editor_;                         ///< Interactive phase editor.
    std::unique_ptr<PhaseEditor> slider_;                         ///< Phase editor used as a reference slider line.
    std::unique_ptr<TextEditor> phase_text_;                      ///< Text editor for manual phase input.
    std::unique_ptr<TextSelector> phase_style_;                   ///< Selector for phase style.
    std::unique_ptr<SynthSlider> mix_;                            ///< Slider for phase mix amount.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PhaseModifierOverlay)
};
