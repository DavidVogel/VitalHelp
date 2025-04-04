/*
Summary:
WaveLineSource constructs wavetables using a line generator that defines a waveform through a set of points. By adjusting the points’ positions and power values, and optionally smoothing the line, it produces a custom waveform shape. Keyframes store configurations of these line-based shapes, and interpolation (including nonlinear “pull_power” effects) allows the waveform to evolve smoothly from one shape to another across the wavetable’s dimension.
 */

#pragma once

#include "JuceHeader.h"
#include "wavetable_component.h"

/**
 * @brief A WavetableComponent that generates waveforms from a series of line segments.
 *
 * WaveLineSource uses a `LineGenerator` internally, which defines a waveform by connecting a set of points with
 * lines (optionally smoothed). Each point has both a position and a power parameter that influence the shape.
 * Interpolation between keyframes allows for dynamic changes in the line-based waveform shape across the
 * wavetable dimension.
 *
 * By controlling the number of points and their arrangement, one can create simple waves (e.g., linear ramps)
 * or more complex curves. The `pull_power_` parameter affects how interpolation behaves, allowing for nonlinear
 * transitions.
 */
class WaveLineSource : public WavetableComponent {
public:
    /// The default number of line points used if none are specified.
    static constexpr int kDefaultLinePoints = 4;

    /**
     * @brief A keyframe class that represents a particular line-based waveform configuration at a given position.
     *
     * Each WaveLineSourceKeyframe holds a `LineGenerator` which stores multiple points defining the wave shape.
     * Points can be added, removed, have their positions and power values adjusted, and the shape can be smoothed.
     * Interpolating between two keyframes merges their line definitions in a power-scaled manner, controlled by
     * `pull_power_`.
     */
    class WaveLineSourceKeyframe : public WavetableKeyframe {
    public:
        /**
         * @brief Constructs a WaveLineSourceKeyframe with default parameters.
         */
        WaveLineSourceKeyframe();
        virtual ~WaveLineSourceKeyframe() = default;

        void copy(const WavetableKeyframe* keyframe) override;
        void interpolate(const WavetableKeyframe* from_keyframe,
                         const WavetableKeyframe* to_keyframe, float t) override;
        void render(vital::WaveFrame* wave_frame) override;
        json stateToJson() override;
        void jsonToState(json data) override;

        /**
         * @brief Retrieves a point (x,y) by index.
         *
         * @param index The point index.
         * @return A pair representing the point’s (x, y) coordinates.
         */
        inline std::pair<float, float> getPoint(int index) const { return line_generator_.getPoint(index); }

        /**
         * @brief Retrieves the power value for a given point.
         *
         * The power influences how the line transitions at that point.
         *
         * @param index The point index.
         * @return The power value for that point.
         */
        inline float getPower(int index) const { return line_generator_.getPower(index); }

        /**
         * @brief Sets a point’s position.
         *
         * @param point The new (x, y) coordinates for the point.
         * @param index The point index to modify.
         */
        inline void setPoint(std::pair<float, float> point, int index) { line_generator_.setPoint(index, point); }

        /**
         * @brief Sets the power for a given point.
         *
         * @param power The new power value.
         * @param index The point index.
         */
        inline void setPower(float power, int index) { line_generator_.setPower(index, power); }

        /**
         * @brief Removes a point from the line definition.
         *
         * @param index The index of the point to remove.
         */
        inline void removePoint(int index) { line_generator_.removePoint(index); }

        /**
         * @brief Inserts a middle point between two existing points.
         *
         * @param index The index at which to insert a new point in the middle.
         */
        inline void addMiddlePoint(int index) { line_generator_.addMiddlePoint(index); }

        /**
         * @brief Gets the number of points defining the line.
         *
         * @return The number of points in the line generator.
         */
        inline int getNumPoints() const { return line_generator_.getNumPoints(); }

        /**
         * @brief Sets whether the line should be smoothed.
         *
         * If smooth is true, the line transitions are more gradual.
         *
         * @param smooth True to enable smoothing, false otherwise.
         */
        inline void setSmooth(bool smooth) { line_generator_.setSmooth(smooth); }

        /**
         * @brief Sets the pull power, which influences how interpolation occurs between keyframes.
         *
         * @param power The new pull power value.
         */
        void setPullPower(float power) { pull_power_ = power; }

        /**
         * @brief Gets the current pull power value.
         *
         * @return The pull power value.
         */
        float getPullPower() const { return pull_power_; }

        /**
         * @brief Provides const access to the underlying LineGenerator.
         *
         * @return A const pointer to the line generator.
         */
        const LineGenerator* getLineGenerator() const { return &line_generator_; }

        /**
         * @brief Provides access to the underlying LineGenerator for modification.
         *
         * @return A pointer to the line generator.
         */
        LineGenerator* getLineGenerator() { return &line_generator_; }

    protected:
        LineGenerator line_generator_; ///< The generator producing the line-based waveform.
        float pull_power_;             ///< Controls nonlinear interpolation between keyframes.

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveLineSourceKeyframe)
    };

    /**
     * @brief Constructs a WaveLineSource with a default number of points.
     */
    WaveLineSource() : num_points_(kDefaultLinePoints), compute_frame_() { }
    virtual ~WaveLineSource() = default;

    WavetableKeyframe* createKeyframe(int position) override;
    void render(vital::WaveFrame* wave_frame, float position) override;
    WavetableComponentFactory::ComponentType getType() override;
    json stateToJson() override;
    void jsonToState(json data) override;

    /**
     * @brief Sets the number of points used by the line generator.
     *
     * @param num_points The desired number of points.
     */
    void setNumPoints(int num_points);

    /**
     * @brief Returns the current number of points in the line generator.
     *
     * @return The number of points.
     */
    int numPoints() { return num_points_; }

    /**
     * @brief Retrieves a WaveLineSourceKeyframe by index.
     *
     * @param index The index of the desired keyframe.
     * @return A pointer to the WaveLineSourceKeyframe.
     */
    WaveLineSourceKeyframe* getKeyframe(int index);

protected:
    int num_points_;                         ///< The number of points defining the line-based waveform.
    WaveLineSourceKeyframe compute_frame_;   ///< A keyframe for intermediate computations.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveLineSource)
};
