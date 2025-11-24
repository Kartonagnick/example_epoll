
clear
echo "--------[make]"
rm -rf "../build"
mkdir "../build"
echo "--------[make]"

cmake -S"." -B"../build" -D"CMAKE_BUILD_TYPE=Release"
cmake --build "../build" --config Release
