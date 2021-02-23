echo off

echo == This script generates a Visual Studio solution.
echo == For more advanced usage, invoke cmake directly
echo == For best results, run this script from a Visual Studio Developer Command Prompt

if not exist "build" (
    echo == Creating build directory: .\build
    mkdir build
)

echo == Generating Visual Studio solution
echo.

pushd build
cmake -DCMAKE_GENERATOR_PLATFORM=x64 ..
popd

echo == You can open the Visual Studio solution with the following command:
echo start build/GFNSDK.sln
echo.
