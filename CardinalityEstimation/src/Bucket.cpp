#include "Bucket.hpp"
#include "HistogramBucketed.hpp"

Bucket::Bucket(int start, int end)
    : max(start), min(end) {}

Bucket::~Bucket() {}

// Move constructor and move assignment operator
Bucket& Bucket::operator=(Bucket&& other) {
    if (this != &other) {
        min = other.min;
        max = other.max;
        totalCount = other.totalCount;
        memcpy(&subCounts, &other.subCounts, sizeof(subCounts));
        hll = std::move(other.hll);
    }
    return *this;
}

Bucket::Bucket(Bucket&& other) {
    min = other.min;
    max = other.max;
    totalCount = other.totalCount;
    memcpy(&subCounts, &other.subCounts, sizeof(subCounts));
    hll = std::move(other.hll);
}

void Bucket::insert(HistogramBucketed* caller, int value) {
    if(value > max) max = value;
    if(value < min) min = value;

    hll.insert(value);

    totalCount++;

    int subBucketIdx = getSubBucketIndex(value, caller);

    subCounts[subBucketIdx]++;
}

void Bucket::del(HistogramBucketed* caller, int value) {
    if(totalCount == 0) return;
    totalCount--;

    int subBucketIdx = getSubBucketIndex(value, caller);
    subCounts[subBucketIdx]--;
}

bool Bucket::contains(int value){
    return value >= min && value <= max;
}

uint32_t Bucket::estimateGreater(HistogramBucketed* caller, int value) {
    if(value < min){
        return totalCount;
    }
    else if(value >= max){
        return 0;
    }

    uint32_t subBucketSize = caller->subBucketSize_;
    auto valueIndex = getSubBucketIndex(value, caller);
    auto minIndex = getSubBucketIndex(min, caller);
    auto maxIndex = getSubBucketIndex(max, caller);

    auto valueCalibrated = ((value - caller->rangeStart_) % caller->bucketSize_) % subBucketSize;
    auto minCalibrated = ((min - caller->rangeStart_) % caller->bucketSize_) % subBucketSize;
    auto maxCalibrated = ((max - caller->rangeStart_) % caller->bucketSize_) % subBucketSize;

    auto localmin = (minIndex == valueIndex) ? minCalibrated : 0;
    auto localmax = (maxIndex == valueIndex) ? maxCalibrated : subBucketSize - 1;
    uint32_t estimate = ((float)(localmax - valueCalibrated) / (localmax - localmin + 1)) * subCounts[valueIndex];

    for(int idx = valueIndex + 1; idx < SUB_BUCKETS; idx++){
        estimate += subCounts[idx];
    }

    return estimate;
}

uint32_t Bucket::estimateDistinctPerSubBucket(int value) {
    auto estimate = hll.estimate();
    if(estimate < SUB_BUCKETS) return estimate;

    return estimate / SUB_BUCKETS;
}

uint32_t Bucket::estimateDistinct() {
    return hll.estimate();
}

uint32_t Bucket::getTotalCount() const {
    return totalCount;
}

uint32_t Bucket::getSubBucketCount(HistogramBucketed* caller, int value) {
    int subBucketIdx = getSubBucketIndex(value, caller);
    
    return subCounts[subBucketIdx];
}

void Bucket::scale(float value) {
    this->totalCount *= value;
    
    for(int i = 0; i < SUB_BUCKETS; i++){
        this->subCounts[i] *= value;
    }
}

float Bucket::getPercentageAbove(int value){
    if(value < min){
        return 1.0;
    }
    else if(value > max){
        return 0.0;
    }

    return (float)(max - value) / (max - min);
}

size_t Bucket::memUsage() {
    size_t memUsage = 0;
    memUsage += sizeof(max) + sizeof(min) + sizeof(totalCount) + sizeof(subCounts);
    memUsage += hll.memUsage();
    return memUsage;
}

inline int Bucket::getSubBucketIndex(int value, HistogramBucketed* caller) {
    int subBucketIdx = (value - caller->rangeStart_) % caller->bucketSize_;
    subBucketIdx = subBucketIdx / caller->subBucketSize_ ;
    return subBucketIdx;
}