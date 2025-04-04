#pragma once

#include "JuceHeader.h"

#include "bar_editor.h"
#include "wavetable_component_overlay.h"
#include "wave_source.h"
#include "wave_source_editor.h"

/** Forward declaration of TextSelector class. */
class TextSelector;

/**
 * @class WaveSourceOverlay
 * @brief Provides an overlay for editing WaveSource objects in the Wavetable component.
 *
 * The WaveSourceOverlay is responsible for handling graphical user interface elements
 * that allow users to edit a WaveSource (time-domain waveform, frequency amplitudes, and phases).
 * It integrates with various editors (WaveSourceEditor, BarEditor) to facilitate editing
 * both the time-domain data and the frequency-domain data.
 */
class WaveSourceOverlay : public WavetableComponentOverlay,
                          public WaveSourceEditor::Listener,
                          public BarEditor::Listener {
public:
    /**
     * @brief Default number of horizontal grid divisions.
     */
    static constexpr int kDefaultXGrid = 6;

    /**
     * @brief Default number of vertical grid divisions.
     */
    static constexpr int kDefaultYGrid = 4;

    /**
     * @brief Default phase value for frequency phase editor.
     *
     * A negative phase indicates a shift in the waveform’s starting angle.
     */
    static constexpr float kDefaultPhase = -0.5f;

    /**
     * @brief Default alpha value for bars in BarEditor components.
     */
    static constexpr float kBarAlpha = 0.75f;

    /**
     * @brief Constructs a new WaveSourceOverlay.
     *
     * Initializes various editors (time-domain, frequency amplitude, frequency phase)
     * and controls (sliders, incrementers, etc.) for modifying the wave source data.
     */
    WaveSourceOverlay();

    /**
     * @brief Called when the component is resized.
     *
     * Updates the layout and colors for the internal sub-components.
     */
    void resized() override;

    /**
     * @brief Called when a keyframe is selected within the Wavetable.
     *
     * If the selected keyframe belongs to this overlay's WaveSource, the
     * overlay's editors become visible and load the current frame data.
     *
     * @param keyframe Pointer to the selected WavetableKeyframe.
     */
    virtual void frameSelected(WavetableKeyframe* keyframe) override;

    /**
     * @brief Called when a keyframe is dragged within the Wavetable.
     *
     * This override does nothing for the WaveSourceOverlay.
     *
     * @param keyframe Pointer to the dragged WavetableKeyframe.
     * @param position The new position of the keyframe.
     */
    virtual void frameDragged(WavetableKeyframe* keyframe, int position) override { }

    /**
     * @brief Sets the bounds for the editable region of this overlay.
     *
     * @param bounds The bounding rectangle for the overlay’s controls.
     */
    virtual void setEditBounds(Rectangle<int> bounds) override;

    /**
     * @brief Sets the bounds for the time-domain editor.
     *
     * @param bounds The bounding rectangle for the time-domain editor.
     * @return True if successfully set.
     */
    virtual bool setTimeDomainBounds(Rectangle<int> bounds) override;

    /**
     * @brief Sets the bounds for the frequency amplitude editor.
     *
     * @param bounds The bounding rectangle for the frequency amplitude editor.
     * @return True if successfully set.
     */
    virtual bool setFrequencyAmplitudeBounds(Rectangle<int> bounds) override;

    /**
     * @brief Sets the bounds for the frequency phase editor.
     *
     * @param bounds The bounding rectangle for the frequency phase editor.
     * @return True if successfully set.
     */
    virtual bool setPhaseBounds(Rectangle<int> bounds) override;

    /**
     * @brief Updates the frequency-domain editors with new data.
     *
     * Populates frequency amplitude and phase editors from a complex float array.
     *
     * @param frequency_domain Pointer to the frequency-domain array of std::complex<float>.
     */
    void updateFrequencyDomain(std::complex<float>* frequency_domain);

    /**
     * @brief Loads frequency-domain data from the editors back into the current frame.
     *
     * Converts the updated frequency-domain data into the time-domain,
     * normalizes, then back to frequency-domain.
     */
    void loadFrequencyDomain();

    /**
     * @brief Called when values in the WaveSourceEditor are changed.
     *
     * Updates the time-domain data in the current frame and recalculates frequency-domain data.
     *
     * @param start   The start index of changed data.
     * @param end     The end index of changed data.
     * @param mouse_up Indicates whether the mouse button is released after changing values.
     */
    void valuesChanged(int start, int end, bool mouse_up) override;

    /**
     * @brief Called when values in the BarEditor are changed.
     *
     * Updates the frequency-domain data, recalculates the time-domain waveform, and reloads the editor.
     *
     * @param start   The start index of changed data.
     * @param end     The end index of changed data.
     * @param mouse_up Indicates whether the mouse button is released after changing values.
     */
    void barsChanged(int start, int end, bool mouse_up) override;

    /**
     * @brief Called when a slider changes value.
     *
     * Used to handle changes in grid size and interpolation type.
     *
     * @param moved_slider Pointer to the slider that has changed.
     */
    void sliderValueChanged(Slider* moved_slider) override;

    /**
     * @brief Enables or disables power scaling for the frequency amplitude editor.
     *
     * @param scale If true, the frequency amplitudes will be power-scaled.
     */
    void setPowerScale(bool scale) override { frequency_amplitudes_->setPowerScale(scale); }

    /**
     * @brief Sets the zoom level for frequency-domain editors.
     *
     * @param zoom The new zoom level.
     */
    void setFrequencyZoom(float zoom) override {
      frequency_amplitudes_->setScale(zoom);
      frequency_phases_->setScale(zoom);
    }

    /**
     * @brief Sets the interpolation type for the WaveSource.
     *
     * @param style The interpolation style.
     * @param mode  The interpolation mode (time or spectral).
     */
    void setInterpolationType(WaveSource::InterpolationStyle style,
                              WaveSource::InterpolationMode mode);

    /**
     * @brief Assigns a WaveSource to this overlay.
     *
     * @param wave_source Pointer to the WaveSource to edit.
     */
    void setWaveSource(WaveSource* wave_source) {
      wave_source_ = wave_source;
      current_frame_ = nullptr;
    }

protected:
    /**
     * @brief Pointer to the WaveSource being edited.
     */
    WaveSource* wave_source_;

    /**
     * @brief Pointer to the currently selected WaveFrame in the WaveSource.
     */
    vital::WaveFrame* current_frame_;

    /**
     * @brief Editor for time-domain waveforms.
     */
    std::unique_ptr<WaveSourceEditor> oscillator_;

    /**
     * @brief BarEditor for frequency amplitudes.
     */
    std::unique_ptr<BarEditor> frequency_amplitudes_;

    /**
     * @brief BarEditor for frequency phases.
     */
    std::unique_ptr<BarEditor> frequency_phases_;

    /**
     * @brief TextSelector for choosing interpolation style and mode.
     */
    std::unique_ptr<TextSelector> interpolation_type_;

    /**
     * @brief SynthSlider for the number of horizontal grid lines in the WaveSourceEditor.
     */
    std::unique_ptr<SynthSlider> horizontal_grid_;

    /**
     * @brief SynthSlider for the number of vertical grid lines in the WaveSourceEditor.
     */
    std::unique_ptr<SynthSlider> vertical_grid_;

    /**
     * @brief Incrementer button set for horizontal grid adjustments.
     */
    std::unique_ptr<Component> horizontal_incrementers_;

    /**
     * @brief Incrementer button set for vertical grid adjustments.
     */
    std::unique_ptr<Component> vertical_incrementers_;

    /**
     * @brief (Unused) Slider for interpolation selection in other contexts.
     */
    std::unique_ptr<Slider> interpolation_selector_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveSourceOverlay)
};
