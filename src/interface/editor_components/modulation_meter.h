#pragma once

#include "JuceHeader.h"
#include "common.h"

class OpenGlMultiQuad;
class SynthSlider;
namespace vital {
    struct Output;
}

/**
 * @class ModulationMeter
 * @brief A visual component that displays the current modulation amount applied to a slider parameter.
 *
 * The ModulationMeter shows how a parameter (represented by a SynthSlider) is modulated by one or more sources.
 * It can display modulation in either a linear or rotary style, depending on the slider it is associated with.
 * The meter updates dynamically based on the current modulation values (mono and poly outputs) and shows the
 * modulated range of the parameter.
 */
class ModulationMeter : public Component {
public:
    /**
     * @brief Constructs a ModulationMeter.
     * @param mono_total A pointer to the mono modulation output associated with this parameter.
     * @param poly_total A pointer to the poly modulation output associated with this parameter.
     * @param slider The SynthSlider representing the parameter being modulated.
     * @param quads A pointer to an OpenGlMultiQuad used for rendering.
     * @param index The index of the quad within the OpenGlMultiQuad to update.
     */
    ModulationMeter(const vital::Output* mono_total, const vital::Output* poly_total,
                    const SynthSlider* slider, OpenGlMultiQuad* quads, int index);

    /**
     * @brief Destructor.
     */
    virtual ~ModulationMeter();

    /**
     * @brief Called when the component is resized.
     *
     * Adjusts the vertices of the modulation meter to fit the new size and updates its state.
     */
    void resized() override;

    /**
     * @brief Sets whether the modulation meter is active (visible and updating).
     * @param active True to activate, false to deactivate.
     */
    void setActive(bool active);

    /**
     * @brief Updates the drawing of the modulation meter, recalculating mod percent and visuals.
     * @param use_poly If true, includes poly outputs in the calculation; otherwise mono only.
     */
    void updateDrawing(bool use_poly);

    /**
     * @brief Sets the given quad to represent a modulation amount line or arc.
     * @param quad The OpenGlQuad to configure.
     * @param amount The amount of modulation to display (e.g., a fraction of a parameter's range).
     * @param bipolar If true, treat the amount as bipolar (centered), otherwise unipolar.
     */
    void setModulationAmountQuad(OpenGlQuad& quad, float amount, bool bipolar);

    /**
     * @brief Sets up the quad coordinates for the modulation amount indicator.
     * @param quad The OpenGlQuad representing the modulation amount indicator.
     */
    void setAmountQuadVertices(OpenGlQuad& quad);

    /**
     * @brief Checks whether there are any modulations on the associated parameter.
     * @return True if the parameter is modulated, false otherwise.
     */
    bool isModulated() const { return modulated_; }

    /**
     * @brief Checks if the associated parameter control is a rotary knob.
     * @return True if rotary; false if linear or other type.
     */
    bool isRotary() const { return rotary_; }

    /**
     * @brief Sets the modulated state of the meter.
     * @param modulated True if the parameter is modulated.
     */
    void setModulated(bool modulated) { modulated_ = modulated; }

    /**
     * @brief Gets the current modulation percentage.
     * @return A poly_float containing the modulation percentage values.
     */
    vital::poly_float getModPercent() { return mod_percent_; }

    /**
     * @brief Gets the destination slider that the meter is visualizing.
     * @return A pointer to the SynthSlider representing the parameter.
     */
    const SynthSlider* destination() { return destination_; }

private:
    ModulationMeter() = delete; ///< Deleted default constructor.

    /**
     * @brief Gets the rectangle bounds of the meter area relative to the component.
     * @return A Rectangle<float> describing the meter area.
     */
    Rectangle<float> getMeterBounds();

    /**
     * @brief Sets up the OpenGL vertices to represent the current meter area and modulation range.
     */
    void setVertices();

    /**
     * @brief Collapses the vertices to a minimal representation when inactive or hidden.
     */
    void collapseVertices();

    const vital::Output* mono_total_;      ///< Mono modulation output.
    const vital::Output* poly_total_;      ///< Poly modulation output (optional).
    const SynthSlider* destination_;       ///< The slider parameter being modulated.

    OpenGlMultiQuad* quads_; ///< The OpenGL multi-quad used for rendering the meter.
    int index_;              ///< The index of this meter within the multi-quad.

    vital::poly_float current_value_; ///< The current modulation-influenced value.
    vital::poly_float mod_percent_;   ///< The modulation percentage relative to the parameter range.

    bool modulated_; ///< True if there are modulation connections affecting the parameter.
    bool rotary_;     ///< True if the parameter is represented by a rotary knob.

    float left_, right_, top_, bottom_; ///< Cached vertex coordinates for rendering.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModulationMeter)
};
