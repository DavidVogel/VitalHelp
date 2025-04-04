#pragma once

#include "JuceHeader.h"
#include "synth_section.h"
#include "open_gl_line_renderer.h"
#include "comb_filter.h"
#include "synth_module.h"

class SynthButton;
class TempoSelector;
class SynthSlider;
class SynthGuiInterface;

/**
 * @class FlangerResponse
 * @brief Renders a visual representation of the flanger effect's filter response.
 *
 * FlangerResponse uses OpenGL to draw the frequency response of a flanger effect. Users can interact
 * with the response graph by clicking and dragging, which adjusts the associated sliders
 * (center frequency and feedback) accordingly.
 */
class FlangerResponse : public OpenGlLineRenderer {
  public:
    /**
     * @brief Number of resolution points used for rendering the response.
     */
    static constexpr int kResolution = 512;

    /**
     * @brief A default sample rate used for visualization purposes.
     */
    static constexpr int kDefaultVisualSampleRate = 200000;

    /**
     * @brief The period used for alternating comb filtering in the displayed response.
     */
    static constexpr int kCombAlternatePeriod = 2;

    /**
     * @brief Constructs the FlangerResponse.
     * @param mono_modulations A map of modulation outputs used to drive certain parameter values.
     */
    FlangerResponse(const vital::output_map& mono_modulations);

    /**
     * @brief Destructor.
     */
    virtual ~FlangerResponse();

    /**
     * @brief Initializes the OpenGL state required for rendering.
     * @param open_gl A reference to the OpenGlWrapper used for OpenGL context management.
     */
    void init(OpenGlWrapper& open_gl) override;

    /**
     * @brief Renders the flanger response curve.
     * @param open_gl A reference to the OpenGlWrapper used for OpenGL context management.
     * @param animate Indicates if any animations or transitions should be handled this frame.
     */
    void render(OpenGlWrapper& open_gl, bool animate) override;

    /**
     * @brief Cleans up and releases OpenGL resources.
     * @param open_gl A reference to the OpenGlWrapper used for OpenGL context management.
     */
    void destroy(OpenGlWrapper& open_gl) override;

    /**
     * @brief Handles mouse down events on the response graph.
     * @param e The mouse event information.
     */
    void mouseDown(const MouseEvent& e) override {
      last_mouse_position_ = e.getPosition();
    }

    /**
     * @brief Handles mouse drag events to adjust flanger parameters interactively.
     * @param e The mouse event information.
     */
    void mouseDrag(const MouseEvent& e) override {
      Point<int> delta = e.getPosition() - last_mouse_position_;
      last_mouse_position_ = e.getPosition();

      float center_range = center_slider_->getMaximum() - center_slider_->getMinimum();
      center_slider_->setValue(center_slider_->getValue() + delta.x * center_range / getWidth());
      float feedback_range = feedback_slider_->getMaximum() - feedback_slider_->getMinimum();
      feedback_slider_->setValue(feedback_slider_->getValue() - delta.y * feedback_range / getHeight());
    }

    /**
     * @brief Associates a SynthSlider with the center frequency parameter.
     * @param slider The SynthSlider controlling the center parameter.
     */
    void setCenterSlider(SynthSlider* slider) { center_slider_ = slider; }

    /**
     * @brief Associates a SynthSlider with the feedback parameter.
     * @param slider The SynthSlider controlling the feedback parameter.
     */
    void setFeedbackSlider(SynthSlider* slider) { feedback_slider_ = slider; }

    /**
     * @brief Associates a SynthSlider with the mix (dry/wet) parameter.
     * @param slider The SynthSlider controlling the dry/wet mix parameter.
     */
    void setMixSlider(SynthSlider* slider) { mix_slider_ = slider; }

    /**
     * @brief Sets the flanger active state, affecting how the response is drawn.
     * @param active True if the flanger is active, false otherwise.
     */
    void setActive(bool active) { active_ = active; }

  private:
    struct FilterResponseShader {
      static constexpr int kMaxStages = 4;
      OpenGLShaderProgram* shader;
      std::unique_ptr<OpenGLShaderProgram::Attribute> position;

      std::unique_ptr<OpenGLShaderProgram::Uniform> mix;
      std::unique_ptr<OpenGLShaderProgram::Uniform> drive;
      std::unique_ptr<OpenGLShaderProgram::Uniform> midi_cutoff;
      std::unique_ptr<OpenGLShaderProgram::Uniform> resonance;
      std::unique_ptr<OpenGLShaderProgram::Uniform> stages[kMaxStages];
    };

    /**
     * @brief Draws the flanger filter response line(s).
     * @param open_gl A reference to the OpenGlWrapper used for context management.
     * @param animate Indicates if animations are needed.
     */
    void drawFilterResponse(OpenGlWrapper& open_gl, bool animate);

    /**
     * @brief Retrieves the total output value from a modulation output or a default value if unavailable.
     * @param output The modulation output to read from.
     * @param default_value The value to use if the output is disabled or null.
     * @return The resulting poly_float value.
     */
    vital::poly_float getOutputTotal(vital::Output* output, vital::poly_float default_value);

    /**
     * @brief Sets up internal filter state variables based on current slider values.
     */
    void setupFilterState();

    /**
     * @brief Configures the shader uniforms for rendering a particular comb filter index.
     * @param index The index of the stage or configuration to load.
     */
    void loadShader(int index);

    /**
     * @brief Binds the necessary OpenGL buffers and vertex arrays for rendering.
     * @param open_gl_context The OpenGL context to bind to.
     */
    void bind(OpenGLContext& open_gl_context);

    /**
     * @brief Unbinds the OpenGL buffers and vertex arrays after rendering.
     * @param open_gl_context The OpenGL context to unbind.
     */
    void unbind(OpenGLContext& open_gl_context);

    /**
     * @brief Computes and sets the line response data for rendering.
     * @param open_gl A reference to the OpenGlWrapper for context management.
     * @param index The index of the line response to render.
     */
    void renderLineResponse(OpenGlWrapper& open_gl, int index);

    SynthGuiInterface* parent_;
    bool active_;
    Point<int> last_mouse_position_;

    vital::CombFilter comb_filter_;
    vital::SynthFilter::FilterState filter_state_;
    vital::poly_float mix_;

    SynthSlider* center_slider_;
    SynthSlider* feedback_slider_;
    SynthSlider* mix_slider_;

    const vital::StatusOutput* flanger_frequency_;
    vital::Output* feedback_output_;
    vital::Output* mix_output_;

    FilterResponseShader response_shader_;
    std::unique_ptr<float[]> line_data_;
    GLuint vertex_array_object_;
    GLuint line_buffer_;
    GLuint response_buffer_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FlangerResponse)
};

/**
 * @class FlangerSection
 * @brief A GUI section representing the flanger effect in the synthesizer.
 *
 * This section displays controls for the flanger effect: enabling/disabling it,
 * adjusting parameters like frequency, tempo sync, feedback, modulation depth,
 * center frequency, phase offset, and dry/wet mix. It also includes a visual
 * response graph (FlangerResponse) to represent the effect's impact on the sound.
 */
class FlangerSection : public SynthSection {
  public:
    /**
     * @brief Constructs a FlangerSection.
     * @param name The component name.
     * @param mono_modulations A map of mono modulation outputs for connecting parameter controls.
     */
    FlangerSection(String name, const vital::output_map& mono_modulations);

    /**
     * @brief Destructor.
     */
    virtual ~FlangerSection();

    /**
     * @brief Paints the background of the flanger section, including labels for each parameter.
     * @param g The graphics context to use for drawing.
     */
    void paintBackground(Graphics& g) override;

    /**
     * @brief Paints the background shadow if active.
     * @param g The graphics context.
     */
    void paintBackgroundShadow(Graphics& g) override { if (isActive()) paintTabShadow(g); }

    /**
     * @brief Positions and sizes the child components when the flanger section is resized.
     */
    void resized() override;

    /**
     * @brief Sets the active state of the flanger section and updates the flanger response accordingly.
     * @param active True to make active, false to deactivate.
     */
    void setActive(bool active) override;

  private:
    std::unique_ptr<SynthButton> on_;
    std::unique_ptr<SynthSlider> frequency_;
    std::unique_ptr<SynthSlider> tempo_;
    std::unique_ptr<TempoSelector> sync_;
    std::unique_ptr<SynthSlider> feedback_;
    std::unique_ptr<SynthSlider> mod_depth_;
    std::unique_ptr<SynthSlider> center_;
    std::unique_ptr<SynthSlider> phase_offset_;
    std::unique_ptr<SynthSlider> dry_wet_;

    std::unique_ptr<FlangerResponse> flanger_response_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FlangerSection)
};

