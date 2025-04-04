#pragma once

#include "JuceHeader.h"
#include "open_gl_multi_quad.h"
#include "synth_gui_interface.h"

/**
 * @class MidiKeyboard
 * @brief A visual and interactive MIDI keyboard component.
 *
 * The MidiKeyboard component displays a piano keyboard that can be interacted with using the mouse.
 * Users can press keys to produce note-on messages and release them to produce note-off messages.
 * Keys are dynamically colored based on their current state (pressed, hovered, white/black), and
 * velocities are determined by the vertical click position. The component supports real-time rendering
 * using OpenGL for performance.
 */
class MidiKeyboard : public OpenGlComponent {
public:
    /**
     * @brief Horizontal offsets for black keys relative to white keys, per octave.
     *
     * These offsets define the placement of black keys within each octave, ensuring a visually
     * accurate piano layout.
     */
    static const float kBlackKeyOffsets[];

    /**
     * @brief Array indicating which notes (semitones) in an octave are white keys.
     *
     * True values represent white keys, false values represent black keys.
     */
    static const bool kWhiteKeys[];

    /// Total number of white keys across the entire MIDI range.
    static constexpr int kNumWhiteKeys = 75;
    /// Number of white keys per octave.
    static constexpr int kNumWhiteKeysPerOctave = 7;
    /// Total number of black keys across the entire MIDI range.
    static constexpr int kNumBlackKeys = vital::kMidiSize - kNumWhiteKeys;
    /// Number of black keys per octave.
    static constexpr int kNumBlackKeysPerOctave = vital::kNotesPerOctave - kNumWhiteKeysPerOctave;

    /// Ratio of keyboard height covered by black keys.
    static constexpr float kBlackKeyHeightRatio = 0.7f;
    /// Horizontal ratio of the black key width relative to white keys.
    static constexpr float kBlackKeyWidthRatio = 0.8f;

    /**
     * @brief Determines if a given MIDI note number corresponds to a white key.
     * @param midi The MIDI note number.
     * @return True if it's a white key, false if it's black.
     */
    force_inline static bool isWhiteKey(int midi) {
        return kWhiteKeys[midi % vital::kNotesPerOctave];
    }

    /**
     * @brief Constructs the MidiKeyboard component.
     * @param state Reference to a MidiKeyboardState managing current pressed notes.
     */
    MidiKeyboard(MidiKeyboardState& state);

    /**
     * @brief Paints any background elements of the keyboard, typically key boundaries.
     * @param g The graphics context.
     */
    void paintBackground(Graphics& g) override;

    /**
     * @brief Called when the parent hierarchy changes.
     *
     * Used to apply color schemes from the parent SynthGuiInterface if available.
     */
    void parentHierarchyChanged() override;

    /**
     * @brief Called when the component is resized.
     *
     * Recalculates key positions and updates OpenGL quads for keys.
     */
    void resized() override;

    /**
     * @brief Determines the MIDI note at a given mouse position.
     * @param position The mouse position relative to the component.
     * @return The MIDI note number, or -1 if the position doesn't correspond to a note.
     */
    int getNoteAtPosition(Point<float> position);

    /**
     * @brief Checks if a given position falls within the vertical range of a black key.
     * @param position The point to check.
     * @return True if the point is within the black key vertical range.
     */
    bool isBlackKeyHeight(Point<float> position) { return position.y / getHeight() < kBlackKeyHeightRatio; }

    /**
     * @brief Calculates the note velocity based on vertical mouse click position.
     * @param midi The MIDI note being triggered.
     * @param position The mouse position relative to the component.
     * @return A velocity value in the range [0, 1].
     */
    float getVelocityForNote(int midi, Point<float> position);

    /**
     * @brief Initializes OpenGL resources for rendering keys.
     * @param open_gl The OpenGL wrapper.
     */
    void init(OpenGlWrapper& open_gl) override {
        black_notes_.init(open_gl);
        white_pressed_notes_.init(open_gl);
        black_pressed_notes_.init(open_gl);
    }

    /**
     * @brief Renders the keyboard and its keys using OpenGL.
     * @param open_gl The OpenGL wrapper.
     * @param animate If true, animate transitions or changes in state.
     */
    void render(OpenGlWrapper& open_gl, bool animate) override;

    /**
     * @brief Updates which keys are displayed as pressed according to the current MidiKeyboardState.
     */
    void setPressedKeyPositions();

    /**
     * @brief Destroys OpenGL resources when the component is removed or reinitialized.
     * @param open_gl The OpenGL wrapper.
     */
    void destroy(OpenGlWrapper& open_gl) override {
        black_notes_.destroy(open_gl);
        white_pressed_notes_.destroy(open_gl);
        black_pressed_notes_.destroy(open_gl);
    }

    /**
     * @brief Handles mouse down events to trigger note-on messages.
     * @param e The mouse event.
     */
    void mouseDown(const MouseEvent& e) override {
        hover_note_ = getNoteAtPosition(e.position);
        state_.noteOn(midi_channel_, hover_note_, getVelocityForNote(hover_note_, e.position));
    }

    /**
     * @brief Handles mouse up events to trigger note-off messages.
     * @param e The mouse event.
     */
    void mouseUp(const MouseEvent& e) override {
        state_.noteOff(midi_channel_, hover_note_, 0.0f);
        hover_note_ = getNoteAtPosition(e.position);
    }

    /**
     * @brief Handles mouse enter events to update the hovered note.
     * @param e The mouse event.
     */
    void mouseEnter(const MouseEvent& e) override {
        hover_note_ = getNoteAtPosition(e.position);
    }

    /**
     * @brief Handles mouse exit events to clear the hovered note.
     * @param e The mouse event.
     */
    void mouseExit(const MouseEvent& e) override {
        hover_note_ = -1;
    }

    /**
     * @brief Handles mouse drag events, allowing note slides across keys.
     * @param e The mouse event.
     */
    void mouseDrag(const MouseEvent& e) override {
        int note = getNoteAtPosition(e.position);
        if (note == hover_note_)
            return;

        state_.noteOff(midi_channel_, hover_note_, 0.0f);
        state_.noteOn(midi_channel_, note, getVelocityForNote(note, e.position));
        hover_note_ = note;
    }

    /**
     * @brief Handles mouse move events to update which note is hovered.
     * @param e The mouse event.
     */
    void mouseMove(const MouseEvent& e) override {
        hover_note_ = getNoteAtPosition(e.position);
    }

    /**
     * @brief Sets the MIDI channel used by note-on and note-off messages.
     * @param channel The MIDI channel number (1-based).
     */
    void setMidiChannel(int channel) { midi_channel_ = channel; }

    /**
     * @brief Updates the color scheme of the keys.
     */
    void setColors();

private:
    /**
     * @brief Configures the quad for a white key in the given OpenGL multi-quad array.
     * @param quads The OpenGLMultiQuad instance for white pressed keys.
     * @param quad_index The index of the quad to set.
     * @param white_key_index The index of the white key across the keyboard.
     */
    void setWhiteKeyQuad(OpenGlMultiQuad* quads, int quad_index, int white_key_index);

    /**
     * @brief Configures the quad for a black key in the given OpenGL multi-quad array.
     * @param quads The OpenGLMultiQuad instance for black pressed keys.
     * @param quad_index The index of the quad to set.
     * @param black_key_index The index of the black key across the keyboard.
     */
    void setBlackKeyQuad(OpenGlMultiQuad* quads, int quad_index, int black_key_index);

    MidiKeyboardState& state_; ///< Reference to the state of the MIDI keyboard (notes on/off).
    int midi_channel_;         ///< MIDI channel to use when sending note messages.
    int hover_note_;           ///< The currently hovered MIDI note (-1 if none).

    OpenGlMultiQuad black_notes_;         ///< OpenGL quads for black keys.
    OpenGlMultiQuad white_pressed_notes_; ///< OpenGL quads for visually indicating pressed white keys.
    OpenGlMultiQuad black_pressed_notes_; ///< OpenGL quads for visually indicating pressed black keys.
    OpenGlQuad hover_note_quad_;          ///< Quad for highlighting hovered keys.

    Colour key_press_color_; ///< Color used for keys that are currently pressed.
    Colour hover_color_;     ///< Color used to highlight the hovered key.
    Colour white_key_color_; ///< Color of white keys.
    Colour black_key_color_; ///< Color of black keys.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiKeyboard)
};
