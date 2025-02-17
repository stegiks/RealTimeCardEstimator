#ifndef BUCKET_HPP
#define BUCKET_HPP

#include <cstdint>
#include "HLL.hpp"
#include "config.hpp"

class HistogramBucketed;

class Bucket {
public:
    /**
     * @brief Construct a new Bucket object
     * 
     * @param start The start of the range of values that can be inserted into the bucket.
     * @param end   The end of the range of values that can be inserted into the bucket.
     */
    Bucket(int start, int end);
    ~Bucket();

    // Move constructor and move assignment operator
    Bucket(Bucket&& other);
    Bucket& operator=(Bucket&& other);

    // Delete copy and move constructors and copy and move assignment operator
    Bucket(const Bucket&) = delete;
    Bucket& operator=(const Bucket&) = delete;

    /**
        * @brief Insert a value into the bucket.
        * 
        * @param caller The histogram that is calling the function.
        * @param value The value to be inserted.
    */
    void insert(HistogramBucketed* caller, int value);

    /**
        * @brief Delete a value from the bucket.
        * 
        * @param caller The histogram that is calling the function.
        * @param value The value to be deleted.
    */
    void del(HistogramBucketed* caller, int value);

    /**
        * @brief Scale the bucket by multiplying it's values by a given factor. This can be used when the buckets data is a subset of the original data.
        *
        * @param value The scale factor.
    */
    void scale(float value);

    /**
        * @brief Query the bucket for the number of distinct values. This will use the HyperLogLog algorithm to estimate the number.
        * 
        * @return An aproximation of the distinct values in the bucket.
    */
    uint32_t estimateDistinct();

    /**
        * @brief Query the bucket for the number of distinct values per sub-bucket. This will use the HyperLogLog algorithm to estimate the number.
        * 
        * @param value The value to be used to calculate the sub-bucket.
        * 
        * @return An aproximation of the distinct values in the sub-bucket.
    */
    uint32_t estimateDistinctPerSubBucket(int value);

    /** 
     * @brief Query the bucket for the number of values that are greater than a given value.
     * 
     * @param caller The histogram that is calling the function.
     * @param value The value to compare against.
     * 
     * @return The number of values that are greater than the given value.
    */
    uint32_t estimateGreater(HistogramBucketed* caller, int value);

    /**
        * @brief Get the amount of values that have been inserted in the value's sub-bucket.
        * 
        * @param caller The histogram that is calling the function.
        * 
        * @return The amount of values that have been inserted in the value's sub-bucket.
    */
    uint32_t getSubBucketCount(HistogramBucketed* caller, int value);

    /**
        * @brief Get the total amount of values that have been inserted in the bucket.
        * 
        * @return The total amount of values that have been inserted in the bucket.
    */
    uint32_t getTotalCount() const;

    /**
        * @brief Get the percentage of values that the bucket collects that are above a given value.
        * 
        * @param value The value to compare against.
        * 
        * @return The percentage of values that the bucket collects that are above the given value.
    */
    float getPercentageAbove(int value);

    /**
        * @brief Check if the bucket contains a value based on the minimum and maximum values.
        * 
        * @return Boolean value indicating if the bucket contains the value.
     */
    bool contains(int value);

    /**
        * @brief Get the memory usage of the bucket.
        * 
        * @return The memory usage of the bucket.
    */
    size_t memUsage();

private:
    int min, max;
    HLL hll;
    uint32_t totalCount = 0;
    uint32_t subCounts[SUB_BUCKETS] = {0};

    inline int getSubBucketIndex(int value, HistogramBucketed* caller);
};

#endif