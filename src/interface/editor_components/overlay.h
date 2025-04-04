#pragma once

#include "JuceHeader.h"
#include "synth_section.h"

#include <set>

/**
 * @class OverlayBackgroundRenderer
 * @brief A component that renders a full-screen overlay background using OpenGL.
 *
 * The OverlayBackgroundRenderer draws a simple rectangular overlay covering its bounds,
 * using a specified color and optional additive blending. It's designed to be used as
 * the background layer for overlay components.
 */
class OverlayBackgroundRenderer : public OpenGlComponent {
public:
    /// Number of vertices for the overlay quad.
    static constexpr int kNumVertices = 4;
    /// Floats per vertex (x and y).
    static constexpr int kNumFloatsPerVertex = 2;
    /// Total floats for the quad (4 vertices * 2 floats).
    static constexpr int kTotalFloats = kNumVertices * kNumFloatsPerVertex;
    /// Number of indices (2 triangles * 3 indices).
    static constexpr int kIndices = 6;

    /**
     * @brief Constructs an OverlayBackgroundRenderer with default settings.
     */
    OverlayBackgroundRenderer() {
        shader_ = nullptr;
        additive_blending_ = false;

        color_ = Colours::black;
        // Default coordinates form a full-screen quad
        data_[0] = -1.0f; data_[1] = -1.0f;
        data_[2] = -1.0f; data_[3] =  1.0f;
        data_[4] =  1.0f; data_[5] = -1.0f;
        data_[6] =  1.0f; data_[7] =  1.0f;

        indices_[0] = 0; indices_[1] = 1; indices_[2] = 2;
        indices_[3] = 1; indices_[4] = 2; indices_[5] = 3;

        setInterceptsMouseClicks(false, false);
    }

    /**
     * @brief Destructor.
     */
    virtual ~OverlayBackgroundRenderer() { }

    /**
     * @brief Initializes the OpenGL shader and buffers.
     * @param open_gl The OpenGlWrapper providing the current OpenGL context.
     */
    virtual void init(OpenGlWrapper& open_gl) override {
        open_gl.context.extensions.glGenBuffers(1, &data_buffer_);
        open_gl.context.extensions.glBindBuffer(GL_ARRAY_BUFFER, data_buffer_);

        GLsizeiptr vert_size = static_cast<GLsizeiptr>(kTotalFloats * sizeof(float));
        open_gl.context.extensions.glBufferData(GL_ARRAY_BUFFER, vert_size, data_, GL_STATIC_DRAW);

        open_gl.context.extensions.glGenBuffers(1, &indices_buffer_);
        open_gl.context.extensions.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_buffer_);

        GLsizeiptr bar_size = static_cast<GLsizeiptr>(kIndices * sizeof(int));
        open_gl.context.extensions.glBufferData(GL_ELEMENT_ARRAY_BUFFER, bar_size, indices_, GL_STATIC_DRAW);

        shader_ = open_gl.shaders->getShaderProgram(Shaders::kPassthroughVertex, Shaders::kColorFragment);
        shader_->use();
        color_uniform_ = getUniform(open_gl, *shader_, "color");
        position_ = getAttribute(open_gl, *shader_, "position");
    }

    /**
     * @brief Renders the overlay using OpenGL.
     * @param open_gl The OpenGlWrapper with the current OpenGL context.
     * @param animate Whether animations are enabled (not typically needed for a static overlay).
     */
    virtual void render(OpenGlWrapper& open_gl, bool animate) override {
        drawOverlay(open_gl);
    }

    /**
     * @brief This overlay does not paint a background using JUCE's Graphics.
     * @param g The graphics context.
     */
    void paintBackground(Graphics& g) override { }

    /**
     * @brief Destroys OpenGL resources allocated for this overlay.
     * @param open_gl The OpenGlWrapper with the OpenGL context.
     */
    virtual void destroy(OpenGlWrapper& open_gl) override {
        shader_ = nullptr;
        position_ = nullptr;
        color_uniform_ = nullptr;
        open_gl.context.extensions.glDeleteBuffers(1, &data_buffer_);
        open_gl.context.extensions.glDeleteBuffers(1, &indices_buffer_);

        data_buffer_ = 0;
        indices_buffer_ = 0;
    }

    /**
     * @brief Sets the overlay color.
     * @param color The new overlay color.
     */
    void setColor(const Colour& color) { color_ = color; }

    /**
     * @brief Enables or disables additive blending.
     * @param additive_blending True for additive blending, false for normal blending.
     */
    force_inline void setAdditiveBlending(bool additive_blending) { additive_blending_ = additive_blending; }

protected:
    /**
     * @brief Draws the overlay quad.
     * @param open_gl The OpenGlWrapper with the current OpenGL context.
     */
    void drawOverlay(OpenGlWrapper& open_gl) {
        if (!setViewPort(open_gl))
            return;

        if (shader_ == nullptr)
            init(open_gl);

        glEnable(GL_BLEND);
        glEnable(GL_SCISSOR_TEST);
        if (additive_blending_)
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        else
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        shader_->use();
        color_uniform_->set(color_.getFloatRed(), color_.getFloatGreen(),
                            color_.getFloatBlue(), color_.getFloatAlpha());

        open_gl.context.extensions.glBindBuffer(GL_ARRAY_BUFFER, data_buffer_);
        open_gl.context.extensions.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_buffer_);

        open_gl.context.extensions.glVertexAttribPointer(position_->attributeID, kNumFloatsPerVertex, GL_FLOAT,
                                                         GL_FALSE, kNumFloatsPerVertex * sizeof(float), nullptr);
        open_gl.context.extensions.glEnableVertexAttribArray(position_->attributeID);

        glDrawElements(GL_TRIANGLES, kIndices, GL_UNSIGNED_INT, nullptr);

        open_gl.context.extensions.glDisableVertexAttribArray(position_->attributeID);
        open_gl.context.extensions.glBindBuffer(GL_ARRAY_BUFFER, 0);
        open_gl.context.extensions.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        glDisable(GL_BLEND);
        glDisable(GL_SCISSOR_TEST);
    }

    OpenGLShaderProgram* shader_;  ///< Shader program used for overlay drawing.
    std::unique_ptr<OpenGLShaderProgram::Uniform> color_uniform_; ///< Uniform for overlay color.
    std::unique_ptr<OpenGLShaderProgram::Attribute> position_;    ///< Attribute for vertex positions.

    Colour color_;               ///< The overlay color.
    bool additive_blending_;     ///< Whether additive blending is enabled.

    float data_[kTotalFloats];   ///< Vertex data for the overlay quad.
    int indices_[kIndices];      ///< Index data for the overlay quad.
    GLuint data_buffer_;         ///< OpenGL buffer for vertex data.
    GLuint indices_buffer_;      ///< OpenGL buffer for index data.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OverlayBackgroundRenderer)
};

/**
 * @class Overlay
 * @brief A SynthSection that displays an overlay with a background and optional listeners.
 *
 * The Overlay class displays a semi-transparent overlay on top of other GUI elements.
 * It uses an OverlayBackgroundRenderer to render a tinted background. Listeners can
 * be attached to respond to the overlay being shown or hidden.
 */
class Overlay : public SynthSection {
public:
    /**
     * @class Listener
     * @brief Interface for receiving notifications about overlay visibility changes.
     */
    class Listener {
    public:
        Listener() { }
        virtual ~Listener() { }

        /**
         * @brief Called when the overlay is shown.
         * @param component The Overlay that was shown.
         */
        virtual void overlayShown(Overlay* component) = 0;

        /**
         * @brief Called when the overlay is hidden.
         * @param component The Overlay that was hidden.
         */
        virtual void overlayHidden(Overlay* component) = 0;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Listener)
    };

    /**
     * @brief Constructs an Overlay with a given name.
     * @param name The name of the overlay.
     */
    Overlay(String name) : SynthSection(name), size_ratio_(1.0f) {
        setSkinOverride(Skin::kOverlay);
        addOpenGlComponent(&background_);
    }

    /**
     * @brief Destructor.
     */
    virtual ~Overlay() { }

    /**
     * @brief Sets the visibility of the overlay and notifies listeners.
     * @param should_be_visible True to show, false to hide.
     */
    void setVisible(bool should_be_visible) override {
        for (Listener* listener : listeners_) {
            if (should_be_visible)
                listener->overlayShown(this);
            else
                listener->overlayHidden(this);
        }
        Component::setVisible(should_be_visible);
    }

    /**
     * @brief Called when the overlay is resized. Updates background color and size.
     */
    virtual void resized() override {
        background_.setColor(findColour(Skin::kOverlayScreen, true));
        background_.setBounds(getLocalBounds());
    }

    /**
     * @brief Paints the background using OpenGL-rendered children.
     * @param g The graphics context.
     */
    virtual void paintBackground(Graphics& g) override { paintOpenGlChildrenBackgrounds(g); }

    /**
     * @brief Adds a listener to be notified of overlay visibility changes.
     * @param listener The listener to add.
     */
    void addOverlayListener(Listener* listener) { listeners_.insert(listener); }

    /**
     * @brief Removes a previously added overlay listener.
     * @param listener The listener to remove.
     */
    void removeOverlayListener(Listener* listener) { listeners_.erase(listener); }

    /**
     * @brief Sets the size ratio for the overlay, used in some layouts.
     * @param ratio The new size ratio.
     */
    void setSizeRatio(float ratio) override { size_ratio_ = ratio; }

protected:
    float size_ratio_;                        ///< A scaling factor for the overlay size.
    std::set<Listener*> listeners_;           ///< Registered overlay listeners.
    OverlayBackgroundRenderer background_;     ///< Renders the overlay background.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Overlay)
};
