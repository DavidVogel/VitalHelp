#pragma once

#include "JuceHeader.h"
#include "open_gl_component.h"
#include <mutex>

/**
 * @class OpenGlMultiImage
 * @brief A component for rendering multiple image quads using OpenGL.
 *
 * The OpenGlMultiImage class manages a set of quads, each displaying a portion (or the entirety)
 * of a single image texture. It supports drawing a configurable number of these image quads at
 * once, each positioned and sized independently. This enables rendering multiple image instances
 * efficiently with a single texture.
 */
class OpenGlMultiImage : public OpenGlComponent {
public:
    /// Number of vertices per quad (2 triangles forming a rectangle).
    static constexpr int kNumVertices = 4;
    /// Number of floats per vertex (x, y, u, v).
    static constexpr int kNumFloatsPerVertex = 4;
    /// Number of floats per quad (4 vertices * 4 floats each).
    static constexpr int kNumFloatsPerQuad = kNumVertices * kNumFloatsPerVertex;
    /// Number of indices per quad (2 triangles * 3 indices each).
    static constexpr int kNumIndicesPerQuad = 6;

    /**
     * @brief Constructs an OpenGlMultiImage with a given maximum number of quads.
     * @param max_images The maximum number of quads (images) that can be drawn.
     */
    OpenGlMultiImage(int max_images);

    /**
     * @brief Destructor. Frees any allocated OpenGL resources.
     */
    virtual ~OpenGlMultiImage();

    /**
     * @brief Initializes OpenGL buffers and shader attributes for rendering.
     * @param open_gl The OpenGlWrapper providing the current OpenGL context.
     */
    virtual void init(OpenGlWrapper& open_gl) override;

    /**
     * @brief Renders the set of image quads using OpenGL.
     * @param open_gl The OpenGlWrapper providing the current OpenGL context.
     * @param animate If true, allows animated transitions (not typically needed for images).
     */
    virtual void render(OpenGlWrapper& open_gl, bool animate) override;

    /**
     * @brief Destroys the OpenGL resources used by this class.
     * @param open_gl The OpenGlWrapper providing the current OpenGL context.
     */
    virtual void destroy(OpenGlWrapper& open_gl) override;

    /**
     * @brief Override to suppress default background painting.
     * @param g The graphics context.
     */
    void paintBackground(Graphics& g) override { }

    /**
     * @brief Called when the component is resized, marks vertex data as dirty to recalculate.
     */
    void resized() override {
        OpenGlComponent::resized();
        dirty_ = true;
    }

    /**
     * @brief Locks the internal mutex for thread-safe image updates.
     */
    void lock() { mutex_.lock(); }

    /**
     * @brief Unlocks the mutex locked by lock().
     */
    void unlock() { mutex_.unlock(); }

    /**
     * @brief Sets an owned image by making a copy and uses it for rendering.
     * @param image The image to copy and use.
     */
    void setOwnImage(Image& image) {
        mutex_.lock();
        owned_image_ = std::make_unique<Image>(image);
        setImage(owned_image_.get());
        mutex_.unlock();
    }

    /**
     * @brief Sets the image to render without ownership.
     * @param image A pointer to the image (must remain valid while used).
     */
    void setImage(Image* image) {
        image_ = image;
        image_width_ = image->getWidth();
        image_height_ = image->getHeight();
    }

    /**
     * @brief Sets the number of quads currently drawn.
     * @param num_quads The number of quads to draw.
     */
    void setNumQuads(int num_quads) { num_quads_ = num_quads; }

    /**
     * @brief Sets the color tint applied to all image quads.
     * @param color The tint color.
     */
    void setColor(Colour color) { color_ = color; }

    /**
     * @brief Sets the position and size of a quad.
     * @param i The quad index.
     * @param x The x-position in normalized device space (-1 to 1).
     * @param y The y-position in normalized device space (-1 to 1).
     * @param w The width of the quad in normalized device units.
     * @param h The height of the quad in normalized device units.
     */
    inline void setQuad(int i, float x, float y, float w, float h) {
        int index = kNumFloatsPerQuad * i;
        data_[index] = x;
        data_[index + 1] = y;
        data_[index + 4] = x;
        data_[index + 5] = y + h;
        data_[index + 8] = x + w;
        data_[index + 9] = y + h;
        data_[index + 12] = x + w;
        data_[index + 13] = y;
        dirty_ = true;
    }

    /**
     * @brief Gets the width of the current image.
     * @return The image width in pixels.
     */
    int getImageWidth() { return image_width_; }

    /**
     * @brief Gets the height of the current image.
     * @return The image height in pixels.
     */
    int getImageHeight() { return image_height_; }

    /**
     * @brief Enables or disables additive blending for the image quads.
     * @param additive True for additive blending, false for normal blending.
     */
    void setAdditive(bool additive) { additive_blending_ = additive; }

private:
    std::mutex mutex_;             ///< Mutex to ensure thread safety for image updates.
    Image* image_;                 ///< Pointer to the current image (not owned).
    int image_width_;              ///< Image width in pixels.
    int image_height_;             ///< Image height in pixels.
    std::unique_ptr<Image> owned_image_; ///< Owned image if set via setOwnImage.
    Colour color_;                 ///< Tint color for the image quads.
    OpenGLTexture texture_;        ///< Texture used for all drawn quads.

    int max_quads_;                ///< Maximum number of quads that can be drawn.
    int num_quads_;                ///< Current number of quads to draw.

    bool dirty_;                   ///< Indicates if vertex data needs re-uploading.
    bool additive_blending_;       ///< Whether to use additive blending.

    std::unique_ptr<float[]> data_;    ///< Vertex data array for all quads.
    std::unique_ptr<int[]> indices_;   ///< Index data for drawing multiple quads.

    OpenGLShaderProgram* image_shader_; ///< Shader program for tinted image rendering.
    std::unique_ptr<OpenGLShaderProgram::Uniform> color_uniform_; ///< Uniform for quad color tint.
    std::unique_ptr<OpenGLShaderProgram::Attribute> position_;    ///< Attribute for vertex positions.
    std::unique_ptr<OpenGLShaderProgram::Attribute> texture_coordinates_; ///< Attribute for texture coords.

    GLuint vertex_buffer_;         ///< OpenGL buffer for vertex data.
    GLuint indices_buffer_;        ///< OpenGL buffer for quad indices.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OpenGlMultiImage)
};
