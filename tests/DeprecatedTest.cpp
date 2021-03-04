// Copyright (c) 2017-2020, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

#include "app_helper.hpp"

#include "gmock/gmock.h"

using ::testing::HasSubstr;
using ::testing::Not;

TEST(Deprecated, Empty) {
    // No deprecated features at this time.
    EXPECT_TRUE(true);
}

// Classic sets

TEST(THelp, Defaults) {
    CLI::App app{"My prog"};

    int one{1}, two{2};
    app.add_option("--one", one, "Help for one", true);
    app.add_option("--set", two, "Help for set", true)->check(CLI::IsMember({2, 3, 4}));

    std::string help = app.help();

    EXPECT_THAT(help, HasSubstr("--one"));
    EXPECT_THAT(help, HasSubstr("--set"));
    EXPECT_THAT(help, HasSubstr("1"));
    EXPECT_THAT(help, HasSubstr("=2"));
    EXPECT_THAT(help, HasSubstr("2,3,4"));
}

TEST(THelp, VectorOpts) {
    CLI::App app{"My prog"};
    std::vector<int> x = {1, 2};
    app.add_option("-q,--quick", x, "", true);

    std::string help = app.help();

    EXPECT_THAT(help, HasSubstr("INT=[1,2] ..."));
}

TEST(THelp, SetLower) {
    CLI::App app{"My prog"};

    std::string def{"One"};
    app.add_option("--set", def, "Help for set", true)->check(CLI::IsMember({"oNe", "twO", "THREE"}));

    std::string help = app.help();

    EXPECT_THAT(help, HasSubstr("--set"));
    EXPECT_THAT(help, HasSubstr("=One"));
    EXPECT_THAT(help, HasSubstr("oNe"));
    EXPECT_THAT(help, HasSubstr("twO"));
    EXPECT_THAT(help, HasSubstr("THREE"));
}

TEST(THelp, ChangingSetDefaulted) {
    CLI::App app;

    std::set<int> vals{1, 2, 3};
    int val = 2;
    app.add_option("--val", val, "", true)->check(CLI::IsMember(&vals));

    std::string help = app.help();

    EXPECT_THAT(help, HasSubstr("1"));
    EXPECT_THAT(help, Not(HasSubstr("4")));

    vals.insert(4);
    vals.erase(1);

    help = app.help();

    EXPECT_THAT(help, Not(HasSubstr("1")));
    EXPECT_THAT(help, HasSubstr("4"));
}

TEST(THelp, ChangingCaselessSetDefaulted) {
    CLI::App app;

    std::set<std::string> vals{"1", "2", "3"};
    std::string val = "2";
    app.add_option("--val", val, "", true)->check(CLI::IsMember(&vals, CLI::ignore_case));

    std::string help = app.help();

    EXPECT_THAT(help, HasSubstr("1"));
    EXPECT_THAT(help, Not(HasSubstr("4")));

    vals.insert("4");
    vals.erase("1");

    help = app.help();

    EXPECT_THAT(help, Not(HasSubstr("1")));
    EXPECT_THAT(help, HasSubstr("4"));
}

TEST_F(TApp, DefaultOpts) {

    int i = 3;
    std::string s = "HI";

    app.add_option("-i,i", i, "", false);
    app.add_option("-s,s", s, "", true);

    args = {"-i2", "9"};

    run();

    EXPECT_EQ(1u, app.count("i"));
    EXPECT_EQ(1u, app.count("-s"));
    EXPECT_EQ(2, i);
    EXPECT_EQ("9", s);
}

TEST_F(TApp, VectorDefaultedFixedString) {
    std::vector<std::string> strvec{"one"};
    std::vector<std::string> answer{"mystring", "mystring2", "mystring3"};

    CLI::Option *opt = app.add_option("-s,--string", strvec, "", true)->expected(3);
    EXPECT_EQ(3, opt->get_expected());

    args = {"--string", "mystring", "mystring2", "mystring3"};
    run();
    EXPECT_EQ(3u, app.count("--string"));
    EXPECT_EQ(answer, strvec);
}

TEST_F(TApp, DefaultedResult) {
    std::string sval = "NA";
    int ival;
    auto opts = app.add_option("--string", sval, "", true);
    auto optv = app.add_option("--val", ival);
    args = {};
    run();
    EXPECT_EQ(sval, "NA");
    std::string nString;
    opts->results(nString);
    EXPECT_EQ(nString, "NA");
    int newIval;
    // EXPECT_THROW(optv->results(newIval), CLI::ConversionError);
    optv->default_str("442");
    optv->results(newIval);
    EXPECT_EQ(newIval, 442);
}

TEST_F(TApp, OptionWithDefaults) {
    int someint = 2;
    app.add_option("-a", someint, "", true);

    args = {"-a1", "-a2"};

    EXPECT_THROW(run(), CLI::ArgumentMismatch);
}

// #209
TEST_F(TApp, CustomUserSepParse) {

    std::vector<int> vals = {1, 2, 3};
    args = {"--idx", "1,2,3"};
    auto opt = app.add_option("--idx", vals)->delimiter(',');
    run();
    EXPECT_EQ(vals, std::vector<int>({1, 2, 3}));
    std::vector<int> vals2;
    // check that the results vector gets the results in the same way
    opt->results(vals2);
    EXPECT_EQ(vals2, vals);

    app.remove_option(opt);

    app.add_option("--idx", vals, "", true)->delimiter(',');
    run();
    EXPECT_EQ(vals, std::vector<int>({1, 2, 3}));
}

// #209
TEST_F(TApp, CustomUserSepParse2) {

    std::vector<int> vals = {1, 2, 3};
    args = {"--idx", "1,2,"};
    auto opt = app.add_option("--idx", vals)->delimiter(',');
    run();
    EXPECT_EQ(vals, std::vector<int>({1, 2}));

    app.remove_option(opt);

    app.add_option("--idx", vals, "", true)->delimiter(',');
    run();
    EXPECT_EQ(vals, std::vector<int>({1, 2}));
}
//
// #209
TEST_F(TApp, CustomUserSepParse4) {

    std::vector<int> vals;
    args = {"--idx", "1,    2"};
    auto opt = app.add_option("--idx", vals, "", true)->delimiter(',');
    run();
    EXPECT_EQ(vals, std::vector<int>({1, 2}));

    app.remove_option(opt);

    app.add_option("--idx", vals)->delimiter(',');
    run();
    EXPECT_EQ(vals, std::vector<int>({1, 2}));
}

// #218
TEST_F(TApp, CustomUserSepParse5) {

    std::vector<std::string> bar;
    args = {"this", "is", "a", "test"};
    auto opt = app.add_option("bar", bar, "bar");
    run();
    EXPECT_EQ(bar, std::vector<std::string>({"this", "is", "a", "test"}));

    app.remove_option(opt);
    args = {"this", "is", "a", "test"};
    app.add_option("bar", bar, "bar", true);
    run();
    EXPECT_EQ(bar, std::vector<std::string>({"this", "is", "a", "test"}));
}
