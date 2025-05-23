#pragma once

#include "JuceHeader.h"
#include "common.h"
#include "json/json.h"

using json = nlohmann::json;

/**
 * @brief A class for generating and storing a line shape, defined by a series of points and associated powers.
 *
 * The LineGenerator manages a set of control points and their associated interpolation powers to generate a
 * line shape. This line can represent a waveform, an envelope, or any parameter that changes smoothly (or linearly)
 * over a normalized 0.0 to 1.0 range.
 *
 * The class supports various presets (linear, triangle, square, sine, sawtooth), optional looping behavior, and
 * configurable smoothing. It also provides JSON serialization and deserialization to save and load states, as well
 * as utility methods to flip the line horizontally or vertically.
 */
class LineGenerator {
public:
    /**
     * @brief Maximum number of points that can define the line.
     */
    static constexpr int kMaxPoints = 100;

    /**
     * @brief Default resolution of the rendered line buffer.
     *
     * This is the number of samples (points) in the generated line buffer. Higher resolution results in finer granularity.
     */
    static constexpr int kDefaultResolution = 2048;

    /**
     * @brief Extra buffer values used for interpolation outside the main range.
     *
     * A small number of extra samples are stored to facilitate safe interpolation at buffer edges.
     */
    static constexpr int kExtraValues = 3;

    /**
     * @brief Smooth transition function for smoothing between points.
     *
     * This function applies a sine-based smoothing to a normalized parameter t, producing a smooth transition curve.
     *
     * @param t A normalized value between 0.0 and 1.0.
     * @return The smoothed value.
     */
    static force_inline float smoothTransition(float t) {
        return 0.5f * sinf((t - 0.5f) * vital::kPi) + 0.5f;
    }

    /**
     * @brief Constructs a LineGenerator with a given resolution.
     *
     * @param resolution The number of samples in the generated buffer. Defaults to kDefaultResolution.
     */
    LineGenerator(int resolution = kDefaultResolution);

    /**
     * @brief Virtual destructor.
     */
    virtual ~LineGenerator() { }

    /**
     * @brief Sets whether the line should loop at the end.
     *
     * If loop is true, the line wraps around from the end back to the start for continuous interpolation.
     *
     * @param loop True to enable looping, false otherwise.
     */
    void setLoop(bool loop) { loop_ = loop; render(); }

    /**
     * @brief Sets a name identifier for the line.
     *
     * @param name A string representing the line's name.
     */
    void setName(const std::string& name) { name_ = name; }

    /**
     * @brief Stores the last browsed file path associated with this line.
     *
     * @param path The file path.
     */
    void setLastBrowsedFile(const std::string& path) { last_browsed_file_ = path; }

    /**
     * @brief Enables or disables smoothing behavior.
     *
     * Smoothing affects how values between points are interpolated. When smoothing is enabled,
     * the transitions between points are softened.
     *
     * @param smooth True to enable smoothing, false to disable.
     */
    void setSmooth(bool smooth) { smooth_ = smooth; checkLineIsLinear(); render(); }

    /**
     * @brief Initializes the line to a simple linear shape (from 1.0 at x=0 to 0.0 at x=1).
     */
    void initLinear();

    /**
     * @brief Initializes the line as a triangle shape.
     */
    void initTriangle();

    /**
     * @brief Initializes the line as a square shape.
     */
    void initSquare();

    /**
     * @brief Initializes the line as a sine-like shape.
     */
    void initSin();

    /**
     * @brief Initializes the line as a "saw up" shape.
     */
    void initSawUp();

    /**
     * @brief Initializes the line as a "saw down" shape.
     */
    void initSawDown();

    /**
     * @brief Renders the line into the internal buffer based on the current points and settings.
     *
     * This method should be called after modifying points, powers, or other parameters. It updates the internal buffer
     * used for fast lookups.
     */
    void render();

    /**
     * @brief Converts the current state of the line into a JSON object.
     *
     * The JSON includes the number of points, their positions, powers, name, and smooth setting.
     *
     * @return A JSON object representing the current line state.
     */
    json stateToJson();

    /**
     * @brief Checks if a given JSON object contains valid line data.
     *
     * @param data The JSON object to check.
     * @return True if valid, false otherwise.
     */
    static bool isValidJson(json data);

    /**
     * @brief Restores the line state from a given JSON object.
     *
     * @param data A JSON object previously generated by stateToJson().
     */
    void jsonToState(json data);

    /**
     * @brief Gets the line value at a given normalized phase.
     *
     * @param phase A value between 0.0 and 1.0 representing a position along the line.
     * @return The interpolated line value at the given phase.
     */
    float valueAtPhase(float phase);

    /**
     * @brief Checks if the line is the default linear shape.
     *
     * This updates the `linear_` member based on the current points and smoothness setting.
     */
    void checkLineIsLinear();

    /**
     * @brief Interpolates a value between two points at a given x position.
     *
     * @param x The x-coordinate between the two points.
     * @param index_from The index of the starting point.
     * @param index_to The index of the ending point.
     * @return The interpolated y-value at x.
     */
    float getValueBetweenPoints(float x, int index_from, int index_to);

    /**
     * @brief Returns the value of the line at a given phase by searching through the points.
     *
     * @param phase A value between 0.0 and 1.0.
     * @return The interpolated line value at the given phase.
     */
    float getValueAtPhase(float phase);

    /**
     * @brief Gets the current name of the line.
     *
     * @return The line name.
     */
    std::string getName() const { return name_; }

    /**
     * @brief Gets the last browsed file path associated with this line.
     *
     * @return The file path as a string.
     */
    std::string getLastBrowsedFile() const { return last_browsed_file_; }

    /**
     * @brief Inserts a new point into the line at a specified index.
     *
     * The number of points increases by one. Existing points from index onward are shifted.
     *
     * @param index The position to insert the new point.
     * @param position The (x, y) coordinates of the new point.
     */
    void addPoint(int index, std::pair<float, float> position);

    /**
     * @brief Inserts a point exactly between two existing points.
     *
     * This finds the midpoint between points[index-1] and points[index], and adds a new point there.
     *
     * @param index The index after which the middle point will be inserted.
     */
    void addMiddlePoint(int index);

    /**
     * @brief Removes the point at a specified index.
     *
     * @param index The position of the point to remove.
     */
    void removePoint(int index);

    /**
     * @brief Flips the line horizontally around the x=0.5 vertical axis.
     *
     * This reverses the order of points and their x-values to mirror the shape horizontally.
     */
    void flipHorizontal();

    /**
     * @brief Flips the line vertically around y=0.5.
     *
     * This inverts the y-values of the points, mirroring the shape vertically.
     */
    void flipVertical();

    /**
     * @brief Returns the last point in the line.
     *
     * @return A pair representing the (x, y) coordinates of the last point.
     */
    std::pair<float, float> lastPoint() const { return points_[num_points_ - 1]; }

    /**
     * @brief Returns the power (interpolation shape factor) of the last point.
     *
     * @return A float representing the power of the last point.
     */
    float lastPower() const { return powers_[num_points_ - 1]; }

    /**
     * @brief Gets the resolution of the line's internal buffer.
     *
     * @return The resolution, typically kDefaultResolution.
     */
    force_inline int resolution() const { return resolution_; }

    /**
     * @brief Indicates whether the line is currently a simple linear shape.
     *
     * @return True if linear, false otherwise.
     */
    force_inline bool linear() const { return linear_; }

    /**
     * @brief Indicates whether smoothing is enabled.
     *
     * @return True if smoothing is enabled, false otherwise.
     */
    force_inline bool smooth() const { return smooth_; }

    /**
     * @brief Gets a pointer to the internal buffer used for interpolation.
     *
     * This returns the buffer starting from index 1, which is typically how the lookup is performed.
     *
     * @return A pointer to the buffer.
     */
    force_inline vital::mono_float* getBuffer() const { return buffer_.get() + 1; }

    /**
     * @brief Gets a pointer to the buffer used for cubic interpolation.
     *
     * This returns the buffer from index 0, allowing for additional interpolation techniques.
     *
     * @return A pointer to the buffer.
     */
    force_inline vital::mono_float* getCubicInterpolationBuffer() const { return buffer_.get(); }

    /**
     * @brief Returns a point at the given index.
     *
     * @param index The point index (0-based).
     * @return A pair (x, y) representing the point's coordinates.
     */
    force_inline std::pair<float, float> getPoint(int index) const {
        VITAL_ASSERT(index < kMaxPoints && index >= 0);
        return points_[index];
    }

    /**
     * @brief Returns the power at the given index.
     *
     * @param index The point index.
     * @return The power value for that point.
     */
    force_inline float getPower(int index) const {
        VITAL_ASSERT(index < kMaxPoints && index >= 0);
        return powers_[index];
    }

    /**
     * @brief Returns the current number of points defining the line.
     *
     * @return The number of points.
     */
    force_inline int getNumPoints() const {
        return num_points_;
    }

    /**
     * @brief Sets the position of a specific point.
     *
     * @param index The point index to modify.
     * @param point The new (x, y) coordinates for that point.
     */
    force_inline void setPoint(int index, std::pair<float, float> point) {
        VITAL_ASSERT(index < kMaxPoints && index >= 0);
        points_[index] = point;
        checkLineIsLinear();
    }

    /**
     * @brief Sets the power for a specific point.
     *
     * @param index The point index to modify.
     * @param power The new power value.
     */
    force_inline void setPower(int index, float power) {
        VITAL_ASSERT(index < kMaxPoints && index >= 0);
        powers_[index] = power;
        checkLineIsLinear();
    }

    /**
     * @brief Sets the number of points currently in use.
     *
     * @param num_points The new number of points.
     */
    force_inline void setNumPoints(int num_points) {
        VITAL_ASSERT(num_points <= kMaxPoints && num_points >= 0);
        num_points_ = num_points;
        checkLineIsLinear();
    }

    /**
     * @brief Gets the number of times the line has been rendered since initialization.
     *
     * @return The render count.
     */
    int getRenderCount() const { return render_count_; }

protected:
    std::string name_;                   ///< The name of the line shape.
    std::string last_browsed_file_;      ///< The last browsed file path for saving/loading this line.
    std::pair<float, float> points_[kMaxPoints];  ///< Array of points defining the line shape.
    float powers_[kMaxPoints];           ///< Powers controlling interpolation shape between points.
    int num_points_;                     ///< Current number of points.
    int resolution_;                     ///< Resolution of the internal buffer.

    std::unique_ptr<vital::mono_float[]> buffer_; ///< Internal buffer holding the rendered line values.
    bool loop_;                         ///< Whether the line loops at the end.
    bool smooth_;                       ///< Whether to apply smoothing between points.
    bool linear_;                       ///< Whether the line is the simple linear shape.
    int render_count_;                  ///< Count of how many times render() was called.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LineGenerator)
};
