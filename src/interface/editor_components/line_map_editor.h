#pragma once

#include "JuceHeader.h"

#include "line_generator.h"
#include "open_gl_image.h"
#include "line_editor.h"
#include "synth_lfo.h"
#include "synth_module.h"

/**
 * @class LineMapEditor
 * @brief A specialized LineEditor that visualizes and optionally animates a line-to-value mapping.
 *
 * The LineMapEditor displays a mapping function or curve and can animate it in real-time based on
 * an input signal (for instance, a parameter or modulation source). It inherits from LineEditor and
 * adds phase-based animation, allowing the visual curve to show a current position indicator that moves
 * along the curve according to an input value.
 */
class LineMapEditor : public LineEditor {
public:
    /// Decay factor for reducing line boost effects over time (for animation smoothing).
    static constexpr float kTailDecay = 0.93f;

    /**
     * @brief Constructs the LineMapEditor.
     * @param line_source The LineGenerator that provides the underlying line data.
     * @param name A name to identify this editor instance.
     */
    LineMapEditor(LineGenerator* line_source, String name);

    /**
     * @brief Destructor.
     */
    virtual ~LineMapEditor();

    /**
     * @brief Called when the component’s parent hierarchy changes.
     *
     * Used to find the parent SynthGuiInterface and locate the status output for raw input values,
     * enabling the editor to animate according to that input.
     */
    void parentHierarchyChanged() override;

    /**
     * @brief Renders the line map with optional animation.
     * @param open_gl The OpenGL wrapper.
     * @param animate Whether to animate the line position based on incoming input.
     */
    virtual void render(OpenGlWrapper& open_gl, bool animate) override;

    /**
     * @brief Enables or disables animation in the line map editor.
     * @param animate True to animate; false otherwise.
     */
    void setAnimate(bool animate) { animate_ = animate; }

private:
    const vital::StatusOutput* raw_input_; ///< The status output providing the raw input value for animation.
    bool animate_;                         ///< Whether animation is currently enabled.
    vital::poly_float last_phase_;         ///< The last recorded phase for smooth animation.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LineMapEditor)
};
