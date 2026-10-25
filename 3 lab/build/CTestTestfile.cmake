# CMake generated Testfile for 
# Source directory: C:/PGTU/parallel programming/3 lab
# Build directory: C:/PGTU/parallel programming/3 lab/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(bank_tests "C:/PGTU/parallel programming/3 lab/build/test_bank.exe")
set_tests_properties(bank_tests PROPERTIES  _BACKTRACE_TRIPLES "C:/PGTU/parallel programming/3 lab/CMakeLists.txt;18;add_test;C:/PGTU/parallel programming/3 lab/CMakeLists.txt;0;")
add_test(economy_single "C:/PGTU/parallel programming/3 lab/build/bank.exe" "--mode" "single" "--seconds" "1" "--quiet")
set_tests_properties(economy_single PROPERTIES  _BACKTRACE_TRIPLES "C:/PGTU/parallel programming/3 lab/CMakeLists.txt;19;add_test;C:/PGTU/parallel programming/3 lab/CMakeLists.txt;0;")
add_test(economy_parallel "C:/PGTU/parallel programming/3 lab/build/bank.exe" "--mode" "parallel" "--seconds" "1" "--quiet")
set_tests_properties(economy_parallel PROPERTIES  _BACKTRACE_TRIPLES "C:/PGTU/parallel programming/3 lab/CMakeLists.txt;20;add_test;C:/PGTU/parallel programming/3 lab/CMakeLists.txt;0;")
