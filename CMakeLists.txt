cmake_minimum_required(VERSION 3.14)
project(RealTimeAudioDSP CXX)

# Enforce Modern C++
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Workaround for Clang + libstdc++ 13 C++23 compatibility inside WSL
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-D__cpp_concepts=202002L -Wno-builtin-macro-redefined)
endif()

# ==========================================
# 1. Threading Subsystem
# ==========================================
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

# ==========================================
# 2. Digital Signal Processing Libraries
# ==========================================
# ==========================================
# 2. Digital Signal Processing Libraries (via vcpkg)
# ==========================================
find_package(portaudio CONFIG REQUIRED)

# ==========================================
# 3. Executable Configuration
# ==========================================
add_executable(RealTimeAudioDSP 
    src/main.cpp 
    src/AudioProcessor.cpp
    src/FFTEngine.cpp
)

target_include_directories(RealTimeAudioDSP PRIVATE include)

# Link the hardware threads and PortAudio library
target_link_libraries(RealTimeAudioDSP 
    PRIVATE 
        Threads::Threads 
        portaudio
)

add_executable(DeviceDiagnostic 
    src/sandbox_refactored.cpp
)

target_include_directories(DeviceDiagnostic PRIVATE include)

target_link_libraries(DeviceDiagnostic 
    PRIVATE 
        Threads::Threads 
        portaudio
)