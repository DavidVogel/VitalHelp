#pragma once

#include "JuceHeader.h"
#include "open_gl_component.h"

#include <mutex>

/**
 * @class OpenGlMultiQuad
 * @brief A component for rendering multiple quads using OpenGL, with customizable colors, rounding, and other effects.
 *
 * The OpenGlMultiQuad class manages and draws a set of rectangular quads on the screen via OpenGL. Each quad can be
 * positioned, sized, and given unique shader values. Various parameters like thickness, rounding, and alpha blending
 * can be adjusted. This makes it suitable for complex GUI elements such as scroll bars, progress bars, or other shapes.
 */
class OpenGlMultiQuad : public OpenGlComponent {
  public:
    /// Number of vertices per quad.
    static constexpr int kNumVertices = 4;
    /// Number of floats per vertex (x, y, w, h, plus custom shader values).
    static constexpr int kNumFloatsPerVertex = 10;
    /// Number of floats total per quad (4 vertices * 10 floats each).
    static constexpr int kNumFloatsPerQuad = kNumVertices * kNumFloatsPerVertex;
    /// Number of indices per quad (2 triangles forming a rectangle).
    static constexpr int kNumIndicesPerQuad = 6;
    /// Decay factor for thickness adjustments over time.
    static constexpr float kThicknessDecay = 0.4f;
    /// Increment for alpha blending adjustments.
    static constexpr float kAlphaInc = 0.2f;

    /**
     * @brief Constructs an OpenGlMultiQuad with a given maximum number of quads.
     * @param max_quads The maximum number of quads that can be drawn.
     * @param shader The fragment shader used to render the quads.
     */
    OpenGlMultiQuad(int max_quads, Shaders::FragmentShader shader = Shaders::kColorFragment);

    /**
     * @brief Destructor. Frees any allocated OpenGL resources.
     */
    virtual ~OpenGlMultiQuad();

    /**
     * @brief Initializes OpenGL buffers and shader attributes.
     * @param open_gl The OpenGlWrapper providing the current OpenGL context.
     */
    virtual void init(OpenGlWrapper& open_gl) override;

    /**
     * @brief Renders the quads using OpenGL.
     * @param open_gl The OpenGlWrapper with the current OpenGL context.
     * @param animate If true, enables animated changes in properties like thickness or alpha.
     */
    virtual void render(OpenGlWrapper& open_gl, bool animate) override;

    /**
     * @brief Releases OpenGL resources when the component is destroyed.
     * @param open_gl The OpenGlWrapper with the OpenGL context.
     */
    virtual void destroy(OpenGlWrapper& open_gl) override;

    /**
     * @brief Suppresses background painting; rendering is handled by OpenGL.
     */
    void paintBackground(Graphics& g) override { }

    /**
     * @brief Called when the component is resized. Marks data as dirty to recalculate positions if needed.
     */
    void resized() override {
        OpenGlComponent::resized();
        dirty_ = true;
    }

    /**
     * @brief Marks all vertex data as dirty, prompting a refresh on the next render.
     */
    void markDirty() {
        dirty_ = true;
    }

    /**
     * @brief Sets the fragment shader used to render the quads.
     * @param shader The fragment shader ID.
     */
    void setFragmentShader(Shaders::FragmentShader shader) { fragment_shader_ = shader; }

    /**
     * @brief Sets how many quads will actually be drawn (up to max_quads).
     * @param num_quads The number of quads to draw.
     */
    void setNumQuads(int num_quads) {
        VITAL_ASSERT(num_quads <= max_quads_);
        num_quads_ = num_quads;
        dirty_ = true;
    }

    /**
     * @brief Sets the base color for the quads.
     * @param color The desired color.
     */
    force_inline void setColor(Colour color) {
      color_ = color;
    }

    /**
     * @brief Gets the current base color.
     * @return The current color.
     */
    force_inline Colour getColor() {
      return color_;
    }

    /**
     * @brief Sets an alternate color, often used by custom shaders.
     * @param color The alternate color.
     */
    force_inline void setAltColor(Colour color) {
      alt_color_ = color;
    }

    /**
     * @brief Sets a modulation color for custom effects in the shader.
     * @param color The modulation color.
     */
    force_inline void setModColor(Colour color) {
      mod_color_ = color;
    }

    /**
     * @brief Sets a "thumb" color, potentially for scroll bars or similar widgets.
     * @param color The thumb color.
     */
    force_inline void setThumbColor(Colour color) {
      thumb_color_ = color;
    }

    /**
     * @brief Sets the amount of thumb exposure (used in certain shader effects).
     * @param amount A value representing the thumb amount.
     */
    force_inline void setThumbAmount(float amount) {
      thumb_amount_ = amount;
    }

    /**
     * @brief Sets a starting position used by some shaders (e.g., arc start).
     * @param pos The starting position.
     */
    force_inline void setStartPos(float pos) {
      start_pos_ = pos;
    }

    /**
     * @brief Sets the maximum arc angle or similar parameter used by some shaders.
     * @param max_arc The maximum arc value.
     */
    force_inline void setMaxArc(float max_arc) {
      max_arc_ = max_arc;
    }

    /**
     * @brief Gets the current maximum arc value.
     * @return The max arc value.
     */
    force_inline float getMaxArc() {
      return max_arc_;
    }

    /**
     * @brief Gets the x-position of a specified quad.
     * @param i The quad index.
     * @return The x-coordinate of the quad's first vertex.
     */
    force_inline float getQuadX(int i) const {
      int index = kNumFloatsPerQuad * i;
      return data_[index];
    }

    /**
     * @brief Gets the y-position of a specified quad.
     */
    force_inline float getQuadY(int i) const {
      int index = kNumFloatsPerQuad * i;
      return data_[index + 1];
    }

    /**
     * @brief Gets the width of the specified quad.
     */
    force_inline float getQuadWidth(int i) const {
      int index = kNumFloatsPerQuad * i;
      return data_[2 * kNumFloatsPerVertex + index] - data_[index];
    }

    /**
     * @brief Gets the height of the specified quad.
     */
    force_inline float getQuadHeight(int i) const {
      int index = kNumFloatsPerQuad * i;
      return data_[kNumFloatsPerVertex + index + 1] - data_[index + 1];
    }

    /**
     * @brief Gets a pointer to the vertex data for a given quad.
     * @param i The quad index.
     * @return A pointer to the float array of vertex data.
     */
    float* getVerticesData(int i) {
      int index = kNumFloatsPerQuad * i;
      return data_.get() + index;
    }

    /**
     * @brief Sets rotated coordinates for a quad, adjusting its texture mapping.
     */
    void setRotatedCoordinates(int i, float x, float y, float w, float h) {
      VITAL_ASSERT(i < max_quads_);
      int index = i * kNumFloatsPerQuad;

      data_[index + 4] = x;
      data_[index + 5] = y + h;
      data_[kNumFloatsPerVertex + index + 4] = x + w;
      data_[kNumFloatsPerVertex + index + 5] = y + h;
      data_[2 * kNumFloatsPerVertex + index + 4] = x + w;
      data_[2 * kNumFloatsPerVertex + index + 5] = y;
      data_[3 * kNumFloatsPerVertex + index + 4] = x;
      data_[3 * kNumFloatsPerVertex + index + 5] = y;
    }

    /**
     * @brief Sets coordinates for a quad in normalized device space.
     */
    void setCoordinates(int i, float x, float y, float w, float h) {
      VITAL_ASSERT(i < max_quads_);
      int index = i * kNumFloatsPerQuad;

      data_[index + 4] = x;
      data_[index + 5] = y;
      data_[kNumFloatsPerVertex + index + 4] = x;
      data_[kNumFloatsPerVertex + index + 5] = y + h;
      data_[2 * kNumFloatsPerVertex + index + 4] = x + w;
      data_[2 * kNumFloatsPerVertex + index + 5] = y + h;
      data_[3 * kNumFloatsPerVertex + index + 4] = x + w;
      data_[3 * kNumFloatsPerVertex + index + 5] = y;
    }

    /**
     * @brief Sets a shader value for all four vertices of a quad.
     * @param i The quad index.
     * @param shader_value The float value to set.
     * @param value_index The index of the shader value slot (0 to 3).
     */
    void setShaderValue(int i, float shader_value, int value_index = 0) {
      VITAL_ASSERT(i < max_quads_);
      int index = i * kNumFloatsPerQuad + 6 + value_index;
      data_[index] = shader_value;
      data_[kNumFloatsPerVertex + index] = shader_value;
      data_[2 * kNumFloatsPerVertex + index] = shader_value;
      data_[3 * kNumFloatsPerVertex + index] = shader_value;
      dirty_ = true;
    }

    /**
     * @brief Sets dimensions for a quad, typically to scale based on component size.
     */
    void setDimensions(int i, float quad_width, float quad_height, float full_width, float full_height) {
      int index = i * kNumFloatsPerQuad;
      float w = quad_width * full_width / 2.0f;
      float h = quad_height * full_height / 2.0f;

      data_[index + 2] = w;
      data_[index + 3] = h;
      data_[kNumFloatsPerVertex + index + 2] = w;
      data_[kNumFloatsPerVertex + index + 3] = h;
      data_[2 * kNumFloatsPerVertex + index + 2] = w;
      data_[2 * kNumFloatsPerVertex + index + 3] = h;
      data_[3 * kNumFloatsPerVertex + index + 2] = w;
      data_[3 * kNumFloatsPerVertex + index + 3] = h;
    }

    /**
     * @brief Sets horizontal position and width for a quad.
     */
    void setQuadHorizontal(int i, float x, float w) {
      VITAL_ASSERT(i < max_quads_);
      int index = i * kNumFloatsPerQuad;
      data_[index] = x;
      data_[kNumFloatsPerVertex + index] = x;
      data_[2 * kNumFloatsPerVertex + index] = x + w;
      data_[3 * kNumFloatsPerVertex + index] = x + w;

      dirty_ = true;
    }

    /**
     * @brief Sets vertical position and height for a quad.
     */
    void setQuadVertical(int i, float y, float h) {
      VITAL_ASSERT(i < max_quads_);
      int index = i * kNumFloatsPerQuad;
      data_[index + 1] = y;
      data_[kNumFloatsPerVertex + index + 1] = y + h;
      data_[2 * kNumFloatsPerVertex + index + 1] = y + h;
      data_[3 * kNumFloatsPerVertex + index + 1] = y;

      dirty_ = true;
    }

    /**
     * @brief Sets the position and size of a quad in normalized device space.
     */
    void setQuad(int i, float x, float y, float w, float h) {
      VITAL_ASSERT(i < max_quads_);
      int index = i * kNumFloatsPerQuad;
      data_[index] = x;
      data_[index + 1] = y;
      data_[kNumFloatsPerVertex + index] = x;
      data_[kNumFloatsPerVertex + index + 1] = y + h;
      data_[2 * kNumFloatsPerVertex + index] = x + w;
      data_[2 * kNumFloatsPerVertex + index + 1] = y + h;
      data_[3 * kNumFloatsPerVertex + index] = x + w;
      data_[3 * kNumFloatsPerVertex + index + 1] = y;

      dirty_ = true;
    }

    /**
     * @brief Activates or deactivates rendering of this component.
     */
    void setActive(bool active) {
      active_ = active;
    }

    /**
     * @brief Sets the thickness used by some shaders and can reset to this thickness.
     */
    void setThickness(float thickness, bool reset = false) {
      thickness_ = thickness;
      if (reset)
        current_thickness_ = thickness_;
    }

    /**
     * @brief Sets the rounding radius of the quads.
     */
    void setRounding(float rounding) {
      float adjusted = 2.0f * rounding;
      if (adjusted != rounding_) {
        dirty_ = true;
        rounding_ = adjusted;
      }
    }

    /**
     * @brief Sets a target component to help position the quads.
     */
    void setTargetComponent(Component* target_component) {
      target_component_ = target_component;
    }

    /**
     * @brief Sets a component for scissoring (clipping) rendering area.
     */
    void setScissorComponent(Component* scissor_component) {
      scissor_component_ = scissor_component;
    }

    /**
     * @brief Gets the current OpenGL shader program.
     */
    OpenGLShaderProgram* shader() { return shader_; }

    /**
     * @brief Enables or disables additive blending for rendering.
     */
    void setAdditive(bool additive) { additive_blending_ = additive; }

    /**
     * @brief Sets the alpha blending multiplier, can reset to this alpha.
     */
    void setAlpha(float alpha, bool reset = false) {
      alpha_mult_ = alpha;
      if (reset)
        current_alpha_mult_ = alpha;
    }

    /**
     * @brief Sets whether to draw even if the component is not visible.
     */
    void setDrawWhenNotVisible(bool draw) { draw_when_not_visible_ = draw; }

  protected:
    Component* target_component_;          ///< The component this relates to for sizing/positioning.
    Component* scissor_component_;         ///< The component used for scissoring (clipping).
    Shaders::FragmentShader fragment_shader_; ///< The fragment shader used for rendering.
    int max_quads_;                       ///< Maximum number of quads.
    int num_quads_;                       ///< Current number of quads to draw.

    bool draw_when_not_visible_;          ///< If true, draw even if the component is not visible.
    bool active_;                         ///< If false, nothing is rendered.
    bool dirty_;                          ///< If true, vertex data is dirty and needs re-upload.
    Colour color_;                        ///< Base color tint.
    Colour alt_color_;                    ///< Alternate color for shader use.
    Colour mod_color_;                    ///< Modulation color for shader.
    Colour thumb_color_;                  ///< Color for a "thumb" element (e.g., in a slider).
    float max_arc_;                       ///< Maximum arc for certain shader effects.
    float thumb_amount_;                  ///< Amount parameter for thumb effects.
    float start_pos_;                     ///< Start position parameter for shader effects.
    float current_alpha_mult_;            ///< Current alpha multiplier for gradual changes.
    float alpha_mult_;                    ///< Target alpha multiplier.
    bool additive_blending_;              ///< Use additive blending if true.
    float current_thickness_;             ///< Current thickness for gradual changes.
    float thickness_;                     ///< Target thickness.
    float rounding_;                      ///< Rounding radius for corners.

    std::unique_ptr<float[]> data_;       ///< Vertex data for all quads.
    std::unique_ptr<int[]> indices_;      ///< Index data for drawing quads.

    OpenGLShaderProgram* shader_;
    std::unique_ptr<OpenGLShaderProgram::Uniform> color_uniform_;
    std::unique_ptr<OpenGLShaderProgram::Uniform> alt_color_uniform_;
    std::unique_ptr<OpenGLShaderProgram::Uniform> mod_color_uniform_;
    std::unique_ptr<OpenGLShaderProgram::Uniform> background_color_uniform_;
    std::unique_ptr<OpenGLShaderProgram::Uniform> thumb_color_uniform_;
    std::unique_ptr<OpenGLShaderProgram::Uniform> thickness_uniform_;
    std::unique_ptr<OpenGLShaderProgram::Uniform> rounding_uniform_;
    std::unique_ptr<OpenGLShaderProgram::Uniform> max_arc_uniform_;
    std::unique_ptr<OpenGLShaderProgram::Uniform> thumb_amount_uniform_;
    std::unique_ptr<OpenGLShaderProgram::Uniform> start_pos_uniform_;
    std::unique_ptr<OpenGLShaderProgram::Uniform> alpha_mult_uniform_;
    std::unique_ptr<OpenGLShaderProgram::Attribute> position_;
    std::unique_ptr<OpenGLShaderProgram::Attribute> dimensions_;
    std::unique_ptr<OpenGLShaderProgram::Attribute> coordinates_;
    std::unique_ptr<OpenGLShaderProgram::Attribute> shader_values_;

    GLuint vertex_buffer_;    ///< OpenGL buffer for vertex data.
    GLuint indices_buffer_;   ///< OpenGL buffer for index data.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OpenGlMultiQuad)
};

/**
 * @class OpenGlQuad
 * @brief A convenience class for a single quad rendered via OpenGL.
 */
class OpenGlQuad : public OpenGlMultiQuad {
  public:
    /**
     * @brief Constructs a single quad with a given fragment shader.
     * @param shader The fragment shader to use.
     */
    OpenGlQuad(Shaders::FragmentShader shader) : OpenGlMultiQuad(1, shader) {
      setQuad(0, -1.0f, -1.0f, 2.0f, 2.0f);
    }
};

/**
 * @class OpenGlScrollQuad
 * @brief A specialized quad used as a scroll indicator, responding to hover and scroll changes.
 */
class OpenGlScrollQuad : public OpenGlQuad {
  public:
    OpenGlScrollQuad() : OpenGlQuad(Shaders::kRoundedRectangleFragment), scroll_bar_(nullptr),
                         hover_(false), shrink_left_(false), hover_amount_(-1.0f) { }

    virtual void render(OpenGlWrapper& open_gl, bool animate) override {
      static constexpr float kHoverChange = 0.2f;
      float last_hover = hover_amount_;
      if (hover_)
        hover_amount_ = std::min(1.0f, hover_amount_ + kHoverChange);
      else
        hover_amount_ = std::max(0.0f, hover_amount_ - kHoverChange);

      if (last_hover != hover_amount_) {
        if (shrink_left_)
          setQuadHorizontal(0, -1.0f, 1.0f + hover_amount_);
        else
          setQuadHorizontal(0, 0.0f - hover_amount_, 1.0f + hover_amount_);
      }

      Range<double> range = scroll_bar_->getCurrentRange();
      Range<double> total_range = scroll_bar_->getRangeLimit();
      float start_ratio = (range.getStart() - total_range.getStart()) / total_range.getLength();
      float end_ratio = (range.getEnd() - total_range.getStart()) / total_range.getLength();
      setQuadVertical(0, 1.0f - 2.0f * end_ratio, 2.0f * (end_ratio - start_ratio));

      OpenGlQuad::render(open_gl, animate);
    }

    void setHover(bool hover) { hover_ = hover; }
    void setShrinkLeft(bool shrink_left) { shrink_left_ = shrink_left; }
    void setScrollBar(ScrollBar* scroll_bar) { scroll_bar_ = scroll_bar; }

  private:
    ScrollBar* scroll_bar_;
    bool hover_;
    bool shrink_left_;
    float hover_amount_;
};

/**
 * @class OpenGlScrollBar
 * @brief A ScrollBar that uses OpenGlMultiQuad for rendering its visual indication.
 */
class OpenGlScrollBar : public ScrollBar {
  public:
    OpenGlScrollBar() : ScrollBar(true) {
      bar_.setTargetComponent(this);
      addAndMakeVisible(bar_);
      bar_.setScrollBar(this);
    }

    OpenGlQuad* getGlComponent() { return &bar_; }

    void resized() override {
      ScrollBar::resized();
      bar_.setBounds(getLocalBounds());
      bar_.setRounding(getWidth() * 0.25f);
    }

    void mouseEnter(const MouseEvent& e) override {
      ScrollBar::mouseEnter(e);
      bar_.setHover(true);
    }

    void mouseExit(const MouseEvent& e) override {
      ScrollBar::mouseExit(e);
      bar_.setHover(false);
    }

    void mouseDown(const MouseEvent& e) override {
      ScrollBar::mouseDown(e);
      bar_.setColor(color_.overlaidWith(color_));
    }

    void mouseUp(const MouseEvent& e) override {
      ScrollBar::mouseDown(e);
      bar_.setColor(color_);
    }

    void setColor(Colour color) { color_ = color; bar_.setColor(color); }
    void setShrinkLeft(bool shrink_left) { bar_.setShrinkLeft(shrink_left); }

  private:
    Colour color_;
    OpenGlScrollQuad bar_;
};

/**
 * @class OpenGlCorners
 * @brief A set of quads forming rounded corners, used to render corner shapes via OpenGL.
 */
class OpenGlCorners : public OpenGlMultiQuad {
  public:
    OpenGlCorners() : OpenGlMultiQuad(4, Shaders::kRoundedCornerFragment) {
      setCoordinates(0, 1.0f, 1.0f, -1.0f, -1.0f);
      setCoordinates(1, 1.0f, 0.0f, -1.0f, 1.0f);
      setCoordinates(2, 0.0f, 0.0f, 1.0f, 1.0f);
      setCoordinates(3, 0.0f, 1.0f, 1.0f, -1.0f);
    }

    /**
     * @brief Configures quads to form all four rounded corners of a rectangle.
     * @param bounds The rectangle whose corners are rounded.
     * @param rounding The rounding radius in pixels.
     */
    void setCorners(Rectangle<int> bounds, float rounding) {
      float width = rounding / bounds.getWidth() * 2.0f;
      float height = rounding / bounds.getHeight() * 2.0f;

      setQuad(0, -1.0f, -1.0f, width, height);
      setQuad(1, -1.0f, 1.0f - height, width, height);
      setQuad(2, 1.0f - width, 1.0f - height, width, height);
      setQuad(3, 1.0f - width, -1.0f, width, height);
    }

    /**
     * @brief Configures quads to form only the bottom rounded corners of a rectangle.
     */
    void setBottomCorners(Rectangle<int> bounds, float rounding) {
      float width = rounding / bounds.getWidth() * 2.0f;
      float height = rounding / bounds.getHeight() * 2.0f;

      setQuad(0, -1.0f, -1.0f, width, height);
      setQuad(1, -2.0f, -2.0f, 0.0f, 0.0f);
      setQuad(2, -2.0f, -2.0f, 0.0f, 0.0f);
      setQuad(3, 1.0f - width, -1.0f, width, height);
    }
};
