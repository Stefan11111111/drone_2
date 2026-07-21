#!/usr/bin/env bash

set -euo pipefail

script_under_test="${1:?Pass the run-clang-tidy.sh path}"
fixture_dir="$(mktemp -d)"
trap 'rm -rf -- "${fixture_dir}"' EXIT

mkdir -p "${fixture_dir}/scripts" "${fixture_dir}/build/development" "${fixture_dir}/bin"
cp "${script_under_test}" "${fixture_dir}/scripts/run-clang-tidy.sh"
chmod +x "${fixture_dir}/scripts/run-clang-tidy.sh"
touch "${fixture_dir}/build/development/compile_commands.json"

cat >"${fixture_dir}/bin/clang-tidy-21" <<'EOF'
#!/usr/bin/env bash
set -euo pipefail
printf '%s\n' "$@" >"${CLANG_TIDY_TEST_OUTPUT:?}"
EOF
chmod +x "${fixture_dir}/bin/clang-tidy-21"

printf 'int changed() { return 0; }\n' >"${fixture_dir}/changed.cpp"
printf 'int unchanged() { return 0; }\n' >"${fixture_dir}/unchanged.cpp"
printf 'documentation\n' >"${fixture_dir}/notes.md"

git -C "${fixture_dir}" init --quiet
git -C "${fixture_dir}" config user.email "verification@example.invalid"
git -C "${fixture_dir}" config user.name "Verification Workflow"
git -C "${fixture_dir}" add changed.cpp unchanged.cpp notes.md
git -C "${fixture_dir}" -c commit.gpgsign=false commit --quiet -m "fixture baseline"

baseline_output="$(PATH="${fixture_dir}/bin:${PATH}" \
    "${fixture_dir}/scripts/run-clang-tidy.sh")"
if [[ "${baseline_output}" != "No changed C++ source files found relative to HEAD." ]]; then
    echo "Unexpected clean-worktree output: ${baseline_output}" >&2
    exit 1
fi

printf 'int changed() { return 1; }\n' >"${fixture_dir}/changed.cpp"
printf 'int staged() { return 0; }\n' >"${fixture_dir}/staged.cpp"
printf 'int untracked() { return 0; }\n' >"${fixture_dir}/untracked.cpp"
printf 'updated documentation\n' >"${fixture_dir}/notes.md"
git -C "${fixture_dir}" add staged.cpp

arguments_file="${fixture_dir}/clang-tidy-arguments.txt"
PATH="${fixture_dir}/bin:${PATH}" CLANG_TIDY_TEST_OUTPUT="${arguments_file}" \
    "${fixture_dir}/scripts/run-clang-tidy.sh"
mapfile -t arguments <"${arguments_file}"

contains_argument() {
    local expected="$1"
    local argument
    for argument in "${arguments[@]}"; do
        if [[ "${argument}" == "${expected}" ]]; then
            return 0
        fi
    done
    return 1
}

for expected in changed.cpp staged.cpp untracked.cpp; do
    if ! contains_argument "${expected}"; then
        echo "Changed-files mode omitted ${expected}" >&2
        exit 1
    fi
done
if contains_argument unchanged.cpp; then
    echo "Changed-files mode unexpectedly selected unchanged.cpp" >&2
    exit 1
fi

PATH="${fixture_dir}/bin:${PATH}" CLANG_TIDY_TEST_OUTPUT="${arguments_file}" \
    "${fixture_dir}/scripts/run-clang-tidy.sh" --all
mapfile -t arguments <"${arguments_file}"
for expected in changed.cpp staged.cpp unchanged.cpp untracked.cpp; do
    if ! contains_argument "${expected}"; then
        echo "Full-project mode omitted ${expected}" >&2
        exit 1
    fi
done
