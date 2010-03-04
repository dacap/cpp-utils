# "test" executable is not a WIN32 (-mwindows) because it printf
# some lines. This should not be in this way in the near future
# (do we need some window::writeln() member function?)
include_directories(.)
add_executable(test tests/test.cpp)
add_executable(condition_variables tests/condition_variables.cpp)
