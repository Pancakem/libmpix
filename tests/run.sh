#!/bin/sh
set -eu

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

cd "$PROJECT_ROOT"

PORTS="posix qemu_cortex_m55"

build_error=0
runtime_error=0
success=0

if [ $# -eq 0 ]; then
    set -- tests/*.on tests/*.off
fi


for test in "$@"; do
    test_name="$(basename "$test")"
    echo "=== Test: $test_name ==="

    for port in $PORTS; do
        printf '%s/%s: ' "$test_name" "$port"

        build_dir="$test/build-$port"
        mkdir -p "$build_dir"

        build_log="$test/build-$port.log"
        runtime_log="$test/runtime-$port.log"

        case "$port" in
            qemu_cortex_m55)
                TOOLCHAIN="$PROJECT_ROOT/ports/qemu_cortex_m55/toolchain-arm-cortex-m55.cmake"
                PORT_SRC="$PROJECT_ROOT/ports/qemu_cortex_m55"

                if ! cmake \
                    -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN" \
                    -S "$PORT_SRC" \
                    -B "$build_dir" >"$build_log" 2>&1 \
                   || ! cmake --build "$build_dir" >>"$build_log" 2>&1; then
                    echo "Build error"
                    build_error=$((build_error + 1))
                    sed 's/^/    | /' "$build_log"
                    continue
                fi

                runtime_failed=0
                found_elf=0
                for elf in "$build_dir"/*.elf; do
                    [ -f "$elf" ] || continue
                    found_elf=1
                    qemu-system-arm -machine mps3-an547 -cpu cortex-m55 \
                        -kernel "$elf" -semihosting -nographic \
                        >"$runtime_log" 2>&1 &

                    QEMU_PID=$!
                    sleep 15

                    if kill -0 $QEMU_PID 2>/dev/null; then
                        kill -9 $QEMU_PID 2>/dev/null
                    fi

                    wait $QEMU_PID 2>/dev/null || runtime_failed=0

                    sed -i 's/^/  | /' "$runtime_log"
                done

                if [ $found_elf -eq 0 ]; then
                    echo "No ELF produced"
                    runtime_failed=1
                fi

                if [ $runtime_failed -eq 1 ]; then
                    echo "Runtime error"
                    runtime_error=$((runtime_error + 1))
                    sed 's/^/    | /' "$runtime_log"
                else
                    echo "Ok (QEMU)"
                    success=$((success + 1))
                fi
                ;;

            posix)
                if ! cmake -S "$test" -B "$build_dir" >"$build_log" 2>&1 \
                   || ! cmake --build "$build_dir" >>"$build_log" 2>&1; then
                    echo "Build error"
                    build_error=$((build_error + 1))
                    sed 's/^/    | /' "$build_log"
                    continue
                fi

                if ! "$build_dir/libmpix_test" >"$runtime_log" 2>&1; then
                    echo "Runtime error"
                    runtime_error=$((runtime_error + 1))
                    sed 's/^/    | /' "$runtime_log"
                else
                    echo "Ok"
                    success=$((success + 1))
                fi
                ;;

            *)
                if ! cmake -S "$test" -B "$build_dir" >"$build_log" 2>&1 \
                   || ! cmake --build "$build_dir" >>"$build_log" 2>&1; then
                    echo "Build error"
                    build_error=$((build_error + 1))
                    sed 's/^/    | /' "$build_log"
                else
                    echo "Built (no runtime)"
                    success=$((success + 1))
                fi
                ;;
        esac
    done

    echo ""
done

echo "=== CI Summary ==="
echo "Build errors:   $build_error"
echo "Runtime errors: $runtime_error"
echo "Successes:      $success"

exit $((build_error + runtime_error))
