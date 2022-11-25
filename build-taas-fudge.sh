#cd lib/cadical-1.3.1
#./configure
#make
#cd ../..
clang++ -O3 -Wno-c++11-compat-deprecated-writable-strings -Wno-deprecated-declarations -Wno-parentheses -Wno-writable-strings -std=c++0x taas-fudge.cpp -o taas-fudge `pkg-config --cflags --libs glib-2.0` -lm
#... -Ilib/cadical-1.3.1/src lib/cadical-1.3.1/build/libcadical.a
