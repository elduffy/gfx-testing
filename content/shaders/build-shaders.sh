set -euo pipefail

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

for filename in "$SCRIPT_DIR"/src/*.vert.hlsl; do
    if [ -f "$filename" ]; then
        outfile=${filename/.hlsl/.spv}
        outfile=${outfile/src/compiled\/spirv}
        shadercross "$filename" -o "$outfile"
    fi
done

for filename in "$SCRIPT_DIR"/src/*.frag.hlsl; do
    if [ -f "$filename" ]; then
        outfile=${filename/.hlsl/.spv}
        outfile=${outfile/src/compiled\/spirv}
        shadercross "$filename" -o "$outfile"
    fi
done

for filename in "$SCRIPT_DIR"/src/*.comp.hlsl; do
    if [ -f "$filename" ]; then
        outfile=${filename/.hlsl/.spv}
        outfile=${outfile/src/compiled\/spirv}
        shadercross "$filename" -o "$outfile"
    fi
done
