#!/usr/bin/env bash

set -euo pipefail

script_under_test="${1:?Pass the run-demonstration.sh path}"
fixture_dir="$(mktemp -d)"
trap 'rm -rf -- "${fixture_dir}"' EXIT

mkdir -p "${fixture_dir}/scripts" "${fixture_dir}/build/development"
cp "${script_under_test}" "${fixture_dir}/scripts/run-demonstration.sh"
chmod +x "${fixture_dir}/scripts/run-demonstration.sh"

cat >"${fixture_dir}/fake-role" <<'EOF'
#!/usr/bin/env bash
set -euo pipefail

role="${0##*/}"
output_dir="${DRONE_DEMO_TEST_OUTPUT:?}"
printf '%s\n' "$@" >"${output_dir}/${role}.arguments"

on_signal() {
    touch "${output_dir}/${role}.terminated"
    exit 0
}
trap on_signal INT TERM
touch "${output_dir}/${role}.started"

if [[ "${DRONE_DEMO_TEST_FAIL_ROLE:-}" == "${role}" ]]; then
    while [[ ! -e "${output_dir}/observer.started" ||
        ! -e "${output_dir}/interceptor.started" ||
        ! -e "${output_dir}/console.started" ]]; do
        sleep 0.01
    done
    exit 7
fi

while true; do
    sleep 0.05
done
EOF
chmod +x "${fixture_dir}/fake-role"
for role in observer interceptor console; do
    cp "${fixture_dir}/fake-role" "${fixture_dir}/build/development/${role}"
done

wait_for_file() {
    local path="$1"
    local launcher_pid="$2"
    local deadline=$((SECONDS + 5))
    while [[ ! -e "${path}" ]]; do
        if ! kill -0 "${launcher_pid}" 2>/dev/null; then
            echo "Launcher exited before creating ${path}" >&2
            return 1
        fi
        if ((SECONDS >= deadline)); then
            echo "Timed out waiting for ${path}" >&2
            return 1
        fi
        sleep 0.05
    done
}

assert_arguments() {
    local path="$1"
    shift
    mapfile -t actual <"${path}"
    local expected=("$@")
    if ((${#actual[@]} != ${#expected[@]})); then
        echo "Unexpected argument count in ${path}: ${actual[*]}" >&2
        exit 1
    fi
    local index
    for ((index = 0; index < ${#expected[@]}; ++index)); do
        if [[ "${actual[index]}" != "${expected[index]}" ]]; then
            echo "Unexpected arguments in ${path}: ${actual[*]}" >&2
            exit 1
        fi
    done
}

signal_output="${fixture_dir}/signal-output"
mkdir -p "${signal_output}"
DRONE_DEMO_TEST_OUTPUT="${signal_output}" \
    "${fixture_dir}/scripts/run-demonstration.sh" --domain-id 88 \
    >"${signal_output}/launcher.log" 2>&1 &
launcher_pid=$!

for role in observer interceptor console; do
    wait_for_file "${signal_output}/${role}.started" "${launcher_pid}"
done

assert_arguments "${signal_output}/observer.arguments" \
    --domain-id 88 --participant-name demo_observer --tick-ms 250
assert_arguments "${signal_output}/interceptor.arguments" \
    --domain-id 88 --participant-name demo_interceptor --tick-ms 100
assert_arguments "${signal_output}/console.arguments" \
    --domain-id 88 --participant-name demo_console --tick-ms 50 --auto-pursuit

kill -TERM "${launcher_pid}"
if wait "${launcher_pid}"; then
    launcher_status=0
else
    launcher_status=$?
fi
if ((launcher_status != 143)); then
    echo "Expected signal-driven launcher exit 143, got ${launcher_status}" >&2
    exit 1
fi
for role in observer interceptor console; do
    if [[ ! -e "${signal_output}/${role}.terminated" ]]; then
        echo "Launcher did not terminate ${role}" >&2
        exit 1
    fi
done

failure_output="${fixture_dir}/failure-output"
mkdir -p "${failure_output}"
if DRONE_DEMO_TEST_OUTPUT="${failure_output}" DRONE_DEMO_TEST_FAIL_ROLE=observer \
    "${fixture_dir}/scripts/run-demonstration.sh" >"${failure_output}/launcher.log" 2>&1; then
    failure_status=0
else
    failure_status=$?
fi
if ((failure_status != 7)); then
    echo "Expected child-failure launcher exit 7, got ${failure_status}" >&2
    exit 1
fi
assert_arguments "${failure_output}/observer.arguments" \
    --domain-id 47 --participant-name demo_observer --tick-ms 250
for role in interceptor console; do
    if [[ ! -e "${failure_output}/${role}.terminated" ]]; then
        echo "Launcher did not clean up ${role} after observer failure" >&2
        exit 1
    fi
done

if "${fixture_dir}/scripts/run-demonstration.sh" --domain-id 233 \
    >"${fixture_dir}/invalid-domain.log" 2>&1; then
    invalid_domain_status=0
else
    invalid_domain_status=$?
fi
if ((invalid_domain_status != 2)); then
    echo "Expected invalid-domain exit 2, got ${invalid_domain_status}" >&2
    exit 1
fi
