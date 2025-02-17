//
// You should modify this file.
//
#include <common/Root.h>
#include <CardinalityEstimation.h>
#include <executer/DataExecuterDemo.h>

#include "config.hpp"

void CEEngine::insertTuple(const std::vector<int>& tuple)
{  
    histograms[0].insert(tuple[0]);
    histograms[1].insert(tuple[1]);
    data_size++;
}

void CEEngine::deleteTuple(const std::vector<int>& tuple, int tupleId)
{
    histograms[0].deleteValue(tuple[0]);
    histograms[1].deleteValue(tuple[1]);
    data_size--;
}

int CEEngine::query(const std::vector<CompareExpression>& quals)
{
    // Implement your query logic here.
    uint32_t est1 = data_size, est2 = data_size, est = data_size;
    bool hasEqual1 = false;
    bool hasEqual2 = false;

    if (quals.size() > 0) {
        const auto& q1 = quals[0];
        if (q1.compareOp == GREATER) {
            est1 = histograms[q1.columnIdx].estimateGreater(q1.value);
        }
        else if (q1.compareOp == EQUAL) {
            est1 = histograms[q1.columnIdx].estimateEqual(q1.value);
            hasEqual1 = true;
        }
    }

    if (quals.size() > 1) {
        const auto& q2 = quals[1];
        if (q2.compareOp == GREATER) {
            est2 = histograms[q2.columnIdx].estimateGreater(q2.value);
        }
        else if (q2.compareOp == EQUAL) {
            est2 = histograms[q2.columnIdx].estimateEqual(q2.value);
            hasEqual2 = true;
        }
    }

    if(hasEqual1 || hasEqual2){
        est = std::min(est1, est2);
    }
    else{
        est = (float)est1 * (float)est2 / data_size;
    }

    return est;
}

void CEEngine::prepare()
{
    // Implement your prepare logic here.
}

CEEngine::CEEngine(int num, DataExecuter* dataExecuter)
    : dataExecuter(dataExecuter), data_size(num) {

    int size_to_read = std::min(num, MAX_READ_SIZE); 
    int mcvSize = MCV_TRACK_SIZE;

    // Construct 2 histograms
    this->histograms = new HistogramBucketed[2]{
        HistogramBucketed(MIN_RANGE, MAX_RANGE, BUCKET_NUM, SUB_BUCKETS, BINARY_CACHE_DEPTH, mcvSize),
        HistogramBucketed(MIN_RANGE, MAX_RANGE, BUCKET_NUM, SUB_BUCKETS, BINARY_CACHE_DEPTH, mcvSize)
    };

    // Based on 4mb memory limit, max batch size is 20833
    std::vector<std::vector<int>> data;

    data.reserve(BATCH_SIZE);

    // Read data in batches to avoid high memory consumption
    for(int i = 0; i < size_to_read; i += BATCH_SIZE)
    {
        dataExecuter->readTuples(i, BATCH_SIZE, data);

        for (const auto& tuple : data) {
            histograms[0].preparationInsert(tuple[0]);
            histograms[1].preparationInsert(tuple[1]);
        }

        data.clear();
    }

    histograms[0].preparationEnd();
    histograms[1].preparationEnd();

    float scaleFactor = (float)num / size_to_read;
    histograms[0].scale(scaleFactor);
    histograms[1].scale(scaleFactor);
}

CEEngine::~CEEngine(){
    delete[] histograms;
}

void CEEngine::printMemUsage(bool printDetails){
    size_t memUsage = 0;
    memUsage += sizeof(data_size);
    memUsage += sizeof(histograms);
    
    memUsage += histograms[0].memUsage();
    memUsage += histograms[1].memUsage();
    memUsage += sizeof(dataExecuter);

    std::cout << "Estimator Memory Usage: " << memUsage / 1024 << " KB" << std::endl;
    if(printDetails){
        std::vector<size_t> details;
        std::cout << "\tHistogram 1 Memory Usage: " << histograms[0].memUsage(&details) / 1024 << " KB" << std::endl;
        std::cout << "\t\tLocal Variables: " << details[0] / 1024 << " KB" << std::endl;
        std::cout << "\t\tBuckets: " << details[1] / 1024 << " KB" << std::endl;
        std::cout << "\t\tMCV: " << details[2] / 1024 << " KB" << std::endl;
        std::cout << "\t\tCache: " << details[3] / 1024 << " KB" << std::endl;
        
        details.clear();
        std::cout << "\tHistogram 2 Memory Usage: " << histograms[1].memUsage() / 1024 << " KB" << std::endl;
        std::cout << "\t\tLocal Variables: " << details[0] / 1024 << " KB" << std::endl;
        std::cout << "\t\tBuckets: " << details[1] / 1024 << " KB" << std::endl;
        std::cout << "\t\tMCV: " << details[2] / 1024 << " KB" << std::endl;
        std::cout << "\t\tCache: " << details[3] / 1024 << " KB" << std::endl;
    }
}
