#!/usr/bin/env bash

set -euo pipefail

repository_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
build_dir="${repository_dir}/build/development"
compile_commands="${build_dir}/compile_commands.json"

if [[ ! -f "${compile_commands}" ]]; then
    echo "Missing ${compile_commands}; run 'cmake --preset development' first." >&2
    exit 1
fi

cd "${repository_dir}"
mapfile -d '' -t source_files < <(
    git ls-files -z --cached --others --exclude-standard -- '*.cc' '*.cpp' '*.cxx'
)

if ((${#source_files[@]} == 0)); then
    echo "No C++ source files found."
    exit 0
fi

clang-tidy-21 --quiet -p "${build_dir}" "${source_files[@]}"
