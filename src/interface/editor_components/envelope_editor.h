#pragma once

#include "JuceHeader.h"
#include "skin.h"
#include "open_gl_image.h"
#include "open_gl_line_renderer.h"
#include "synth_module.h"
#include "synth_slider.h"

/**
 * @class EnvelopeEditor
 * @brief A graphical editor for envelope shapes with interactive points and power curves.
 *
 * The EnvelopeEditor class provides an interface to visualize and edit a complex envelope (ADHSR)
 * using interactive markers. The user can adjust delay, attack, hold, decay, sustain, and release
 * times, as well as the curvature (power) of the envelope segments. The envelope is rendered using
 * OpenGL and supports zooming and hovering over individual envelope points or power handles.
 */
class EnvelopeEditor : public OpenGlLineRenderer, public SynthSlider::SliderListener {
public:
    /// Width in pixels of the main markers (ADHSR points).
    static constexpr float kMarkerWidth = 9.0f;
    /// Thickness fraction for the marker rings.
    static constexpr float kRingThickness = 0.45f;
    /// Width in pixels of the power markers (for adjusting curve power).
    static constexpr float kPowerMarkerWidth = 7.0f;
    /// Radius in pixels for hovering detection over markers.
    static constexpr float kMarkerHoverRadius = 12.0f;
    /// Radius in pixels for grabbing markers.
    static constexpr float kMarkerGrabRadius = 20.0f;
    /// Decay factor for tail end animations.
    static constexpr float kTailDecay = 0.965f;
    /// Horizontal padding ratio for the display.
    static constexpr float kPaddingX = 0.018f;
    /// Vertical padding ratio for the display.
    static constexpr float kPaddingY = 0.06f;
    /// Minimum point distance for enabling power handle editing.
    static constexpr float kMinPointDistanceForPower = 3.0f;
    /// Multiplier for mouse movements when adjusting power.
    static constexpr float kPowerMouseMultiplier = 0.06f;
    /// Display size ratio for time text.
    static constexpr float kTimeDisplaySize = 0.05f;

    /// Division size for major time ruler lines.
    static constexpr int kRulerDivisionSize = 4;
    /// Maximum number of grid lines displayed.
    static constexpr int kMaxGridLines = 36;
    /// Maximum number of time markers shown.
    static constexpr int kMaxTimesShown = 24;
    /// Number of points per envelope section.
    static constexpr int kNumPointsPerSection = 98;
    /// Number of envelope sections (e.g., Delay->Attack->Hold->Decay->Release mapped to 4 sections plus 1 extra point).
    static constexpr int kNumSections = 4;
    /// Total number of points for the entire envelope line.
    static constexpr int kTotalPoints = kNumSections * kNumPointsPerSection + 1;

    /**
     * @brief Constructs an EnvelopeEditor.
     * @param prefix Prefix for envelope parameters (e.g., "env1").
     * @param mono_modulations A map of mono modulation outputs.
     * @param poly_modulations A map of poly modulation outputs.
     */
    EnvelopeEditor(const String& prefix,
                   const vital::output_map& mono_modulations, const vital::output_map& poly_modulations);

    /**
     * @brief Destructor.
     */
    ~EnvelopeEditor();

    /**
     * @brief Paints the background of the editor.
     * @param g Graphics context.
     */
    void paintBackground(Graphics& g) override;

    /**
     * @brief Resized callback to layout and reposition components.
     */
    void resized() override {
        OpenGlLineRenderer::resized();
        drag_circle_.setBounds(getLocalBounds());
        hover_circle_.setBounds(getLocalBounds());
        grid_lines_.setBounds(getLocalBounds());
        sub_grid_lines_.setBounds(getLocalBounds());
        position_circle_.setBounds(getLocalBounds());
        point_circles_.setBounds(getLocalBounds());
        power_circles_.setBounds(getLocalBounds());

        float font_height = kTimeDisplaySize * getHeight();
        for (int i = 0; i < kMaxTimesShown; ++i)
            times_[i]->setTextSize(font_height);

        setTimePositions();
        reset_positions_ = true;
    }

    /**
     * @brief Resets the envelope line for a given voice index.
     * @param index Voice index (-1 for default envelope).
     */
    void resetEnvelopeLine(int index);

    /**
     * @brief Called when a slider value changes.
     * @param slider The slider that changed.
     */
    void guiChanged(SynthSlider* slider) override;

    /// Sets the sliders corresponding to different envelope parameters.
    void setDelaySlider(SynthSlider* delay_slider);
    void setAttackSlider(SynthSlider* attack_slider);
    void setAttackPowerSlider(SynthSlider* attack_slider);
    void setHoldSlider(SynthSlider* hold_slider);
    void setDecaySlider(SynthSlider* decay_slider);
    void setDecayPowerSlider(SynthSlider* decay_slider);
    void setSustainSlider(SynthSlider* sustain_slider);
    void setReleaseSlider(SynthSlider* release_slider);
    void setReleasePowerSlider(SynthSlider* release_slider);

    /**
     * @brief Sets the size ratio for UI scaling.
     * @param ratio The new size ratio.
     */
    void setSizeRatio(float ratio) { size_ratio_ = ratio; }

    /**
     * @brief Called when the component’s parent hierarchy changes.
     */
    void parentHierarchyChanged() override;

    /**
     * @brief Determines hover state based on mouse position.
     * @param position The mouse position.
     */
    void pickHoverPosition(Point<float> position);

    /**
     * @brief Mouse event callbacks for interactions.
     */
    void mouseMove(const MouseEvent& e) override;
    void mouseExit(const MouseEvent& e) override;
    void mouseDown(const MouseEvent& e) override;
    void mouseDrag(const MouseEvent& e) override;
    void mouseDoubleClick(const MouseEvent& e) override;
    void mouseUp(const MouseEvent& e) override;

    /**
     * @brief Handles mouse wheel events for zooming.
     * @param e Mouse event.
     * @param wheel Wheel details.
     */
    void mouseWheelMove(const MouseEvent& e, const MouseWheelDetails& wheel) override;

    /**
     * @brief Zooms the envelope display via magnification (e.g., pinch gestures).
     * @param delta Scroll delta for magnification.
     */
    void magnifyZoom(Point<float> delta);

    /**
     * @brief Resets the magnification zoom to a default level.
     */
    void magnifyReset();

    /**
     * @brief Initializes OpenGL resources.
     * @param open_gl The OpenGL wrapper.
     */
    void init(OpenGlWrapper& open_gl) override;

    /**
     * @brief Renders the envelope using OpenGL.
     * @param open_gl The OpenGL wrapper.
     * @param animate Whether to animate the line based on current voice states.
     */
    void render(OpenGlWrapper& open_gl, bool animate) override;

    /**
     * @brief Destroys the OpenGL resources.
     * @param open_gl The OpenGL wrapper.
     */
    void destroy(OpenGlWrapper& open_gl) override;

    /**
     * @brief Flags that the positions need to be reset and recalculated.
     */
    void resetPositions() { reset_positions_ = true; }

  private:
    /**
     * @brief Sets the editing circle bounds for drag and hover visuals.
     */
    void setEditingCircleBounds();

    /**
     * @brief Positions grid lines and sub-grid lines for the envelope background.
     */
    void setGridPositions();

    /**
     * @brief Positions the time labels along the horizontal axis.
     */
    void setTimePositions();

    /**
     * @brief Positions the envelope’s point markers (A, D, S, R points and power points).
     */
    void setPointPositions();

    /**
     * @brief Sets the main line and marker positions in the OpenGL coordinate space.
     */
    void setGlPositions();

    /**
     * @brief Sets colors for lines, fills, and other UI elements based on the current skin.
     */
    void setColors();

    /**
     * @brief Zooms the display by a given amount.
     * @param amount Zoom factor.
     */
    void zoom(float amount);

    /**
     * @brief Retrieves outputs and returns them as a pair of pointers for mono/poly.
     * @param mono_modulations Mono modulations map.
     * @param poly_modulations Poly modulations map.
     * @param name The parameter name.
     * @return A pair of pointers to mono and poly outputs.
     */
    static std::pair<vital::Output*, vital::Output*> getOutputs(const vital::output_map& mono_modulations,
                                                                const vital::output_map& poly_modulations,
                                                                const String& name) {
      return {
        mono_modulations.at(name.toStdString()),
        poly_modulations.at(name.toStdString())
      };
    }

    /**
     * @brief Retrieves total output value combining mono and poly outputs.
     * @param outputs A pair of mono/poly outputs.
     * @param default_value The default slider value if outputs are unavailable or disabled.
     * @return The combined poly_float output value.
     */
    vital::poly_float getOutputsTotal(std::pair<vital::Output*, vital::Output*> outputs,
                                      vital::poly_float default_value);

    /**
     * @brief Draws the current position indicator for a given voice index.
     * @param open_gl The OpenGL wrapper.
     * @param index The voice index.
     */
    void drawPosition(OpenGlWrapper& open_gl, int index);

    /**
     * @brief Gets the (x,y) position for a given voice index based on its envelope phase.
     * @param index The voice index.
     * @return The (x,y) position as a pair of floats.
     */
    std::pair<float, float> getPosition(int index);

    // Utility functions for coordinate padding and unpadding.
    float padX(float x);
    float padY(float y);
    float unpadX(float x);
    float unpadY(float y);
    float padOpenGlX(float x);
    float padOpenGlY(float y);

    // Slider-specific getter methods for times and positions.
    float getSliderDelayX();
    float getSliderAttackX();
    float getSliderHoldX();
    float getSliderDecayX();
    float getSliderSustainY();
    float getSliderReleaseX();
    float getDelayTime(int index);
    float getAttackTime(int index);
    float getHoldTime(int index);
    float getDecayTime(int index);
    float getReleaseTime(int index);
    float getDelayX(int index);
    float getAttackX(int index);
    float getHoldX(int index);
    float getDecayX(int index);
    float getSustainY(int index);
    float getReleaseX(int index);

    float getBackupPhase(float phase, int index);
    vital::poly_float getBackupPhase(vital::poly_float phase);
    float getEnvelopeValue(float t, float power, float start, float end);
    float getSliderAttackValue(float t);
    float getSliderDecayValue(float t);
    float getSliderReleaseValue(float t);
    float getAttackValue(float t, int index);
    float getDecayValue(float t, int index);
    float getReleaseValue(float t, int index);

    // Setters for envelope parameters based on mouse interactions.
    void setDelayX(float x);
    void setAttackX(float x);
    void setHoldX(float x);
    void setDecayX(float x);
    void setSustainY(float y);
    void setReleaseX(float x);

    void setPower(SynthSlider* slider, float power);
    void setAttackPower(float power);
    void setDecayPower(float power);
    void setReleasePower(float power);

    SynthGuiInterface* parent_;
    bool delay_hover_;
    bool attack_hover_;
    bool hold_hover_;
    bool sustain_hover_;
    bool release_hover_;
    bool attack_power_hover_;
    bool decay_power_hover_;
    bool release_power_hover_;
    bool mouse_down_;
    Point<float> last_edit_position_;

    bool animate_;
    float size_ratio_;
    float window_time_;

    vital::poly_float current_position_alpha_;
    vital::poly_float last_phase_;

    Colour line_left_color_;
    Colour line_right_color_;
    Colour line_center_color_;
    Colour fill_left_color_;
    Colour fill_right_color_;
    Colour background_color_;
    Colour time_color_;

    bool reset_positions_;
    OpenGlQuad drag_circle_;
    OpenGlQuad hover_circle_;
    OpenGlMultiQuad grid_lines_;
    OpenGlMultiQuad sub_grid_lines_;
    OpenGlQuad position_circle_;
    OpenGlMultiQuad point_circles_;
    OpenGlMultiQuad power_circles_;
    std::unique_ptr<PlainTextComponent> times_[kMaxTimesShown];

    const vital::StatusOutput* envelope_phase_;

    SynthSlider* delay_slider_;
    SynthSlider* attack_slider_;
    SynthSlider* hold_slider_;
    SynthSlider* attack_power_slider_;
    SynthSlider* decay_slider_;
    SynthSlider* decay_power_slider_;
    SynthSlider* sustain_slider_;
    SynthSlider* release_slider_;
    SynthSlider* release_power_slider_;

    std::pair<vital::Output*, vital::Output*> delay_outputs_;
    std::pair<vital::Output*, vital::Output*> attack_outputs_;
    std::pair<vital::Output*, vital::Output*> hold_outputs_;
    std::pair<vital::Output*, vital::Output*> decay_outputs_;
    std::pair<vital::Output*, vital::Output*> sustain_outputs_;
    std::pair<vital::Output*, vital::Output*> release_outputs_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EnvelopeEditor)
};
