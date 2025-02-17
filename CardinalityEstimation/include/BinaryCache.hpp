#ifndef BINARYCACHE_HPP
#define BINARYCACHE_HPP

#include <cstdint>
#include <cstddef>

class BinaryCache {

public:
    /**
     * @brief Construct a new Binary Cache object
     * 
     * @param max_range The maximum range of values that can be inserted into the cache.
     * @param depth The depth of the binary cache. (The operations are logarithmic in nature)
    */
    BinaryCache(int max_range, int depth);
    ~BinaryCache();

    /**
        * @brief Insert a value into the cache.
        * @param value The value to be inserted.
    */
    void insert(int value);

    /**
        * @brief Delete a value from the cache.
        * @param value The value to be deleted.
    */
    void del(int value);
    
    /**
        * @brief Query the cache for the number of values greater than the given value.
        *
        * @param value The value to be queried.
        * @param range The lower bound of the range of values that were counted. Set to nullptr if not needed.
        * 
        * @return The number of values less than the given value.
    */
    uint32_t query(int value, int* range = nullptr);

    /**
        * @brief Update the cache by multiplying all values by a given factor.
        * @param value The factor to multiply by.
    */
    void scaleCache(float value);

    /**
     * @brief Get the memory usage of the cache.
     * 
     * @return The memory usage of the cache. 
    */
    size_t memUsage();

    /**
     * @brief Insert a value only in the lower level of the cache.
     * 
     * @param value The value to be inserted.
     */
    void lowerLevelInsert(int value);

    /**
     * @brief Recalculate the layers of the cache.
     */
    void recalculateLayers();

private:
    int max_range, depth, min_insert_val, start_offset;
    uint32_t* cache;
    uint32_t cache_size; 
    uint32_t* cache_steps;

    float bucket_size;
};

#endif