#include <executer/DataExecuterPrecomputed.h>
#include <fstream>

DataExecuterPrecomputed::DataExecuterPrecomputed(const std::string& dataFilePath, const std::string& answerFilePath)
: columnDist_(0, 1), compareOpDist_(0, 1), numOfCompDist_(1, 2), valueDist_(1, 20000000)
{
    // 1. Read the dataset
    std::ifstream dataFile(dataFilePath, std::ios::binary);
    dataFile.read(reinterpret_cast<char*>(&mtSeed_), sizeof(mtSeed_));
    dataFile.read(reinterpret_cast<char*>(&totalRows_), sizeof(totalRows_));
    rng_.seed(mtSeed_);

    // Call the rng as many times as it was called in the data generation to bring the state to the same point.
    for(int i = 0; i < totalRows_; i++){
        int value1, value2;
        value1 = valueDist_(rng_);
        value2 = valueDist_(rng_);
    }

    data_.resize(totalRows_);
    for(int i = 0; i < totalRows_; i++){
        int value1, value2;
        dataFile.read(reinterpret_cast<char*>(&value1), sizeof(value1));
        dataFile.read(reinterpret_cast<char*>(&value2), sizeof(value2));
        data_[i] = {value1, value2};
    }
    dataFile.close();

    // 2. Read the correct answers
    std::ifstream answerFile(answerFilePath, std::ios::binary);
    answerFile.read(reinterpret_cast<char*>(&totalOps_), sizeof(totalOps_));
    answerFile.read(reinterpret_cast<char*>(&numQueries_), sizeof(numQueries_));

    correctAnswer_.resize(numQueries_);
    for(int i = 0; i < numQueries_; i++){
        int ans;
        answerFile.read(reinterpret_cast<char*>(&ans), sizeof(ans));
        correctAnswer_[i] = ans;
    }
    answerFile.close();

    curEnd_ = totalRows_;
}

void DataExecuterPrecomputed::readTuples(int tupleId, int offset, std::vector<std::vector<int>> &vec){
    for(int i = tupleId; i < tupleId + offset; i++){
        if(i >= curEnd_) break;

        if(!deletedRows_[i]){
            vec.emplace_back(data_[i]);
        }
    }
}

Action DataExecuterPrecomputed::getNextAction(){
    if(currentOp_ == totalOps_){
        Action action;
        action.actionType = NONE;
        return action;
    }
       
    Action action = buildAction();
    currentOp_++;
    return action;
}

double DataExecuterPrecomputed::answer(int ans){
    if(queryCount_ == numQueries_){
        return 0;
    }

    int correctAns = correctAnswer_[queryCount_];
    queryCount_++;

    double error = fabs(std::log((ans + 1) * 1.0 / (correctAns + 1)));
    return error;    
}

Action DataExecuterPrecomputed::buildAction(){
    Action action = {};

    if(currentOp_ == totalOps_ - 1){
        // Last operation
        action.actionType = NONE;
        return action;
    }

    if(currentOp_ % 100 < 90){
        action.actionType = INSERT;
        int value1 = valueDist_(rng_);
        int value2 = valueDist_(rng_);
        
        data_.push_back({value1, value2});

        action.actionTuple.push_back(value1);
        action.actionTuple.push_back(value2);
        curEnd_++;
    }
    else if(currentOp_ % 100 < 99){
        action.actionType = DELETE;
        
        // Randomly delete a row
        int row = (rand()) % curEnd_;
        while(deletedRows_[row]){
            row = (rand()) % curEnd_;
        }

        deletedRows_[row] = true;
        action.tupleId = row;
        action.actionTuple = data_[row];
    }
    else{
        action.actionType = QUERY;
        int numComp = numOfCompDist_(rng_);
        if(numComp == 1){
            CompareExpression expr = {columnDist_(rng_), CompareOp(compareOpDist_(rng_)), valueDist_(rng_)};
            action.quals.push_back(expr);
        }
        else{
            int columnid = columnDist_(rng_);
            CompareExpression expr = {columnid, CompareOp(compareOpDist_(rng_)), valueDist_(rng_)};
            action.quals.push_back(expr);

            // The second columnid must be different from the first columnid
            if(columnid == 0) columnid = 1;
            else columnid = 0;

            CompareExpression expr2 = {columnid, CompareOp(compareOpDist_(rng_)), valueDist_(rng_)};
            action.quals.push_back(expr2);
        }
    }

    return action;
}