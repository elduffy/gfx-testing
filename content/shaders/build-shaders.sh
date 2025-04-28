#!/bin/bash
set -euo pipefail

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
mkdir -p "$SCRIPT_DIR"/compiled/spirv
rm -f "$SCRIPT_DIR"/compiled/spirv/*.spv
rm -f "$SCRIPT_DIR"/compiled/spirv/*.json

for filename in "$SCRIPT_DIR"/src/*.vert.hlsl; do
    if [ -f "$filename" ]; then
        spv_outfile=${filename/.hlsl/.spv}
        spv_outfile=${spv_outfile/src/compiled\/spirv}
        shadercross "$filename" -o "$spv_outfile" -I "$SCRIPT_DIR/include"
        json_outfile=${spv_outfile/.spv/.json}
        spirv-cross "$spv_outfile" --reflect --output "$json_outfile"
    fi
done

for filename in "$SCRIPT_DIR"/src/*.frag.hlsl; do
    if [ -f "$filename" ]; then
        spv_outfile=${filename/.hlsl/.spv}
        spv_outfile=${spv_outfile/src/compiled\/spirv}
        shadercross "$filename" -o "$spv_outfile" -I "$SCRIPT_DIR/include"
        json_outfile=${spv_outfile/.spv/.json}
        spirv-cross "$spv_outfile" --reflect --output "$json_outfile"
    fi
done

for filename in "$SCRIPT_DIR"/src/*.comp.hlsl; do
    if [ -f "$filename" ]; then
        spv_outfile=${filename/.hlsl/.spv}
        spv_outfile=${spv_outfile/src/compiled\/spirv}
        shadercross "$filename" -o "$spv_outfile" -I "$SCRIPT_DIR/include"
        json_outfile=${spv_outfile/.spv/.json}
        spirv-cross "$spv_outfile" --reflect --output "$json_outfile"
    fi
done
