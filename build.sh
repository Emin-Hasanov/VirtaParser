#!/bin/sh

export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:.

echo "Checking checksum..."
sha256sum --check sources.sha256

ReturnChange=$?

#echo "Check ended like: "${ReturnChange}

if [ $ReturnChange -eq 0 ]; then
echo "There are no changes in source! No need to compile"
else
find ./src -type f -exec sha256sum {} \; > sources.sha256
#sha256sum {./src/menu.cpp ./src/json.hpp} > sources.sha256

echo "Styling source..."
astyle -A1 --pad-oper -n ./src/menu.cpp ./src/virta.cpp ./src/virta.h ./src/retail.cpp ./src/retail.h ./src/company.cpp ./src/company.h ./src/unit.cpp ./src/unit.h

echo "Compiling..."
#g++ -c -I./curlplus ./src/VC.cpp
g++  ./src/retail.cpp ./src/company.cpp ./src/menu.cpp ./src/virta.cpp ./src/unit.cpp -o VirtaParse -lcurl -std=gnu++20

ReturnComp=$?

if [ $ReturnComp -eq 0 ]; then
echo "Compiled!"
else
echo "Error!"
fi #ReturnComp

fi #ReturnChange
