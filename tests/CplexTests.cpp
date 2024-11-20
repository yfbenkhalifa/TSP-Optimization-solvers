//
// Created by wiz on 3/29/24.
//

#include <gtest/gtest.h>
#include "cplex_builder.c"
#define ROOTDIR "../../"



TEST(CplexTests, BuildCplexInstanceTest) {

    std::string file = ROOTDIR"data/tsp_mock.tsp";
    // Read input from file
    instance inst;
    strcpy(inst.input_file, "NULL");
    strcpy(inst.input_file, file.c_str());
    read_input(&inst);

    // // open CPLEX model
    int error;
    CPXENVptr env = CPXopenCPLEX(&error);
    if ( error ) print_error("CPXopenCPLEX() error");
    CPXLPptr lp = CPXcreateprob(env, &error, "TSP model version 1");
    if ( error ) print_error("CPXcreateprob() error");
    build_model(&inst, env, lp);

    int ncols = CPXgetnumcols(env, lp);
    EXPECT_EQ(ncols, 45);


}
