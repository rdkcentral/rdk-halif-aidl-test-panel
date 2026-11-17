#!/bin/bash
set -euo pipefail

RUN_DIR="$(cd "$(dirname "$0")" && pwd)"
if [ -x "${RUN_DIR}/vts_panel_hal_test" ]; then
    BIN_DIR="${RUN_DIR}"
else
    BIN_DIR="${RUN_DIR}/build/bin"
fi
export LD_LIBRARY_PATH="${BIN_DIR}:${LD_LIBRARY_PATH:-}"
cd "${BIN_DIR}"

for required_file in vts_panel_hal_test libpanel-vcurrent-cpp.so \
    libvideodecoder-vcurrent-cpp.so libcommon-vcurrent-cpp.so; do
    if [ ! -e "${BIN_DIR}/${required_file}" ]; then
        echo "Missing packaged runtime file: ${BIN_DIR}/${required_file}" >&2
        exit 1
    fi
done

if [[ "${USE_MOCK_SERVICE:-0}" == "1" ]]; then
    if ! pgrep -f "servicemanager" >/dev/null; then
        "${BIN_DIR}/servicemanager" &
        service_manager_pid=$!
        sleep 1
    fi

    if ! pgrep -f "MockPanelService" >/dev/null; then
        "${BIN_DIR}/MockPanelService" &
        mock_service_pid=$!
        sleep 1
    fi

    trap 'kill "${mock_service_pid:-}" 2>/dev/null || true; kill "${service_manager_pid:-}" 2>/dev/null || true' EXIT
fi

"${BIN_DIR}/vts_panel_hal_test" "$@"
