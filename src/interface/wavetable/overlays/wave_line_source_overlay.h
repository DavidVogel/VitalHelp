#pragma once

#include "JuceHeader.h"

#include "wave_line_source.h"
#include "line_editor.h"
#include "wavetable_component_overlay.h"

/**
 * @class WaveLineSourceOverlay
 * @brief An overlay for controlling a WaveLineSource in the wavetable editor.
 *
 * This overlay provides an interface to edit a custom line-based waveform source.
 * It features a line editor where users can manipulate points that define the waveform shape,
 * as well as controls for grid size and a "pull power" parameter that influences how the lines
 * interpolate between points.
 */
class WaveLineSourceOverlay : public WavetableComponentOverlay, public LineEditor::Listener {
  public:
    static constexpr int kDefaultXGrid = 6; ///< Default horizontal grid size.
    static constexpr int kDefaultYGrid = 4; ///< Default vertical grid size.
    static constexpr float kFillAlpha = 0.6f; ///< Alpha value for fill rendering.

    /**
     * @brief Constructor.
     *
     * Initializes all UI components including line editor and control sliders for grid and pull power.
     */
    WaveLineSourceOverlay();

    /**
     * @brief Destructor.
     */
    virtual ~WaveLineSourceOverlay();

    /**
     * @brief Called when the overlay is resized.
     *
     * Sets colors and fill parameters on the line editor.
     */
    void resized() override;

    /**
     * @brief Called when a new frame is selected in the wavetable editor.
     * @param keyframe The newly selected WavetableKeyframe or nullptr if none.
     *
     * If the frame belongs to the associated WaveLineSource, it updates the editor with the frame's line model.
     */
    virtual void frameSelected(WavetableKeyframe* keyframe) override;

    /**
     * @brief Called when a frame is dragged, but this overlay does not respond to frame dragging.
     * @param keyframe The dragged WavetableKeyframe.
     * @param position The new position of the frame.
     */
    virtual void frameDragged(WavetableKeyframe* keyframe, int position) override { }

    /**
     * @brief Sets the editing bounds for the UI controls.
     * @param bounds The rectangular area representing the editing UI space.
     */
    virtual void setEditBounds(Rectangle<int> bounds) override;

    /**
     * @brief Sets the bounding box for the time domain display area (line editor).
     * @param bounds The rectangular area for the time domain editor.
     * @return True if successfully set, else false.
     */
    virtual bool setTimeDomainBounds(Rectangle<int> bounds) override;

    /**
     * @brief Renders any openGL components, including the line editor.
     * @param open_gl Reference to the OpenGL wrapper.
     * @param animate Whether to animate the rendering.
     */
    void renderOpenGlComponents(OpenGlWrapper& open_gl, bool animate) override {
      editor_->setSizeRatio(getSizeRatio());
      SynthSection::renderOpenGlComponents(open_gl, animate);
    }

    /**
     * @brief Sets the waveform phase (not used here).
     * @param phase The new phase value.
     */
    void setPhase(float phase) override { }

    /**
     * @brief Called when the line editor is scrolled.
     * @param e Mouse event details.
     * @param wheel The wheel details.
     */
    void lineEditorScrolled(const MouseEvent& e, const MouseWheelDetails& wheel) override;

    /**
     * @brief Toggles painting mode (not used in this overlay).
     * @param enabled Whether painting mode is enabled.
     * @param temporary_switch Whether the switch is temporary.
     */
    void togglePaintMode(bool enabled, bool temporary_switch) override { }

    /**
     * @brief Callback for file loading completion (not used here).
     */
    void fileLoaded() override;

    /**
     * @brief Imports an LFO (not used in this overlay).
     */
    void importLfo() override { }

    /**
     * @brief Exports an LFO (not used in this overlay).
     */
    void exportLfo() override { }

    /**
     * @brief Callback when a line point changes its position.
     * @param index The index of the changed point.
     * @param position The new position of the point.
     * @param mouse_up True if the mouse button was released after this change.
     */
    void pointChanged(int index, Point<float> position, bool mouse_up) override;

    /**
     * @brief Callback when line powers/curvature change.
     * @param mouse_up True if changes concluded with a mouse release.
     */
    void powersChanged(bool mouse_up) override;

    /**
     * @brief Callback when a line point is added.
     * @param index The index at which the new point was inserted.
     * @param position The position of the new point.
     */
    void pointAdded(int index, Point<float> position) override;

    /**
     * @brief Callback when a point is removed.
     * @param index The index of the removed point.
     */
    void pointRemoved(int index) override;

    /**
     * @brief Callback when multiple points are added.
     * @param index The insertion index.
     * @param num_points_added The number of points added.
     */
    void pointsAdded(int index, int num_points_added) override;

    /**
     * @brief Callback when multiple points are removed.
     * @param index The starting index of removal.
     * @param num_points_removed The number of points removed.
     */
    void pointsRemoved(int index, int num_points_removed) override;

    /**
     * @brief Called when a slider in this overlay changes its value.
     * @param moved_slider Pointer to the changed slider.
     */
    void sliderValueChanged(Slider* moved_slider) override;

    /**
     * @brief Called when a slider in this overlay finishes being dragged.
     * @param moved_slider Pointer to the finished slider.
     */
    void sliderDragEnded(Slider* moved_slider) override;

    /**
     * @brief Sets the WaveLineSource associated with this overlay.
     * @param line_source The WaveLineSource to control.
     */
    void setLineSource(WaveLineSource* line_source) {
      line_source_ = line_source;
      editor_->setModel(default_line_generator_.get());
      current_frame_ = nullptr;
    }

  protected:
    WaveLineSource* line_source_; ///< The associated WaveLineSource.
    WaveLineSource::WaveLineSourceKeyframe* current_frame_; ///< Currently selected frame data.

    std::unique_ptr<LineGenerator> default_line_generator_; ///< Default line generator model.
    std::unique_ptr<LineEditor> editor_;                    ///< Line editor for modifying line source points.

    std::unique_ptr<SynthSlider> pull_power_;  ///< Control for pull power parameter.
    std::unique_ptr<SynthSlider> horizontal_grid_; ///< Control for horizontal grid size.
    std::unique_ptr<SynthSlider> vertical_grid_;   ///< Control for vertical grid size.

    std::unique_ptr<Component> horizontal_incrementers_; ///< Incrementer buttons for horizontal grid slider.
    std::unique_ptr<Component> vertical_incrementers_;   ///< Incrementer buttons for vertical grid slider.

    std::unique_ptr<Slider> interpolation_selector_; ///< Possibly unused, defined in code for future functionality.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveLineSourceOverlay)
};

