#pragma once

#include "JuceHeader.h"
#include "open_gl_component.h"
#include "synth_module.h"

/**
 * @class PeakMeterViewer
 * @brief A visual component to display a peak meter using OpenGL.
 *
 * The PeakMeterViewer class shows a peak meter (such as levels of audio signals) in a graphical form.
 * It uses two sets of vertices: one for the current peak and one for a peak memory reading.
 * The meter is displayed as a vertical bar that moves based on the dB values of the signal.
 */
class PeakMeterViewer : public OpenGlComponent {
public:
    /**
     * @brief Constructs a PeakMeterViewer.
     * @param left If true, displays the left channel of the peak meter, else the right channel.
     */
    PeakMeterViewer(bool left);

    /**
     * @brief Destructor.
     */
    virtual ~PeakMeterViewer();

    /**
     * @brief Called when the component is resized. Initializes status outputs if needed.
     */
    void resized() override;

    /**
     * @brief Initializes the OpenGL buffers and shader for drawing the meter.
     * @param open_gl The OpenGlWrapper with the current OpenGL context.
     */
    void init(OpenGlWrapper& open_gl) override;

    /**
     * @brief Renders the peak meter using OpenGL.
     * @param open_gl The OpenGlWrapper providing the current OpenGL context.
     * @param animate If true, any animations or transitions may be applied.
     */
    void render(OpenGlWrapper& open_gl, bool animate) override;

    /**
     * @brief Draws the peak meter quads according to current peak and memory values.
     * @param open_gl The OpenGlWrapper with the current OpenGL context.
     */
    void draw(OpenGlWrapper& open_gl);

    /**
     * @brief Releases any OpenGL resources associated with this component.
     * @param open_gl The OpenGlWrapper with the current OpenGL context.
     */
    void destroy(OpenGlWrapper& open_gl) override;

    /**
     * @brief Paints a background for the meter, showing ranges or thresholds.
     * @param g The JUCE Graphics context.
     */
    void paintBackground(Graphics& g) override;

private:
    /**
     * @brief Updates the vertex data for the current peak reading.
     */
    void updateVertices();

    /**
     * @brief Updates the vertex data for the peak memory reading.
     */
    void updateVerticesMemory();

    static constexpr int kNumPositions = 8;     ///< Number of floats describing vertex positions (4 vertices * 2 floats).
    static constexpr int kNumTriangleIndices = 6; ///< Number of indices for drawing 2 triangles.

    const vital::StatusOutput* peak_output_;        ///< Status output for the current peak level.
    const vital::StatusOutput* peak_memory_output_; ///< Status output for the peak memory (max peak).

    OpenGLShaderProgram* shader_; ///< Shader program for rendering the meter.
    std::unique_ptr<OpenGLShaderProgram::Attribute> position_;    ///< Attribute for vertex positions.
    std::unique_ptr<OpenGLShaderProgram::Uniform> color_from_;    ///< Uniform for gradient start color.
    std::unique_ptr<OpenGLShaderProgram::Uniform> color_to_;      ///< Uniform for gradient end color.

    float clamped_;                  ///< Tracks if the meter has hit a "clamped" (max) state.
    float position_vertices_[kNumPositions]; ///< Vertex position array.
    int position_triangles_[kNumTriangleIndices]; ///< Index array for drawing the meter quad.
    GLuint vertex_buffer_;           ///< OpenGL buffer handle for vertex data.
    GLuint triangle_buffer_;         ///< OpenGL buffer handle for index data.
    bool left_;                      ///< True if displaying the left channel, false if right.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PeakMeterViewer)
};
