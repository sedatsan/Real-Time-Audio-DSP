#ifndef SPSC_RING_BUFFER_H
#define SPSC_RING_BUFFER_H

#include <array>
#include <atomic>
#include <bit>
#include <cstddef>
#include <optional>

/**
 * @brief A lock-free Single-Producer Single-Consumer (SPSC) Ring Buffer.
 * 
 * Performance & High-Performance Computing (HPC) Design:
 * 1. Lock-Free Operation: Does not use mutexes, condition variables, or OS-level locks.
 *    Relies entirely on atomic operations with acquire-release memory semantics.
 * 2. False Sharing Prevention: The write and read indices are cache-line aligned (alignas(64))
 *    so they reside on separate cache lines. This prevents cache invalidation traffic (bouncing)
 *    between the producer and consumer threads.
 * 3. Power-of-Two Size: Enforces that the BufferSize is a power of two at compile time.
 *    This allows modulo operations to compile to a fast bitwise AND mask instead of a slow division.
 * 
 * Safety Constraint:
 * Only ONE thread is allowed to call push() (the Producer).
 * Only ONE thread is allowed to call pop() (the Consumer).
 */
template <typename T, size_t BufferSize>
class SPSCRingBuffer {
    static_assert(std::has_single_bit(BufferSize), "BufferSize must be a power of 2!");

public:
    SPSCRingBuffer() = default;
    ~SPSCRingBuffer() = default;

    // Non-copyable and non-movable for safety in concurrent environments
    SPSCRingBuffer(const SPSCRingBuffer&) = delete;
    SPSCRingBuffer& operator=(const SPSCRingBuffer&) = delete;
    SPSCRingBuffer(SPSCRingBuffer&&) = delete;
    SPSCRingBuffer& operator=(SPSCRingBuffer&&) = delete;

    /**
     * @brief Pushes an item into the queue. Called ONLY by the Producer thread.
     * 
     * @param item The value to push.
     * @return true If the item was successfully pushed.
     * @return false If the queue is full.
     */
    bool push(const T& item) {
        // Load our own index with relaxed ordering (no other thread writes to it)
        const size_t currentWrite = m_writeIdx.load(std::memory_order_relaxed);
        // Load the consumer's index with acquire ordering to synchronize reads
        const size_t currentRead = m_readIdx.load(std::memory_order_acquire);

        const size_t nextWrite = (currentWrite + 1) & Mask;

        // If the queue is full, we cannot overwrite data
        if (nextWrite == currentRead) {
            return false;
        }

        m_storage[currentWrite] = item;

        // Store the new write index with release ordering. This acts as a memory barrier
        // ensuring the data write to m_storage is visible before the new index is seen.
        m_writeIdx.store(nextWrite, std::memory_order_release);
        return true;
    }

    /**
     * @brief Pops an item from the queue. Called ONLY by the Consumer thread.
     * 
     * @return std::optional<T> Containing the popped item, or std::nullopt if empty.
     */
    std::optional<T> pop() {
        // Load our own index with relaxed ordering (no other thread writes to it)
        const size_t currentRead = m_readIdx.load(std::memory_order_relaxed);
        // Load the producer's index with acquire ordering to synchronize writes
        const size_t currentWrite = m_writeIdx.load(std::memory_order_acquire);

        // If the indices are equal, the queue is empty
        if (currentRead == currentWrite) {
            return std::nullopt;
        }

        T item = m_storage[currentRead];
        const size_t nextRead = (currentRead + 1) & Mask;

        // Store the new read index with release ordering to signal to the producer
        // that a slot has been freed.
        m_readIdx.store(nextRead, std::memory_order_release);

        return item;
    }

    /**
     * @brief Checks if the queue is empty.
     */
    bool empty() const {
        return m_readIdx.load(std::memory_order_relaxed) == m_writeIdx.load(std::memory_order_relaxed);
    }

    /**
     * @brief Returns the approximate number of items currently in the queue.
     */
    size_t size() const {
        const size_t write = m_writeIdx.load(std::memory_order_relaxed);
        const size_t read = m_readIdx.load(std::memory_order_relaxed);
        if (write >= read) {
            return write - read;
        }
        return BufferSize - (read - write);
    }

private:
    std::array<T, BufferSize> m_storage;
    static constexpr size_t Mask = BufferSize - 1;

    // Align to 64 bytes (standard L1 cache line size) to prevent false sharing
    alignas(64) std::atomic<size_t> m_writeIdx{0};
    alignas(64) std::atomic<size_t> m_readIdx{0};
};

#endif // SPSC_RING_BUFFER_H
