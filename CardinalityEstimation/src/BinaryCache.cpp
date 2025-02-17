#include "BinaryCache.hpp"
#include <iostream>

BinaryCache::BinaryCache(int max_range, int depth):
    max_range(max_range), depth(depth) {

    this->min_insert_val = max_range / (1 << depth);
    this->cache_size = (1 << (depth + 1)) - 1;
    this->cache = new uint32_t[cache_size]();

    // Precompute the steps for the binary search
    this->cache_steps = new uint32_t[depth];
    this->cache_steps[0] = max_range / 4;
    for (int i = 1; i < depth; i++) {
        this->cache_steps[i] = this->cache_steps[i - 1] >> 1;
    }

    this->start_offset =(1 << depth) - 1;
    this->bucket_size = (float)(1 << depth) / max_range;
}

BinaryCache::~BinaryCache() {
    delete[] this->cache;
    delete[] this->cache_steps;
}

void BinaryCache::insert(int value) {
    if (value < this->min_insert_val) {
        return;
    }

    int index = 0;
    uint32_t cur_middle = this->max_range / 2;

    for (int i = 0; i < this->depth; i++) {
        if (value > cur_middle) {
            index = 2 * index + 2;
            this->cache[index]++;
            cur_middle = cur_middle + this->cache_steps[i];
        } else if (value < cur_middle) {
            index = 2 * index + 1;
            cur_middle = cur_middle - this->cache_steps[i];
        }
        else {
            this->cache[2 * index + 2]++;
            break;
        }
    }
}

uint32_t BinaryCache::query(int value, int* range){
    int index = 0;
    uint32_t cur_middle = this->max_range / 2;
    uint32_t result = 0;
    uint32_t cur_range = max_range;

    for (int i = 0; i < this->depth; i++) {
        if (value > cur_middle) {
            index = 2 * index + 2;
            cur_middle = cur_middle + this->cache_steps[i];
        } else if (value < cur_middle) {
            index = 2 * index + 1;
            cur_range = cur_middle;
            cur_middle = cur_middle - this->cache_steps[i];
            result += this->cache[index + 1];
        }
        else {
            result += this->cache[index + 1];
            cur_range = cur_middle;
            break;
        }
    }

    if(range != nullptr) {
        *range = cur_range;
    }
    return result;
}

void BinaryCache::del(int value) {
    if (value < this->min_insert_val) {
        return;
    }

    int index = 0;
    uint32_t cur_middle = this->max_range / 2;

    for (int i = 0; i < this->depth; i++) {
        if (value > cur_middle) {
            index = 2 * index + 2;
            if(this->cache[index] > 0)
                this->cache[index]--;
            cur_middle = cur_middle + this->cache_steps[i];
        } else if (value < cur_middle) {
            index = 2 * index + 1;
            cur_middle = cur_middle - this->cache_steps[i];
        }
        else {
            if(this->cache[index] > 0)
                this->cache[index]--;
            break;
        }
    }
}

void BinaryCache::scaleCache(float value){
    if(float(value) == 1.0){
        return;
    }

    for(int i = 0; i < this->cache_size; ++i){
        this->cache[i] = this->cache[i] * value;
    }
}

size_t BinaryCache::memUsage(){
    size_t mem = sizeof(max_range) + sizeof(depth) + sizeof(min_insert_val) + sizeof(start_offset);
    mem += sizeof(cache_size);
    mem += sizeof(uint32_t) * this->cache_size;
    mem += sizeof(uint32_t) * this->depth;
    mem += sizeof(cache) + sizeof(cache_steps);
    mem += sizeof(bucket_size);

    return mem;
}

void BinaryCache::lowerLevelInsert(int value){
    if (value < this->min_insert_val) {
        return;
    }

    uint32_t index = (value-1) * bucket_size;

    this->cache[index + start_offset]++;
}

void BinaryCache::recalculateLayers(){
    for(int layer = depth - 1; layer > 0; layer --)
    {
        int offset = (1 << layer) - 1;
        for(int i = 0; i < (1 << layer); i++)
        {   
            int index = offset + i;
            this->cache[index] = this->cache[2 * index + 1] + this->cache[2 * index + 2];
        }
    }

}