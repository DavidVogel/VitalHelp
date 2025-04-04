#pragma once

#include "JuceHeader.h"

#include "common.h"
#include "open_gl_component.h"
#include "open_gl_multi_quad.h"

/**
 * @class BarRenderer
 * @brief A renderer for drawing a series of bars using OpenGL.
 *
 * This class creates and manages a collection of bars (vertical or horizontal),
 * and handles their geometry, scaling, coloring, and rendering. It uses OpenGL
 * for efficient rendering and can apply different scaling modes (linear, power,
 * square) to the bar heights.
 */
class BarRenderer : public OpenGlComponent {
public:
    /// A scaling constant used when applying power scaling.
    static constexpr float kScaleConstant = 5.0f;

    /// Number of float values per vertex (x, y, [and potentially other attributes]).
    static constexpr int kFloatsPerVertex = 3;

    /// Number of vertices per bar.
    static constexpr int kVerticesPerBar = 4;

    /// Number of floats per bar (4 vertices * 3 floats each).
    static constexpr int kFloatsPerBar = kVerticesPerBar * kFloatsPerVertex;

    /// Number of triangle indices per bar (each bar represented as two triangles).
    static constexpr int kTriangleIndicesPerBar = 6;

    /// Number of corner floats per vertex (used to determine corner coordinates).
    static constexpr int kCornerFloatsPerVertex = 2;

    /// Number of corner floats per bar.
    static constexpr int kCornerFloatsPerBar = kVerticesPerBar * kCornerFloatsPerVertex;

    /**
     * @brief Constructs a BarRenderer.
     * @param num_points The number of bars to render.
     * @param vertical If true, bars are oriented vertically; otherwise horizontally.
     */
    BarRenderer(int num_points, bool vertical = true);

    /**
     * @brief Destructor.
     */
    virtual ~BarRenderer();

    /**
     * @brief Initializes the renderer with an OpenGL context.
     * @param open_gl The OpenGL wrapper to use for initialization.
     */
    virtual void init(OpenGlWrapper& open_gl) override;

    /**
     * @brief Renders the bars using the current OpenGL context.
     * @param open_gl The OpenGL wrapper to use for rendering.
     * @param animate If true, allows for animated updates (unused here).
     */
    virtual void render(OpenGlWrapper& open_gl, bool animate) override;

    /**
     * @brief Destroys any allocated OpenGL resources.
     * @param open_gl The OpenGL wrapper to use for resource cleanup.
     */
    virtual void destroy(OpenGlWrapper& open_gl) override;

    /**
     * @brief Sets the color of the bars.
     * @param color The colour to set.
     */
    void setColor(const Colour& color) { color_ = color; }

    /**
     * @brief Sets the scaling factor for the bars.
     * @param scale The scaling factor.
     */
    void setScale(float scale) { scale_ = scale; }

    /**
     * @brief Sets an offset applied to the bar positions.
     * @param offset The offset value.
     */
    void setOffset(float offset) { offset_ = offset; }

    /**
     * @brief Sets the relative width of each bar.
     * @param bar_width The bar width factor.
     */
    void setBarWidth(float bar_width) { bar_width_ = bar_width; }

    /**
     * @brief Updates the number of points (bars) to display.
     * @param num_points The new number of bars.
     */
    void setNumPoints(int num_points) { num_points_ = num_points; }

    /**
     * @brief Gets the current bar width factor.
     * @return The bar width factor.
     */
    float getBarWidth() { return bar_width_; }

    /**
     * @brief Gets the x-position of the top-left vertex of the given bar.
     * @param index The bar index.
     * @return The x-position.
     */
    inline float xAt(int index) { return bar_data_[kFloatsPerBar * index]; }

    /**
     * @brief Gets the x-position of the top-right vertex of the given bar.
     * @param index The bar index.
     * @return The x-position.
     */
    inline float rightAt(int index) { return bar_data_[kFloatsPerBar * index + kFloatsPerVertex]; }

    /**
     * @brief Gets the y-position of the top-left vertex of the given bar.
     * @param index The bar index.
     * @return The y-position.
     */
    inline float yAt(int index) { return bar_data_[kFloatsPerBar * index + 1]; }

    /**
     * @brief Gets the y-position of the bottom-left vertex of the given bar.
     * @param index The bar index.
     * @return The y-position.
     */
    inline float bottomAt(int index) { return bar_data_[kFloatsPerBar * index + 2 * kFloatsPerVertex + 1]; }

    /**
     * @brief Sets the x-position for all vertices of a specific bar.
     * @param index The bar index.
     * @param val The new x-position.
     */
    force_inline void setX(int index, float val) {
        bar_data_[kFloatsPerBar * index] = val;
        bar_data_[kFloatsPerBar * index + 2 * kFloatsPerVertex] = val;
        bar_data_[kFloatsPerBar * index + kFloatsPerVertex] = val;
        bar_data_[kFloatsPerBar * index + 3 * kFloatsPerVertex] = val;
        dirty_ = true;
    }

    /**
     * @brief Sets the top y-position of a specific bar.
     * @param index The bar index.
     * @param val The new top y-position.
     */
    force_inline void setY(int index, float val) {
        bar_data_[kFloatsPerBar * index + 1] = val;
        bar_data_[kFloatsPerBar * index + kFloatsPerVertex + 1] = val;
        dirty_ = true;
    }

    /**
     * @brief Sets the bottom y-position of a specific bar.
     * @param index The bar index.
     * @param val The new bottom y-position.
     */
    force_inline void setBottom(int index, float val) {
        bar_data_[kFloatsPerBar * index + 2 * kFloatsPerVertex + 1] = val;
        bar_data_[kFloatsPerBar * index + 3 * kFloatsPerVertex + 1] = val;
        dirty_ = true;
    }

    /**
     * @brief Positions a bar at a specific rectangle.
     * @param index The bar index.
     * @param x The left x-position.
     * @param y The top y-position.
     * @param width The width of the bar.
     * @param height The height of the bar.
     */
    inline void positionBar(int index, float x, float y, float width, float height) {
        bar_data_[kFloatsPerBar * index] = x;
        bar_data_[kFloatsPerBar * index + 1] = y;

        bar_data_[kFloatsPerBar * index + kFloatsPerVertex] = x + width;
        bar_data_[kFloatsPerBar * index + kFloatsPerVertex + 1] = y;

        bar_data_[kFloatsPerBar * index + 2 * kFloatsPerVertex] = x;
        bar_data_[kFloatsPerBar * index + 2 * kFloatsPerVertex + 1] = y + height;

        bar_data_[kFloatsPerBar * index + 3 * kFloatsPerVertex] = x + width;
        bar_data_[kFloatsPerBar * index + 3 * kFloatsPerVertex + 1] = y + height;
        dirty_ = true;
    }

    /**
     * @brief Updates the bar sizes based on their positions and scaling.
     */
    void setBarSizes();

    /**
     * @brief Enables or disables power scaling of bar heights.
     * @param scale True to enable power scaling, false to disable.
     */
    void setPowerScale(bool scale);

    /**
     * @brief Enables or disables square scaling of bar heights.
     * @param scale True to enable square scaling, false to disable.
     */
    void setSquareScale(bool scale);

    /**
     * @brief Gets the scaled y-value of a bar at a given index.
     *
     * The scaling is applied based on the current mode (power/square).
     * @param index The bar index.
     * @return The scaled y-value in [0, 1].
     */
    force_inline float scaledYAt(int index) {
        float value = yAt(index) * 0.5f + 0.5f;
        if (square_scale_)
            value *= value;
        if (power_scale_)
            value /= std::max(index, 1) / kScaleConstant;

        return value;
    }

    /**
     * @brief Sets the scaled y-value at a specific index.
     *
     * This function takes a value in [0,1], applies inverse scaling based on the
     * current scaling modes, and updates the bar's actual y-position.
     * @param index The bar index.
     * @param val The scaled y-value [0,1].
     */
    force_inline void setScaledY(int index, float val) {
        float value = val;

        if (power_scale_)
            value *= std::max(index, 1) / kScaleConstant;
        if (square_scale_)
            value = sqrtf(value);

        setY(index, 2.0f * value - 1.0f);
    }

    /**
     * @brief Enables or disables additive blending for the bar rendering.
     * @param additive_blending True for additive blending, false for normal alpha blending.
     */
    force_inline void setAdditiveBlending(bool additive_blending) { additive_blending_ = additive_blending; }

protected:
    /**
     * @brief Draws the bars to the currently active OpenGL context.
     * @param open_gl The OpenGL wrapper.
     */
    void drawBars(OpenGlWrapper& open_gl);

    OpenGLShaderProgram* shader_;                                ///< The shader program used for rendering.
    std::unique_ptr<OpenGLShaderProgram::Uniform> color_uniform_; ///< Uniform for bar color.
    std::unique_ptr<OpenGLShaderProgram::Uniform> dimensions_uniform_; ///< Uniform for viewport dimensions.
    std::unique_ptr<OpenGLShaderProgram::Uniform> offset_uniform_; ///< Uniform for position offset.
    std::unique_ptr<OpenGLShaderProgram::Uniform> scale_uniform_; ///< Uniform for scaling factor.
    std::unique_ptr<OpenGLShaderProgram::Uniform> width_percent_uniform_; ///< Uniform for bar width factor.
    std::unique_ptr<OpenGLShaderProgram::Attribute> position_;    ///< Attribute for vertex position.
    std::unique_ptr<OpenGLShaderProgram::Attribute> corner_;      ///< Attribute for corner coordinates.

    Colour color_;         ///< Current color of the bars.
    bool vertical_;        ///< True if bars are rendered vertically.
    float scale_;          ///< Scale factor for bar dimensions.
    float offset_;         ///< Offset applied to bar positions.
    float bar_width_;      ///< Relative width of each bar.
    bool additive_blending_; ///< If true, uses additive blending for rendering.
    float display_scale_;  ///< Additional scaling factor applied to bar sizes.

    bool power_scale_;     ///< True if power scaling is applied to bar heights.
    bool square_scale_;    ///< True if square scaling is applied to bar heights.
    bool dirty_;           ///< True if bar data needs to be re-uploaded to the GPU.
    int num_points_;       ///< Number of bars to render.
    int total_points_;     ///< Total number of allocated points (bars).
    std::unique_ptr<float[]> bar_data_;       ///< Raw bar vertex position data.
    std::unique_ptr<float[]> bar_corner_data_;///< Bar corner coordinate data.
    std::unique_ptr<int[]> bar_indices_;      ///< Triangle index data for bars.
    GLuint bar_buffer_;    ///< OpenGL buffer object for bar positions.
    GLuint bar_corner_buffer_; ///< OpenGL buffer object for bar corners.
    GLuint bar_indices_buffer_; ///< OpenGL buffer object for bar indices.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BarRenderer)
};
