#/bin/bash
rm -rf build
mkdir -p build
cd build

EXECUTABLE_NAME=sdl_main
SO_NAME="game.so"

GAME_SOURCES="../src/game.cpp ../src/gl_rendering.cpp ../src/glad.c"
GAME_OBJS="game.o gl_rendering.o glad.o"
PLATFORM_SOURCES="../src/sdl_main.cpp"
PLATFORM_OBJS="sdl_main.o"
INCLUDE_DIR="../src/include"

OTHER_FLAGS="-DSTDOUT_DEBUG -DFIXED_GAME_MEMORY -DPLATFORM_GL_MAJOR_VERSION=3 -DPLATFORM_GL_MINOR_VERSION=3 -DASSETS_DIR=\"assets/\""""
COMMON_COMPILER_FLAGS="-c -Wall -I$INCLUDE_DIR"
PLATFORM_COMPILER_FLAGS="${COMMON_COMPILER_FLAGS}"
GAME_COMPILER_FLAGS="${COMMON_COMPILER_FLAGS} -fPIC"

COMMON_LINKER_FLAGS=""
PLATFORM_LINKER_FLAGS="${COMMON_LINKER_FLAGS} -lSDL2 -lSDL2_image"
GAME_LINKER_FLAGS="${COMMON_LINKER_FLAGS} -shared"

echo "compiling platform"
for src in ${PLATFORM_SOURCES}; do
    echo "  $src"
    g++ ${src} ${PLATFORM_COMPILER_FLAGS} ${OTHER_FLAGS} || exit 1
done

echo "compiling game"
for src in ${GAME_SOURCES}; do
    echo "  $src"
    g++ ${src} ${GAME_COMPILER_FLAGS} ${OTHER_FLAGS} || exit 1
done

echo "linking"
g++ ${PLATFORM_OBJS} ${PLATFORM_LINKER_FLAGS} -o ${EXECUTABLE_NAME}
g++ ${GAME_OBJS} ${GAME_LINKER_FLAGS} -o ${SO_NAME}
echo "done"


cd ..
mv build/${EXECUTABLE_NAME} .
mv build/${SO_NAME} .
