#!/usr/bin/env bash

set -euo pipefail

repository_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
cd "${repository_dir}"

mapfile -d '' -t source_files < <(
    git ls-files -z --cached --others --exclude-standard -- \
        '*.c' '*.cc' '*.cpp' '*.cxx' '*.h' '*.hh' '*.hpp' '*.hxx'
)

if ((${#source_files[@]} == 0)); then
    echo "No C or C++ files found."
    exit 0
fi

case "${1:---check}" in
    --check)
        clang-format-21 --dry-run --Werror "${source_files[@]}"
        ;;
    --write)
        clang-format-21 -i "${source_files[@]}"
        ;;
    *)
        echo "Usage: $0 [--check|--write]" >&2
        exit 2
        ;;
esac
