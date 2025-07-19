xmake f -p windows -a x64 -m releasedbg --toolchain=clang-cl
xmake project -k compile_commands ./build
xmake 