/**
 * @file circular_queue_test.h
 * @brief Declares the CircularQueueTest class for testing the CircularQueue data structure.
 */

#pragma once

#include "JuceHeader.h"

/**
 * @class CircularQueueTest
 * @brief A test class for verifying the correctness, stability, and reliability of the CircularQueue data structure.
 *
 * The CircularQueueTest performs various tests on the CircularQueue, such as adding/removing elements,
 * clearing, counting, resizing, iterating, popping elements from both ends, and sorting. These tests ensure that
 * the queue behaves as expected under normal and extreme conditions.
 */
class CircularQueueTest : public UnitTest {
public:
    /**
     * @brief Constructs a CircularQueueTest with a specified name and category.
     */
    CircularQueueTest() : UnitTest("Circular Queue", "Framework") { }

    /**
     * @brief Runs all circular queue tests, verifying that the queue handles all operations correctly.
     */
    void runTest() override;

    /**
     * @brief Tests adding and removing elements from the queue, including checking element existence.
     */
    void testAddingRemoving();

    /**
     * @brief Tests clearing the queue of all elements.
     */
    void testClearing();

    /**
     * @brief Tests popping elements from both the front and back of the queue.
     */
    void testPopping();

    /**
     * @brief Tests operations on a long queue to ensure stability and correctness over multiple cycles.
     */
    void testLongQueue();

    /**
     * @brief Tests resizing the queue and ensuring elements remain valid after resizing.
     */
    void testResizing();

    /**
     * @brief Tests iteration over the elements of the queue using range-based for loops.
     */
    void testIterator();

    /**
     * @brief Tests counting the occurrences of elements and verifying that count operations behave as expected.
     */
    void testCount();

    /**
     * @brief Tests sorting the queue's elements in ascending and descending order.
     */
    void testSorting();
};
