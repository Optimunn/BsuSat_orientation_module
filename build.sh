cmake -DCMAKE_BUILD_TYPE=Release -B build
cmake --build build
if [ -e build/BsuSat_orientation_module.uf2 ]
then
    echo "Build complete successfully\!"
else
    echo "Build failed\!"
fi
