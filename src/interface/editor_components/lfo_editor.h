#pragma once

#include "JuceHeader.h"

#include "line_generator.h"
#include "open_gl_image.h"
#include "line_editor.h"
#include "synth_lfo.h"
#include "synth_module.h"

class SynthGuiInterface;

/**
 * @class LfoEditor
 * @brief An editor component for displaying and editing an LFO (Low-Frequency Oscillator) shape.
 *
 * The LfoEditor allows for interactive manipulation of an LFO curve. Users can set LFO points,
 * adjust the phase, import/export LFO shapes, and apply transformations like flipping horizontally
 * or vertically. This component integrates with the synth’s internal LFO system and updates in
 * real-time based on the current LFO phase and frequency.
 */
class LfoEditor : public LineEditor {
public:
    /// Decay factor used when reducing boost intensity over time.
    static constexpr float kBoostDecay = 0.9f;

    /// Multiplier for adjusting boost decay speed depending on phase changes.
    static constexpr float kSpeedDecayMult = 5.0f;

    enum {
        kSetPhaseToPoint = kNumMenuOptions, ///< Menu option to set the LFO start phase to a point’s phase.
        kSetPhaseToPower,                   ///< Menu option to set the LFO start phase to a segment midpoint.
        kSetPhaseToGrid,                    ///< Menu option to set the LFO start phase to a grid division.
        kImportLfo,                         ///< Menu option to import an LFO shape from file.
        kExportLfo,                         ///< Menu option to export the current LFO shape to file.
    };

    /**
     * @brief Constructs the LfoEditor.
     * @param lfo_source The LineGenerator representing the LFO’s internal data source.
     * @param prefix A string prefix to identify this LFO (e.g., "lfo1").
     * @param mono_modulations Map of mono modulation outputs.
     * @param poly_modulations Map of poly modulation outputs.
     */
    LfoEditor(LineGenerator* lfo_source, String prefix,
              const vital::output_map& mono_modulations, const vital::output_map& poly_modulations);

    /**
     * @brief Destructor.
     */
    virtual ~LfoEditor();

    /**
     * @brief Called when the component’s parent hierarchy changes.
     *
     * Used to find the SynthGuiInterface parent and to locate LFO-related status outputs
     * such as phase and frequency.
     */
    void parentHierarchyChanged() override;

    /**
     * @brief Handles mouse down events.
     * @param e The mouse event.
     *
     * Displays a context menu if the user right-clicks (popup menu). Otherwise, delegates to LineEditor.
     */
    virtual void mouseDown(const MouseEvent& e) override;

    /**
     * @brief Handles mouse double-click events.
     * @param e The mouse event.
     *
     * Double-clicking interacts with the editor points or power handles. Delegates to LineEditor for default behavior.
     */
    virtual void mouseDoubleClick(const MouseEvent& e) override;

    /**
     * @brief Handles mouse up events.
     * @param e The mouse event.
     *
     * If no popup menu was open, delegates to LineEditor’s default behavior.
     */
    virtual void mouseUp(const MouseEvent& e) override;

    /**
     * @brief Responds to selection callbacks from the popup menu.
     * @param point The currently active point index, or -1 if none.
     * @param power The currently active power handle index, or -1 if none.
     * @param result The selected menu item ID.
     */
    void respondToCallback(int point, int power, int result) override;

    /**
     * @brief Sets the LFO’s start phase.
     * @param phase The phase value to set (0.0 to 1.0).
     */
    void setPhase(float phase);

    /**
     * @brief Renders the LFO editor using OpenGL.
     * @param open_gl The OpenGL wrapper.
     * @param animate Whether to animate the LFO curve or not.
     */
    void render(OpenGlWrapper& open_gl, bool animate) override;

private:
    SynthGuiInterface* parent_;          ///< Pointer to the parent SynthGuiInterface.
    const vital::StatusOutput* wave_phase_; ///< Status output for the LFO wave phase.
    const vital::StatusOutput* frequency_;  ///< Status output for the LFO frequency.
    vital::poly_float last_phase_;       ///< The last LFO phase recorded for smooth animation.
    vital::poly_float last_voice_;       ///< The last voice ID recorded to detect voice changes.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LfoEditor)
};
