#!/bin/sh
TARGET="app"
CFLAGS="-std=c11"
build() {

	cc "$CFLAGS" ./*/*.c -o $TARGET
}
build_debug() {

	cc "$CFLAGS" -g  -O1 -fsanitize=address -fno-omit-frame-pointer -Wall -Wextra ./*/*.c -o $TARGET
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
		./$TARGET
		;;
	debug)
		build_debug
		;;
	*)
		echo "Error: Unknown command '$ACTION'"
		echo "Usage: $0 [build|run|debug]"
		;;

esac
