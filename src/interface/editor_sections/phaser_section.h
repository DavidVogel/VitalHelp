#pragma once

#include "JuceHeader.h"
#include "synth_section.h"
#include "open_gl_line_renderer.h"
#include "phaser_filter.h"
#include "synth_module.h"

class SynthButton;
class TempoSelector;
class SynthSlider;
class SynthGuiInterface;

/**
 * @class PhaserResponse
 * @brief Visualizes the frequency response of the phaser effect.
 *
 * This class uses OpenGL to render a phaser filter's response curve in real-time.
 * It supports user interaction through mouse dragging on the visual to change
 * parameters such as cutoff and resonance.
 */
class PhaserResponse : public OpenGlLineRenderer {
public:
    /** @brief The resolution of the frequency response drawing. */
    static constexpr int kResolution = 256;
    /** @brief The default visual sample rate used for filter calculations. */
    static constexpr int kDefaultVisualSampleRate = 200000;

    /**
     * @brief Constructs a PhaserResponse object.
     * @param mono_modulations A map of mono modulation outputs used for the phaser parameters.
     */
    PhaserResponse(const vital::output_map& mono_modulations);

    /** @brief Destructor. */
    virtual ~PhaserResponse();

    /**
     * @brief Initializes the OpenGL context and shaders for rendering the phaser response.
     * @param open_gl Reference to the OpenGlWrapper for the current OpenGL context.
     */
    void init(OpenGlWrapper& open_gl) override;

    /**
     * @brief Renders the phaser response curve.
     * @param open_gl Reference to the OpenGlWrapper for the current OpenGL context.
     * @param animate Whether to animate parameter changes.
     */
    void render(OpenGlWrapper& open_gl, bool animate) override;

    /**
     * @brief Cleans up the OpenGL resources used by this class.
     * @param open_gl Reference to the OpenGlWrapper for the current OpenGL context.
     */
    void destroy(OpenGlWrapper& open_gl) override;

    /**
     * @brief Handles mouse down events on the phaser response view.
     * @param e MouseEvent reference containing event details.
     */
    void mouseDown(const MouseEvent& e) override {
        last_mouse_position_ = e.getPosition();
    }

    /**
     * @brief Handles mouse drag events to adjust phaser parameters (cutoff/resonance).
     * @param e MouseEvent reference containing event details.
     */
    void mouseDrag(const MouseEvent& e) override {
        Point<int> delta = e.getPosition() - last_mouse_position_;
        last_mouse_position_ = e.getPosition();

        float cutoff_range = cutoff_slider_->getMaximum() - cutoff_slider_->getMinimum();
        cutoff_slider_->setValue(cutoff_slider_->getValue() + delta.x * cutoff_range / getWidth());
        float resonance_range = resonance_slider_->getMaximum() - resonance_slider_->getMinimum();
        resonance_slider_->setValue(resonance_slider_->getValue() - delta.y * resonance_range / getHeight());
    }

    /**
     * @brief Sets the cutoff slider that controls the phaser cutoff.
     * @param slider Pointer to the SynthSlider used for cutoff control.
     */
    void setCutoffSlider(SynthSlider* slider) { cutoff_slider_ = slider; }

    /**
     * @brief Sets the resonance slider that controls the phaser resonance.
     * @param slider Pointer to the SynthSlider used for resonance control.
     */
    void setResonanceSlider(SynthSlider* slider) { resonance_slider_ = slider; }

    /**
     * @brief Sets the blend slider that controls the phaser pass blend parameter.
     * @param slider Pointer to the SynthSlider used for blend control.
     */
    void setBlendSlider(SynthSlider* slider) { blend_slider_ = slider; }

    /**
     * @brief Sets the mix slider that controls the dry/wet mix of the phaser.
     * @param slider Pointer to the SynthSlider used for mix control.
     */
    void setMixSlider(SynthSlider* slider) { mix_slider_ = slider; }

    /**
     * @brief Sets whether this phaser response visualization is active.
     * @param active True if active, false if not.
     */
    void setActive(bool active) { active_ = active; }

    /**
     * @brief Sets the filter style.
     * @param style The filter style index.
     */
    void setStyle(int style) { filter_state_.style = style; }

    /**
     * @brief Sets the default blend setting for the phaser.
     * @param blend The default blend value.
     */
    void setDefaultBlend(vital::poly_float blend) { blend_setting_ = blend; }

private:
    /**
     * @struct FilterResponseShader
     * @brief Holds references to shader uniforms and attributes for rendering the filter response.
     */
    struct FilterResponseShader {
        static constexpr int kMaxStages = 3;
        OpenGLShaderProgram* shader;
        std::unique_ptr<OpenGLShaderProgram::Attribute> position;

        std::unique_ptr<OpenGLShaderProgram::Uniform> mix;
        std::unique_ptr<OpenGLShaderProgram::Uniform> midi_cutoff;
        std::unique_ptr<OpenGLShaderProgram::Uniform> resonance;
        std::unique_ptr<OpenGLShaderProgram::Uniform> db24;
        std::unique_ptr<OpenGLShaderProgram::Uniform> stages[kMaxStages];
    };

    /**
     * @brief Draws the phaser's filter response curve using the current shader and state.
     * @param open_gl Reference to the OpenGlWrapper.
     * @param animate True if animation is desired.
     */
    void drawFilterResponse(OpenGlWrapper& open_gl, bool animate);

    /**
     * @brief Retrieves an output value if available, otherwise returns a default value.
     * @param output Pointer to a vital::Output object.
     * @param default_value The default value if the output is not enabled.
     * @return The current value from the output or the default value.
     */
    vital::poly_float getOutputTotal(const vital::Output* output, vital::poly_float default_value);

    /**
     * @brief Sets up the internal filter state from current sliders and modulation outputs.
     */
    void setupFilterState();

    /**
     * @brief Loads shader parameters for a specific channel index.
     * @param index The channel index to load parameters for.
     */
    void loadShader(int index);

    /**
     * @brief Binds OpenGL buffers and sets up attribute pointers.
     * @param open_gl_context Reference to the OpenGLContext.
     */
    void bind(OpenGLContext& open_gl_context);

    /**
     * @brief Unbinds OpenGL buffers and attribute pointers.
     * @param open_gl_context Reference to the OpenGLContext.
     */
    void unbind(OpenGLContext& open_gl_context);

    /**
     * @brief Renders the line response into a transform feedback buffer and updates the line data.
     * @param open_gl Reference to the OpenGlWrapper.
     */
    void renderLineResponse(OpenGlWrapper& open_gl);

    SynthGuiInterface* parent_;
    bool active_;
    Point<int> last_mouse_position_;

    vital::PhaserFilter phaser_filter_;
    vital::SynthFilter::FilterState filter_state_;
    vital::poly_float mix_;

    SynthSlider* cutoff_slider_;
    SynthSlider* resonance_slider_;
    SynthSlider* blend_slider_;
    SynthSlider* mix_slider_;

    const vital::StatusOutput* phaser_cutoff_;
    const vital::Output* filter_mix_output_;
    const vital::Output* resonance_output_;
    const vital::Output* blend_output_;

    vital::poly_float blend_setting_;

    FilterResponseShader response_shader_;
    std::unique_ptr<float[]> line_data_;
    GLuint vertex_array_object_;
    GLuint line_buffer_;
    GLuint response_buffer_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PhaserResponse)
};

/**
 * @class PhaserSection
 * @brief A UI section for controlling a phaser effect in the synthesizer.
 *
 * This class provides controls for phaser parameters like frequency, tempo sync,
 * feedback, blend, and more. It also incorporates a PhaserResponse visualization
 * to display the frequency response of the phaser in real-time.
 */
class PhaserSection : public SynthSection {
public:
    /**
     * @brief Constructs a PhaserSection.
     * @param name The name of this section.
     * @param mono_modulations A map of mono modulation outputs relevant to the phaser parameters.
     */
    PhaserSection(String name, const vital::output_map& mono_modulations);

    /** @brief Destructor. */
    virtual ~PhaserSection();

    /**
     * @brief Paints the background of the phaser section, including labels and backgrounds.
     * @param g The Graphics context to draw into.
     */
    void paintBackground(Graphics& g) override;

    /**
     * @brief Paints the background shadow if the section is active.
     * @param g The Graphics context.
     */
    void paintBackgroundShadow(Graphics& g) override { if (isActive()) paintTabShadow(g); }

    /**
     * @brief Called when the component is resized.
     */
    void resized() override;

    /**
     * @brief Sets the active state of the phaser section.
     * @param active True to activate the phaser section, false otherwise.
     */
    void setActive(bool active) override;

private:
    std::unique_ptr<SynthButton> on_;
    std::unique_ptr<SynthSlider> frequency_;
    std::unique_ptr<SynthSlider> tempo_;
    std::unique_ptr<TempoSelector> sync_;
    std::unique_ptr<SynthSlider> feedback_;
    std::unique_ptr<SynthSlider> center_;
    std::unique_ptr<SynthSlider> mod_depth_;
    std::unique_ptr<SynthSlider> phase_offset_;
    std::unique_ptr<SynthSlider> dry_wet_;
    std::unique_ptr<SynthSlider> blend_;

    std::unique_ptr<PhaserResponse> phaser_response_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PhaserSection)
};
