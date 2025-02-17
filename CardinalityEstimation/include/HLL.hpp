#ifndef HLL_H
#define HLL_H

#include <cstdint>
#include <cassert>
#include <cmath>
#include <cstring>
#include <iostream>
#include <vector>

#ifndef HLL_PRECISION
#define HLL_PRECISION 4
#endif

#define SHIFTS 32 - HLL_PRECISION
#define HLL_REGISTER_COUNT (1 << HLL_PRECISION)

#if HLL_PRECISION == 4
    #define HLL_ALPHA 0.673
#elif HLL_PRECISION == 5
    #define HLL_ALPHA 0.697
#elif HLL_PRECISION == 6
    #define HLL_ALPHA 0.709
#else
    #define HLL_ALPHA 0.7213 / (1 + 1.079 / HLL_REGISTER_COUNT)
#endif

class HLL{
public:

    /**
        * @brief Construct a new HLL object
    */
    HLL();
    ~HLL();

    // Move constructor and move assignment operator
    HLL(HLL&& other);
    HLL& operator=(HLL&& other);

    // Delete copy and move constructors and copy and move assignment operator
    HLL(const HLL&) = delete;
    HLL& operator=(const HLL&) = delete;

    /**
        * @brief Insert a value into the HyperLogLog algorithm.
        * 
        * @param value The value to be inserted.
    */
    void insert(int value);

    /** 
        * @brief Get an estimation of the unique values in the HLL object.
        * 
        * @return An estimation of the unique values in the HLL object.
    */
    uint32_t estimate();

    /**
        * @brief Hash a value using the rapidhash algorithm.
        * 
        * @return The hash of the value.
    */
    static uint32_t Hash(int value);                // Static method can be called without an instance of the class
   
    /**
        * @brief Calculate the memory usage of the HLL estimator.
        * 
        * @return The memory usage of the HLL estimator.
    */
    size_t memUsage();  

private:
    uint8_t registers_[HLL_REGISTER_COUNT] = {0}; // The registers of the HLL estimator.
};

#endif // HLL_H