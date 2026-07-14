#include <vector>
#include <mutex>
#include <condition_variable>

class ThreadSafeAudioQueue {
private:
    // The Data-Oriented storage
    std::vector<float> leftChannel;
    std::vector<float> rightChannel;
    
    // Synchronization primitives
    std::mutex queueMutex;
    std::condition_variable dataReadyCondition;
    bool hasNewData = false;

public:
    ThreadSafeAudioQueue(size_t bufferSize) {
        leftChannel.resize(bufferSize, 0.0f);
        rightChannel.resize(bufferSize, 0.0f);
    }

    // Called by the Producer Thread (PortAudio callback)
    void pushAudioChunk(const float* incomingLeft, const float* incomingRight, size_t size) {
        {
            // 1. Lock the mutex safely using RAII
            std::lock_guard<std::mutex> lock(queueMutex);
            
            // 2. Fast memory copy into our DOD arrays
            std::copy(incomingLeft, incomingLeft + size, leftChannel.begin());
            std::copy(incomingRight, incomingRight + size, rightChannel.begin());
            
            hasNewData = true;
        } // Mutex automatically unlocks here
        
        // 3. Wake up the sleeping DSP thread
        dataReadyCondition.notify_one();
    }

    // Called by the Consumer Thread (The FFT Engine)
    void waitForAndProcessAudio() {
        // 1. Unique lock required for condition variables
        std::unique_lock<std::mutex> lock(queueMutex);
        
        // 2. Go to sleep (0% CPU) UNTIL hasNewData becomes true
        dataReadyCondition.wait(lock, [this]() { return hasNewData; });
        
        // 3. We are awake and locked. Process the contiguous arrays!
        // ExecuteFastFourierTransform(leftChannel);
        
        // 4. Reset the flag
        hasNewData = false;
    }
};