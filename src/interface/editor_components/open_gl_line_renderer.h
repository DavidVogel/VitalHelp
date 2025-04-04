#pragma once

#include "JuceHeader.h"
#include "common.h"
#include "open_gl_component.h"
#include "open_gl_multi_quad.h"

/**
 * @class OpenGlLineRenderer
 * @brief A component for rendering lines with optional filling and boost effects using OpenGL.
 *
 * The OpenGlLineRenderer class takes a set of points and renders them as a smooth line using OpenGL,
 * optionally adding fill and adjustable "boost" effects to create animated line deformations. It
 * supports looping lines, color customization, and variable line widths.
 */
class OpenGlLineRenderer : public OpenGlComponent {
public:
    /// Floats per vertex in the line data (x, y, and potentially others).
    static constexpr int kLineFloatsPerVertex = 3;
    /// Floats per vertex in the fill data (x, y, and boost value).
    static constexpr int kFillFloatsPerVertex = 4;
    /// Number of vertices per point in the line representation.
    static constexpr int kLineVerticesPerPoint = 6;
    /// Number of vertices per point in the fill representation.
    static constexpr int kFillVerticesPerPoint = 2;
    /// Floats per point in the line data (6 vertices * 3 floats each).
    static constexpr int kLineFloatsPerPoint = kLineVerticesPerPoint * kLineFloatsPerVertex;
    /// Floats per point in the fill data (2 vertices * 4 floats each).
    static constexpr int kFillFloatsPerPoint = kFillVerticesPerPoint * kFillFloatsPerVertex;

    /**
     * @brief Constructs an OpenGlLineRenderer for a given number of points.
     * @param num_points The number of points in the line.
     * @param loop If true, the line is treated as looping (connects end-to-start).
     */
    OpenGlLineRenderer(int num_points, bool loop = false);

    /**
     * @brief Destructor.
     */
    virtual ~OpenGlLineRenderer();

    /**
     * @brief Initializes OpenGL resources for rendering the line.
     * @param open_gl The OpenGlWrapper providing the OpenGL context.
     */
    virtual void init(OpenGlWrapper& open_gl) override;

    /**
     * @brief Renders the line using OpenGL.
     * @param open_gl The OpenGlWrapper providing the OpenGL context.
     * @param animate If true, animations (like boosts) are allowed.
     */
    virtual void render(OpenGlWrapper& open_gl, bool animate) override;

    /**
     * @brief Destroys OpenGL resources allocated by this line renderer.
     * @param open_gl The OpenGlWrapper providing the OpenGL context.
     */
    virtual void destroy(OpenGlWrapper& open_gl) override;

    /// Sets the line color.
    force_inline void setColor(Colour color) { color_ = color; }

    /// Sets the line width in pixels.
    force_inline void setLineWidth(float width) { line_width_ = width; }

    /// Sets a global boost value affecting line thickness.
    force_inline void setBoost(float boost) { boost_ = boost; }

    /// Gets the left-side boost at a given point index.
    force_inline float boostLeftAt(int index) const { return boost_left_[index]; }

    /// Gets the right-side boost at a given point index.
    force_inline float boostRightAt(int index) const { return boost_right_[index]; }

    /// Gets the y-coordinate of a point at a given index.
    force_inline float yAt(int index) const { return y_[index]; }

    /// Gets the x-coordinate of a point at a given index.
    force_inline float xAt(int index) const { return x_[index]; }

    /// Sets the left-side boost for a point, marking data as dirty.
    force_inline void setBoostLeft(int index, float val) {
        boost_left_[index] = val;
        dirty_ = true;
        VITAL_ASSERT(num_points_ > index);
    }

    /// Sets the right-side boost for a point, marking data as dirty.
    force_inline void setBoostRight(int index, float val) {
        boost_right_[index] = val;
        dirty_ = true;
        VITAL_ASSERT(num_points_ > index);
    }

    /// Sets the y-coordinate of a point, marking data as dirty.
    force_inline void setYAt(int index, float val) {
        y_[index] = val;
        dirty_ = true;
        VITAL_ASSERT(num_points_ > index);
    }

    /// Sets the x-coordinate of a point, marking data as dirty.
    force_inline void setXAt(int index, float val) {
        x_[index] = val;
        dirty_ = true;
        VITAL_ASSERT(num_points_ > index);
    }

    /**
     * @brief Sets fill vertices according to the current line and boost data.
     * @param left If true, sets vertices based on left-side boosting, otherwise right side.
     */
    void setFillVertices(bool left);

    /**
     * @brief Sets line vertices according to the current line and boost data.
     * @param left If true, sets vertices based on left-side boosting, otherwise right side.
     */
    void setLineVertices(bool left);

    /// Enables or disables filling below the line.
    force_inline void setFill(bool fill) { fill_ = fill; }

    /// Sets a uniform fill color if only one color is needed.
    force_inline void setFillColor(Colour fill_color) {
        setFillColors(fill_color, fill_color);
    }

    /// Sets a gradient fill from one color to another.
    force_inline void setFillColors(Colour fill_color_from, Colour fill_color_to) {
        fill_color_from_ = fill_color_from;
        fill_color_to_ = fill_color_to;
    }

    /// Sets the vertical center for the fill area.
    force_inline void setFillCenter(float fill_center) { fill_center_ = fill_center; }

    /// Enables fitting the line inside the available area.
    force_inline void setFit(bool fit) { fit_ = fit; }

    /// Sets the boost amount that affects line thickness.
    force_inline void setBoostAmount(float boost_amount) { boost_amount_ = boost_amount; }

    /// Sets the boost amount that affects fill thickness.
    force_inline void setFillBoostAmount(float boost_amount) { fill_boost_amount_ = boost_amount; }

    /// Sets an index used for custom behavior (e.g., multiple line sets).
    force_inline void setIndex(int index) { index_ = index; }

    /**
     * @brief Boosts left-side range of the line.
     * @param start The normalized start position in [0, 1].
     * @param end The normalized end position in [0, 1].
     * @param buffer_vertices How many vertices to skip at start/end (padding).
     * @param min The minimum boost value.
     */
    void boostLeftRange(float start, float end, int buffer_vertices, float min);

    /**
     * @brief Boosts right-side range of the line.
     */
    void boostRightRange(float start, float end, int buffer_vertices, float min);

    /**
     * @brief Boosts a range for the given boost array.
     */
    void boostRange(float* boosts, float start, float end, int buffer_vertices, float min);

    /**
     * @brief Boosts left and right arrays using poly_float parameters.
     */
    void boostRange(vital::poly_float start, vital::poly_float end, int buffer_vertices, vital::poly_float min);

    /**
     * @brief Decays all boosts by a multiplicative factor, allowing animated damping.
     * @param mult The multiplicative factor to decay the boosts by.
     */
    void decayBoosts(vital::poly_float mult);

    /// Enables backward boost calculation for symmetrical line deformation.
    void enableBackwardBoost(bool enable) { enable_backward_boost_ = enable; }

    /// Gets the number of points in the line.
    force_inline int numPoints() const { return num_points_; }

    /// Gets the current line color.
    force_inline Colour color() const { return color_; }

    /**
     * @brief Draws the line and optional fill using OpenGL.
     * @param open_gl The OpenGlWrapper with the current OpenGL context.
     * @param left If true, uses left-side boosts; otherwise right-side.
     */
    void drawLines(OpenGlWrapper& open_gl, bool left);

    /**
     * @brief Checks if any boost value is set.
     * @return True if there's any non-zero boost, otherwise false.
     */
    bool anyBoostValue() { return any_boost_value_; }

private:
    Colour color_;              ///< The line color.
    Colour fill_color_from_;    ///< The start color for fill gradient.
    Colour fill_color_to_;      ///< The end color for fill gradient.

    int num_points_;            ///< Number of points defining the line.
    float line_width_;          ///< The line width in pixels.
    float boost_;               ///< Global boost affecting line thickness.
    bool fill_;                 ///< If true, draw a filled area under the line.
    float fill_center_;         ///< The vertical center position for the fill.
    bool fit_;                  ///< If true, attempt to fit the line in the available space.

    float boost_amount_;        ///< Additional boost applied to line thickness.
    float fill_boost_amount_;   ///< Additional boost applied to fill thickness.
    bool enable_backward_boost_;///< If true, allow negative direction boosts.
    int index_;                 ///< Index for multiple line sets if needed.

    bool dirty_;                ///< Indicates if line data needs re-uploading to GPU.
    bool last_drawn_left_;      ///< Tracks last drawn side for boosting.
    bool last_negative_boost_;  ///< Tracks if last boost was negative.
    bool loop_;                 ///< If true, the line is closed (looping).
    bool any_boost_value_;      ///< True if any boost value is active.
    int num_padding_;           ///< Number of padding points for looped lines.
    int num_line_vertices_;     ///< Number of line vertices total.
    int num_fill_vertices_;     ///< Number of fill vertices total.
    int num_line_floats_;       ///< Number of floats in the line data array.
    int num_fill_floats_;       ///< Number of floats in the fill data array.

    OpenGLShaderProgram* shader_;                 ///< Shader program for line rendering.
    std::unique_ptr<OpenGLShaderProgram::Uniform> scale_uniform_;     ///< Uniform for scaling line.
    std::unique_ptr<OpenGLShaderProgram::Uniform> color_uniform_;     ///< Uniform for line color.
    std::unique_ptr<OpenGLShaderProgram::Uniform> boost_uniform_;     ///< Uniform for global boost.
    std::unique_ptr<OpenGLShaderProgram::Uniform> line_width_uniform_;///< Uniform for line width.
    std::unique_ptr<OpenGLShaderProgram::Attribute> position_;        ///< Attribute for line vertex positions.

    OpenGLShaderProgram* fill_shader_;             ///< Shader program for fill rendering.
    std::unique_ptr<OpenGLShaderProgram::Uniform> fill_scale_uniform_; ///< Uniform for fill scale.
    std::unique_ptr<OpenGLShaderProgram::Uniform> fill_color_from_uniform_; ///< Uniform for fill start color.
    std::unique_ptr<OpenGLShaderProgram::Uniform> fill_color_to_uniform_;   ///< Uniform for fill end color.
    std::unique_ptr<OpenGLShaderProgram::Uniform> fill_center_uniform_;     ///< Uniform for fill vertical center.
    std::unique_ptr<OpenGLShaderProgram::Uniform> fill_boost_amount_uniform_;///< Uniform for fill boost.
    std::unique_ptr<OpenGLShaderProgram::Attribute> fill_position_;          ///< Attribute for fill vertex positions.

    GLuint vertex_array_object_; ///< Vertex array object handle.
    GLuint line_buffer_;         ///< GPU buffer for line vertices.
    GLuint fill_buffer_;         ///< GPU buffer for fill vertices.
    GLuint indices_buffer_;      ///< GPU buffer for triangle indices.

    std::unique_ptr<float[]> x_;           ///< X-coordinates of points.
    std::unique_ptr<float[]> y_;           ///< Y-coordinates of points.
    std::unique_ptr<float[]> boost_left_;  ///< Left-side boost values.
    std::unique_ptr<float[]> boost_right_; ///< Right-side boost values.
    std::unique_ptr<float[]> line_data_;   ///< Combined line vertex data.
    std::unique_ptr<float[]> fill_data_;   ///< Combined fill vertex data.
    std::unique_ptr<int[]> indices_data_;  ///< Index data for drawing line and fill.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OpenGlLineRenderer)
};
