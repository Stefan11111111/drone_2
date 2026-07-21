#!/usr/bin/env bash

set -euo pipefail

repository_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
binary_dir="${repository_dir}/build/development"
domain_id=47
domain_id_seen=false
child_pids=()
shutdown_started=false

usage() {
    echo "Usage: $0 [--domain-id ID]"
}

configuration_error() {
    echo "run-demonstration: $1" >&2
    usage >&2
    exit 2
}

parse_arguments() {
    while (($# > 0)); do
        case "$1" in
            --domain-id)
                if [[ "${domain_id_seen}" == true ]]; then
                    configuration_error "--domain-id may be specified only once"
                fi
                if (($# < 2)); then
                    configuration_error "--domain-id requires a value"
                fi
                domain_id="$2"
                domain_id_seen=true
                shift 2
                ;;
            --help)
                usage
                exit 0
                ;;
            *)
                configuration_error "unknown option '$1'"
                ;;
        esac
    done

    if [[ ! "${domain_id}" =~ ^[0-9]+$ ]] || ((${#domain_id} > 3)) ||
        ((10#${domain_id} > 232)); then
        configuration_error "--domain-id must be an integer from 0 through 232"
    fi
    domain_id="$((10#${domain_id}))"
}

require_executables() {
    local role
    for role in observer interceptor console; do
        if [[ ! -x "${binary_dir}/${role}" ]]; then
            echo "run-demonstration: missing executable ${binary_dir}/${role}" >&2
            echo "Build the demonstration with: cmake --build --preset development" >&2
            exit 2
        fi
    done
}

child_is_running() {
    kill -0 "$1" 2>/dev/null
}

shutdown_children() {
    if [[ "${shutdown_started}" == true ]]; then
        return
    fi
    shutdown_started=true

    local pid
    local signalled=false
    for pid in "${child_pids[@]}"; do
        if child_is_running "${pid}"; then
            if [[ "${signalled}" == false ]]; then
                echo "run-demonstration: stopping all roles" >&2
                signalled=true
            fi
            kill -TERM "${pid}" 2>/dev/null || true
        fi
    done

    local deadline=$((SECONDS + 5))
    while ((SECONDS < deadline)); do
        local running=false
        for pid in "${child_pids[@]}"; do
            if child_is_running "${pid}"; then
                running=true
                break
            fi
        done
        if [[ "${running}" == false ]]; then
            break
        fi
        sleep 0.05
    done

    for pid in "${child_pids[@]}"; do
        if child_is_running "${pid}"; then
            echo "run-demonstration: process ${pid} did not stop; sending SIGKILL" >&2
            kill -KILL "${pid}" 2>/dev/null || true
        fi
    done
    for pid in "${child_pids[@]}"; do
        wait "${pid}" 2>/dev/null || true
    done
}

on_exit() {
    local status=$?
    trap - EXIT INT TERM
    shutdown_children
    exit "${status}"
}

start_role() {
    local role="$1"
    shift
    echo "run-demonstration: starting ${role}"
    "$@" &
    child_pids+=("$!")
}

parse_arguments "$@"
require_executables

trap on_exit EXIT
trap 'exit 130' INT
trap 'exit 143' TERM

echo "run-demonstration: DDS domain ${domain_id}; press Ctrl-C to stop all roles"
start_role observer \
    "${binary_dir}/observer" \
    --domain-id "${domain_id}" \
    --participant-name demo_observer \
    --tick-ms 250
start_role interceptor \
    "${binary_dir}/interceptor" \
    --domain-id "${domain_id}" \
    --participant-name demo_interceptor \
    --tick-ms 100
start_role console \
    "${binary_dir}/console" \
    --domain-id "${domain_id}" \
    --participant-name demo_console \
    --tick-ms 50 \
    --auto-pursuit

if wait -n "${child_pids[@]}"; then
    child_status=0
else
    child_status=$?
fi
echo "run-demonstration: a role exited with status ${child_status}" >&2
exit "${child_status}"
