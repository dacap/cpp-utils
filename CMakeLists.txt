# "test" executable is not a WIN32 (-mwindows) because it printf
# some lines. This should not be in this way in the near future
# (do we need some window::writeln() member function?)
add_executable(test test.cpp)
add_executable(condition_variables condition_variables.cpp)

