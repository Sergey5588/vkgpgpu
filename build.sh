#!/bin/sh
TARGET="app"
CFLAGS=" -std=c11 "
build() {

		cc $CFLAGS */*.c -o $TARGET
}


ACTION="$1"
if [[ -z "$ACTION" ]]; then
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
	*)
		echo "Error: Unknown command '$ACTION'"
		echo "Usage: $0 [build|run]"
		;;

esac
