/**
 * @file memory.h
 * @brief Declares classes for time-domain memory storage and retrieval with cubic interpolation.
 *
 * The Memory and StereoMemory classes store a history of samples in a ring buffer and allow
 * retrieval of past samples using cubic interpolation. They are used to implement audio
 * feedback loops, delays, or other time-domain manipulations efficiently.
 */

#pragma once

#include "common.h"

#include <algorithm>
#include <cmath>
#include <memory>

#include "poly_utils.h"

namespace vital {

    /**
     * @class MemoryTemplate
     * @brief A template for a memory buffer that stores time-domain samples for one or more channels.
     *
     * MemoryTemplate provides a ring buffer of samples. Samples can be pushed into the buffer
     * and later retrieved based on a certain delay or offset. It supports clearing sections
     * of the buffer and ensures that size is always a power-of-two for indexing efficiency.
     *
     * @tparam kChannels The number of channels stored in the memory.
     */
    template<size_t kChannels>
    class MemoryTemplate {
    public:
        static constexpr mono_float kMinPeriod = 2.0f;          ///< Minimum allowed period of time delay.
        static constexpr int kExtraInterpolationValues = 3;     ///< Extra values to support cubic interpolation.

        /**
         * @brief Constructs the memory with a given size (rounded up to a power of two).
         * @param size The desired size of the memory buffer.
         */
        MemoryTemplate(int size) : offset_(0) {
            size_ = utils::nextPowerOfTwo(size);
            bitmask_ = size_ - 1;
            for (int i = 0; i < static_cast<int>(kChannels); ++i) {
                memories_[i] = std::make_unique<mono_float[]>(2 * size_);
                buffers_[i] = memories_[i].get();
            }
        }

        /**
         * @brief Copy constructor.
         * @param other Another MemoryTemplate to copy from.
         */
        MemoryTemplate(const MemoryTemplate& other) {
            for (int i = 0; i < static_cast<int>(kChannels); ++i) {
                memories_[i] = std::make_unique<mono_float[]>(2 * other.size_);
                buffers_[i] = memories_[i].get();
            }

            size_ = other.size_;
            bitmask_ = other.bitmask_;
            offset_ = other.offset_;
        }

        /**
         * @brief Destructor.
         */
        virtual ~MemoryTemplate() { }

        /**
         * @brief Pushes a poly_float of samples (one sample per channel) into the memory.
         * @param sample The sample to push for each channel (interleaved in poly_float).
         */
        void push(poly_float sample) {
            offset_ = (offset_ + 1) & bitmask_;
            for (int i = 0; i < static_cast<int>(kChannels); ++i) {
                mono_float val = sample[i];
                buffers_[i][offset_] = val;
                buffers_[i][offset_ + size_] = val;
            }

            VITAL_ASSERT(utils::isFinite(sample));
        }

        /**
         * @brief Clears a specified number of samples in the memory for channels indicated by a mask.
         * @param num The number of samples to clear.
         * @param clear_mask A poly_mask indicating which channels to clear.
         */
        void clearMemory(int num, poly_mask clear_mask) {
            int start = (offset_ - (num + kExtraInterpolationValues)) & bitmask_;
            int end = (offset_ + kExtraInterpolationValues) & bitmask_;

            for (int p = 0; p < static_cast<int>(kChannels); ++p) {
                if (clear_mask[p]) {
                    mono_float* buffer = buffers_[p];
                    for (int i = start; i != end; i = (i + 1) & bitmask_)
                        buffer[i] = 0.0f;
                    buffer[end] = 0.0f;

                    for (int i = 0; i < kExtraInterpolationValues; ++i)
                        buffer[size_ + i] = 0.0f;
                }
            }
        }

        /**
         * @brief Clears all samples in the memory for all channels.
         */
        void clearAll() {
            for (int c = 0; c < static_cast<int>(kChannels); ++c)
                memset(buffers_[c], 0, 2 * size_ * sizeof(mono_float));
        }

        /**
         * @brief Reads samples from the memory into an output buffer.
         * @param output Pointer to the output array to fill.
         * @param num_samples Number of samples to read.
         * @param offset The offset (delay) from the current write position.
         * @param channel The channel to read from.
         */
        void readSamples(mono_float* output, int num_samples, int offset, int channel) const {
            mono_float* buffer = buffers_[channel];
            int bitmask = bitmask_;
            int start_index = (offset_ - num_samples - offset) & bitmask;
            for (int i = 0; i < num_samples; ++i)
                output[i] = buffer[(i + start_index) & bitmask];
        }

        /**
         * @brief Gets the current offset (write position) in the buffer.
         * @return The current write offset.
         */
        unsigned int getOffset() const { return offset_; }

        /**
         * @brief Sets the current offset (write position) in the buffer.
         * @param offset The new offset.
         */
        void setOffset(int offset) { offset_ = offset; }

        /**
         * @brief Gets the size of the memory buffer.
         * @return The size of the memory buffer.
         */
        int getSize() const {
            return size_;
        }

        /**
         * @brief Gets the maximum allowed period for reading samples.
         * @return The maximum period that can be addressed.
         */
        int getMaxPeriod() const {
            return size_ - kExtraInterpolationValues;
        }

    protected:
        std::unique_ptr<mono_float[]> memories_[kChannels]; ///< Unique pointers to each channel's buffer.
        mono_float* buffers_[kChannels];                    ///< Raw pointers to each channel buffer.
        unsigned int size_;                                 ///< The size of the memory buffer.
        unsigned int bitmask_;                              ///< Bitmask for efficient modulo operations.
        unsigned int offset_;                               ///< Current write offset in the buffer.
    };

    /**
     * @class Memory
     * @brief A specialized MemoryTemplate for poly_float::kSize channels.
     *
     * Memory supports retrieval of past samples with cubic interpolation.
     * It assumes that the period requested is between kMinPeriod and getMaxPeriod().
     */
    class Memory : public MemoryTemplate<poly_float::kSize> {
    public:
        /**
         * @brief Constructs a polyphonic memory with the given size.
         * @param size The initial size of the memory buffer.
         */
        Memory(int size) : MemoryTemplate(size) { }

        /**
         * @brief Copy constructor.
         * @param other Another Memory to copy from.
         */
        Memory(Memory& other) : MemoryTemplate(other) { }

        /**
         * @brief Retrieves a poly_float of samples from the memory using cubic interpolation.
         * @param past A poly_float of "time ago" values. Each lane corresponds to how many samples
         *        back in time we want to read.
         * @return The interpolated sample values at the requested times.
         */
        force_inline poly_float get(poly_float past) const {
            VITAL_ASSERT(poly_float::lessThan(past, kMinPeriod).sum() == 0);
            VITAL_ASSERT(poly_float::greaterThan(past, getMaxPeriod()).sum() == 0);

            poly_int past_index = utils::toInt(past);
            poly_float t = utils::toFloat(past_index) - past + 1.0f;
            matrix interpolation_matrix = utils::getCatmullInterpolationMatrix(t);

            poly_int indices = (poly_int(offset_) - past_index - 2) & poly_int(bitmask_);
            matrix value_matrix = utils::getValueMatrix(buffers_, indices);
            value_matrix.transpose();
            return interpolation_matrix.multiplyAndSumRows(value_matrix);
        }
    };

    /**
     * @class StereoMemory
     * @brief A specialized MemoryTemplate for two-channel (stereo) audio.
     *
     * StereoMemory stores two channels of audio samples and can retrieve past samples
     * for both channels simultaneously using cubic interpolation.
     */
    class StereoMemory : public MemoryTemplate<2> {
    public:
        /**
         * @brief Constructs a stereo memory with the given size.
         * @param size The initial size of the memory buffer.
         */
        StereoMemory(int size) : MemoryTemplate(size) { }

        /**
         * @brief Copy constructor.
         * @param other Another StereoMemory to copy from.
         */
        StereoMemory(StereoMemory& other) : MemoryTemplate(other) { }

        /**
         * @brief Retrieves a poly_float of samples from the stereo memory using cubic interpolation.
         * @param past A poly_float of "time ago" values (for one channel pair).
         * @return The interpolated stereo sample values at the requested times.
         */
        force_inline poly_float get(poly_float past) const {
            VITAL_ASSERT(poly_float::lessThan(past, 2.0f).anyMask() == 0);
            VITAL_ASSERT(poly_float::greaterThan(past, getMaxPeriod()).anyMask() == 0);

            poly_int past_index = utils::toInt(past);
            poly_float t = utils::toFloat(past_index) - past + 1.0f;
            matrix interpolation_matrix = utils::getCatmullInterpolationMatrix(t);

            poly_int indices = (poly_int(offset_) - past_index - 2) & poly_int(bitmask_);
            // Construct a value matrix for stereo (2 channels), zero-filling other rows if needed.
            matrix value_matrix(utils::toPolyFloatFromUnaligned(buffers_[0] + indices[0]),
                                utils::toPolyFloatFromUnaligned(buffers_[1] + indices[1]), 0.0f, 0.0f);
            value_matrix.transpose();
            return interpolation_matrix.multiplyAndSumRows(value_matrix);
        }
    };

} // namespace vital
