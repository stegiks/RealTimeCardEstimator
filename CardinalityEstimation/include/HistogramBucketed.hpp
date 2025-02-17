#include <vector>
#include <cmath>
#include <iostream>
#include <unordered_map>
#include <cstring>

#include "Bucket.hpp"
#include "BinaryCache.hpp"
#include "MCVTrack.hpp"

// A class to accumulate statistics for a range of values.
class HistogramBucketed{
public:

    HistogramBucketed(int rangeStart, int rangeEnd, int numBuckets, int numSubBuckets, int cacheDepth, int mcvTrackSize);
    ~HistogramBucketed();

    /**
        * @brief Insert a value into the histogram.
        * 
        * @param value The value to be inserted.
    */
    void insert(int value);
    
    /**
     * @brief Used to prepare the histogram with data already inserted
     * 
     * @param value The value to be inserted.
     */
    void preparationInsert(int value);

    /**
        * @brief Delete a value from the histogram.
        * 
        * @param value The value to be deleted.
    */
    void deleteValue(int value);

    /**
        * @brief Query the histogram for the aproximate amount of values equal to the one given.
        *
        * @param value The value of the query.
        * 
        * @return An aproximation of the amount of values equal to the given value.
    */
    uint32_t estimateEqual(int value);

    /**
        * @brief Query the histogram for the aproximate amount of values greater than the one given.
        *
        * @param value The value of the query.
        * 
        * @return An aproximation of the amount of values greater than the given value.
    */
    uint32_t estimateGreater(int value);

    /**
        * @brief Scale the histogram by multiplying all values by a given factor. This can be used when the histogram data is a subset of the original data.
        *
        * @param value The scale factor.
    */
    void scale(float value);

    /**
        * @brief Get the memory usage of the histogram.
        * 
        * @param details A vector to store the memory usage of the histogram components.(Local variables, Buckets, MCV, Cache)
        * 
        * @return The memory usage of the histogram. 
    */
    size_t memUsage(std::vector<size_t>* details = nullptr);
    
    /**
     * @brief End the preparation phase of the histogram.
     */
    void preparationEnd();

protected:
    int rangeStart_, rangeEnd_, bucketSize_, subBucketSize_;

private:
    int numBuckets_, numSubBuckets_;
    std::vector<Bucket> buckets_;
    MCVTrack* mcvTrack = nullptr; // MCV for the histogram
    BinaryCache* cache; // Binary cache for the first column

    /**
     * @brief Get the index of the bucket that the value belongs to.
     * 
     * @param value The value to be inserted.
     * 
     * @return The index of the bucket that the value belongs to.
    */
    inline int getBucketIndex(int value);

    friend class Bucket;
};
