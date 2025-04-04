/*
Summary:
ShepardToneSource uses a single keyframe and a special looping technique to produce a continuously rising (or falling) tone—an auditory illusion known as a Shepard tone. By rearranging frequency components into a loop frame and interpolating between these frames, this source can create a stable, looping sonic texture that simulates infinite pitch movement.
 */

#pragma once

#include "JuceHeader.h"
#include "wave_source.h"

/**
 * @brief A WaveSource that constructs a special looped waveform reminiscent of a Shepard tone.
 *
 * ShepardToneSource uses a single keyframe and loops the spectrum in a manner that can create an auditory illusion
 * similar to a Shepard tone—where the tone seems to continually ascend (or descend) without actually changing pitch.
 * Internally, it duplicates and interleaves frequency components to produce a repetitive, seamless effect when traversing frames.
 */
class ShepardToneSource : public WaveSource {
public:
    /**
     * @brief Constructs a ShepardToneSource with a loop frame for seamless frequency content.
     */
    ShepardToneSource();
    virtual ~ShepardToneSource();

    /**
     * @brief Renders a frame of the Shepard tone wavetable at a given position.
     *
     * The ShepardToneSource reads from its single keyframe, duplicates frequency bins in a certain pattern, and
     * interpolates between the base and loop frames to create a continuous tone effect.
     *
     * @param wave_frame The WaveFrame to render into.
     * @param position The normalized position along the wavetable (0 to 1).
     */
    virtual void render(vital::WaveFrame* wave_frame, float position) override;

    /**
     * @brief Returns the component type associated with this source.
     *
     * @return The ComponentType enumerated value representing a Shepard tone source.
     */
    virtual WavetableComponentFactory::ComponentType getType() override;

    /**
     * @brief Indicates whether this component has keyframes.
     *
     * ShepardToneSource uses a single keyframe and a looping frame; it effectively doesn't operate on multiple keyframes.
     *
     * @return False since it does not rely on a series of keyframes.
     */
    virtual bool hasKeyframes() override { return false; }

protected:
    std::unique_ptr<WaveSourceKeyframe> loop_frame_; ///< A loop frame used to create the repetitive Shepard effect.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ShepardToneSource)
};
