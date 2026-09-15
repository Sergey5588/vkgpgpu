#!/bin/sh
TARGET="test"
LIB="libvkgpgpu.so"
STATIC="vkgpgpu.a"

CFLAGS="-std=c11"

build_static() {
	cc $CFLAGS -DVKGPGPU_BUILD -c ./*/*.c
	ar rcs $STATIC *.o
	rm -f *.o

}
build_shared() {
	cc $CFLAGS -DVKGPGPU_BUILD -fPIC -shared ./*/*.c -o $LIB
}
build_example() {

	cc $CFLAGS ./*/*.c "./example/hello/main.c" -o $TARGET
}
build_debug() {

	cc $CFLAGS -DVKGPGPU_DEBUG -g  -O1 -fsanitize=address -fno-omit-frame-pointer -Wall -Wextra ./*/*.c "./example/hello/main.c" -o $TARGET
}
build_glsl() {
	glslangValidator -V --target-env vulkan1.3 -S comp shaders/test.comp -o shaders/test.comp.spv
}
usage() {
	echo "Usage: $0 [build|example|debug|glsl|shared|static|clean]"
}


ACTION="$1"
if [ -z "$ACTION" ]; then
	ACTION="example"
fi

case "$ACTION" in
	shared)
		build_shared
		;;
	static)
		build_static
		;;
	example)
		build_example
		;;
	run)
		build_example
		build_glsl
		./$TARGET
		;;
	debug)
		build_debug
		;;
	glsl)
		build_glsl
		;;
	clean)
		rm -f $TARGET
		rm -f $LIB
		rm -f $STATIC
		;; 
	help)
		usage
		;;
	*)
		echo "Error: Unknown command '$ACTION'"
		usage
		;;

esac
