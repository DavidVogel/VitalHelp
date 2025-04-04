#pragma once

#include "JuceHeader.h"
#include "digital_svf.h"
#include "open_gl_line_renderer.h"
#include "open_gl_multi_quad.h"
#include "synth_slider.h"

/**
 * @class EqualizerResponse
 * @brief A visualization component for an equalizer or filter response curve.
 *
 * The EqualizerResponse component displays the frequency response of a combination
 * of low, band, and high filters (e.g., for an EQ or reverb shelving filters). It
 * supports interactive control via mouse dragging of filter cutoff and gain points
 * and includes adjustable parameters for scaling the dB range and switching filter styles.
 */
class EqualizerResponse : public OpenGlLineRenderer, SynthSlider::SliderListener {
  public:
    /// Number of points used for resolution in the frequency response display.
    static constexpr int kResolution = 128;

    /// A high view sample rate for accurate visualization (not actual audio processing).
    static constexpr int kViewSampleRate = 100000;

    /// Ratio of dB range used as a buffer around min/max gain values.
    static constexpr float kDefaultDbBufferRatio = 0.2f;

    /// Mouse drag multiplier for gain/cutoff adjustments.
    static constexpr float kMouseMultiplier = 0.3f;

    /**
     * @class Listener
     * @brief Interface for objects that want to be notified when a band is selected.
     */
    class Listener {
      public:
        virtual ~Listener() = default;

        /**
         * @brief Called when the low band is selected by the user.
         */
        virtual void lowBandSelected() = 0;

        /**
         * @brief Called when the mid band is selected by the user.
         */
        virtual void midBandSelected() = 0;

        /**
         * @brief Called when the high band is selected by the user.
         */
        virtual void highBandSelected() = 0;
    };

    /**
     * @brief Constructs an EqualizerResponse component.
     */
    EqualizerResponse();

    /**
     * @brief Destructor.
     */
    ~EqualizerResponse();

    /**
     * @brief Initializes the Equalizer response for a standard 3-band EQ using the provided outputs.
     * @param mono_modulations A map of modulation outputs from the synthesizer.
     */
    void initEq(const vital::output_map& mono_modulations);

    /**
     * @brief Initializes the Equalizer response for a reverb's shelving EQ.
     * @param mono_modulations A map of modulation outputs from the synthesizer.
     */
    void initReverb(const vital::output_map& mono_modulations);

    /**
     * @brief Initializes OpenGL resources.
     * @param open_gl The OpenGlWrapper context.
     */
    virtual void init(OpenGlWrapper& open_gl) override;

    /**
     * @brief Renders the EQ response and control points.
     * @param open_gl The OpenGlWrapper context.
     * @param animate Whether to animate the response.
     */
    virtual void render(OpenGlWrapper& open_gl, bool animate) override;

    /**
     * @brief Destroys OpenGL resources.
     * @param open_gl The OpenGlWrapper context.
     */
    virtual void destroy(OpenGlWrapper& open_gl) override;


    /**
     * @brief Sets the bounds for the control points (selected and unselected) in normalized coordinates.
     * @param selected_x X position of the selected band.
     * @param selected_y Y position of the selected band.
     * @param unselected_x1 X position of the first unselected band.
     * @param unselected_y1 Y position of the first unselected band.
     * @param unselected_x2 X position of the second unselected band.
     * @param unselected_y2 Y position of the second unselected band.
     */
    void setControlPointBounds(float selected_x, float selected_y,
                               float unselected_x1, float unselected_y1,
                               float unselected_x2, float unselected_y2);

    /**
     * @brief Draws the control points (markers) for the EQ bands.
     * @param open_gl The OpenGlWrapper context.
     */
    void drawControlPoints(OpenGlWrapper& open_gl);

    /**
     * @brief Draws the response of the filters.
     * @param open_gl The OpenGlWrapper context.
     * @param index Index to indicate stereo or multiple instances if needed.
     */
    void drawResponse(OpenGlWrapper& open_gl, int index);

    /**
     * @brief Computes the filter coefficients based on current slider values and states.
     */
    void computeFilterCoefficients();

    /**
     * @brief Moves the currently selected filter's cutoff and gain based on a mouse drag position.
     * @param position The mouse position in the component's coordinate space.
     */
    void moveFilterSettings(Point<float> position);

    /**
     * @brief Assigns the sliders for the low band.
     * @param cutoff The low band cutoff slider.
     * @param resonance The low band resonance slider.
     * @param gain The low band gain slider.
     */
    void setLowSliders(SynthSlider* cutoff, SynthSlider* resonance, SynthSlider* gain);

    /**
     * @brief Assigns the sliders for the mid (band) band.
     * @param cutoff The band cutoff slider.
     * @param resonance The band resonance slider.
     * @param gain The band gain slider.
     */
    void setBandSliders(SynthSlider* cutoff, SynthSlider* resonance, SynthSlider* gain);

    /**
     * @brief Assigns the sliders for the high band.
     * @param cutoff The high band cutoff slider.
     * @param resonance The high band resonance slider.
     * @param gain The high band gain slider.
     */
    void setHighSliders(SynthSlider* cutoff, SynthSlider* resonance, SynthSlider* gain);

    /**
     * @brief Sets the currently selected band.
     * @param selected_band An integer representing the band (0 = low, 1 = mid, 2 = high).
     */
    void setSelectedBand(int selected_band);

    /**
     * @brief Returns the current low band control point position in component coordinates.
     * @return A point representing the low band's x and y position.
     */
    Point<float> getLowPosition();

    /**
     * @brief Returns the current mid (band) control point position in component coordinates.
     * @return A point representing the mid band's x and y position.
     */
    Point<float> getBandPosition();

    /**
     * @brief Returns the current high band control point position in component coordinates.
     * @return A point representing the high band's x and y position.
     */
    Point<float> getHighPosition();

    /**
     * @brief Called when the component is resized.
     */
    void resized() override {
      OpenGlLineRenderer::resized();

      unselected_points_.setBounds(getLocalBounds());
      selected_point_.setBounds(getLocalBounds());
      dragging_point_.setBounds(getLocalBounds());
    }

    /**
     * @brief Paints the background and optionally draws frequency lines.
     * @param g The Graphics context to use for painting.
     */
    void paintBackground(Graphics& g) override;

    /**
     * @brief Handles mouse wheel movements for adjusting resonance if hovering over a band.
     * @param e The mouse event.
     * @param wheel The details of the mouse wheel movement.
     */
    void mouseWheelMove(const MouseEvent& e, const MouseWheelDetails& wheel) override;

    /**
     * @brief Handles mouse down events to select a band.
     * @param e The mouse event.
     */
    void mouseDown(const MouseEvent& e) override;

    /**
     * @brief Handles mouse drag events to adjust filter settings.
     * @param e The mouse event.
     */
    void mouseDrag(const MouseEvent& e) override;

    /**
     * @brief Handles mouse up events, finalizing any changes made during drag.
     * @param e The mouse event.
     */
    void mouseUp(const MouseEvent& e) override;

    /**
     * @brief Handles mouse exit events to hide popups or reset state.
     * @param e The mouse event.
     */
    void mouseExit(const MouseEvent& e) override;

    /**
     * @brief Determines which band is currently hovered by the mouse.
     * @param e The mouse event.
     * @return An integer representing the hovered band (0 = low, 1 = mid, 2 = high, -1 = none).
     */
    int getHoveredBand(const MouseEvent& e);

    /**
     * @brief Sets the active state of the EQ visualization.
     * @param active True if the EQ should be displayed as active, false otherwise.
     */
    void setActive(bool active);

    /**
     * @brief Configures the low band as a high-pass filter (or a shelf).
     * @param high_pass True to make it a high-pass, false for shelving.
     */
    void setHighPass(bool high_pass);

    /**
     * @brief Configures the mid band as a notch filter (or a shelf).
     * @param notch True to make it a notch, false for shelving.
     */
    void setNotch(bool notch);

    /**
     * @brief Configures the high band as a low-pass filter (or a shelf).
     * @param low_pass True to make it low-pass, false for shelving.
     */
    void setLowPass(bool low_pass);

    /**
     * @brief Sets the ratio of the dB range used as buffer.
     * @param ratio The dB buffer ratio.
     */
    void setDbBufferRatio(float ratio) { db_buffer_ratio_ = ratio; }

    /**
     * @brief Sets whether to draw frequency grid lines in the background.
     * @param draw_lines True to draw frequency lines, false otherwise.
     */
    void setDrawFrequencyLines(bool draw_lines) { draw_frequency_lines_ = draw_lines; }

    /**
     * @brief Adds a listener to be notified of band selections.
     * @param listener The listener to add.
     */
    void addListener(Listener* listener) { listeners_.push_back(listener); }

  private:
    /**
     * @brief Retrieves the total value from a given output and slider, considering if modulations are active.
     * @param output The status output for the parameter.
     * @param slider The corresponding slider.
     * @return The computed parameter value.
     */
    vital::poly_float getOutputTotal(vital::Output* output, Slider* slider);

    int resolution_; ///< The resolution used for plotting the response.
    bool active_; ///< Whether the EQ is active.
    bool high_pass_; ///< Whether the low band is configured as high-pass.
    bool notch_; ///< Whether the mid band is configured as a notch.
    bool low_pass_; ///< Whether the high band is configured as low-pass.
    bool animate_; ///< Whether the response should be animated.
    bool draw_frequency_lines_; ///< Whether frequency lines are drawn in the background.

    int selected_band_; ///< The currently selected band index.
    float db_buffer_ratio_; ///< The ratio for dB buffering around min/max gain.
    float min_db_; ///< Minimum dB value displayed.
    float max_db_; ///< Maximum dB value displayed.

    OpenGlMultiQuad unselected_points_; ///< The OpenGL quads for unselected bands.
    OpenGlQuad selected_point_; ///< The OpenGL quad for the selected band.
    OpenGlQuad dragging_point_; ///< The OpenGL quad for the band currently being dragged.

    vital::DigitalSvf low_filter_;
    vital::DigitalSvf band_filter_;
    vital::DigitalSvf high_filter_;

    vital::SynthFilter::FilterState low_filter_state_;
    vital::SynthFilter::FilterState band_filter_state_;
    vital::SynthFilter::FilterState high_filter_state_;

    SynthSlider* low_cutoff_;
    SynthSlider* low_resonance_;
    SynthSlider* low_gain_;
    SynthSlider* band_cutoff_;
    SynthSlider* band_resonance_;
    SynthSlider* band_gain_;
    SynthSlider* high_cutoff_;
    SynthSlider* high_resonance_;
    SynthSlider* high_gain_;

    vital::Output* low_cutoff_output_;
    vital::Output* low_resonance_output_;
    vital::Output* low_gain_output_;
    vital::Output* band_cutoff_output_;
    vital::Output* band_resonance_output_;
    vital::Output* band_gain_output_;
    vital::Output* high_cutoff_output_;
    vital::Output* high_resonance_output_;
    vital::Output* high_gain_output_;

    SynthSlider* current_cutoff_; ///< The slider currently being dragged for cutoff.
    SynthSlider* current_gain_; ///< The slider currently being dragged for gain.

    std::unique_ptr<float[]> line_data_; ///< Temporary storage for line data.
    OpenGLShaderProgram* shader_; ///< Shader for the frequency response.
    std::unique_ptr<OpenGLShaderProgram::Attribute> position_attribute_;

    std::unique_ptr<OpenGLShaderProgram::Uniform> midi_cutoff_uniform_;
    std::unique_ptr<OpenGLShaderProgram::Uniform> resonance_uniform_;
    std::unique_ptr<OpenGLShaderProgram::Uniform> low_amount_uniform_;
    std::unique_ptr<OpenGLShaderProgram::Uniform> band_amount_uniform_;
    std::unique_ptr<OpenGLShaderProgram::Uniform> high_amount_uniform_;

    GLuint vertex_array_object_;
    GLuint line_buffer_;
    GLuint response_buffer_;
    std::vector<Listener*> listeners_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EqualizerResponse)
};

