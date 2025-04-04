#pragma once

#include "JuceHeader.h"
#include "common.h"
#include <map>

/**
 * @class Shaders
 * @brief Manages and provides access to vertex and fragment shaders used by the OpenGL rendering pipeline.
 *
 * This class compiles and links various vertex and fragment shaders used throughout the UI.
 * Shaders are retrieved and cached as needed. It supports multiple categories of shaders
 * for different rendering tasks, such as image rendering, filter response curves, modulation arcs,
 * and more.
 *
 * Shaders are stored as enums and can be requested by their enum values. The class ensures that each
 * shader is compiled once, and it creates specialized shader programs by linking vertex and fragment shaders.
 */
class Shaders {
public:
    /**
     * @enum VertexShader
     * @brief An enumeration of all available vertex shaders.
     *
     * Each vertex shader corresponds to a certain type of geometry or pipeline configuration.
     */
    enum VertexShader {
        kImageVertex,
        kPassthroughVertex,
        kScaleVertex,
        kRotaryModulationVertex,
        kLinearModulationVertex,
        kGainMeterVertex,
        kAnalogFilterResponseVertex,
        kCombFilterResponseVertex,
        kPositiveFlangeFilterResponseVertex,
        kNegativeFlangeFilterResponseVertex,
        kDigitalFilterResponseVertex,
        kDiodeFilterResponseVertex,
        kDirtyFilterResponseVertex,
        kFormantFilterResponseVertex,
        kLadderFilterResponseVertex,
        kPhaserFilterResponseVertex,
        kEqFilterResponseVertex,
        kLineVertex,
        kFillVertex,
        kBarHorizontalVertex,
        kBarVerticalVertex,
        kNumVertexShaders
    };

    /**
     * @enum FragmentShader
     * @brief An enumeration of all available fragment shaders.
     *
     * Fragment shaders handle pixel-level rendering for different effects and graphical elements.
     */
    enum FragmentShader {
        kImageFragment,
        kTintedImageFragment,
        kGainMeterFragment,
        kFilterResponseFragment,
        kColorFragment,
        kFadeSquareFragment,
        kCircleFragment,
        kRingFragment,
        kDiamondFragment,
        kRoundedCornerFragment,
        kRoundedRectangleFragment,
        kRoundedRectangleBorderFragment,
        kRotarySliderFragment,
        kRotaryModulationFragment,
        kHorizontalSliderFragment,
        kVerticalSliderFragment,
        kLinearModulationFragment,
        kModulationKnobFragment,
        kLineFragment,
        kFillFragment,
        kBarFragment,
        kNumFragmentShaders
    };

    /**
     * @brief Constructs a Shaders object associated with an OpenGLContext.
     * @param open_gl_context The OpenGLContext used for shader compilation and linking.
     */
    Shaders(OpenGLContext& open_gl_context);

    /**
     * @brief Retrieves the OpenGL shader ID for a given vertex shader.
     * @param shader The vertex shader enum value.
     * @return The compiled OpenGL shader ID.
     */
    GLuint getVertexShaderId(VertexShader shader) {
        if (vertex_shader_ids_[shader] == 0)
            vertex_shader_ids_[shader] = createVertexShader(open_gl_context_->extensions, shader);
        return vertex_shader_ids_[shader];
    }

    /**
     * @brief Retrieves the OpenGL shader ID for a given fragment shader.
     * @param shader The fragment shader enum value.
     * @return The compiled OpenGL shader ID.
     */
    GLuint getFragmentShaderId(FragmentShader shader) {
        if (fragment_shader_ids_[shader] == 0)
            fragment_shader_ids_[shader] = createFragmentShader(open_gl_context_->extensions, shader);
        return fragment_shader_ids_[shader];
    }

    /**
     * @brief Retrieves or creates an OpenGLShaderProgram from a given vertex and fragment shader pair.
     * @param vertex_shader The vertex shader enum value.
     * @param fragment_shader The fragment shader enum value.
     * @param varyings Optional transform feedback varyings.
     * @return A pointer to the linked OpenGLShaderProgram.
     */
    OpenGLShaderProgram* getShaderProgram(VertexShader vertex_shader, FragmentShader fragment_shader,
                                          const GLchar** varyings = nullptr);

private:
    /**
     * @brief Retrieves the source code for a given VertexShader.
     * @param shader The vertex shader enum value.
     * @return The source code as a const char*.
     */
    static const char* getVertexShader(VertexShader shader);

    /**
     * @brief Retrieves the source code for a given FragmentShader.
     * @param shader The fragment shader enum value.
     * @return The source code as a const char*.
     */
    static const char* getFragmentShader(FragmentShader shader);

    /**
     * @brief Checks if a shader compiled successfully.
     * @param extensions The OpenGLExtensionFunctions for querying shader info.
     * @param shader_id The OpenGL shader object ID.
     * @return True if the shader compiled successfully, false otherwise.
     */
    bool checkShaderCorrect(OpenGLExtensionFunctions& extensions, GLuint shader_id) const;

    /**
     * @brief Compiles a given vertex shader.
     * @param extensions The OpenGLExtensionFunctions used for shader compilation.
     * @param shader The vertex shader enum value.
     * @return The compiled OpenGL shader ID.
     */
    GLuint createVertexShader(OpenGLExtensionFunctions& extensions, VertexShader shader) const;

    /**
     * @brief Compiles a given fragment shader.
     * @param extensions The OpenGLExtensionFunctions used for shader compilation.
     * @param shader The fragment shader enum value.
     * @return The compiled OpenGL shader ID.
     */
    GLuint createFragmentShader(OpenGLExtensionFunctions& extensions, FragmentShader shader) const;

    OpenGLContext* open_gl_context_; ///< Pointer to the associated OpenGLContext.
    GLuint vertex_shader_ids_[kNumVertexShaders];   ///< Cached vertex shader IDs.
    GLuint fragment_shader_ids_[kNumFragmentShaders]; ///< Cached fragment shader IDs.

    std::map<int, std::unique_ptr<OpenGLShaderProgram>> shader_programs_; ///< Cache of linked shader programs.
};

/**
 * @struct OpenGlWrapper
 * @brief A helper struct containing references to OpenGL context, shaders, and display scale.
 *
 * This struct simplifies passing around OpenGL-related parameters (context, shader manager, display scale)
 * to rendering functions.
 */
struct OpenGlWrapper {
    /**
     * @brief Constructs an OpenGlWrapper.
     * @param c Reference to the OpenGLContext.
     */
    OpenGlWrapper(OpenGLContext& c) : context(c), shaders(nullptr), display_scale(1.0f) { }

    OpenGLContext& context; ///< The OpenGLContext for current rendering.
    Shaders* shaders;       ///< Pointer to the Shaders instance providing compiled shaders.
    float display_scale;    ///< Display scaling factor for high-DPI rendering.
};
