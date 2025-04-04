#pragma once

#include "common.h"

namespace vital {

    /**
     * @struct matrix
     * @brief A structure representing a 4x1 matrix of poly_float rows.
     *
     * This structure holds four poly_float rows and provides utility functions for operations such as
     * transposing and interpolating between matrix states. It is designed to work with poly_float, a
     * SIMD-friendly vector type used throughout Vital's DSP code.
     *
     * Each @c rowN is a poly_float vector representing one row of the matrix. Although named "matrix,"
     * it essentially acts as a 4-row structure that can be manipulated for certain vectorized math operations,
     * including transposing these rows into columns and performing interpolations.
     */
    struct matrix {
        poly_float row0, row1, row2, row3; ///< The four rows of the matrix as poly_float values.

        /**
         * @brief Default constructor leaves rows uninitialized.
         */
        matrix() { }

        /**
         * @brief Constructs a matrix with specified rows.
         *
         * @param r0 The first row.
         * @param r1 The second row.
         * @param r2 The third row.
         * @param r3 The fourth row.
         */
        matrix(poly_float r0, poly_float r1, poly_float r2, poly_float r3) :
                row0(r0), row1(r1), row2(r2), row3(r3) { }

        /**
         * @brief Transposes the matrix in-place.
         *
         * This uses poly_float's static transpose function, effectively transposing 4x1 vectors
         * treating them as a 4x4 set of values for vector operations.
         */
        force_inline void transpose() {
            poly_float::transpose(row0.value, row1.value, row2.value, row3.value);
        }

        /**
         * @brief Linearly interpolates each column of the matrix towards another matrix.
         *
         * The same interpolation factor @p t is applied to each column.
         *
         * @param other The target matrix to interpolate towards.
         * @param t The interpolation factor (0.0 = original, 1.0 = other).
         */
        force_inline void interpolateColumns(const matrix& other, poly_float t) {
            row0 = poly_float::mulAdd(row0, other.row0 - row0, t);
            row1 = poly_float::mulAdd(row1, other.row1 - row1, t);
            row2 = poly_float::mulAdd(row2, other.row2 - row2, t);
            row3 = poly_float::mulAdd(row3, other.row3 - row3, t);
        }

        /**
         * @brief Performs row-wise interpolation, using a separate interpolation factor from each row of @p t.
         *
         * Each element of @p t corresponds to one row.
         *
         * @param other The target matrix to interpolate towards.
         * @param t A poly_float containing individual interpolation factors per row.
         */
        force_inline void interpolateRows(const matrix& other, poly_float t) {
            row0 = poly_float::mulAdd(row0, other.row0 - row0, t[0]);
            row1 = poly_float::mulAdd(row1, other.row1 - row1, t[1]);
            row2 = poly_float::mulAdd(row2, other.row2 - row2, t[2]);
            row3 = poly_float::mulAdd(row3, other.row3 - row3, t[3]);
        }

        /**
         * @brief Sums all the rows together.
         *
         * @return A poly_float representing the element-wise sum of row0, row1, row2, and row3.
         */
        force_inline poly_float sumRows() {
            return row0 + row1 + row2 + row3;
        }

        /**
         * @brief Multiplies and sums corresponding rows of this matrix with another matrix.
         *
         * @param other The other matrix to multiply and sum rows with.
         * @return A poly_float representing the sum of the element-wise products of corresponding rows.
         */
        force_inline poly_float multiplyAndSumRows(const matrix& other) {
            poly_float row01 = poly_float::mulAdd(row0 * other.row0, row1, other.row1);
            poly_float row012 = poly_float::mulAdd(row01, row2, other.row2);
            return poly_float::mulAdd(row012, row3, other.row3);
        }
    };
} // namespace vital
