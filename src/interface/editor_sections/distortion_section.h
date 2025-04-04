/// @file distortion_section.h
/// @brief Declares the DistortionSection class and related components for displaying and controlling a distortion effect.

#pragma once

#include "JuceHeader.h"
#include "open_gl_line_renderer.h"
#include "synth_section.h"
#include "digital_svf.h"

class SynthButton;
class SynthSlider;
class TabSelector;
class TextSelector;
class DistortionViewer;

/// @class DistortionFilterResponse
/// @brief An OpenGL renderer showing the frequency response of the distortion's filter.
///
/// This viewer uses a DigitalSvf filter model and shaders to render the frequency response. It allows
/// dragging the mouse to adjust cutoff and resonance. The user can interact with the filter curve
/// to visually adjust filter parameters.
class DistortionFilterResponse : public OpenGlLineRenderer {
  public:
    static constexpr int kResolution = 256;              ///< Resolution of the filter response line.
    static constexpr int kDefaultVisualSampleRate = 200000; ///< Default sample rate for visualization calculations.

    /// Constructor.
    /// @param mono_modulations A map of mono modulation outputs from the synth.
    DistortionFilterResponse(const vital::output_map& mono_modulations);
    virtual ~DistortionFilterResponse();

    /// Initializes OpenGL resources.
    /// @param open_gl The OpenGlWrapper managing OpenGL state.
    void init(OpenGlWrapper& open_gl) override;

    /// Renders the filter response line each frame.
    /// @param open_gl The OpenGlWrapper.
    /// @param animate If true, animates transitions.
    void render(OpenGlWrapper& open_gl, bool animate) override;

    /// Destroys OpenGL resources.
    /// @param open_gl The OpenGlWrapper.
    void destroy(OpenGlWrapper& open_gl) override;

    /// Called when the mouse is pressed down. Stores the initial mouse position.
    /// @param e The mouse event.
    void mouseDown(const MouseEvent& e) override {
      last_mouse_position_ = e.getPosition();
    }

    /// Called when the mouse is dragged. Adjusts cutoff and resonance based on mouse movement.
    /// @param e The mouse event.
    void mouseDrag(const MouseEvent& e) override {
      Point<int> delta = e.getPosition() - last_mouse_position_;
      last_mouse_position_ = e.getPosition();

      float cutoff_range = cutoff_slider_->getMaximum() - cutoff_slider_->getMinimum();
      cutoff_slider_->setValue(cutoff_slider_->getValue() + delta.x * cutoff_range / getWidth());
      float resonance_range = resonance_slider_->getMaximum() - resonance_slider_->getMinimum();
      resonance_slider_->setValue(resonance_slider_->getValue() - delta.y * resonance_range / getHeight());
    }

    /// Sets the slider controlling the filter cutoff.
    /// @param slider The cutoff slider.
    void setCutoffSlider(SynthSlider* slider) { cutoff_slider_ = slider; }

    /// Sets the slider controlling the filter resonance.
    /// @param slider The resonance slider.
    void setResonanceSlider(SynthSlider* slider) { resonance_slider_ = slider; }

    /// Sets the slider controlling the filter blend.
    /// @param slider The blend slider.
    void setBlendSlider(SynthSlider* slider) { blend_slider_ = slider; }

    /// Sets whether this viewer is active.
    /// @param active True if active, false otherwise.
    void setActive(bool active) { active_ = active; }

  private:
    /// Internal struct holding shader-related objects for rendering filter response.
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
    };

    /// Draws the filter response lines using the current filter settings.
    /// @param open_gl The OpenGlWrapper.
    /// @param animate If true, animates transitions.
    void drawFilterResponse(OpenGlWrapper& open_gl, bool animate);

    /// Gets the current output total for a given output, defaulting to a specified value if not enabled.
    vital::poly_float getOutputTotal(vital::Output* output, vital::poly_float default_value);

    /// Sets up the internal filter state from the sliders and outputs.
    void setupFilterState();

    /// Loads the shader settings for a given channel index.
    void loadShader(int index);

    /// Binds VAO and buffers for rendering.
    /// @param open_gl_context The OpenGLContext.
    void bind(OpenGLContext& open_gl_context);

    /// Unbinds VAO and buffers after rendering.
    /// @param open_gl_context The OpenGLContext.
    void unbind(OpenGLContext& open_gl_context);

    /// Renders the filter line response using transform feedback.
    /// @param open_gl The OpenGlWrapper.
    void renderLineResponse(OpenGlWrapper& open_gl);

    bool active_;                       ///< Whether the viewer is active.
    Point<int> last_mouse_position_;    ///< Last mouse position for drag calculations.
    vital::DigitalSvf filter_;          ///< Digital state variable filter for response calculation.
    vital::SynthFilter::FilterState filter_state_;

    SynthSlider* cutoff_slider_;        ///< The cutoff slider control.
    SynthSlider* resonance_slider_;     ///< The resonance slider control.
    SynthSlider* blend_slider_;         ///< The blend slider control.

    vital::Output* cutoff_output_;      ///< Cutoff modulation output.
    vital::Output* resonance_output_;   ///< Resonance modulation output.
    vital::Output* blend_output_;       ///< Blend modulation output.

    FilterResponseShader response_shader_; ///< Shader for rendering filter response.
    std::unique_ptr<float[]> line_data_;    ///< Vertex data for the line.
    GLuint vertex_array_object_;           ///< VAO for line rendering.
    GLuint line_buffer_;                   ///< Buffer for line vertices.
    GLuint response_buffer_;               ///< Buffer for transform feedback results.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DistortionFilterResponse)
};

/// @class DistortionSection
/// @brief A UI section for configuring a distortion effect, including type, drive, mix, and filtering.
///
/// The DistortionSection provides controls for selecting distortion type, adjusting drive, mix,
/// and configuring a filter with adjustable order, cutoff, resonance, and blend. It also includes
/// an OpenGL viewer to visualize the distortion curve and filter frequency response.
class DistortionSection : public SynthSection {
  public:
    static constexpr int kViewerResolution = 124; ///< Resolution for visualizing distortion line.

    /// Constructor.
    /// @param name The name of this section.
    /// @param mono_modulations A map of mono modulation outputs from the synth.
    DistortionSection(String name, const vital::output_map& mono_modulations);

    /// Destructor.
    virtual ~DistortionSection();

    /// Paints the background and labels for distortion parameters.
    /// @param g The graphics context.
    void paintBackground(Graphics& g) override;

    /// Paints a background shadow for visual depth.
    /// @param g The graphics context.
    void paintBackgroundShadow(Graphics& g) override { if (isActive()) paintTabShadow(g); }

    /// Resizes and lays out child components.
    void resized() override;

    /// Sets whether this section is active.
    /// @param active True if active, false otherwise.
    void setActive(bool active) override;

    /// Called when a slider value changes, checks if the filter should be active.
    /// @param changed_slider The slider that changed.
    void sliderValueChanged(Slider* changed_slider) override;

    /// Sets all parameter values from a control map.
    /// @param controls The control map.
    void setAllValues(vital::control_map& controls) override;

    /// Enables or disables the filter based on filter order and active state.
    /// @param active True if the filter should be active, false otherwise.
    void setFilterActive(bool active);

  private:
    std::unique_ptr<SynthButton> on_;             ///< On/off button for the distortion.
    std::unique_ptr<TextSelector> type_;          ///< Distortion type selector.
    std::unique_ptr<TextSelector> filter_order_;  ///< Filter order selector.
    std::unique_ptr<SynthSlider> drive_;          ///< Drive slider.
    std::unique_ptr<SynthSlider> mix_;            ///< Mix slider.
    std::unique_ptr<SynthSlider> filter_cutoff_;  ///< Filter cutoff slider.
    std::unique_ptr<SynthSlider> filter_resonance_; ///< Filter resonance slider.
    std::unique_ptr<SynthSlider> filter_blend_;   ///< Filter blend slider.
    std::unique_ptr<DistortionViewer> distortion_viewer_; ///< Viewer for distortion curve.
    std::unique_ptr<DistortionFilterResponse> filter_response_; ///< Viewer for filter frequency response.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DistortionSection)
};

