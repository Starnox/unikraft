#!/bin/bash
set -e

if [[ $# -ne 3 ]]; then
	echo "Usage: $0 [-b <binary_dump> | -c <console_text>] <build_dir>" 1>&2
	exit 1
fi

if [ -n "${CROSS_COMPILE}" ]; then
	GCOV=${CROSS_COMPILE}/gcov
else
	GCOV=$(which gcov)
fi

if [ -z "$GCOV" ]; then
	echo "$0: Could not find gcov" 1>&2
	exit 1
fi

SCRIPT_DIR=$(realpath $(dirname "$0"))
CONVERTED_FILE=$(realpath dump.mem)
PARSE_SCRIPT=$(realpath "$SCRIPT_DIR"/gcov_serial_parse.py)
OUTPUT_FILE=$(realpath "$2")
BUILD_DIR=$(realpath "$3")

cd "$SCRIPT_DIR"

mkdir -p objs
mkdir -p results

rm -rf objs/*
rm -rf results/*

find "$BUILD_DIR" -type d -name objs -prune -o -name \*.gcno -exec cp {} objs/ \;

lcov --gcov-tool "$CROSS_COMPILE"gcov --capture --initial \
     --directory objs/ -o results/baseline.info

if [[ $1 == "-c" ]]; then
	"$PARSE_SCRIPT" --filename "$OUTPUT_FILE" --output "$CONVERTED_FILE"
	gcov-tool merge-stream < "$CONVERTED_FILE"
else
	gcov-tool merge-stream < "$OUTPUT_FILE"
fi

find "$BUILD_DIR" -type d -name objs -prune -o -name \*.gcda -exec cp {} objs/ \;

echo

./lcov_newcoverage.sh
./lcov_combine_new_base.sh
./genhtml_report.sh

echo -e "\nlcov report in file://$(realpath results/html)/index.html\n"
