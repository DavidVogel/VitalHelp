/**
 * @file circular_queue_test.cpp
 * @brief Implements the CircularQueueTest class, performing a series of tests on the CircularQueue data structure.
 */

#include "circular_queue_test.h"
#include "circular_queue.h"

namespace {
    /// The number of elements added in certain tests.
    constexpr int kAddNumber = 100;
    /// The number of repeated loop cycles in certain tests.
    constexpr int kLoopNumber = 10;

    /**
     * @brief Comparison function for ascending order sorting.
     * @param left The left integer.
     * @param right The right integer.
     * @return A positive value if left > right, 0 if equal, negative if left < right.
     */
    int compareAscend(int left, int right) {
        return right - left;
    }

    /**
     * @brief Comparison function for descending order sorting.
     * @param left The left integer.
     * @param right The right integer.
     * @return A positive value if left < right, 0 if equal, negative if left > right.
     */
    int compareDescend(int left, int right) {
        return left - right;
    }
} // namespace

void CircularQueueTest::runTest() {
    testAddingRemoving();
    testLongQueue();
    testCount();
    testPopping();
    testResizing();
    testIterator();
    testClearing();
    testSorting();
}

void CircularQueueTest::testAddingRemoving() {
    vital::CircularQueue<int> queue;
    queue.reserve(kAddNumber);

    beginTest("Adding and Removing");
    expect(queue.capacity() == kAddNumber, "Capacity should match reserved amount.");

    for (int j = 0; j < kLoopNumber; ++j) {
        expect(queue.size() == 0, "Queue should start empty.");

        // Add elements and check conditions.
        for (int i = 0; i < kAddNumber; ++i) {
            queue.push_back(i);
            expect(queue.size() == i + 1, "Size should increment after each push.");
            expect(queue[i] == i, "Element should match pushed value.");
            expect(queue.count(i) == 1, "Element count should be 1 after adding it.");
        }

        for (int i = 0; i < kAddNumber; ++i)
            expect(queue.contains(i), "Queue should contain all inserted elements.");

        expect(!queue.contains(-1), "Queue should not contain element -1.");
        expect(!queue.contains(kAddNumber), "Queue should not contain element beyond range.");

        int remove = kAddNumber / 2;
        queue.remove(remove);
        expect(queue.size() == kAddNumber - 1, "Size should decrement after removal.");

        for (int i = 0; i < kAddNumber; ++i)
            expect(queue.contains(i) == (i != remove), "Removed element should no longer be contained.");

        // Remove all elements and check size.
        for (int i = 0; i < kAddNumber; ++i) {
            queue.remove(i);
            expect(!queue.contains(i), "Element should be removed from the queue.");

            if (i < remove)
                expect(queue.size() == kAddNumber - i - 2, "Size should match expected after removals.");
            else
                expect(queue.size() == kAddNumber - i - 1, "Size should match expected after removals.");
        }

        for (int i = 0; i < kAddNumber; ++i)
            expect(!queue.contains(i), "Queue should be empty after all removals.");
    }
    expect(queue.size() == 0, "Queue should end up empty.");
    expect(queue.capacity() == kAddNumber, "Capacity should remain unchanged.");
}

void CircularQueueTest::testClearing() {
    vital::CircularQueue<float> queue;
    queue.reserve(kAddNumber);

    beginTest("Clearing");
    expect(queue.capacity() == kAddNumber, "Capacity should match reserved amount.");

    for (int j = 0; j < kLoopNumber; ++j) {
        expect(queue.size() == 0, "Queue should start empty.");

        for (int i = 0; i < kAddNumber; ++i) {
            queue.push_back(i);
            expect(queue.size() == i + 1, "Size should increment.");
            expect(queue[i] == i, "Value should match.");
            expect(queue.count(i) == 1, "Count of added element should be 1.");
        }

        for (int i = 0; i < kAddNumber; ++i)
            expect(queue.contains(i), "Queue should contain all elements.");

        expect(!queue.contains(-1), "Should not contain out-of-range element.");
        expect(!queue.contains(kAddNumber), "Should not contain out-of-range element.");

        int remove = kAddNumber / 2;
        queue.remove(remove);
        expect(queue.size() == kAddNumber - 1, "Size should decrement after removal.");

        // Clear the queue and verify emptiness.
        queue.clear();

        for (int i = 0; i < kAddNumber; ++i)
            expect(!queue.contains(i), "Queue should be empty after clearing.");
    }
    expect(queue.size() == 0, "Queue should end up empty.");
    expect(queue.capacity() == kAddNumber, "Capacity remains unchanged after clear.");
}

void CircularQueueTest::testLongQueue() {
    vital::CircularQueue<float> queue;
    queue.reserve(kAddNumber);

    beginTest("Long Queue");

    // Initialize queue with descending numbers.
    for (int i = 0; i < kAddNumber; ++i) {
        int number = kAddNumber - i - 1;
        queue.push_back(number);
        expect(queue.size() == i + 1, "Size should match number of pushes.");
        expect(queue[i] == number, "Stored value should match pushed value.");
        expect(queue.count(number) == 1, "Count should be 1 for newly added element.");
    }

    int remove_number = kAddNumber / 2;

    // Run multiple cycles of removal and addition.
    for (int j = 0; j < kLoopNumber; ++j) {
        expect(queue.size() == kAddNumber, "Queue should be full.");

        for (int i = 0; i < remove_number; ++i) {
            int number = i + j * remove_number;
            expect(queue.count(number) == 1, "Number should be present before removal.");
            queue.remove(number);
            expect(queue.size() == kAddNumber - i - 1, "Size should decrement after removal.");
            expect(queue.count(number) == 0, "Number should be removed.");
        }

        expect(queue.size() == kAddNumber - remove_number, "Size matches after removals.");

        // Add a new set of numbers.
        for (int i = 0; i < remove_number; ++i) {
            int number = i + j * remove_number + kAddNumber;
            if (i % 2)
                queue.push_back(number);
            else
                queue.push_front(number);

            expect(queue.size() == kAddNumber - remove_number + i + 1, "Size should increment after additions.");
            expect(queue.count(number) == 1, "Newly added number count should be 1.");
        }

        for (int i = 0; i < kAddNumber; ++i) {
            int number = i + (j + 1) * remove_number;
            expect(queue.contains(number), "All previously added numbers should be present.");
        }
    }
    expect(queue.size() == kAddNumber, "Queue should be full again.");
    expect(queue.capacity() == kAddNumber, "Capacity should remain constant.");
}

void CircularQueueTest::testCount() {
    vital::CircularQueue<float> queue;
    queue.reserve(kAddNumber * kLoopNumber);

    beginTest("Count");

    // Add elements in various ways to create duplicates.
    for (int j = 0; j < kLoopNumber; ++j) {
        for (int i = 0; i < kAddNumber; ++i) {
            if ((i + j) % 2)
                queue.push_back(i + j);
            else
                queue.push_front(i + j);
        }
    }

    // Check counts for each element.
    for (int i = 0; i < kLoopNumber + kAddNumber; ++i) {
        int count = std::min(std::min(kLoopNumber, i + 1), kLoopNumber + kAddNumber - i - 1);
        expect(queue.count(i) == count, "Count should match expected number of occurrences.");
    }

    queue.clear();

    // After clearing, no elements should be present.
    for (int i = 0; i < kLoopNumber + kAddNumber; ++i) {
        expect(queue.count(i) == 0, "Count should be zero after clearing.");
        expect(!queue.contains(i), "Should not contain any elements after clearing.");
    }
}

void CircularQueueTest::testResizing() {
    vital::CircularQueue<float> queue_ensure;
    vital::CircularQueue<float> queue_reserve;
    queue_ensure.reserve(kAddNumber);
    queue_reserve.reserve(kAddNumber);

    beginTest("Resizing");

    for (int j = 0; j < kLoopNumber; ++j) {
        for (int i = 0; i < kAddNumber; ++i) {
            int number = j * kAddNumber + i;
            queue_ensure.push_back(number);
            expect(queue_ensure.size() == number + 1, "Size should match the number of pushed elements.");
            expect(queue_ensure[number] == number, "Value should match the pushed number.");
            expect(queue_ensure.count(number) == 1, "Count should be 1 after adding a new element.");

            queue_reserve.push_back(number);
            expect(queue_reserve.size() == number + 1, "Size should match the number of pushed elements.");
            expect(queue_reserve[number] == number, "Value should match the pushed number.");
            expect(queue_reserve.count(number) == 1, "Count should be 1 after adding a new element.");
        }

        // Increase capacity and ensure no data loss.
        queue_reserve.reserve((j + 2) * kAddNumber);
        queue_ensure.ensureSpace(kAddNumber);

        for (int i = 0; i < (j + 1) * kAddNumber; ++i) {
            expect(queue_reserve[i] == i, "Elements should remain consistent after reserving more space.");
            expect(queue_reserve.count(i) == 1, "Count should remain consistent.");
            expect(queue_ensure[i] == i, "Elements should remain consistent after ensureSpace.");
            expect(queue_ensure.count(i) == 1, "Count should remain consistent.");
        }
    }
}

void CircularQueueTest::testIterator() {
    vital::CircularQueue<float> queue;
    queue.reserve(kAddNumber);
    beginTest("Iterator");

    for (int i = 0; i < kAddNumber; ++i) {
        queue.push_back(i);
        expect(queue.size() == i + 1, "Size should match the number of elements.");
        expect(queue[i] == i, "Value should match the pushed element.");
        expect(queue.count(i) == 1, "Count should be 1 after insertion.");
    }

    int i = 0;
    for (auto iter : queue) {
        expect(iter == i, "Iterated value should match stored value.");
        i++;
    }
}

void CircularQueueTest::testPopping() {
    vital::CircularQueue<float> queue;
    queue.reserve(kAddNumber * kLoopNumber);
    beginTest("Popping");

    // Add elements from both ends.
    for (int j = 0; j < kLoopNumber; ++j) {
        for (int i = 0; i < kAddNumber; ++i) {
            if ((i + j) % 2) {
                queue.push_back(i + j);
                expect(queue[queue.size() - 1] == i + j, "Pushed-back element should be at the end.");
            }
            else {
                queue.push_front(i + j);
                expect(queue[0] == i + j, "Pushed-front element should be at the beginning.");
            }
        }
    }

    int i = 0;
    // Pop elements from both front and back and ensure consistency.
    while (queue.size()) {
        if (i % 3 == 0) {
            int front = queue[0];
            int count = queue.count(front);
            expect(count > 0, "Front element should exist before popping.");
            queue.pop_front();
            expect(count - 1 == queue.count(front), "Count should decrement after popping front.");
            expect(count != 1 || !queue.contains(front), "Front element should be removed if count was 1.");
        }
        else {
            int back = queue[queue.size() - 1];
            int count = queue.count(back);
            expect(count > 0, "Back element should exist before popping.");
            queue.pop_back();
            expect(count - 1 == queue.count(back), "Count should decrement after popping back.");
            expect(count != 1 || !queue.contains(back), "Back element should be removed if count was 1.");
        }
        i++;
    }

    expect(queue.size() == 0, "Queue should be empty after popping all elements.");
    for (int i = 0; i < kLoopNumber + kAddNumber; ++i) {
        expect(queue.count(i) == 0, "No elements should remain.");
        expect(!queue.contains(i), "No elements should remain.");
    }
}

void CircularQueueTest::testSorting() {
    vital::CircularQueue<int> queue;
    queue.reserve(kAddNumber);
    beginTest("Sorting");

    queue.push_back(5);
    queue.push_back(-2);
    queue.push_back(2);
    queue.push_back(9);
    queue.push_back(1);
    queue.push_back(0);
    queue.sort<compareAscend>();
    expect(queue[0] == -2, "Queue should be sorted ascending.");
    expect(queue[1] == 0);
    expect(queue[2] == 1);
    expect(queue[3] == 2);
    expect(queue[4] == 5);
    expect(queue[5] == 9);
    queue.sort<compareDescend>();
    expect(queue[0] == 9, "Queue should be sorted descending.");
    expect(queue[1] == 5);
    expect(queue[2] == 2);
    expect(queue[3] == 1);
    expect(queue[4] == 0);
    expect(queue[5] == -2);

    queue.clear();

    for (int i = 0; i < kAddNumber; ++i)
        queue.push_back((i + kAddNumber / 2) % kAddNumber);

    queue.sort<compareAscend>();
    for (int i = 0; i < kAddNumber; ++i)
        expect(queue[i] == i, "Elements should be in ascending order.");

    queue.sort<compareDescend>();
    for (int i = 0; i < kAddNumber; ++i)
        expect(queue[i] == kAddNumber - i - 1, "Elements should be in descending order after sorting.");

}

// Registers the test instance so it will be discovered and run automatically.
static CircularQueueTest circular_queue_test;
