#!/usr/bin/env bash

set -euo pipefail

repository_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
build_dir="${repository_dir}/build/development"
compile_commands="${build_dir}/compile_commands.json"
source_patterns=('*.cc' '*.cpp' '*.cxx')

print_usage() {
    echo "usage: $0 [--all]"
}

analyze_all=false
if (($# > 1)); then
    print_usage >&2
    exit 2
fi
if (($# == 1)); then
    case "$1" in
        --all)
            analyze_all=true
            ;;
        -h | --help)
            print_usage
            exit 0
            ;;
        *)
            print_usage >&2
            exit 2
            ;;
    esac
fi

if [[ ! -f "${compile_commands}" ]]; then
    echo "Missing ${compile_commands}; run 'cmake --preset development' first." >&2
    exit 1
fi

cd "${repository_dir}"
if [[ "${analyze_all}" == true ]]; then
    mapfile -d '' -t source_files < <(
        git ls-files -z --cached --others --exclude-standard -- "${source_patterns[@]}"
    )
else
    mapfile -d '' -t source_files < <(
        {
            git diff --name-only --diff-filter=ACMR -z HEAD -- "${source_patterns[@]}"
            git ls-files -z --others --exclude-standard -- "${source_patterns[@]}"
        } | sort -zu
    )
fi

if ((${#source_files[@]} == 0)); then
    if [[ "${analyze_all}" == true ]]; then
        echo "No C++ source files found."
    else
        echo "No changed C++ source files found relative to HEAD."
    fi
    exit 0
fi

clang-tidy-21 --quiet -p "${build_dir}" "${source_files[@]}"
