#include "HLL.hpp"
#include "rapidhash.h"

#define REGISTER_INDEX(has) (has >> SHIFTS)

// Create an instance of a HyperLogLog estimator with the given precision.
HLL::HLL() {
}

HLL::~HLL(){
}

// Move constructor and move assignment operator
HLL::HLL(HLL&& other){
    memcpy(registers_, other.registers_, HLL_REGISTER_COUNT * sizeof(uint8_t));
}

HLL& HLL::operator=(HLL&& other){
    if(this != &other){
        memcpy(registers_, other.registers_, HLL_REGISTER_COUNT * sizeof(uint8_t));
    }
    return *this;
}

void HLL::insert(int value){    
    uint32_t hash = this->Hash(value);
    uint32_t index = REGISTER_INDEX(hash);
    uint8_t zero_count = __builtin_clz(hash << HLL_PRECISION);
    registers_[index] = registers_[index] > zero_count ? registers_[index] : zero_count;
}

uint32_t HLL::estimate(){
    double harmonic_mean = 0.0;

    // ! Maybe calculate it better for less time complexity
    for(uint8_t i = 0; i < HLL_REGISTER_COUNT; ++i){
        // harmonic_mean += std::pow(2.0, -static_cast<double>(registers_[i]));
        harmonic_mean += 1.0 / (1ULL << registers_[i]);
    }

    uint32_t E = static_cast<uint32_t>(HLL_ALPHA * HLL_REGISTER_COUNT * HLL_REGISTER_COUNT / harmonic_mean);

    return E;
}

uint32_t HLL::Hash(int value) {
    // Call rapidhash with the input value as the key, its size, and a seed (can be 0 or any constant).
    uint32_t result = rapidhash(&value, sizeof(value)); // Seed is set to 0 for simplicity.
    return result;
}

size_t HLL::memUsage(){
    size_t mem = sizeof(registers_);
    return mem;
}
