#include "MCVTrack.hpp"
#include <iostream>

MCVTrack::MCVTrack(int max_holds) : max_holds(max_holds){
    mcv_map.reserve(max_holds);
}

void MCVTrack::recordValue(int value){
    if(mcv_map.find(value) != mcv_map.end()){
        mcv_map[value]++;
    }
    else if(mcv_map.size() < max_holds){
        mcv_map[value] = 1;
    }
}

void MCVTrack::decrementValue(int value){
    if(mcv_map.find(value) != mcv_map.end()){
        mcv_map[value]--;
        if(mcv_map[value] == 0){
            mcv_map.erase(value);
        }
    }
}

uint32_t MCVTrack::getMCV(int value){
    auto it = mcv_map.find(value);
    return (it == mcv_map.end()) ? 0 : it->second;
}

void MCVTrack::scale(float value){
    for(auto& [key, val] : mcv_map){
        val *= value;
    }
}

void MCVTrack::printMCV(){
    for(auto& [key, value] : mcv_map){
        std::cout << key << " : " << value << std::endl;
    }
}

size_t MCVTrack::memUsage(){
    size_t mapSize = mcv_map.size() * (sizeof(int) + sizeof(uint32_t));     // key and value
    size_t bucketSize = mcv_map.mask() + 1;                                 // bucket size
    size_t bucketMem = bucketSize * (sizeof(void*) + sizeof(uint8_t));      // bucket memory

    return sizeof(max_holds) + sizeof(mcv_map) + mapSize + bucketMem;
}