//
// Created by wiz on 3/29/24.
//

#include <gtest/gtest.h>
#include "utils.c"

#define ROOTDIR "../../"

// Demonstrate some basic assertions.
TEST(UnitTests, ReadInputTest) {
    std::string file = ROOTDIR"data/att48.tsp";
    // Read input from file
    instance inst;
    strcpy(inst.input_file, "NULL");
    strcpy(inst.input_file, file.c_str());
    read_input(&inst);
    EXPECT_EQ(inst.nnodes, 48);
    EXPECT_EQ(inst.best_known_solution_value, 10628.0);
}

TEST(UnitTestst, EuclideanDistanceTest) {
    double x1 = 0.0;
    double y1 = 0.0;
    double x2 = 3.0;
    double y2 = 4.0;
    bool round = false;
    double distance = euclidean_distance(x1, y1, x2, y2, round);
    EXPECT_EQ(distance, 5.0);
}
