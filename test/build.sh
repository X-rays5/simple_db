git submodule update --init --recursive
cmake CMakeLists.txt
cmake --build . --target simple_db
read -n 1 -s -r -p "Press any key to continue..."