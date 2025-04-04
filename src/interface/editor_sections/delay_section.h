/// @file delay_section.h
/// @brief Declares the DelaySection class and related viewer classes for displaying and controlling a delay effect.

#pragma once

#include "JuceHeader.h"
#include "delay.h"
#include "open_gl_line_renderer.h"
#include "synth_section.h"

class DelayViewer;
class SynthButton;
class SynthSlider;
class TempoSelector;
class TextSelector;

/// @class DelayFilterViewer
/// @brief A viewer that displays and allows interaction with the delay effect's filter response.
///
/// This class uses an OpenGL line renderer to show the frequency response of the delay's filter.
/// It supports dragging to modify filter cutoff and spread values. Observers implementing
/// the Listener interface can respond to mouse drag delta movements.
class DelayFilterViewer : public OpenGlLineRenderer {
  public:
    /// The MIDI note range for drawing the filter response.
    static constexpr float kMidiDrawStart = 8.0f;
    static constexpr float kMidiDrawEnd = 132.0f;
    /// Minimum and maximum dB values for the filter response display.
    static constexpr float kMinDb = -18.0f;
    static constexpr float kMaxDb = 6.0f;

    /// @class Listener
    /// @brief Interface for objects that want to respond to mouse drag movements in the filter viewer.
    class Listener {
      public:
        virtual ~Listener() = default;
        /// Called when the user drags the mouse, providing a normalized delta movement.
        /// @param x The normalized horizontal delta.
        /// @param y The normalized vertical delta.
        virtual void deltaMovement(float x, float y) = 0;
    };

    /// Constructor.
    /// @param prefix A string prefix to identify the delay's parameters.
    /// @param resolution The number of points to use when rendering the line.
    /// @param mono_modulations A map of modulation outputs from the synth.
    DelayFilterViewer(const std::string& prefix, int resolution, const vital::output_map& mono_modulations) :
        OpenGlLineRenderer(resolution), active_(true), cutoff_slider_(nullptr), spread_slider_(nullptr) {
      setFill(true);
      setFillCenter(-1.0f);

      cutoff_ = mono_modulations.at(prefix + "_cutoff");
      spread_ = mono_modulations.at(prefix + "_spread");
    }

    /// Gets the current cutoff value (modulated or from slider).
    vital::poly_float getCutoff() {
      if (cutoff_slider_ && !cutoff_->owner->enabled())
        return cutoff_slider_->getValue();
      return cutoff_->trigger_value;
    }

    /// Gets the current spread value (modulated or from slider).
    vital::poly_float getSpread() {
      if (spread_slider_ && !spread_->owner->enabled())
        return spread_slider_->getValue();
      return spread_->trigger_value;
    }


    /// Draws the filter lines given the high and low MIDI cutoff values.
    void drawLines(OpenGlWrapper& open_gl, bool animate, float high_midi_cutoff, float low_midi_cutoff) {
      float midi_increment = (kMidiDrawEnd - kMidiDrawStart) / (numPoints() - 1);
      float mult_increment = vital::utils::centsToRatio(midi_increment * vital::kCentsPerNote);

      float high_midi_offset_start = kMidiDrawStart - high_midi_cutoff;
      float high_ratio = vital::utils::centsToRatio(high_midi_offset_start * vital::kCentsPerNote);
      float low_midi_offset_start = kMidiDrawStart - low_midi_cutoff;
      float low_ratio = vital::utils::centsToRatio(low_midi_offset_start * vital::kCentsPerNote);

      float gain = vital::utils::centsToRatio((high_midi_cutoff - low_midi_cutoff) * vital::kCentsPerNote) + 1.0f;

      float width = getWidth();
      float height = getHeight();
      int num_points = numPoints();
      for (int i = 0; i < num_points; ++i) {
        float high_response = high_ratio / sqrtf(1 + high_ratio * high_ratio);
        float low_response = 1.0f / sqrtf(1 + low_ratio * low_ratio);
        float response = gain * low_response * high_response;
        float db = vital::utils::magnitudeToDb(response);
        float y = (db - kMinDb) / (kMaxDb - kMinDb);

        setXAt(i, width * i / (num_points - 1.0f));
        setYAt(i, (1.0f - y) * height);

        high_ratio *= mult_increment;
        low_ratio *= mult_increment;
      }

      OpenGlLineRenderer::render(open_gl, animate);
    }

    /// Renders the filter line based on current cutoff and spread values.
    /// @param open_gl The OpenGlWrapper.
    /// @param animate Whether to animate transitions.
    void render(OpenGlWrapper& open_gl, bool animate) override {
      vital::poly_float cutoff = getCutoff();
      vital::poly_float radius = vital::StereoDelay::getFilterRadius(getSpread());
      vital::poly_float high_midi_cutoff = cutoff - radius;
      vital::poly_float low_midi_cutoff = cutoff + radius;

      setLineWidth(findValue(Skin::kWidgetLineWidth));
      setFillCenter(findValue(Skin::kWidgetFillCenter));

      float fill_fade = findValue(Skin::kWidgetFillFade);
      Colour color_fill_to;
      if (active_) {
        setColor(findColour(Skin::kWidgetPrimary1, true));
        color_fill_to = findColour(Skin::kWidgetSecondary1, true);
      }
      else {
        setColor(findColour(Skin::kWidgetPrimaryDisabled, true));
        color_fill_to = findColour(Skin::kWidgetSecondaryDisabled, true);
      }

      Colour color_fill_from = color_fill_to.withMultipliedAlpha(1.0f - fill_fade);
      setFillColors(color_fill_from, color_fill_to);
      drawLines(open_gl, animate, high_midi_cutoff[0], low_midi_cutoff[0]);

      if (active_) {
        setColor(findColour(Skin::kWidgetPrimary2, true));
        color_fill_to = findColour(Skin::kWidgetSecondary2, true);
      }
      else {
        setColor(findColour(Skin::kWidgetPrimaryDisabled, true));
        color_fill_to = findColour(Skin::kWidgetSecondaryDisabled, true);
      }
      color_fill_from = color_fill_to.withMultipliedAlpha(1.0f - fill_fade);
      setFillColors(color_fill_from, color_fill_to);
      drawLines(open_gl, animate, high_midi_cutoff[1], low_midi_cutoff[1]);

      renderCorners(open_gl, animate);
    }

    /// Handles mouse down events, storing the initial mouse position.
    /// @param e The mouse event.
    void mouseDown(const MouseEvent& e) override {
      last_mouse_position_ = e.getPosition();
    }

    /// Handles mouse drag events, calculating delta movements and notifying listeners.
    /// @param e The mouse event.
    void mouseDrag(const MouseEvent& e) override {
      Point<int> delta = e.getPosition() - last_mouse_position_;
      last_mouse_position_ = e.getPosition();

      for (Listener* listener : listeners_)
        listener->deltaMovement(delta.x * 1.0f / getWidth(), -delta.y * 1.0f / getHeight());
    }

    /// Sets the slider controlling the cutoff frequency.
    /// @param slider The cutoff slider.
    void setCutoffSlider(Slider* slider) { cutoff_slider_ = slider; }

    /// Sets the slider controlling the filter spread.
    /// @param slider The spread slider.
    void setSpreadSlider(Slider* slider) { spread_slider_ = slider; }

    /// Sets whether the viewer is active.
    /// @param active True if active, false otherwise.
    void setActive(bool active) { active_ = active; }

    /// Adds a listener to receive mouse drag events.
    /// @param listener The listener to add.
    void addListener(Listener* listener) { listeners_.push_back(listener); }

  private:
    bool active_;                          ///< Whether the viewer is active.
    std::vector<Listener*> listeners_;      ///< Listeners for drag movements.
    Point<int> last_mouse_position_;        ///< Last mouse position for calculating deltas.

    vital::Output* cutoff_;                ///< Cutoff modulation output.
    vital::Output* spread_;                ///< Spread modulation output.
    Slider* cutoff_slider_;                ///< Cutoff slider control.
    Slider* spread_slider_;                ///< Spread slider control.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DelayFilterViewer)
};

/// @class DelaySection
/// @brief A UI section providing controls for a delay effect, including tempo-synced delays, filters, and mixing.
///
/// The DelaySection manages parameters for the delay effect, such as frequency, tempo sync,
/// feedback, dry/wet mix, filtering, and style (mono, ping-pong, etc.). It includes viewers
/// to display the delay line and filter frequency response, and supports dragging on the filter viewer
/// to adjust cutoff and spread.
class DelaySection : public SynthSection, DelayFilterViewer::Listener {
  public:

    /// Constructor.
    /// @param name The name of this section.
    /// @param mono_modulations A map of mono modulation outputs from the synth.
    DelaySection(const String& name, const vital::output_map& mono_modulations);

    /// Destructor.
    virtual ~DelaySection();

    /// Paints the background and labels for the delay section.
    /// @param g The graphics context.
    void paintBackground(Graphics& g) override;

    /// Paints a background shadow for visual depth.
    /// @param g The graphics context.
    void paintBackgroundShadow(Graphics& g) override { if (isActive()) paintTabShadow(g); }

    /// Resizes and lays out child components, including placing tempo controls based on the delay style.
    void resized() override;

    /// Sets whether this section is active.
    /// @param active True if active, false otherwise.
    void setActive(bool active) override;

    /// Resizes the tempo controls depending on the delay style (e.g., mono or stereo).
    void resizeTempoControls();

    /// Sets all parameter values from a control map.
    /// @param controls The control map to load values from.
    void setAllValues(vital::control_map& controls) override {
      SynthSection::setAllValues(controls);
      resizeTempoControls();
    }

    /// Called when a slider value changes, re-layouts tempo controls if the style changes.
    /// @param changed_slider The slider that changed.
    void sliderValueChanged(Slider* changed_slider) override;

    /// Called when the user drags the mouse on the filter viewer, adjusting cutoff and spread.
    /// @param x The normalized horizontal delta.
    /// @param y The normalized vertical delta.
    void deltaMovement(float x, float y) override;

  private:
    std::unique_ptr<SynthButton> on_;             ///< On/off button for the delay.
    std::unique_ptr<SynthSlider> frequency_;      ///< Delay frequency slider.
    std::unique_ptr<SynthSlider> tempo_;          ///< Delay tempo slider.
    std::unique_ptr<TempoSelector> sync_;         ///< Tempo sync selector for the primary delay line.
    std::unique_ptr<SynthSlider> aux_frequency_;  ///< Auxiliary delay frequency slider.
    std::unique_ptr<SynthSlider> aux_tempo_;      ///< Auxiliary delay tempo slider.
    std::unique_ptr<TempoSelector> aux_sync_;     ///< Tempo sync selector for the auxiliary delay line.
    std::unique_ptr<SynthSlider> feedback_;       ///< Feedback amount slider.
    std::unique_ptr<SynthSlider> dry_wet_;        ///< Dry/Wet mix slider.
    std::unique_ptr<SynthSlider> filter_cutoff_;  ///< Filter cutoff slider.
    std::unique_ptr<SynthSlider> filter_spread_;  ///< Filter spread slider.
    std::unique_ptr<TextSelector> style_;         ///< Delay style selector (mono, ping-pong, etc.).

    std::unique_ptr<DelayFilterViewer> delay_filter_viewer_; ///< Viewer for the filter frequency response.
    std::unique_ptr<DelayViewer> delay_viewer_;             ///< Viewer for displaying the delay line.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DelaySection)
};

