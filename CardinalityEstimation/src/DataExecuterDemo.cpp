//
// Demo data generator for local debugging. You can implement your own data generator for debugging based on this class.
//

#include <executer/DataExecuterDemo.h>

std::unordered_map<int, bool> vis;
DataExecuterDemo::DataExecuterDemo(int end, int count) : DataExecuter()
{
    srand(time(NULL));
    this->end = end;
    this->count = count;
    for (int i = 0; i <= end; ++i) {
        std::vector<int> tuple;
        tuple.push_back(rand() % MAX_VALUE + 1);
        tuple.push_back(rand() % MAX_VALUE + 1);
        set.emplace_back(tuple);
    }
}

std::vector<int> DataExecuterDemo::generateInsert()
{
    std::vector<int> tuple;
    tuple.push_back(rand() % MAX_VALUE + 1);
    tuple.push_back(rand() % MAX_VALUE + 1);
    set.push_back(tuple);
    end++;
    return tuple;
}

int DataExecuterDemo::generateDelete()
{
    int x = (rand()) % end;
    while (vis[x]) {
        x = (rand()) % end;
    }
    vis[x] = true;
    return x;
}

void DataExecuterDemo::readTuples(int start, int offset, std::vector<std::vector<int>> &vec)
{
    for (int i = start; i < start + offset; ++i) {
        if(i > end) break;
        
        if (!vis[i]) {
            vec.emplace_back(set[i]);
        }
    }
    return;
};

Action DataExecuterDemo::getNextAction()
{
    Action action;
    if (count == 0) {
        action.actionType = NONE;
        return action;
    }
    if (count % 100 == 99) {
        action.actionType = QUERY;
        if(rand() % 2 == 0){
            CompareExpression expr = {rand() % 2, CompareOp(rand() % 2), rand() % MAX_VALUE + 1};
            action.quals.push_back(expr);
        } 
        else{
            int columnid = rand() % 2;
            CompareExpression expr = {columnid, CompareOp(rand() % 2), rand() % MAX_VALUE + 1};
            action.quals.push_back(expr);

            // The second condition must be different from the first condition
            if(columnid == 0) columnid = 1;
            else columnid = 0;

            expr = {columnid, CompareOp(rand() % 2), rand() % MAX_VALUE + 1};
            action.quals.push_back(expr);
        }
    } else if (count % 100 < 90) {
        action.actionType = INSERT;
        action.actionTuple = generateInsert();
    } else {
        action.actionType = DELETE;
        action.tupleId = generateDelete();
        action.actionTuple = set[action.tupleId];
    }
    count--;
    curAction = action;
    return action;
};

double DataExecuterDemo::answer(int ans)
{

    int cnt = 0;
    for (int i = 0; i <= end; ++i) {
        if (vis[i])
            continue;
        bool flag = true;
        for (int j = 0; j < curAction.quals.size(); ++j) {
            CompareExpression &expr = curAction.quals[j];
            if (expr.compareOp == GREATER && set[i][expr.columnIdx] <= expr.value) {
                flag = false;
                break;
            }
            if (expr.compareOp == EQUAL && set[i][expr.columnIdx] != expr.value) {
                flag = false;
                break;
            }
        }
        if (flag)
            cnt++;
    }

    double error = fabs(std::log((ans + 1) * 1.0 / (cnt + 1)));
    return error;
};

double DataExecuterDemo::memUsageFromDataExecuter(){
    double memUsage = 0;
    memUsage += sizeof(end) + sizeof(count);
    memUsage += sizeof(curAction.actionType) + sizeof(curAction.tupleId);
    
    memUsage += sizeof(curAction.actionTuple);
    for(int& tuple : curAction.actionTuple){
        memUsage += sizeof(tuple);
    }

    memUsage += sizeof(curAction.quals);
    for(CompareExpression& expr : curAction.quals){
        memUsage += sizeof(expr);
    }

    memUsage += sizeof(set);
    for(const auto& tuple : set){
        memUsage += sizeof(tuple);
        memUsage += tuple.size() * sizeof(int);
    }
    return memUsage;
}
