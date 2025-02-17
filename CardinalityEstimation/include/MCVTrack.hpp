#ifndef MCVTRACK_HPP
#define MCVTRACK_HPP

#include <unordered_map>
#include <vector>
#include <algorithm>
#include <cstdint>
#include <robin_hood.h>


class MCVTrack {

public:
    
    /**
     * @brief Construct a new MCVTrack object
     * 
     * @param max_holds The maximum number of values to be held in the MCV.
     */
    MCVTrack(int max_holds);

    /**
     * @brief Record a value in the MCV.
     * 
     * @param value The value to be recorded.
     */
    void recordValue(int value);

    /**
     * @brief Decrement the value in the MCV.
     * 
     * @param value The value to be decremented.
     */
    void decrementValue(int value);

    /**
     * @brief Get the MCV of a value.
     * 
     * @param value The value to get the MCV.
     * @return The MCV of the value.
     */
    uint32_t getMCV(int value);

    /**
     * @brief Scale the MCV by multiplying all values by a given factor.
     * 
     * @param value The scale factor.
     */
    void scale(float value);
    
    void printMCV();

    size_t memUsage();

private:

    int max_holds;
    robin_hood::unordered_map<int, uint32_t> mcv_map;

};


#endif // MCVTRACK_HPP