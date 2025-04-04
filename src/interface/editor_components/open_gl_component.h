#pragma once

#include "common.h"
#include "shaders.h"
#include "skin.h"
#include "synth_module.h"

class SynthSection;
class OpenGlCorners;

/**
 * @class OpenGlComponent
 * @brief A base component class that integrates JUCE's Component with OpenGL rendering.
 *
 * The OpenGlComponent class acts as a foundation for GUI elements that require OpenGL rendering.
 * It provides utility functions to set the viewport and scissor bounds, handle parent hierarchy
 * changes, manage OpenGL corners (rounded rectangles), and integrate skin values from a parent
 * SynthSection. Derived classes can override the rendering functions to draw custom visuals using OpenGL.
 */
class OpenGlComponent : public Component {
public:
    /**
     * @brief Sets the OpenGL viewport to match a specified rectangle within a component.
     *
     * This function calculates the global position of the given bounds inside the component's hierarchy
     * and sets the OpenGL viewport and scissor regions accordingly.
     * @param component The component for which the viewport is being set.
     * @param bounds The local bounds within the component to set as the viewport.
     * @param open_gl The OpenGlWrapper that holds the OpenGL context and scaling factors.
     * @return True if the viewport is successfully set, false otherwise.
     */
    static bool setViewPort(Component* component, Rectangle<int> bounds, OpenGlWrapper& open_gl);

    /**
     * @brief Convenience overload that sets the viewport for the entire component's local bounds.
     * @param component The component to set the viewport for.
     * @param open_gl The OpenGlWrapper with the OpenGL context.
     * @return True if the viewport is successfully set.
     */
    static bool setViewPort(Component* component, OpenGlWrapper& open_gl);

    /**
     * @brief Sets the OpenGL scissor region to the entire component's local bounds.
     * @param component The component for which the scissor is being set.
     * @param open_gl The OpenGlWrapper with the OpenGL context.
     */
    static void setScissor(Component* component, OpenGlWrapper& open_gl);

    /**
     * @brief Sets the OpenGL scissor region to a specified rectangle within a component.
     * @param component The component for which the scissor is being set.
     * @param bounds The local bounds to restrict rendering to.
     * @param open_gl The OpenGlWrapper with the OpenGL context.
     */
    static void setScissorBounds(Component* component, Rectangle<int> bounds, OpenGlWrapper& open_gl);

    /**
     * @brief Retrieves a uniform from the shader program if it exists.
     * @param open_gl The OpenGlWrapper containing the GL context.
     * @param program The shader program to query.
     * @param name The name of the uniform variable.
     * @return A unique_ptr to the Uniform, or nullptr if not found.
     */
    static std::unique_ptr<OpenGLShaderProgram::Uniform> getUniform(const OpenGlWrapper& open_gl,
                                                                    const OpenGLShaderProgram& program,
                                                                    const char* name) {
        if (open_gl.context.extensions.glGetUniformLocation(program.getProgramID(), name) >= 0)
            return std::make_unique<OpenGLShaderProgram::Uniform>(program, name);
        return nullptr;
    }

    /**
     * @brief Retrieves an attribute from the shader program if it exists.
     * @param open_gl The OpenGlWrapper containing the GL context.
     * @param program The shader program to query.
     * @param name The name of the attribute variable.
     * @return A unique_ptr to the Attribute, or nullptr if not found.
     */
    static std::unique_ptr<OpenGLShaderProgram::Attribute> getAttribute(const OpenGlWrapper& open_gl,
                                                                        const OpenGLShaderProgram& program,
                                                                        const char* name) {
        if (open_gl.context.extensions.glGetAttribLocation(program.getProgramID(), name) >= 0)
            return std::make_unique<OpenGLShaderProgram::Attribute>(program, name);
        return nullptr;
    }

    /**
     * @brief Constructs an OpenGlComponent.
     * @param name Optional name for the component.
     */
    OpenGlComponent(String name = "");

    /**
     * @brief Destructor.
     */
    virtual ~OpenGlComponent();

    /**
     * @brief Called when the component is resized.
     *
     * Adjusts internal states or sub-components (like corners).
     */
    virtual void resized() override;

    /**
     * @brief Called when the component's parent hierarchy changes.
     *
     * Used to retrieve parent references (like num_voices_readout_ from SynthGuiInterface).
     */
    virtual void parentHierarchyChanged() override;

    /**
     * @brief Adds rounded corners to the component's edges.
     */
    void addRoundedCorners();

    /**
     * @brief Adds rounded corners only at the bottom of the component.
     */
    void addBottomRoundedCorners();

    /**
     * @brief Initializes any OpenGL-specific resources needed by the component.
     * @param open_gl The OpenGlWrapper with the current OpenGL context and shaders.
     */
    virtual void init(OpenGlWrapper& open_gl);

    /**
     * @brief Pure virtual function to render the component using OpenGL.
     * @param open_gl The OpenGlWrapper with the current OpenGL context.
     * @param animate If true, the rendering might include animations.
     */
    virtual void render(OpenGlWrapper& open_gl, bool animate) = 0;

    /**
     * @brief Renders the corner shapes using the given color and rounding amount.
     * @param open_gl The OpenGlWrapper for OpenGL context.
     * @param animate If true, enable animated rendering.
     * @param color The color to draw the corners.
     * @param rounding The rounding radius for corners.
     */
    void renderCorners(OpenGlWrapper& open_gl, bool animate, Colour color, float rounding);

    /**
     * @brief Renders corners with default body color and rounding.
     * @param open_gl The OpenGlWrapper for OpenGL context.
     * @param animate If true, enable animated rendering.
     */
    void renderCorners(OpenGlWrapper& open_gl, bool animate);

    /**
     * @brief Destroys any OpenGL-specific resources allocated by this component.
     * @param open_gl The OpenGlWrapper with the current OpenGL context.
     */
    virtual void destroy(OpenGlWrapper& open_gl);

    /**
     * @brief Paints a standard background for the component.
     * @param g The graphics context.
     */
    virtual void paintBackground(Graphics& g);

    /**
     * @brief Requests a repaint of the component's background on the OpenGL layer.
     */
    void repaintBackground();

    /**
     * @brief Retrieves the component's body color.
     * @return The body color.
     */
    Colour getBodyColor() const { return body_color_; }

    /**
     * @brief Sets a pointer to the parent SynthSection for skin value lookups.
     * @param parent The parent SynthSection.
     */
    void setParent(const SynthSection* parent) { parent_ = parent; }

    /**
     * @brief Finds a float value from the skin associated with this component's parent.
     * @param value_id The Skin::ValueId to lookup.
     * @return The retrieved float value.
     */
    float findValue(Skin::ValueId value_id);

    /**
     * @brief Applies the skin overrides to this component's colors.
     * @param skin The Skin object containing color definitions.
     */
    void setSkinValues(const Skin& skin) {
        skin.setComponentColors(this, skin_override_, false);
    }

    /**
     * @brief Sets a skin override to control the component's color scheme.
     * @param skin_override The skin override enum value.
     */
    void setSkinOverride(Skin::SectionOverride skin_override) { skin_override_ = skin_override; }

    /**
     * @brief Translates a fragment shader code snippet to be compatible with the current GL version.
     * @param code The original shader code.
     * @return The translated shader code.
     */
    static inline String translateFragmentShader(const String& code) {
#if OPENGL_ES
        return String("#version 300 es\n") + "out mediump vec4 fragColor;\n" +
             code.replace("varying", "in").replace("texture2D", "texture").replace("gl_FragColor", "fragColor");
#else
        return OpenGLHelpers::translateFragmentShaderToV3(code);
#endif
    }

    /**
     * @brief Translates a vertex shader code snippet to be compatible with the current GL version.
     * @param code The original shader code.
     * @return The translated shader code.
     */
    static inline String translateVertexShader(const String& code) {
#if OPENGL_ES
        return String("#version 300 es\n") + code.replace("attribute", "in").replace("varying", "out");
#else
        return OpenGLHelpers::translateVertexShaderToV3(code);
#endif
    }

    /**
     * @brief Checks for and asserts that there are no OpenGL errors.
     */
    force_inline void checkGlError() {
#if DEBUG
        int error = glGetError();
      assert(error == GL_NO_ERROR);
#endif
    }

    /**
     * @brief Sets the background color of the component for painting operations.
     * @param color The new background color.
     */
    void setBackgroundColor(const Colour& color) { background_color_ = color; }

protected:
    /**
     * @brief Sets the viewport for this component using the current OpenGlWrapper.
     * @param open_gl The OpenGlWrapper providing the OpenGL context.
     * @return True if viewport setup succeeded.
     */
    bool setViewPort(OpenGlWrapper& open_gl);

    std::unique_ptr<OpenGlCorners> corners_; ///< Optional corners for rounded edges.
    bool only_bottom_corners_;               ///< Flag to round only the bottom corners.
    Colour background_color_;                ///< The background color of the component.
    Colour body_color_;                      ///< The body color of the component.
    const SynthSection* parent_;             ///< Pointer to parent SynthSection for skin lookups.
    Skin::SectionOverride skin_override_;    ///< Skin override for custom appearance.
    const vital::StatusOutput* num_voices_readout_; ///< StatusOutput for voice count lookups.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OpenGlComponent)
};
