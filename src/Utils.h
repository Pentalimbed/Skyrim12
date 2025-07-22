#pragma once

// Helps locate hang in a function with many procedures
class CounterLogger {
    uint32_t counter = 0;

public:
    // TODO: make it debug
    inline void log() { Logger::info("CounterLogger: {}", counter++); }
};