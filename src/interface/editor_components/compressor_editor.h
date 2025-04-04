#pragma once

#include "JuceHeader.h"

#include "line_generator.h"
#include "open_gl_component.h"
#include "open_gl_multi_quad.h"
#include "synth_slider.h"
#include "synth_module.h"

class SynthGuiInterface;

/**
 * @class CompressorEditor
 * @brief A graphical interface component for editing a multiband compressor's thresholds and ratios.
 *
 * The CompressorEditor allows for the visualization and interactive editing of multiple
 * compression bands (low, band, and high). It displays threshold lines, input/output levels,
 * and ratio lines, and allows users to manipulate these values via mouse interaction.
 * The component uses OpenGL for efficient rendering.
 */
class CompressorEditor : public OpenGlComponent, public SynthSlider::SliderListener {
  public:
    /// Grab radius in pixels for clickable points (thresholds, ratio handles).
    static constexpr float kGrabRadius = 8.0f;

    /// Minimum decibel value displayed in the compressor editor.
    static constexpr float kMinDb = -80.0f;
    /// Maximum decibel value displayed in the compressor editor.
    static constexpr float kMaxDb = 0.0f;

    /// Decibel buffer around the min and max dB edit range.
    static constexpr float kDbEditBuffer = 1.0f;
    /// Minimum editable dB value (slightly above kMinDb).
    static constexpr float kMinEditDb = kMinDb + kDbEditBuffer;
    /// Maximum editable dB value (slightly below kMaxDb).
    static constexpr float kMaxEditDb = kMaxDb - kDbEditBuffer;

    /// Minimum ratio value for the lower ratio segments.
    static constexpr float kMinLowerRatio = -1.0f;
    /// Maximum ratio value for the lower ratio segments.
    static constexpr float kMaxLowerRatio = 1.0f;
    /// Minimum ratio value for the upper ratio segments.
    static constexpr float kMinUpperRatio = 0.0f;
    /// Maximum ratio value for the upper ratio segments.
    static constexpr float kMaxUpperRatio = 1.0f;

    /// Multiplier applied to ratio edits for finer control.
    static constexpr float kRatioEditMultiplier = 0.6f;

    /// Buffer area around the compressor visualization.
    static constexpr float kCompressorAreaBuffer = 0.05f;
    /// Width of the displayed bars relative to the component width.
    static constexpr float kBarWidth = 1.0f / 5.0f;

    /// Radius for input line rendering in normalized coordinates.
    static constexpr float kInputLineRadius = 0.02f;

    /// General multiplier for mouse-drag edits.
    static constexpr float kMouseMultiplier = 1.0f;

    /// Maximum number of bands supported.
    static constexpr int kMaxBands = 3;
    /// Number of channels (2 channels per band).
    static constexpr int kNumChannels = kMaxBands * 2;

    /// Number of dB line sections in the display.
    static constexpr int kDbLineSections = 8;
    /// Extra dB lines to draw for extended resolution.
    static constexpr int kExtraDbLines = 6;
    /// Total number of dB lines for ratio calculations.
    static constexpr int kRatioDbLines = kDbLineSections + kExtraDbLines;
    /// Total number of ratio lines drawn for all bands.
    static constexpr int kTotalRatioLines = kRatioDbLines * kNumChannels;

    /**
     * @brief Constructs a new CompressorEditor.
     */
    CompressorEditor();

    /**
     * @brief Destructor.
     */
    virtual ~CompressorEditor();

    /**
     * @brief Draws the background of the editor.
     * @param g The JUCE graphics context to draw with.
     */
    void paintBackground(Graphics& g) override;

    /**
     * @brief Called when the component is resized.
     */
    void resized() override;

    /**
     * @brief Handles mouse down events.
     * @param e The mouse event.
     */
    void mouseDown(const MouseEvent& e) override;

    /**
     * @brief Handles mouse double-click events.
     * @param e The mouse event.
     */
    void mouseDoubleClick(const MouseEvent& e) override;

    /**
     * @brief Handles mouse move events (primarily used for updating hover state).
     * @param e The mouse event.
     */
    void mouseMove(const MouseEvent& e) override;

    /**
     * @brief Handles mouse drag events.
     * @param e The mouse event.
     */
    void mouseDrag(const MouseEvent& e) override;

    /**
     * @brief Handles mouse up events.
     * @param e The mouse event.
     */
    void mouseUp(const MouseEvent& e) override;

    /**
     * @brief Handles mouse exit events.
     * @param e The mouse event.
     */
    void mouseExit(const MouseEvent& e) override;

    /**
     * @brief Called when the component's parent hierarchy changes, used for initialization.
     */
    void parentHierarchyChanged() override;

    /**
     * @brief Initializes the OpenGL context and related objects.
     * @param open_gl The OpenGlWrapper containing OpenGL context information.
     */
    void init(OpenGlWrapper& open_gl) override;

    /**
     * @brief Renders the compressor visualization.
     * @param open_gl The OpenGlWrapper containing OpenGL context information.
     * @param animate If true, indicates that animation may be applied.
     */
    void render(OpenGlWrapper& open_gl, bool animate) override;

    /**
     * @brief Renders the compressor-specific elements.
     * @param open_gl The OpenGlWrapper containing OpenGL context information.
     * @param animate If true, indicates that animation may be applied.
     */
    void renderCompressor(OpenGlWrapper& open_gl, bool animate);

    /**
     * @brief Destroys and cleans up OpenGL-related objects.
     * @param open_gl The OpenGlWrapper containing OpenGL context information.
     */
    void destroy(OpenGlWrapper& open_gl) override;

    /**
     * @brief Sets the size ratio, scaling the display proportionally.
     * @param ratio The new size ratio.
     */
    void setSizeRatio(float ratio) { size_ratio_ = ratio; }

    /**
     * @brief Sets all threshold and ratio values from a given control map.
     * @param controls A control map containing named parameters and their values.
     */
    void setAllValues(vital::control_map& controls);

    /**
     * @brief Sets the activity state of the high band.
     * @param active True to activate the high band, false otherwise.
     */
    void setHighBandActive(bool active) { high_band_active_ = active; }

    /**
     * @brief Sets the activity state of the low band.
     * @param active True to activate the low band, false otherwise.
     */
    void setLowBandActive(bool active) { low_band_active_ = active; }

    /**
     * @brief Sets the active state of the entire compressor editor.
     * @param active True to activate, false to deactivate.
     */
    void setActive(bool active) { active_ = active; }

  private:
    /**
     * @enum DragPoint
     * @brief Identifies different draggable points in the compressor editor (thresholds/ratios).
     */
    enum DragPoint {
      kNone,
      kLowUpperThreshold,
      kBandUpperThreshold,
      kHighUpperThreshold,
      kLowLowerThreshold,
      kBandLowerThreshold,
      kHighLowerThreshold,
      kLowUpperRatio,
      kBandUpperRatio,
      kHighUpperRatio,
      kLowLowerRatio,
      kBandLowerRatio,
      kHighLowerRatio,
      kNumDragPoints
    };

    /**
     * @brief Checks if a given drag point corresponds to a ratio control.
     * @param drag_point The drag point to check.
     * @return True if the point corresponds to a ratio control, false otherwise.
     */
    bool isRatio(DragPoint drag_point) {
      return drag_point == kLowLowerRatio || drag_point == kBandLowerRatio || drag_point == kHighLowerRatio ||
             drag_point == kLowUpperRatio || drag_point == kBandUpperRatio || drag_point == kHighUpperRatio;
    }

    /**
     * @brief Updates threshold positions on the display.
     * @param low_start Start pixel for the low band area.
     * @param low_end End pixel for the low band area.
     * @param band_start Start pixel for the band area.
     * @param band_end End pixel for the band area.
     * @param high_start Start pixel for the high band area.
     * @param high_end End pixel for the high band area.
     * @param ratio_match The ratio value to match for drawing threshold quads.
     */
    void setThresholdPositions(int low_start, int low_end, int band_start, int band_end,
                               int high_start, int high_end, float ratio_match);

    /**
     * @brief Sets the ratio line quads for a given band segment.
     * @param start_index The starting index in the ratio_lines_ quad array.
     * @param start_x The start pixel x-position of the band segment.
     * @param end_x The end pixel x-position of the band segment.
     * @param threshold The threshold dB value for the band.
     * @param ratio The ratio value for the band.
     * @param upper True if this is an upper ratio line, false if lower.
     * @param hover True if the user is hovering over this ratio line.
     */
    void setRatioLines(int start_index, int start_x, int end_x, float threshold, float ratio, bool upper, bool hover);

    /**
     * @brief Positions the ratio lines for all bands.
     * @param low_start Start pixel for the low band area.
     * @param low_end End pixel for the low band area.
     * @param band_start Start pixel for the band area.
     * @param band_end End pixel for the band area.
     * @param high_start Start pixel for the high band area.
     * @param high_end End pixel for the high band area.
     */
    void setRatioLinePositions(int low_start, int low_end, int band_start, int band_end,
                               int high_start, int high_end);

    /**
     * @brief Renders hover highlights over thresholds or ratio lines when hovered.
     * @param open_gl The OpenGlWrapper containing OpenGL context information.
     * @param low_start Start pixel for the low band area.
     * @param low_end End pixel for the low band area.
     * @param band_start Start pixel for the band area.
     * @param band_end End pixel for the band area.
     * @param high_start Start pixel for the high band area.
     * @param high_end End pixel for the high band area.
     */
    void renderHover(OpenGlWrapper& open_gl, int low_start, int low_end,
                     int band_start, int band_end, int high_start, int high_end);

    /**
     * @brief Sets various threshold values for the low, band, and high bands.
     * @param db The threshold dB value to set.
     * @param clamp If true, clamps the threshold within allowable edit range.
     */
    void setLowUpperThreshold(float db, bool clamp);
    void setBandUpperThreshold(float db, bool clamp);
    void setHighUpperThreshold(float db, bool clamp);
    void setLowLowerThreshold(float db, bool clamp);
    void setBandLowerThreshold(float db, bool clamp);
    void setHighLowerThreshold(float db, bool clamp);

    /**
     * @brief Sets ratio values for the low, band, and high bands.
     * @param ratio The ratio value to set (clamped).
     */
    void setLowUpperRatio(float ratio);
    void setBandUpperRatio(float ratio);
    void setHighUpperRatio(float ratio);
    void setLowLowerRatio(float ratio);
    void setBandLowerRatio(float ratio);
    void setHighLowerRatio(float ratio);

    /**
     * @brief Formats a given value into a string for display (e.g., popup).
     * @param value The value to format.
     * @return A formatted string representation.
     */
    String formatValue(float value);

    /**
     * @brief Determines which draggable point (if any) is being hovered by the mouse.
     * @param e The mouse event.
     * @return The DragPoint under the cursor, or kNone if none.
     */
    DragPoint getHoverPoint(const MouseEvent& e);

    /**
     * @brief Converts a dB value to a pixel Y position on the editor.
     * @param db The decibel value to convert.
     * @return The Y position in pixels.
     */
    float getYForDb(float db);

    /**
     * @brief Calculates the output dB after compression based on thresholds and ratios.
     * @param input_db Input decibel value.
     * @param upper_threshold The upper threshold dB.
     * @param upper_ratio The upper ratio.
     * @param lower_threshold The lower threshold dB.
     * @param lower_ratio The lower ratio.
     * @return The compressed dB value.
     */
    float getCompressedDb(float input_db, float upper_threshold, float upper_ratio,
                          float lower_threshold, float lower_ratio);

    /**
     * @brief Determines the color to use for a given ratio value.
     * @param ratio The ratio value.
     * @return A JUCE Colour based on the ratio state.
     */
    Colour getColorForRatio(float ratio);

    SynthGuiInterface* parent_;          ///< Pointer to the parent synth GUI interface.
    SynthSection* section_parent_;       ///< Pointer to the parent synth section.

    DragPoint hover_;                    ///< Current drag point being hovered.
    Point<int> last_mouse_position_;     ///< Last known mouse position.

    OpenGlQuad hover_quad_;              ///< Quad used for hover highlighting.
    OpenGlMultiQuad input_dbs_;          ///< Quads representing input dB values for each band.
    OpenGlMultiQuad output_dbs_;         ///< Quads representing output dB values for each band.
    OpenGlMultiQuad thresholds_;         ///< Quads representing threshold areas.
    OpenGlMultiQuad ratio_lines_;        ///< Quads representing ratio lines.

    float low_upper_threshold_;          ///< Low band upper threshold in dB.
    float band_upper_threshold_;         ///< Band upper threshold in dB.
    float high_upper_threshold_;         ///< High band upper threshold in dB.
    float low_lower_threshold_;          ///< Low band lower threshold in dB.
    float band_lower_threshold_;         ///< Band lower threshold in dB.
    float high_lower_threshold_;         ///< High band lower threshold in dB.

    float low_upper_ratio_;              ///< Low band upper ratio.
    float band_upper_ratio_;             ///< Band upper ratio.
    float high_upper_ratio_;             ///< High band upper ratio.
    float low_lower_ratio_;              ///< Low band lower ratio.
    float band_lower_ratio_;             ///< Band lower ratio.
    float high_lower_ratio_;             ///< High band lower ratio.

    const vital::StatusOutput* low_input_ms_;   ///< Low band input magnitude squared.
    const vital::StatusOutput* band_input_ms_;  ///< Band input magnitude squared.
    const vital::StatusOutput* high_input_ms_;  ///< High band input magnitude squared.

    const vital::StatusOutput* low_output_ms_;  ///< Low band output magnitude squared.
    const vital::StatusOutput* band_output_ms_; ///< Band output magnitude squared.
    const vital::StatusOutput* high_output_ms_; ///< High band output magnitude squared.

    float size_ratio_;                   ///< Scaling ratio for display size.
    bool animate_;                       ///< Indicates if the display should animate.
    bool active_;                        ///< Whether the compressor editor is active.
    bool high_band_active_;              ///< Whether the high band is active.
    bool low_band_active_;               ///< Whether the low band is active.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CompressorEditor)
};
