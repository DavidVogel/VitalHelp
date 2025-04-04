#pragma once

#include "JuceHeader.h"
#include "open_gl_component.h"

#include <mutex>

/**
 * @class OpenGlBackground
 * @brief A class that manages and renders a background image using OpenGL.
 *
 * The OpenGlBackground class is responsible for loading, updating, and displaying
 * a background image with OpenGL. It provides thread-safe updating of the background image,
 * manages vertex buffers, and sets up shader attributes for rendering the image.
 */
class OpenGlBackground {
public:
    /**
     * @brief Constructs an OpenGlBackground with no initial image.
     */
    OpenGlBackground();

    /**
     * @brief Destructor. Frees OpenGL resources if allocated.
     */
    virtual ~OpenGlBackground();

    /**
     * @brief Updates the background image to a new one.
     * @param background The new background Image to display.
     *
     * This method is thread-safe. It sets a flag so the image will be loaded
     * and updated on the next render call.
     */
    void updateBackgroundImage(Image background);

    /**
     * @brief Initializes OpenGL buffers and shader resources.
     * @param open_gl The OpenGlWrapper providing the OpenGL context and shader resources.
     */
    virtual void init(OpenGlWrapper& open_gl);

    /**
     * @brief Renders the background image.
     * @param open_gl The OpenGlWrapper providing the OpenGL context.
     *
     * This method locks a mutex to ensure thread safety when updating or rendering the background.
     */
    virtual void render(OpenGlWrapper& open_gl);

    /**
     * @brief Cleans up OpenGL resources when the background is no longer needed.
     * @param open_gl The OpenGlWrapper providing the OpenGL context.
     */
    virtual void destroy(OpenGlWrapper& open_gl);

    /**
     * @brief Locks the mutex for thread-safe operations.
     *
     * Call unlock() afterwards to release the lock.
     */
    void lock() { mutex_.lock(); }

    /**
     * @brief Unlocks the mutex previously locked by lock().
     */
    void unlock() { mutex_.unlock(); }

    /**
     * @brief Gets the shader used to render the image.
     * @return Pointer to the OpenGLShaderProgram for the image.
     */
    OpenGLShaderProgram* shader() { return image_shader_; }

    /**
     * @brief Gets the shader uniform for the texture.
     * @return Pointer to the Uniform object for the texture.
     */
    OpenGLShaderProgram::Uniform* texture_uniform() { return texture_uniform_.get(); }

    /**
     * @brief Binds the vertex and element arrays, and the background texture.
     * @param open_gl_context The OpenGL context.
     */
    void bind(OpenGLContext& open_gl_context);

    /**
     * @brief Enables vertex attribute arrays for position and texture coordinates.
     * @param open_gl_context The OpenGL context.
     */
    void enableAttributes(OpenGLContext& open_gl_context);

    /**
     * @brief Disables vertex attribute arrays for position and texture coordinates.
     * @param open_gl_context The OpenGL context.
     */
    void disableAttributes(OpenGLContext& open_gl_context);

private:
    OpenGLShaderProgram* image_shader_;   ///< Shader program for rendering the image.
    std::unique_ptr<OpenGLShaderProgram::Uniform> texture_uniform_;  ///< Uniform for the texture sampler.
    std::unique_ptr<OpenGLShaderProgram::Attribute> position_;       ///< Attribute for vertex positions.
    std::unique_ptr<OpenGLShaderProgram::Attribute> texture_coordinates_; ///< Attribute for texture coordinates.

    float vertices_[16];  ///< Vertex buffer data (positions and texture coords).

    std::mutex mutex_;    ///< Mutex to protect background image updates.
    OpenGLTexture background_;  ///< The current OpenGL texture for the background.
    bool new_background_;  ///< Flag indicating if a new background image needs to be loaded.
    Image background_image_; ///< The new background image to load on the next render.

    GLuint vertex_buffer_;   ///< OpenGL vertex buffer object for the quad.
    GLuint triangle_buffer_; ///< OpenGL element buffer object for the quad indices.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OpenGlBackground)
};
