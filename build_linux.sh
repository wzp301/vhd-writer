BUILD_ROOT=build_linux
mkdir -p $BUILD_ROOT
mkdir -p $BUILD_ROOT/bin

cd ./$BUILD_ROOT
cmake -DTAG_LINUX=1 -DCMAKE_VERBOSE_MAKEFILE=false ..
make -j8