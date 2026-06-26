#!/bin/sh
TARGET="app"
CFLAGS="-std=c11"
build() {

	cc "$CFLAGS" ./*/*.c -o $TARGET
}
build_debug() {

	cc "$CFLAGS" -g  -O1 -fsanitize=address -fno-omit-frame-pointer -Wall -Wextra ./*/*.c -o $TARGET
}
build_glsl() {
	glslangValidator -V --target-env vulkan1.3 -S comp shaders/test.comp -o shaders/test.comp.spv
}


ACTION="$1"
if [ -z "$ACTION" ]; then
	ACTION="build"
fi

case "$ACTION" in
	build)
		build
		;;
	run)
		build
		build_glsl
		./$TARGET
		;;
	debug)
		build_debug
		;;
	glsl)
		build_glsl
		;;
	*)
		echo "Error: Unknown command '$ACTION'"
		echo "Usage: $0 [build|run|debug|glsl]"
		;;

esac
