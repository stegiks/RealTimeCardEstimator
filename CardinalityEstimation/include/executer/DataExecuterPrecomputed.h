#ifndef CARDINALITYESTIMATION_DATAEXECUTERPRECOMPUTED
#define CARDINALITYESTIMATION_DATAEXECUTERPRECOMPUTED

#include "DataExecuterDemo.h"
#include <unordered_map>

class DataExecuterPrecomputed : public DataExecuter{
private:
    
    int mtSeed_;

    std::vector<std::vector<int>> data_;
    std::vector<int> correctAnswer_;

    // Action generation
    int queryCount_{0}, numQueries_{0};
    int curEnd_{0}, currentOp_{0};
    std::unordered_map<int, bool> deletedRows_;
    std::mt19937 rng_;

    // Distributions used for generating the operations.
    std::uniform_int_distribution<int> columnDist_;
    std::uniform_int_distribution<int> compareOpDist_;
    std::uniform_int_distribution<int> numOfCompDist_;
    std::uniform_int_distribution<int> valueDist_;

    Action buildAction();

public:
    int totalRows_, totalOps_;

    /**
     * @brief Construct a new Data Executer Precomputed object
     * 
     * @param dataFilePath The path to the data file with the metadata for it.
     * @param answerFilePath The path to the answer file with the metadata for it.
    */
    DataExecuterPrecomputed(const std::string& dataFilePath, const std::string& answerFilePath);

    /**
     * @brief Read tuples from the data file.
     * 
     * @param tupleId The id of the tuple to start reading from.
     * @param offset The number of tuples to read.
     * @param vec The vector to store the tuples.
    */
    void readTuples(int tupleId, int offset, std::vector<std::vector<int>> &vec);
    
    /**
     * @brief Get the next action using the seed and total number of operations. 
    */
    Action getNextAction();

    /**
     * @brief Get the error of the CEEngine answer using the precomputed answer.
     * 
     * @param ans CEEngine answer to the query.
     * @return The error of the answer.
    */
    double answer(int ans);
};

#endif