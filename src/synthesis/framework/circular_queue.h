#pragma once

#include "common.h"
#include <cstddef>
#include <memory>

namespace vital {

    /**
     * @class CircularQueue
     * @brief A generic circular buffer (FIFO) data structure that allows adding and removing elements efficiently.
     *
     * CircularQueue is a template-based circular buffer that can store a sequence of elements.
     * It supports operations like push_back, pop_back, push_front, pop_front, as well as inserting,
     * removing, and accessing elements by index. By using a circular indexing strategy, it can reuse
     * a fixed-size buffer and avoid costly memory allocations once reserved capacity is sufficient.
     *
     * Template Parameters:
     * - T: The type of elements stored in the queue.
     *
     * Operations:
     * - push_back, pop_back: Add and remove elements from the end.
     * - push_front, pop_front: Add and remove elements from the front.
     * - remove, removeAt: Remove elements at given positions or by value.
     * - at, operator[]: Access elements by index.
     * - Contains iterator support for iteration through elements.
     *
     * This queue uses a one-element gap strategy to differentiate between full and empty states,
     * thus when constructing with capacity `n`, it effectively stores up to `n-1` elements.
     */
    template<class T>
    class CircularQueue {
    public:

        /**
         * @class iterator
         * @brief A forward and backward iterator for iterating over the elements in the CircularQueue.
         *
         * The iterator supports increment and decrement operations and wraps around the circular buffer.
         */
        class iterator {
        public:
            /**
             * @brief Constructs an iterator.
             *
             * @param pointer The current pointer to an element in the queue.
             * @param front The pointer to the start of the internal buffer.
             * @param end The pointer to the end of the internal buffer.
             */
            iterator(T* pointer, T* front, T* end) : pointer_(pointer), front_(front), end_(end) { }

            /**
             * @brief Moves the iterator to the next element.
             */
            force_inline void increment() {
                if (pointer_ == end_)
                    pointer_ = front_;
                else
                    pointer_++;
            }

            /**
             * @brief Moves the iterator to the previous element.
             */
            force_inline void decrement() {
                if (pointer_ == front_)
                    pointer_ = end_;
                else
                    pointer_--;
            }

            force_inline iterator operator++() {
                iterator iter = *this;
                increment();
                return iter;
            }

            force_inline const iterator operator++(int i) {
                iterator iter = *this;
                increment();
                return iter;
            }

            force_inline iterator operator--() {
                iterator iter = *this;
                decrement();
                return iter;
            }

            force_inline const iterator operator--(int i) {
                iterator iter = *this;
                decrement();
                return iter;
            }

            force_inline T& operator*() {
                return *pointer_;
            }

            force_inline T* operator->() {
                return pointer_;
            }

            force_inline T* get() {
                return pointer_;
            }

            force_inline bool operator==(const iterator& rhs) {
                return pointer_ == rhs.pointer_;
            }

            force_inline bool operator!=(const iterator& rhs) {
                return pointer_ != rhs.pointer_;
            }

        protected:
            T* pointer_;
            T* front_;
            T* end_;
        };

        /**
         * @brief Constructs a CircularQueue with a given capacity.
         *
         * Note that the effective capacity for storing elements is `capacity - 1`
         * because one slot is used to differentiate between empty and full states.
         *
         * @param capacity The desired capacity.
         */
        CircularQueue(int capacity) : capacity_(capacity + 1), start_(0), end_(0) {
            data_ = std::make_unique<T[]>(capacity_);
        }

        /**
         * @brief Copy constructor.
         *
         * @param other Another CircularQueue to copy from.
         */
        CircularQueue(const CircularQueue& other) {
            capacity_ = other.capacity_;
            data_ = std::make_unique<T[]>(capacity_);
            memcpy(data_.get(), other.data_.get(), capacity_ * sizeof(T));
            start_ = other.start_;
            end_ = other.end_;
        }

        /**
         * @brief Default constructor creates an empty queue with no capacity.
         */
        CircularQueue() : data_(nullptr), capacity_(0), start_(0), end_(0) { }

        /**
         * @brief Ensures that the queue has at least the given capacity.
         *
         * If the new capacity is larger, re-allocates and copies the existing elements.
         *
         * @param capacity The desired minimum capacity.
         */
        void reserve(int capacity) {
            int new_capacity = capacity + 1;
            if (new_capacity < capacity_)
                return;

            std::unique_ptr<T[]> tmp = std::make_unique<T[]>(new_capacity);

            if (capacity_) {
                end_ = size();
                for (int i = 0; i < end_; ++i)
                    tmp[i] = std::move(at(i));
            }

            data_ = std::move(tmp);
            capacity_ = new_capacity;
            start_ = 0;
        }

        /**
         * @brief Assigns `num` elements with a given value.
         *
         * Resizes if necessary and sets the queue to contain `num` copies of `value`.
         *
         * @param num The number of elements to assign.
         * @param value The value to fill.
         */
        force_inline void assign(int num, T value) {
            if (num > capacity_)
                reserve(num);

            for (int i = 0; i < num; ++i)
                data_[i] = value;

            start_ = 0;
            end_ = num;
        }

        /**
         * @brief Accesses an element by index in the queue.
         *
         * @param index The element index [0..size()-1].
         * @return A reference to the element.
         */
        force_inline T& at(std::size_t index) {
            VITAL_ASSERT(index >= 0 && index < size());
            return data_[(start_ + static_cast<int>(index)) % capacity_];
        }

        force_inline T& operator[](std::size_t index) {
            return at(index);
        }

        force_inline const T& operator[](std::size_t index) const {
            return at(index);
        }

        /**
         * @brief Pushes an element to the back of the queue.
         *
         * @param entry The element to push.
         */
        force_inline void push_back(T entry) {
            data_[end_] = std::move(entry);
            end_ = (end_ + 1) % capacity_;
            VITAL_ASSERT(end_ != start_);
        }

        /**
         * @brief Pops an element from the back of the queue.
         *
         * @return The popped element.
         */
        force_inline T pop_back() {
            VITAL_ASSERT(end_ != start_);
            end_ = (end_ - 1 + capacity_) % capacity_;
            return data_[end_];
        }

        /**
         * @brief Pushes an element to the front of the queue.
         *
         * @param entry The element to push.
         */
        force_inline void push_front(T entry) {
            start_ = (start_ - 1 + capacity_) % capacity_;
            data_[start_] = entry;
            VITAL_ASSERT(end_ != start_);
        }

        /**
         * @brief Pops an element from the front of the queue.
         *
         * @return The popped element.
         */
        force_inline T pop_front() {
            VITAL_ASSERT(end_ != start_);
            int last = start_;
            start_ = (start_ + 1) % capacity_;
            return data_[last];
        }

        /**
         * @brief Removes an element at a given index.
         *
         * Elements after the index are shifted left.
         *
         * @param index The index to remove at.
         */
        force_inline void removeAt(int index) {
            VITAL_ASSERT(end_ != start_);
            int i = (index + start_) % capacity_;
            end_ = (end_ - 1 + capacity_) % capacity_;
            while (i != end_) {
                int next = (i + 1) % capacity_;
                data_[i] = data_[next];
                i = next;
            }
        }

        /**
         * @brief Removes the first occurrence of an element if found.
         *
         * @param entry The element to remove.
         */
        force_inline void remove(T entry) {
            for (int i = start_; i != end_; i = (i + 1) % capacity_) {
                if (data_[i] == entry) {
                    removeAt((i - start_ + capacity_) % capacity_);
                    return;
                }
            }
        }

        /**
         * @brief Removes all occurrences of a given element.
         *
         * @param entry The element to remove.
         */
        void removeAll(T entry) {
            for (int i = start_; i != end_; i = (i + 1) % capacity_) {
                if (data_[i] == entry) {
                    removeAt((i - start_ + capacity_) % capacity_);
                    i--;
                }
            }
        }

        /**
         * @brief Sorts the elements according to a compare function.
         *
         * @tparam compare A function that returns <0 if a<b, 0 if a==b, >0 if a>b.
         */
        template<int(*compare)(T, T)>
        void sort() {
            int total = size();
            for (int i = 1; i < total; ++i) {
                T value = at(i);
                int j = i - 1;
                for (; j >= 0 && compare(at(j), value) < 0; --j)
                    at(j + 1) = at(j);

                at(j + 1) = value;
            }
        }

        /**
         * @brief Ensures that there is at least `space` extra capacity beyond the current size.
         *
         * @param space Extra space needed.
         */
        void ensureSpace(int space = 2) {
            if (size() + space >= capacity())
                reserve(capacity_ + std::max(capacity_, space));
        }

        /**
         * @brief Ensures the queue has at least `min_capacity` total capacity.
         *
         * @param min_capacity The required minimum capacity.
         */
        void ensureCapacity(int min_capacity) {
            if (min_capacity >= capacity())
                reserve(capacity_ + std::max(capacity_, min_capacity));
        }

        /**
         * @brief Erases the element at the iterator position and returns an iterator to the next element.
         *
         * @param iter The iterator pointing to the element to erase.
         * @return The iterator to the next element.
         */
        force_inline iterator erase(iterator& iter) {
            int index = static_cast<int>(iter.get() - data_.get());
            removeAt((index - start_ + capacity_) % capacity_);
            return iter;
        }

        /**
         * @brief Counts how many times `entry` appears in the queue.
         *
         * @param entry The element to count.
         * @return The number of occurrences.
         */
        int count(T entry) const {
            int number = 0;
            for (int i = start_; i != end_; i = (i + 1) % capacity_) {
                if (data_[i] == entry)
                    number++;
            }
            return number;
        }

        /**
         * @brief Checks if the queue contains a given element.
         *
         * @param entry The element to check for.
         * @return True if found, false otherwise.
         */
        bool contains(T entry) const {
            for (int i = start_; i != end_; i = (i + 1) % capacity_) {
                if (data_[i] == entry)
                    return true;
            }
            return false;
        }

        /**
         * @brief Clears all elements in the queue.
         */
        force_inline void clear() {
            start_ = 0;
            end_ = 0;
        }

        /**
         * @brief Returns the element at the front of the queue.
         */
        force_inline T front() const {
            return data_[start_];
        }

        /**
         * @brief Returns the element at the back of the queue.
         */
        force_inline T back() const {
            return data_[(end_ - 1 + capacity_) % capacity_];
        }

        /**
         * @brief Returns the current number of elements in the queue.
         */
        force_inline int size() const {
            VITAL_ASSERT(capacity_ > 0);
            return (end_ - start_ + capacity_) % capacity_;
        }

        /**
         * @brief Returns the maximum number of elements the queue can hold.
         */
        force_inline int capacity() const {
            return capacity_ - 1;
        }

        /**
         * @brief Returns an iterator to the first element of the queue.
         */
        force_inline iterator begin() const {
            return iterator(data_.get() + start_, data_.get(), data_.get() + (capacity_ - 1));
        }

        /**
         * @brief Returns an iterator to the past-the-end element of the queue.
         */
        force_inline iterator end() const {
            return iterator(data_.get() + end_, data_.get(), data_.get() + (capacity_ - 1));
        }

    private:
        std::unique_ptr<T[]> data_; ///< The underlying buffer for the circular queue.
        int capacity_;              ///< The allocated capacity (including one unused slot).
        int start_;                 ///< The index of the start of the queue.
        int end_;                   ///< The index of the end of the queue.
    };
} // namespace vital
