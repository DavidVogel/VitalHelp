#pragma once

#include "JuceHeader.h"
#include "synth_section.h"

class PeakMeterViewer;
class VolumeSlider;

/**
 * @class VolumeSection
 * @brief A UI section that provides a master volume control and visual peak meters.
 *
 * The VolumeSection allows the user to adjust the master volume output and monitors the audio levels
 * through peak meter displays for the left and right channels. It visually represents the volume setting
 * and includes a custom VolumeSlider that shows the current value.
 */
class VolumeSection : public SynthSection {
public:
    /**
     * @brief Constructs a VolumeSection with a given name.
     * @param name The display name of this section.
     */
    VolumeSection(String name);

    /** @brief Destructor. */
    ~VolumeSection();

    /**
     * @brief Computes the height of each peak meter.
     * @return The height in pixels of the peak meters.
     */
    int getMeterHeight();

    /**
     * @brief Computes a vertical buffer space used to layout components.
     * @return The buffer size in pixels.
     */
    int getBuffer();

    /**
     * @brief Lays out and positions child components after a resize event.
     */
    void resized() override;

    /**
     * @brief Paints the background of the volume section, including meters and volume slider.
     * @param g The Graphics context.
     */
    void paintBackground(Graphics& g) override;

private:
    std::unique_ptr<VolumeSlider> volume_;             ///< Slider for controlling volume.
    std::unique_ptr<PeakMeterViewer> peak_meter_left_; ///< Peak meter for the left audio channel.
    std::unique_ptr<PeakMeterViewer> peak_meter_right_;///< Peak meter for the right audio channel.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VolumeSection)
};
