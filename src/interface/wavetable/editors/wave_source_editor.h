#pragma once

#include "JuceHeader.h"

#include "audio_file_drop_source.h"
#include "open_gl_line_renderer.h"
#include "open_gl_multi_quad.h"
#include "wavetable_component_overlay.h"
#include "common.h"

/**
 * @class WaveSourceEditor
 * @brief A graphical editor for manipulating a single-cycle waveform's sample values.
 *
 * The WaveSourceEditor displays a waveform as a line renderer and supports editing the waveform
 * by clicking and dragging. It can snap points to a user-defined grid and provides optional context
 * menu actions like clearing or flipping the waveform. It also supports drag-and-drop of audio files
 * that are translated into waveform data.
 */
class WaveSourceEditor : public OpenGlLineRenderer, public AudioFileDropSource {
  public:
    /**
     * @class Listener
     * @brief Interface for receiving notifications about waveform modifications.
     */
    class Listener {
      public:
        virtual ~Listener() { }

        /**
         * @brief Called when a portion of the waveform's values have changed.
         *
         * @param start   The start index of changed waveform samples.
         * @param end     The end index of changed waveform samples.
         * @param mouse_up Indicates if this update was triggered by a mouse-up event (finalizing changes).
         */
        virtual void valuesChanged(int start, int end, bool mouse_up) = 0;
    };

    /** Maximum grid divisions as defined by the WavetableComponentOverlay class. */
    static constexpr int kMaxGridParts = WavetableComponentOverlay::kMaxGrid + 1;
    /** The number of circles for grid intersections. */
    static constexpr int kNumCircles = kMaxGridParts * kMaxGridParts;

    /**
     * @enum WaveSourceMenu
     * @brief Context menu actions for waveform editing.
     */
    enum WaveSourceMenu {
      kCancel = 0,       /**< No operation. */
      kFlipHorizontal,   /**< Flip the waveform horizontally. */
      kFlipVertical,     /**< Flip the waveform vertically. */
      kClear,            /**< Clear the entire waveform to zero. */
      kInitSaw,          /**< Initialize waveform as a sawtooth wave (not currently implemented). */
    };

    /**
     * @brief Constructs a WaveSourceEditor with a given waveform size.
     *
     * @param size The number of samples in the waveform cycle to be edited.
     */
    WaveSourceEditor(int size);

    /**
     * @brief Destructor.
     */
    virtual ~WaveSourceEditor();

    /**
     * @brief Paints the background of the waveform editor.
     *
     * @param g Graphics context to draw into.
     */
    void paintBackground(Graphics& g) override;

    /**
     * @brief Adjusts layout and internal structures on component resize.
     */
    void resized() override;

    /**
     * @brief Initializes the OpenGL objects for drawing.
     *
     * @param open_gl A wrapper containing the OpenGL context and shaders.
     */
    virtual void init(OpenGlWrapper& open_gl) override {
      grid_lines_.init(open_gl);
      grid_circles_.init(open_gl);
      hover_circle_.init(open_gl);
      editing_line_.init(open_gl);
      OpenGlLineRenderer::init(open_gl);
    }

    /**
     * @brief Renders the waveform, grid lines, circles, and hover elements.
     *
     * @param open_gl A wrapper containing the OpenGL context.
     * @param animate Indicates if this render is part of an animation.
     */
    virtual void render(OpenGlWrapper& open_gl, bool animate) override {
      grid_lines_.render(open_gl, animate);
      grid_circles_.render(open_gl, animate);
      hover_circle_.render(open_gl, animate);
      if (editing_)
        editing_line_.render(open_gl, animate);
      OpenGlLineRenderer::render(open_gl, animate);
    }

    /**
     * @brief Cleans up the OpenGL resources used by the editor.
     *
     * @param open_gl A wrapper containing the OpenGL context.
     */
    virtual void destroy(OpenGlWrapper& open_gl) override {
      grid_lines_.destroy(open_gl);
      grid_circles_.destroy(open_gl);
      hover_circle_.destroy(open_gl);
      editing_line_.destroy(open_gl);
      OpenGlLineRenderer::destroy(open_gl);
    }

    /**
     * @brief Handles a mouse down event to start editing or show the context menu.
     *
     * @param e The mouse event.
     */
    void mouseDown(const MouseEvent& e) override;

    /**
     * @brief Handles a mouse up event to finalize editing and notify listeners.
     *
     * @param e The mouse event.
     */
    void mouseUp(const MouseEvent& e) override;

    /**
     * @brief Handles a mouse move event to update the hover position.
     *
     * @param e The mouse event.
     */
    void mouseMove(const MouseEvent& e) override;

    /**
     * @brief Handles a mouse drag event to continuously update waveform samples.
     *
     * @param e The mouse event.
     */
    void mouseDrag(const MouseEvent& e) override;

    /**
     * @brief Handles a mouse exit event to clear the hover indication.
     *
     * @param e The mouse event.
     */
    void mouseExit(const MouseEvent& e) override;

    /**
     * @brief Retrieves the waveform sample value at a given index.
     *
     * @param index The sample index.
     * @return The sample value.
     */
    float valueAt(int index) { return values_[index]; }

    /**
     * @brief Loads a complete waveform into the editor.
     *
     * @param waveform A pointer to an array of samples representing the waveform.
     */
    void loadWaveform(float* waveform);

    /**
     * @brief Adds a listener to be notified of value changes.
     *
     * @param listener The listener to add.
     */
    void addListener(Listener* listener) { listeners_.push_back(listener); }

    /**
     * @brief Enables or disables editing of the waveform.
     *
     * @param editable If true, the waveform can be edited by mouse.
     */
    void setEditable(bool editable);

    /**
     * @brief Defines a grid overlay for snapping points.
     *
     * @param horizontal The number of vertical grid lines.
     * @param vertical The number of horizontal grid lines.
     */
    void setGrid(int horizontal, int vertical);

    /**
     * @brief Called when an audio file is dropped and loaded successfully.
     *
     * @param file The file loaded.
     */
    void audioFileLoaded(const File& file) override;

    /**
     * @brief Called when dragging audio files enters the component.
     *
     * @param files The files being dragged.
     * @param x The x position of the drag.
     * @param y The y position of the drag.
     */
    void fileDragEnter(const StringArray& files, int x, int y) override;

    /**
     * @brief Called when dragging audio files leaves the component.
     *
     * @param files The files that were being dragged.
     */
    void fileDragExit(const StringArray& files) override;

    /**
     * @brief Clears the entire waveform to zero.
     */
    void clear();

    /**
     * @brief Flips the waveform vertically, inverting all sample values.
     */
    void flipVertical();

    /**
     * @brief Flips the waveform horizontally, reversing its sample order.
     */
    void flipHorizontal();

  private:
    /**
     * @brief Updates the grid positions of lines and circles for visualization.
     */
    void setGridPositions();

    /**
     * @brief Updates the hover circle to match the current mouse or edit position.
     */
    void setHoverPosition();

    /**
     * @brief Changes waveform values based on the mouse drag action.
     *
     * @param e The mouse event containing the new mouse position.
     */

    void changeValues(const MouseEvent& e);

    /**
     * @brief Returns a snapped point to the grid if close enough to a grid intersection.
     *
     * @param input The original mouse position.
     * @return The snapped point or the original position if no snap occurred.
     */
    Point<int> getSnappedPoint(Point<int> input);

    /**
     * @brief Snaps the input position to the nearest grid point if within snap radius.
     *
     * @param input The original mouse position.
     * @return The snapped point or last edit position if no snap occurred.
     */
    Point<int> snapToGrid(Point<int> input);

    /**
     * @brief Gets the index of the hovered sample based on mouse position.
     *
     * @param position Mouse position.
     * @return The closest sample index.
     */
    int getHoveredIndex(Point<int> position);

    /**
     * @brief Calculates the snap radius for determining if we should snap to grid points.
     *
     * @return The snap radius in pixels.
     */
    float getSnapRadius();

    /**
     * @brief Sets the line values after modifying the internal sample array.
     */
    void setLineValues();

    std::vector<Listener*> listeners_;   /**< Listeners for waveform value changes. */
    Point<int> last_edit_position_;      /**< Last position used for editing. */
    Point<int> current_mouse_position_;  /**< Current mouse position. */

    OpenGlMultiQuad grid_lines_;         /**< Renders grid lines for snapping visualization. */
    OpenGlMultiQuad grid_circles_;       /**< Renders circles at grid intersections. */
    OpenGlQuad hover_circle_;            /**< Renders a circle at the hovered point. */
    OpenGlLineRenderer editing_line_;     /**< Shows a line while editing between last and current mouse positions. */

    std::unique_ptr<AudioFileDropSource> audio_file_drop_source_; /**< Handles audio file drops. */
    std::unique_ptr<float[]> values_;    /**< The waveform samples being edited. */
    bool editing_;                       /**< True if currently editing waveform. */
    bool dragging_audio_file_;           /**< True if currently dragging an audio file over the component. */
    bool editable_;                      /**< True if waveform is editable. */
    int horizontal_grid_;                /**< Number of vertical grid lines. */
    int vertical_grid_;                  /**< Number of horizontal grid lines. */

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveSourceEditor)
};

