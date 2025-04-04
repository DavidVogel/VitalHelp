#pragma once

#include "open_gl_component.h"
#include "fonts.h"
#include "open_gl_image.h"

class SynthSection;

/**
 * @class OpenGlImageComponent
 * @brief A component that uses OpenGL to render a cached image of a JUCE component or custom drawing.
 *
 * The OpenGlImageComponent captures the output of a Component (or itself) into a JUCE Image, then
 * uploads that image as a texture and renders it using OpenGL. This approach allows complex components
 * to be drawn once and reused for efficient OpenGL rendering. It supports redrawing when necessary
 * and can be set to a static (non-updating) image.
 */
class OpenGlImageComponent : public OpenGlComponent {
public:
    /**
     * @brief Constructs an OpenGlImageComponent.
     * @param name Optional name for the component.
     */
    OpenGlImageComponent(String name = "");

    /**
     * @brief Destructor.
     */
    virtual ~OpenGlImageComponent() = default;

    /**
     * @brief Paints the background by redrawing the image if needed.
     * @param g The graphics context.
     */
    virtual void paintBackground(Graphics& g) override {
        redrawImage(false);
    }

    /**
     * @brief Renders the associated component (or itself) into the provided Graphics context.
     * @param g The graphics context where the component is painted.
     *
     * If paint_entire_component_ is true, the entire component hierarchy is painted,
     * otherwise only this component is painted.
     */
    virtual void paintToImage(Graphics& g) {
        Component* component = component_ ? component_ : this;
        if (paint_entire_component_)
            component->paintEntireComponent(g, false);
        else
            component->paint(g);
    }

    /**
     * @brief Initializes any OpenGL resources for rendering this component.
     * @param open_gl The OpenGlWrapper with the current OpenGL context.
     */
    virtual void init(OpenGlWrapper& open_gl) override;

    /**
     * @brief Renders the image using OpenGL.
     * @param open_gl The OpenGlWrapper with the current OpenGL context.
     * @param animate If true, allows for animated drawing.
     */
    virtual void render(OpenGlWrapper& open_gl, bool animate) override;

    /**
     * @brief Destroys OpenGL-related resources used by this component.
     * @param open_gl The OpenGlWrapper with the current OpenGL context.
     */
    virtual void destroy(OpenGlWrapper& open_gl) override;

    /**
     * @brief Redraws the image if necessary, creating or updating the internal Image.
     * @param force If true, forces a redraw even if conditions haven't changed.
     */
    virtual void redrawImage(bool force);

    /**
     * @brief Sets the component to be drawn into the OpenGL image. If not set, uses this component.
     * @param component The component to draw.
     */
    void setComponent(Component* component) { component_ = component; }

    /**
     * @brief Enables or disables scissor testing when drawing the image.
     * @param scissor True to enable scissor testing, false otherwise.
     */
    void setScissor(bool scissor) { image_.setScissor(scissor); }

    /**
     * @brief Enables or disables alpha blending for the image.
     * @param use_alpha True to respect alpha, false to ignore.
     */
    void setUseAlpha(bool use_alpha) { image_.setUseAlpha(use_alpha); }

    /**
     * @brief Sets a color tint for the image.
     * @param color The tint color.
     */
    void setColor(Colour color) { image_.setColor(color); }

    /**
     * @brief Provides access to the underlying OpenGlImage.
     * @return Reference to the OpenGlImage object.
     */
    OpenGlImage& image() { return image_; }

    /**
     * @brief Sets whether this component is active (rendered) or not.
     * @param active True if active, false if inactive.
     */
    void setActive(bool active) { active_ = active; }

    /**
     * @brief Sets whether the image should be treated as static (not redrawn unless forced).
     * @param static_image True if static, false for dynamic re-rendering.
     */
    void setStatic(bool static_image) { static_image_ = static_image; }

    /**
     * @brief Controls whether paintToImage should paint the entire component hierarchy or just itself.
     * @param paint_entire_component True to paint entire hierarchy, false otherwise.
     */
    void paintEntireComponent(bool paint_entire_component) { paint_entire_component_ = paint_entire_component; }

    /**
     * @brief Checks if this component is currently active.
     * @return True if active, false otherwise.
     */
    bool isActive() const { return active_; }

protected:
    Component* component_;            ///< The component being drawn into the image (if any).
    bool active_;                     ///< Whether this component is active and should render.
    bool static_image_;               ///< Whether the image is static or updated on events.
    bool paint_entire_component_;     ///< If true, paint entire component hierarchy to image.
    std::unique_ptr<Image> draw_image_;  ///< The cached image that stores the drawn component.
    OpenGlImage image_;               ///< The OpenGlImage used to upload and draw the cached image.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OpenGlImageComponent)
};


/**
 * @class OpenGlAutoImageComponent
 * @brief A template class that wraps a given ComponentType and automatically redraws an OpenGlImageComponent on mouse events.
 *
 * This class automatically triggers redraws of the image whenever mouse events occur, ensuring
 * the image stays up-to-date with the component's state.
 */
template <class ComponentType>
class OpenGlAutoImageComponent : public ComponentType {
public:
    using ComponentType::ComponentType;

    virtual void mouseDown(const MouseEvent& e) override {
        ComponentType::mouseDown(e);
        redoImage();
    }

    virtual void mouseUp(const MouseEvent& e) override {
        ComponentType::mouseUp(e);
        redoImage();
    }

    virtual void mouseDoubleClick(const MouseEvent& e) override {
        ComponentType::mouseDoubleClick(e);
        redoImage();
    }

    virtual void mouseEnter(const MouseEvent& e) override {
        ComponentType::mouseEnter(e);
        redoImage();
    }

    virtual void mouseExit(const MouseEvent& e) override {
        ComponentType::mouseExit(e);
        redoImage();
    }

    virtual void mouseWheelMove(const MouseEvent& e, const MouseWheelDetails& wheel) override {
        ComponentType::mouseWheelMove(e, wheel);
        redoImage();
    }

    /**
     * @brief Gets the underlying OpenGlImageComponent.
     * @return Pointer to the image component.
     */
    OpenGlImageComponent* getImageComponent() { return &image_component_; }

    /**
     * @brief Redraws the image after a state change.
     */
    virtual void redoImage() { image_component_.redrawImage(true); }

protected:
    OpenGlImageComponent image_component_; ///< The OpenGlImageComponent for rendering this component.
};


/**
 * @class OpenGlTextEditor
 * @brief A text editor that uses an OpenGlImageComponent for rendering and updates the image on text changes.
 *
 * The OpenGlTextEditor applies a font and redraws its image on various text events, mouse events, and focus changes.
 */
class OpenGlTextEditor : public OpenGlAutoImageComponent<TextEditor>, public TextEditor::Listener {
public:
    /**
     * @brief Constructs an OpenGlTextEditor with a given name.
     * @param name The name of the text editor.
     */
    OpenGlTextEditor(String name) : OpenGlAutoImageComponent(name) {
        monospace_ = false;
        image_component_.setComponent(this);
        addListener(this);
    }

    /**
     * @brief Constructs an OpenGlTextEditor with a given name and password character.
     * @param name The name of the text editor.
     * @param password_char The character used to mask text if used as a password field.
     */
    OpenGlTextEditor(String name, wchar_t password_char) : OpenGlAutoImageComponent(name, password_char) {
        monospace_ = false;
        image_component_.setComponent(this);
        addListener(this);
    }

    bool keyPressed(const KeyPress& key) override {
        bool result = TextEditor::keyPressed(key);
        redoImage();
        return result;
    }

    void textEditorTextChanged(TextEditor&) override { redoImage(); }
    void textEditorFocusLost(TextEditor&) override { redoImage(); }

    virtual void mouseDrag(const MouseEvent& e) override {
        TextEditor::mouseDrag(e);
        redoImage();
    }

    /**
     * @brief Applies the appropriate font based on monospace setting and component size.
     */
    void applyFont() {
        Font font;
        if (monospace_)
            font = Fonts::instance()->monospace().withPointHeight(getHeight() / 2.0f);
        else
            font = Fonts::instance()->proportional_light().withPointHeight(getHeight() / 2.0f);

        applyFontToAllText(font);
        redoImage();
    }

    void visibilityChanged() override {
        TextEditor::visibilityChanged();

        if (isVisible() && !isMultiLine())
            applyFont();
    }

    void resized() override {
        TextEditor::resized();
        if (isMultiLine()) {
            float indent = image_component_.findValue(Skin::kLabelBackgroundRounding);
            setIndents(indent, indent);
            return;
        }

        if (monospace_)
            setIndents(getHeight() * 0.2, getHeight() * 0.17);
        else
            setIndents(getHeight() * 0.2, getHeight() * 0.15);
        if (isVisible())
            applyFont();
    }

    /**
     * @brief Sets the text editor to use a monospace font.
     */
    void setMonospace() {
        monospace_ = true;
    }

private:
    bool monospace_; ///< Whether to use a monospace font for the text editor.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OpenGlTextEditor)
};


/**
 * @class PlainTextComponent
 * @brief A text component rendered into an OpenGlImageComponent with configurable font and justification.
 */
class PlainTextComponent : public OpenGlImageComponent {
public:
    /**
     * @enum FontType
     * @brief Different font types available for text rendering.
     */
    enum FontType {
        kTitle,
        kLight,
        kRegular,
        kMono,
        kNumFontTypes
    };

    /**
     * @brief Constructs a PlainTextComponent.
     * @param name The name of the component.
     * @param text The initial text to display.
     */
    PlainTextComponent(String name, String text) : OpenGlImageComponent(name), text_(std::move(text)),
                                                   text_size_(1.0f), font_type_(kRegular),
                                                   justification_(Justification::centred),
                                                   buffer_(0) {
        setInterceptsMouseClicks(false, false);
    }

    void resized() override {
        OpenGlImageComponent::resized();
        redrawImage(true);
    }

    /**
     * @brief Sets the displayed text and redraws the image.
     * @param text The new text to display.
     */
    void setText(String text) {
        if (text_ == text)
            return;

        text_ = text;
        redrawImage(true);
    }

    /**
     * @brief Gets the current displayed text.
     * @return The current text as a String.
     */
    String getText() const { return text_; }

    void paintToImage(Graphics& g) override {
        g.setColour(Colours::white);

        if (font_type_ == kTitle)
            g.setFont(Fonts::instance()->proportional_title().withPointHeight(text_size_));
        else if (font_type_ == kLight)
            g.setFont(Fonts::instance()->proportional_light().withPointHeight(text_size_));
        else if (font_type_ == kRegular)
            g.setFont(Fonts::instance()->proportional_regular().withPointHeight(text_size_));
        else
            g.setFont(Fonts::instance()->monospace().withPointHeight(text_size_));

        Component* component = component_ ? component_ : this;

        g.drawFittedText(text_, buffer_, 0, component->getWidth() - 2 * buffer_,
                         component->getHeight(), justification_, false);
    }

    /**
     * @brief Sets the size of the text in points.
     * @param size The new text size.
     */
    void setTextSize(float size) {
        text_size_ = size;
        redrawImage(true);
    }

    /**
     * @brief Sets the font type (Title, Light, Regular, Mono).
     * @param font_type The desired font type.
     */
    void setFontType(FontType font_type) {
        font_type_ = font_type;
    }

    /**
     * @brief Sets the text justification (e.g., centered, left, right).
     * @param justification The Justification enum value.
     */
    void setJustification(Justification justification) {
        justification_ = justification;
    }

    /**
     * @brief Sets a buffer (padding) around the text.
     * @param buffer The number of pixels to pad around the text.
     */
    void setBuffer(int buffer) { buffer_ = buffer; }

private:
    String text_;                ///< The text to display.
    float text_size_;            ///< The font size in points.
    FontType font_type_;         ///< The type of font to use.
    Justification justification_;///< The justification for text.
    int buffer_;                 ///< Padding around the text.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PlainTextComponent)
};


/**
 * @class PlainShapeComponent
 * @brief A component that draws a shape into an OpenGlImageComponent.
 *
 * The PlainShapeComponent draws a specified Path, scaled and placed according to a given justification,
 * into its image. The shape can be easily updated by providing a new Path.
 */
class PlainShapeComponent : public OpenGlImageComponent {
public:
    /**
     * @brief Constructs a PlainShapeComponent with a given name.
     * @param name The component's name.
     */
    PlainShapeComponent(String name) : OpenGlImageComponent(name), justification_(Justification::centred) {
        setInterceptsMouseClicks(false, false);
    }

    void paintToImage(Graphics& g) override {
        Component* component = component_ ? component_ : this;
        Rectangle<float> bounds = component->getLocalBounds().toFloat();
        Path shape = shape_;
        shape.applyTransform(shape.getTransformToScaleToFit(bounds, true, justification_));

        g.setColour(Colours::white);
        g.fillPath(shape);
    }

    /**
     * @brief Sets the shape to be drawn.
     * @param shape The Path representing the new shape.
     */
    void setShape(Path shape) {
        shape_ = shape;
        redrawImage(true);
    }

    /**
     * @brief Sets the justification used when scaling the shape.
     * @param justification The Justification enum value.
     */
    void setJustification(Justification justification) { justification_ = justification; }

private:
    Path shape_;                  ///< The path of the shape to draw.
    Justification justification_; ///< How the shape is justified within the component.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PlainShapeComponent)
};
