#include <CardinalityEstimation.h>
#include <executer/DataExecuterPrecomputed.h>
#include <fstream>
#include <iostream>

int main(int argc, char *argv[])
{
    double score = 0;
    int cnt = 0;

    if(argc != 3){
        std::cerr << "Usage: " << argv[0] << " <dataFilePath> <answerFilePath>" << std::endl;
        return 1;
    }

    std::string dataFilePath = argv[1];
    std::string answerFilePath = argv[2];
    
    // DataExecuterDemo dataExecuter(initSize - 1, opSize);
    DataExecuterPrecomputed dataExecuter(dataFilePath, answerFilePath);
    CEEngine ceEngine(dataExecuter.totalRows_, &dataExecuter);
    Action action = dataExecuter.getNextAction();

    // Print memory usage by CEEngine.
    ceEngine.printMemUsage(true);

    while (action.actionType != NONE) {
        ceEngine.prepare();
        if (action.actionType == INSERT) {
            ceEngine.insertTuple(action.actionTuple);
        } else if (action.actionType == DELETE) {
            ceEngine.deleteTuple(action.actionTuple, action.tupleId);
        } else if (action.actionType == QUERY) {
            int ans = ceEngine.query(action.quals);
            score += dataExecuter.answer(ans);
            cnt++;
        }
        action = dataExecuter.getNextAction();
    }

    // Print memory usage by CEEngine after all operations.
    ceEngine.printMemUsage();
    std::cout << score / cnt << std::endl;
}