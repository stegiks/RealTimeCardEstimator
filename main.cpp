#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <unordered_map>
#include <omp.h>

/**
 * An enum stands for the column filter operator.
 */
enum CompareOp { EQUAL = 0, GREATER = 1 };

/**
 * A struct for the expression of a column compare operation.
 * For example, for the structure {columnIdx: 1, compareOp: 0, value: 12321}, the expression is that the first column is
 * equal to 12321.
 */
typedef struct CompareExpression {
    int columnIdx;
    CompareOp compareOp;
    int value;
} CompareExpression;

/**
 * An enum stands for the action operator.
 */
enum ActionType { NONE = 0, INSERT = 1, DELETE = 2, QUERY = 3 };

/**
 * A class for the action.
 */
typedef struct Action {
    ActionType actionType;
    std::vector<int> actionTuple;
    int tupleId;
    std::vector<CompareExpression> quals;
} Action;

struct Row {
    int col1;
    int col2;
};

int main(int argc, char** argv){

    if(argc < 4 || argc > 5){
        std::cerr << "Usage: " << argv[0] << " <seed> <total_rows> <total_ops> [data-only]" << std::endl;
        return 1;
    }

    int seed = std::stoi(argv[1]);
    int total_rows = std::stoi(argv[2]);
    int total_ops = std::stoi(argv[3]);
    bool data_only = (argc == 5 && std::string(argv[4]) == "data-only");

    // 1. Generate a large dataset and store it in a file.
    std::ofstream dataFile("dataset.bin", std::ios::binary);

    // ! Write first the seed and the total rows to the file.
    dataFile.write(reinterpret_cast<const char*>(&seed), sizeof(seed));
    dataFile.write(reinterpret_cast<const char*>(&total_rows), sizeof(total_rows));
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> dist(1, 20000000);

    // Have the data in memory as well to generate operations and then to calculate the correct answer.
    std::vector<Row> data;
    data.reserve(total_rows);
    
    for(int i = 0; i < total_rows; i++){
        int value1 = dist(rng);
        int value2 = dist(rng);
        data.push_back({value1, value2});
        dataFile.write(reinterpret_cast<const char*>(&value1), sizeof(value1));
        dataFile.write(reinterpret_cast<const char*>(&value2), sizeof(value2));
    }
    dataFile.close();

    std::cout << "Data generation completed." << std::endl;

    if(data_only){
        std::cout << "Data-only mode : Skipping operations generation and correct answers calculation." << std::endl;
        return 0;
    }

    // 2. Generate operations with 90% insert, 9% delete, and 1% query and only the last beign NONE to stop the loop.
    std::uniform_int_distribution<int> columnDist(0, 1);
    std::uniform_int_distribution<int> compareOpDist(0, 1);
    std::uniform_int_distribution<int> numOfCompDist(1, 2);
    std::uniform_int_distribution<int> valueDist(1, 20000000);
    int cur_end = total_rows;
    std::vector<bool> deleted_rows(total_rows, false);

    // Have the operations in memory as well to calculate the correct answer.
    std::vector<Action> operations;
    int queries = 0;

    for(int i = 0; i < total_ops; i++){
        Action action = {};
        if(i == total_ops - 1){
            action.actionType = NONE;
        }
        else if(i % 100 < 90){
            action.actionType = INSERT;
            int value1 = dist(rng);
            int value2 = dist(rng);
            data.push_back({value1, value2});
            action.actionTuple.push_back(value1);
            action.actionTuple.push_back(value2);
            cur_end++;
            deleted_rows.push_back(false);
        }
        else if(i % 100 < 99){
            action.actionType = DELETE;
            
            // Randomly delete a row
            int row = (rand()) % cur_end;
            while(deleted_rows[row]){
                row = (rand()) % cur_end;
            }

            deleted_rows[row] = true;
            action.tupleId = row;
            std::vector<int> dataRow = {data[row].col1, data[row].col2};
            action.actionTuple = dataRow;
        }
        else{
            action.actionType = QUERY;
            queries++;
            int numComp = numOfCompDist(rng);
            if(numComp == 1){
                CompareExpression expr = {columnDist(rng), CompareOp(compareOpDist(rng)), valueDist(rng)};
                action.quals.push_back(expr);
            }
            else{
                int columnid = columnDist(rng);
                CompareExpression expr = {columnid, CompareOp(compareOpDist(rng)), valueDist(rng)};
                action.quals.push_back(expr);

                // The second columnid must be different from the first columnid
                if(columnid == 0) columnid = 1;
                else columnid = 0;

                CompareExpression expr2 = {columnid, CompareOp(compareOpDist(rng)), valueDist(rng)};
                action.quals.push_back(expr2);
            }
        }
        operations.push_back(action);
    }
    deleted_rows.clear();

    for(int i = 0; i < total_rows; i++){
        deleted_rows.push_back(false);
    }

    std::cout << "Operations generation completed." << std::endl;

    // 3. Calculate the correct answer for every query operation.
    std::ofstream correctAnswersFile("correctAnswers.bin", std::ios::binary);

    // ! Write first the number of operations and queries to the file.
    correctAnswersFile.write(reinterpret_cast<const char*>(&total_ops), sizeof(total_ops));
    correctAnswersFile.write(reinterpret_cast<const char*>(&queries), sizeof(queries));
    cur_end = total_rows;

    for(int i = 0; i < total_ops; i++){

        if(i % 500000 == 0)
            std::cout << "Processed " << i << " operations." << std::endl;

        if(operations[i].actionType == QUERY){
            int ans = 0;

            #pragma omp parallel for reduction(+:ans)
            for(int z = 0; z < cur_end; z++){
                if(deleted_rows[z]) continue;

                bool valid = true;
                for(std::size_t j = 0; j < operations[i].quals.size(); j++){
                    CompareExpression& expr = operations[i].quals[j];
                    if((expr.compareOp == GREATER && (expr.columnIdx == 0 ? data[z].col1 <= expr.value : data[z].col2 <= expr.value)) ||
                        (expr.compareOp == EQUAL && (expr.columnIdx == 0 ? data[z].col1 != expr.value : data[z].col2 != expr.value))){
                        valid = false;
                        break;
                    }
                }

                if(valid) ans++;
            }
            correctAnswersFile.write(reinterpret_cast<const char*>(&ans), sizeof(ans));
        }
        else{
            if(operations[i].actionType == DELETE){
                deleted_rows[operations[i].tupleId] = true;
            }
            else if(operations[i].actionType == INSERT){
                cur_end++;
                deleted_rows.push_back(false);
            }
        }
    }
    correctAnswersFile.close();

    std::cout << "Correct answers calculation completed." << std::endl;
    return 0;
}