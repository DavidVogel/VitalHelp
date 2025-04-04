#pragma once

#include "JuceHeader.h"
#include "open_gl_component.h"

#include <mutex>

/**
 * @class OpenGlImage
 * @brief A utility class for rendering a single image using OpenGL.
 *
 * The OpenGlImage class manages loading an Image into an OpenGL texture and provides methods
 * for drawing that image in a specified position using OpenGL. It supports thread-safe image
 * updates and can apply various blending modes.
 */
class OpenGlImage {
public:
    /**
     * @brief Constructs an OpenGlImage with default settings.
     */
    OpenGlImage();

    /**
     * @brief Destructor.
     *
     * Ensures that OpenGL resources are properly released if still allocated.
     */
    virtual ~OpenGlImage();

    /**
     * @brief Initializes the OpenGL buffers and shader attributes needed for rendering the image.
     * @param open_gl The OpenGlWrapper that provides the OpenGL context and shaders.
     */
    void init(OpenGlWrapper& open_gl);

    /**
     * @brief Draws the image to the current OpenGL context.
     * @param open_gl The OpenGlWrapper that provides the OpenGL context.
     *
     * The image is drawn using the currently set vertex positions, color, and blending mode.
     * If a new image was provided, it will load it before drawing.
     */
    void drawImage(OpenGlWrapper& open_gl);

    /**
     * @brief Releases any OpenGL resources allocated by this object.
     * @param open_gl The OpenGlWrapper that provides the OpenGL context.
     */
    void destroy(OpenGlWrapper& open_gl);

    /**
     * @brief Locks the internal mutex for thread-safe operations.
     *
     * Call unlock() to release the lock.
     */
    void lock() { mutex_.lock(); }

    /**
     * @brief Unlocks the mutex previously locked with lock().
     */
    void unlock() { mutex_.unlock(); }

    /**
     * @brief Sets the image from an owned copy.
     * @param image The image to copy and use internally.
     *
     * A copy of the image is made and stored internally. This is thread-safe.
     */
    void setOwnImage(Image& image) {
        mutex_.lock();
        owned_image_ = std::make_unique<Image>(image);
        setImage(owned_image_.get());
        mutex_.unlock();
    }

    /**
     * @brief Sets the image to render without taking ownership.
     * @param image Pointer to an Image, must remain valid during the usage.
     *
     * No copy is made. The provided image must remain valid while this object uses it.
     */
    void setImage(Image* image) {
        image_ = image;
        image_width_ = image->getWidth();
        image_height_ = image->getHeight();
    }

    /**
     * @brief Sets the color tint applied to the image.
     * @param color The tint color.
     */
    void setColor(Colour color) { color_ = color; }

    /**
     * @brief Sets a specific vertex position by index.
     * @param x The x-coordinate in normalized device space.
     * @param y The y-coordinate in normalized device space.
     * @param index The starting index in the position vertex array (0, 4, 8, 12).
     */
    inline void setPosition(float x, float y, int index) {
        position_vertices_[index] = x;
        position_vertices_[index + 1] = y;
        dirty_ = true;
    }

    /**
     * @brief Sets the top-left corner position of the image quad.
     * @param x The x-coordinate in normalized device space.
     * @param y The y-coordinate in normalized device space.
     */
    inline void setTopLeft(float x, float y) { setPosition(x, y, 0); }

    /**
     * @brief Sets the bottom-left corner position of the image quad.
     */
    inline void setBottomLeft(float x, float y) { setPosition(x, y, 4); }

    /**
     * @brief Sets the bottom-right corner position of the image quad.
     */
    inline void setBottomRight(float x, float y) { setPosition(x, y, 8); }

    /**
     * @brief Sets the top-right corner position of the image quad.
     */
    inline void setTopRight(float x, float y) { setPosition(x, y, 12); }

    /**
     * @brief Gets the width of the currently set image.
     * @return The image width in pixels.
     */
    int getImageWidth() { return image_width_; }

    /**
     * @brief Gets the height of the currently set image.
     * @return The image height in pixels.
     */
    int getImageHeight() { return image_height_; }

    /**
     * @brief Enables or disables additive blending mode.
     * @param additive True for additive blending, false for normal blending.
     */
    void setAdditive(bool additive) { additive_ = additive; }

    /**
     * @brief Enables or disables alpha blending.
     * @param use_alpha True to respect image alpha, false to ignore it.
     */
    void setUseAlpha(bool use_alpha) { use_alpha_ = use_alpha; }

    /**
     * @brief Enables or disables scissor test when drawing the image.
     * @param scissor True to enable scissor test, false to disable.
     */
    void setScissor(bool scissor) { scissor_ = scissor; }

private:
    std::mutex mutex_; ///< Mutex to ensure thread safety when updating the image.
    bool dirty_;        ///< Flag indicating whether vertex data needs to be re-uploaded.

    Image* image_;               ///< Pointer to the current image (not owned).
    int image_width_;            ///< Width of the current image in pixels.
    int image_height_;           ///< Height of the current image in pixels.
    std::unique_ptr<Image> owned_image_; ///< Owned image if set via setOwnImage.
    Colour color_;               ///< Tint color for the image.

    OpenGLTexture texture_;      ///< The OpenGL texture used for rendering the image.
    bool additive_;              ///< Whether additive blending is used.
    bool use_alpha_;             ///< Whether to use alpha blending.
    bool scissor_;               ///< Whether to use scissor test.

    OpenGLShaderProgram* image_shader_; ///< The shader program for rendering the image.
    std::unique_ptr<OpenGLShaderProgram::Uniform> image_color_; ///< Uniform for image color tint.
    std::unique_ptr<OpenGLShaderProgram::Attribute> image_position_;      ///< Attribute for vertex positions.
    std::unique_ptr<OpenGLShaderProgram::Attribute> texture_coordinates_; ///< Attribute for texture coordinates.

    std::unique_ptr<float[]> position_vertices_; ///< Vertex buffer data for positions and texture coordinates.
    std::unique_ptr<int[]> position_triangles_;  ///< Element buffer data for drawing the quad.
    GLuint vertex_buffer_;    ///< OpenGL buffer ID for vertex data.
    GLuint triangle_buffer_;  ///< OpenGL buffer ID for element (index) data.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OpenGlImage)
};
