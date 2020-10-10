#/bin/bash
rm -rf build
mkdir -p build
cd build

EXECUTABLE_NAME=sim

SRC_DIR="../src"
INCLUDE_DIR="../src/include"
GAME_SRCS="game.cpp gl_rendering.cpp glad.c math.cpp"
GAME_OBJS="game.o gl_rendering.o glad.o math.o"
PLATFORM_SOURCES="sdl_main.cpp"
PLATFORM_OBJS="sdl_main.o"

OTHER_FLAGS="-DSTDOUT_DEBUG -DFIXED_GAME_MEMORY -DPLATFORM_GL_MAJOR_VERSION=3 -DPLATFORM_GL_MINOR_VERSION=3 -DASSETS_DIR=\"assets/\""""
COMPILER_FLAGS="-c -Wall -I$INCLUDE_DIR -fPIC"

LINKER_FLAGS="-lSDL2 -ldl" # -lSDL2_image

echo "compiling platform"
for src in ${PLATFORM_SOURCES}; do
    echo "  $src"
    g++ ${SRC_DIR}/${src} ${COMPILER_FLAGS} ${OTHER_FLAGS} || exit 1
done

echo "compiling game"
for src in ${GAME_SRCS}; do
    echo "  $src"
    g++ ${SRC_DIR}/${src} ${COMPILER_FLAGS} ${OTHER_FLAGS} || exit 1
done

echo "linking"
g++ ${PLATFORM_OBJS} ${GAME_OBJS} ${LINKER_FLAGS} -o ${EXECUTABLE_NAME} || exit 1
echo "done"


cd ..
mv build/${EXECUTABLE_NAME} .
