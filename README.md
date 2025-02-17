# Real Time Cardinality Estimator

This project was created for the [Future of Database Programming Contest](https://databasecontest2024.athenarc.gr/). We achieved first place in the challenge organized by [Athena Research Center](https://www.athenarc.gr/en/home) and the [DareLab group](https://darelab.athenarc.gr/).

## Competition Overview

In this contest the task was to build a __real-time cardinality estimator__ for a table containing two integer columns. Cardinality estimation is at the heart of query optimization in relational databases. Accurate estimations guide the optimizer to choose a good execution plan; an error in estimation can lead to inefficient query plans.

This year's challenge had to tackle:

- Frequent data changes.
- Correlation between columns.
- Data skewness.

Except for the estimation, the estimator had to be efficient in terms of memory usage and execution time. The estimator was evaluated based on the following criteria:

- __Data and Operations__: The initial data volume of each test case cannot exceed 50,000,000, including 20,000,000 insert, delete, and query operations. The data value ranges from 1 to 20,000,000.
- __Memory Usage__: Limit the memory usage to 4MB.
- __Execution Time__: The estimator must be able to run in under 10s for each test case.

More details about the task can be seen on the [contest details page](https://databasecontest2024.athenarc.gr/Task_Details).

## Build Instructions

### Generating Data and Answers

For the competition, a class named [DataExecuterDemo](CardinalityEstimation/src/DataExecuterDemo.cpp) was provided to generate random data at runtime of the estimator. However, the correct answers were computed also at runtime, which made the execution time longer. To improve the performance, a new class named [DataExecuterPrecomputed](CardinalityEstimation/src/DataExecutorPrecomputed.cpp) was created. This class reads data and precomputed answers from files.

Now to take advantage of this new class, we needed to generate the data and correct answers files. The data generation is handled by the main file in the project root ([main.cpp](main.cpp)). A provided [Makefile](Makefile) in the project root lets you control the size of the data used. Once the main program is run, it generates two files:

- A data file.
- A file containing the correct answers.

The generation process is quite similar to the one used in the provided DataExecuterDemo class. The main difference is that for the calculation of the correct answers, OpenMP is used to speed up the process. Using this method, a dataset of 5 million rows for each column and 2 million queries can be generated in 2 minutes on a 12-thread machine.

### Compiling the Estimator

To compile the estimator, follow these steps:

1. __Create a Build Directory__  

   In your terminal, in the CardinalityEstimation folder run:

   ```sh
   mkdir build && cd build
   ```

2. __Run CMake__

    Generate the build files with CMake:

    ```sh
    cmake ..
    ```

3. __Build the Project__

    Compile the project:

    ```sh
    make
    ```

4. __Run the Estimator__

    The executable is named `main` and it accepts the following arguments:

    ```sh
    ./main <data_file> <answers_file>
    ```

## Project Overview

The estimator is composed of several components:

- __Histogram-Based Estimators__:  
    Histograms are used to divide data values into different intervals (buckets) and record the frequency of data in each bucket ([HistogramBucketed.hpp](CardinalityEstimation/include/HistogramBucketed.hpp)). Two key classes for the histogram implementation are the following.

- __Buckets__:  
    Of course, having a histogram structure that contains all the possible values is not feasible in practice. To address this issue, we use a class named [`Bucket`](CardinalityEstimation/include/Bucket.hpp) that contains a range of values and their frequencies. The HistogramBucketed class employs Bucket objects to track value frequencies across specified ranges. To enhance estimation accuracy, each Bucket is further divided into sub-buckets. This hierarchical structure allows for more precise frequency tracking when it is required.

- __BinaryCache__:  
    The [`BinaryCache`](CardinalityEstimation/include/BinaryCache.hpp) class is used in the estimation when we have greater (>) as an operation in the query. Instead of iterating through all the buckets and adding their frequencies, we use a binary structure where in each lower level we have smaller intervals that contain the right ammount of elements to add to the final result. This way we can achieve a logarithmic complexity in the estimation process while keeping the memory usage low.

- __HyperLogLog (HLL)__:  
    The [`HLL`](CardinalityEstimation/include/HLL.hpp) class provides a fast estimation of distinct values based on the [HyperLogLog algorithm](https://en.wikipedia.org/wiki/HyperLogLog). The implementation of this class is based on the [HyperLogLog paper](https://algo.inria.fr/flajolet/Publications/FlFuGaMe07.pdf)

- __MCV (Most Common Value) Tracking__:  
    The [`MCVTrack`](CardinalityEstimation/include/MCVTrack.hpp) class keeps counts for values that occur frequently. This helps further improve the accuracy of the estimator.

- __Scoring__:  
    The estimator outputs a score based on the formula:
  
   $$\text{score} = \frac{\sum_{i=0}^{n} \left| \log\left(\frac{\text{EstimatedValue}_i + 1}{\text{RealAnswer}_i + 1}\right) \right|}{n}$$

    It also details the memory usage of internal components (e.g., buckets, cache, MCV).

- __Third-Party Tools__:  
    Besides the standard library, the project uses:

    - [rapidhash](https://github.com/Nicoshev/rapidhash) for an efficient hash function used in HLL.
    - [robin hood](https://github.com/martinus/robin-hood-hashing) unordered map for improved performance over STL’s unordered map.
