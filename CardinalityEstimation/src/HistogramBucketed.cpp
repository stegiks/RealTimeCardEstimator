#include "HistogramBucketed.hpp"

HistogramBucketed::HistogramBucketed(int rangeStart, int rangeEnd, int numBuckets, int numSubBuckets, int cacheDepth, int mcvTrackSize)
    : rangeStart_(rangeStart), rangeEnd_(rangeEnd), numBuckets_(numBuckets), numSubBuckets_(numSubBuckets) {

    // Allocate MCV if we have read all the data
    if(mcvTrackSize > 0)
        mcvTrack = new MCVTrack(mcvTrackSize);

    bucketSize_ = ceil((float)(rangeEnd - rangeStart + 1) / numBuckets);
    subBucketSize_ = ceil((float)bucketSize_ / SUB_BUCKETS);
    buckets_.reserve(numBuckets);
    for (int i = 0; i < numBuckets; ++i) {
        buckets_.emplace_back(rangeStart + i * bucketSize_, rangeStart + (i + 1) * bucketSize_ - 1);
    }

    // Initialize cache
    cache = new BinaryCache(rangeEnd, cacheDepth);
}

HistogramBucketed::~HistogramBucketed(){
    buckets_.clear();
    delete cache;
    delete mcvTrack;
}

void HistogramBucketed::insert(int value) {
    int bucketIdx = getBucketIndex(value);
    buckets_[bucketIdx].insert(this, value);
    cache->insert(value);

    // Update MCV
    if(mcvTrack != nullptr) {
        mcvTrack->recordValue(value);
    }
}

void HistogramBucketed::preparationInsert(int value){
    int bucketIdx = getBucketIndex(value);
    buckets_[bucketIdx].insert(this, value);
    cache->lowerLevelInsert(value);

    // Update MCV
    if(mcvTrack != nullptr) {
        mcvTrack->recordValue(value);
    }
}

void HistogramBucketed::preparationEnd(){
    cache->recalculateLayers();
}

void HistogramBucketed::deleteValue(int value){
    int bucketIdx = getBucketIndex(value);
    buckets_[bucketIdx].del(this, value);
    cache->del(value);

    // Update MCV
    if(mcvTrack != nullptr){
        mcvTrack->decrementValue(value);
    }
}

uint32_t HistogramBucketed::estimateEqual(int value){

    if(mcvTrack != nullptr){
        uint32_t estimate = mcvTrack->getMCV(value);

        if(estimate > 0){
            return estimate;
        }
    }

    int bucketIdx = this->getBucketIndex(value);
    if(!(buckets_[bucketIdx].contains(value))) return 0;
    return buckets_[bucketIdx].getSubBucketCount(this, value) / buckets_[bucketIdx].estimateDistinctPerSubBucket(value);
}

uint32_t HistogramBucketed::estimateGreater(int value){
    int limit;
    uint32_t estimate = cache->query(value, &limit);

    int bucketIdx = getBucketIndex(value);
    int bucketIdxLimit = getBucketIndex(limit);

    estimate += buckets_[bucketIdx].estimateGreater(this, value);

    if(bucketIdx == bucketIdxLimit){
        return estimate;
    }

    for(int i = bucketIdx + 1; i < bucketIdxLimit; ++i){
        estimate += buckets_[i].getTotalCount();
    }

    return estimate;
}

inline int HistogramBucketed::getBucketIndex(int value){
    return (value - rangeStart_) / bucketSize_;
}

void HistogramBucketed::scale(float value){
    if(float(value) == 1.0){
        return;
    }

    for (auto& bucket : buckets_) {
        bucket.scale(value);
    }

    cache->scaleCache(value);
}


size_t HistogramBucketed::memUsage(std::vector<size_t>* details){
    size_t localVars = 0;
    localVars += sizeof(rangeStart_) + sizeof(rangeEnd_) + sizeof(numBuckets_);
    localVars += sizeof(buckets_);

    if(details != nullptr){
        details->push_back(localVars);
    }

    size_t bucketUsage = 0;
    for(Bucket& bucket : buckets_){
        bucketUsage += bucket.memUsage();
    }

    if(details != nullptr){
        details->push_back(bucketUsage);
    }

    size_t mcvUsage = 0;
    if(mcvTrack != nullptr)
        mcvUsage += mcvTrack->memUsage();

    if(details != nullptr){
        details->push_back(mcvUsage);
    }

    size_t cacheUsage = cache->memUsage();
    if (cache != nullptr)
        cacheUsage += cache->memUsage();

    if(details != nullptr){
        details->push_back(cacheUsage);
    }

    return localVars + bucketUsage + mcvUsage + cacheUsage;
}