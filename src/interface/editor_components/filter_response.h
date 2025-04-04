#pragma once

#include "open_gl_line_renderer.h"

#include "skin.h"
#include "comb_filter.h"
#include "digital_svf.h"
#include "diode_filter.h"
#include "dirty_filter.h"
#include "formant_filter.h"
#include "ladder_filter.h"
#include "phaser_filter.h"
#include "sallen_key_filter.h"
#include "synth_types.h"

class SynthSlider;

/**
 * @class FilterResponse
 * @brief Displays the frequency response of various filter models in Vital.
 *
 * FilterResponse visualizes how a chosen filter (analog, digital, ladder, comb, diode, formant, etc.)
 * affects the audio spectrum. It allows real-time interaction by clicking and dragging to change filter
 * cutoff and resonance or formant positions. The rendering is done via OpenGL for efficiency.
 */
class FilterResponse : public OpenGlLineRenderer {
public:
    /// Number of points used for drawing the filter response curve.
    static constexpr int kResolution = 512;
    /// High sample rate used for filter response visualization (not actual audio rate).
    static constexpr int kDefaultVisualSampleRate = 200000;
    /// Period used for alternating patterns (specifically with comb filters).
    static constexpr int kCombAlternatePeriod = 3;

    /// Sensitivity multipliers for mouse interaction along the X and Y axes.
    static constexpr double kMouseSensitivityX = 0.3;
    static constexpr double kMouseSensitivityY = 0.3;

    /**
     * @enum FilterShader
     * @brief An enumeration of different filter shader programs used for rendering.
     */
    enum FilterShader {
        kAnalog,
        kDirty,
        kLadder,
        kDigital,
        kDiode,
        kFormant,
        kComb,
        kPositiveFlange,
        kNegativeFlange,
        kPhase,
        kNumFilterShaders
    };

    /**
     * @brief Constructs a FilterResponse given a suffix and modulation outputs for a single filter configuration.
     * @param suffix The suffix to identify which filter's parameters to use.
     * @param mono_modulations A map of mono modulation outputs.
     */
    FilterResponse(String suffix, const vital::output_map& mono_modulations);

    /**
     * @brief Constructs a FilterResponse for a specific filter index, with both mono and poly modulation outputs.
     * @param index The filter index.
     * @param mono_modulations A map of mono modulation outputs.
     * @param poly_modulations A map of poly modulation outputs.
     */
    FilterResponse(int index, const vital::output_map& mono_modulations, const vital::output_map& poly_modulations);

    /**
     * @brief Destructor.
     */
    virtual ~FilterResponse();

    /**
     * @brief Initializes the OpenGL resources for the filter response visualization.
     * @param open_gl The OpenGL wrapper instance.
     */
    void init(OpenGlWrapper& open_gl) override;

    /**
     * @brief Renders the filter response curve to the screen.
     * @param open_gl The OpenGL wrapper instance.
     * @param animate Whether animations should be performed.
     */
    void render(OpenGlWrapper& open_gl, bool animate) override;

    /**
     * @brief Destroys the OpenGL resources used by this component.
     * @param open_gl The OpenGL wrapper instance.
     */
    void destroy(OpenGlWrapper& open_gl) override;

    /**
     * @brief Draws the background of the component (e.g. coloring, grid lines).
     * @param g The JUCE Graphics context.
     */
    void paintBackground(Graphics& g) override;

    /**
     * @brief Sets the slider controlling the cutoff frequency.
     * @param slider Pointer to the cutoff SynthSlider.
     */
    void setCutoffSlider(SynthSlider* slider) { cutoff_slider_ = slider; }

    /**
     * @brief Sets the slider controlling the filter resonance.
     * @param slider Pointer to the resonance SynthSlider.
     */
    void setResonanceSlider(SynthSlider* slider) { resonance_slider_ = slider; }

    /**
     * @brief Sets the slider controlling the formant X parameter (for formant filter models).
     * @param slider Pointer to the formant X SynthSlider.
     */
    void setFormantXSlider(SynthSlider* slider) { formant_x_slider_ = slider; }

    /**
     * @brief Sets the slider controlling the formant Y parameter (for formant filter models).
     * @param slider Pointer to the formant Y SynthSlider.
     */
    void setFormantYSlider(SynthSlider* slider) { formant_y_slider_ = slider; }

    /**
     * @brief Sets the slider controlling the filter mix parameter.
     * @param slider Pointer to the filter mix SynthSlider.
     */
    void setFilterMixSlider(SynthSlider* slider) { filter_mix_slider_ = slider; }

    /**
     * @brief Sets the slider controlling the blend parameter (e.g., for morphing filter responses).
     * @param slider Pointer to the blend SynthSlider.
     */
    void setBlendSlider(SynthSlider* slider) { blend_slider_ = slider; }

    /**
     * @brief Sets the slider controlling transposition of certain filter parameters.
     * @param slider Pointer to the transpose SynthSlider.
     */
    void setTransposeSlider(SynthSlider* slider) { transpose_slider_ = slider; }

    /**
     * @brief Sets the slider for formant transpose (formant shifting) parameter.
     * @param slider Pointer to the formant transpose SynthSlider.
     */
    void setFormantTransposeSlider(SynthSlider* slider) { formant_transpose_slider_ = slider; }

    /**
     * @brief Sets the slider for formant resonance parameter.
     * @param slider Pointer to the formant resonance SynthSlider.
     */
    void setFormantResonanceSlider(SynthSlider* slider) { formant_resonance_slider_ = slider; }

    /**
     * @brief Sets the slider for formant spread parameter.
     * @param slider Pointer to the formant spread SynthSlider.
     */
    void setFormantSpreadSlider(SynthSlider* slider) { formant_spread_slider_ = slider; }

    /**
     * @brief Handles mouse down events, initiating user interaction with filter parameters.
     * @param e The mouse event.
     */
    void mouseDown(const MouseEvent& e) override;

    /**
     * @brief Handles mouse drag events, allowing interactive adjustment of filter parameters.
     * @param e The mouse event.
     */
    void mouseDrag(const MouseEvent& e) override;

    /**
     * @brief Handles mouse exit events, finalizing interactive changes and hiding tooltips.
     * @param e The mouse event.
     */
    void mouseExit(const MouseEvent& e) override;

    /**
     * @brief Handles mouse wheel events for fine-tuning filter parameters.
     * @param e The mouse event.
     * @param wheel Details about the mouse wheel motion.
     */
    void mouseWheelMove(const MouseEvent& e, const MouseWheelDetails& wheel) override;

    /**
     * @brief Sets whether the filter visualization is active.
     * @param active True to activate visualization, False to disable.
     */
    void setActive(bool active) { active_ = active; }

    /**
     * @brief Sets the currently selected filter model to visualize.
     * @param model The filter model enum value.
     */
    void setModel(vital::constants::FilterModel model) { filter_model_ = model; }

    /**
     * @brief Sets the style of the filter (e.g., 12dB/oct, 24dB/oct).
     * @param style The style integer associated with the filter model.
     */
    void setStyle(int style) { filter_state_.style = style; }

private:
    /**
     * @struct FilterResponseShader
     * @brief Holds references to shader uniforms and attributes used when drawing the filter response.
     */
    struct FilterResponseShader {
        static constexpr int kMaxStages = 5;

        OpenGLShaderProgram* shader;
        std::unique_ptr<OpenGLShaderProgram::Attribute> position;

        std::unique_ptr<OpenGLShaderProgram::Uniform> mix;
        std::unique_ptr<OpenGLShaderProgram::Uniform> midi_cutoff;
        std::unique_ptr<OpenGLShaderProgram::Uniform> resonance;
        std::unique_ptr<OpenGLShaderProgram::Uniform> drive;
        std::unique_ptr<OpenGLShaderProgram::Uniform> db24;
        std::unique_ptr<OpenGLShaderProgram::Uniform> stages[kMaxStages];

        std::unique_ptr<OpenGLShaderProgram::Uniform> formant_cutoff;
        std::unique_ptr<OpenGLShaderProgram::Uniform> formant_resonance;
        std::unique_ptr<OpenGLShaderProgram::Uniform> formant_spread;
        std::unique_ptr<OpenGLShaderProgram::Uniform> formant_low;
        std::unique_ptr<OpenGLShaderProgram::Uniform> formant_band;
        std::unique_ptr<OpenGLShaderProgram::Uniform> formant_high;
    };

    /**
     * @brief Default constructor (private, not used).
     */
    FilterResponse();

    /**
     * @brief Interprets mouse position changes as adjustments to filter cutoff/resonance or formant parameters.
     * @param position The new mouse position.
     */
    void setFilterSettingsFromPosition(Point<int> position);

    /**
     * @brief Draws the filter response curve using the currently active filter model and style.
     * @param open_gl The OpenGL wrapper instance.
     */
    void drawFilterResponse(OpenGlWrapper& open_gl);

    /**
     * @brief Helper method to aggregate mono and poly outputs.
     * @param outputs A pair of pointers to mono and poly outputs.
     * @param default_value The default value if output is not modulated.
     * @return The aggregated poly_float value.
     */
    vital::poly_float getOutputsTotal(std::pair<vital::Output*, vital::Output*> outputs,
                                      vital::poly_float default_value);

    /**
     * @brief Sets up the filter state from the current parameters and returns whether a new response must be computed.
     * @param model The current filter model.
     * @return True if new calculations are required, false otherwise.
     */
    bool setupFilterState(vital::constants::FilterModel model);

    /**
     * @brief Determines if the current filter state is stereo.
     * @return True if it's stereo, false otherwise.
     */
    bool isStereoState();

    /**
     * @brief Loads the appropriate shader with the current filter parameters.
     * @param shader The selected FilterShader enumeration.
     * @param model The current filter model.
     * @param index The voice index (usually 0 or 1 for stereo).
     */
    void loadShader(FilterShader shader, vital::constants::FilterModel model, int index);

    /**
     * @brief Binds the shader and buffer objects before drawing.
     * @param shader The shader to bind.
     * @param open_gl_context The OpenGL context reference.
     */
    void bind(FilterShader shader, OpenGLContext& open_gl_context);

    /**
     * @brief Unbinds the shader and buffer objects after drawing.
     * @param shader The shader to unbind.
     * @param open_gl_context The OpenGL context reference.
     */
    void unbind(FilterShader shader, OpenGLContext& open_gl_context);

    /**
     * @brief Performs the GPU transform feedback operation and updates the response line buffers.
     * @param open_gl The OpenGL wrapper instance.
     */
    void renderLineResponse(OpenGlWrapper& open_gl);

    bool active_;
    bool animate_;
    Point<int> last_mouse_position_;
    double current_resonance_value_;
    double current_cutoff_value_;
    double current_formant_x_value_;
    double current_formant_y_value_;

    Colour line_left_color_;
    Colour line_right_color_;
    Colour line_disabled_color_;
    Colour fill_left_color_;
    Colour fill_right_color_;
    Colour fill_disabled_color_;

    vital::SallenKeyFilter analog_filter_;
    vital::CombFilter comb_filter_;
    vital::DigitalSvf digital_filter_;
    vital::DiodeFilter diode_filter_;
    vital::DirtyFilter dirty_filter_;
    vital::FormantFilter formant_filter_;
    vital::LadderFilter ladder_filter_;
    vital::PhaserFilter phaser_filter_;

    int last_filter_style_;
    vital::constants::FilterModel last_filter_model_;
    vital::constants::FilterModel filter_model_;
    vital::SynthFilter::FilterState filter_state_;
    vital::poly_float mix_;

    SynthSlider* cutoff_slider_;
    SynthSlider* resonance_slider_;
    SynthSlider* formant_x_slider_;
    SynthSlider* formant_y_slider_;
    SynthSlider* filter_mix_slider_;
    SynthSlider* blend_slider_;
    SynthSlider* transpose_slider_;
    SynthSlider* formant_transpose_slider_;
    SynthSlider* formant_resonance_slider_;
    SynthSlider* formant_spread_slider_;

    std::pair<vital::Output*, vital::Output*> filter_mix_outputs_;
    std::pair<vital::Output*, vital::Output*> midi_cutoff_outputs_;
    std::pair<vital::Output*, vital::Output*> resonance_outputs_;
    std::pair<vital::Output*, vital::Output*> blend_outputs_;
    std::pair<vital::Output*, vital::Output*> transpose_outputs_;
    std::pair<vital::Output*, vital::Output*> interpolate_x_outputs_;
    std::pair<vital::Output*, vital::Output*> interpolate_y_outputs_;
    std::pair<vital::Output*, vital::Output*> formant_resonance_outputs_;
    std::pair<vital::Output*, vital::Output*> formant_spread_outputs_;
    std::pair<vital::Output*, vital::Output*> formant_transpose_outputs_;

    FilterResponseShader shaders_[kNumFilterShaders];
    std::unique_ptr<float[]> line_data_;
    GLuint vertex_array_object_;
    GLuint line_buffer_;
    GLuint response_buffer_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FilterResponse)
};
