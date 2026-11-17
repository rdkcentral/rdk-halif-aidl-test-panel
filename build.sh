#!/bin/bash
#* ******************************************************************************
#*  If not stated otherwise in this file or this component's LICENSE
#*  file the following copyright and licenses apply:
#*
#*  Copyright 2026 RDK Management
#*
#*  Licensed under the Apache License, Version 2.0 (the "License");
#*  you may not use this file except in compliance with the License.
#*  You may obtain a copy of the License at
#*
#*  http://www.apache.org/licenses/LICENSE-2.0
#*
#*  Unless required by applicable law or agreed to in writing, software
#*  distributed under the License is distributed on an AS IS BASIS,
#*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#*  See the License for the specific language governing permissions and
#*  limitations under the License.
#*
#* ******************************************************************************

set -euo pipefail

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
BOLD='\033[1m'
RESET='\033[0m'

declare -r TOP="$(cd "$(dirname "$0")" && pwd)"
declare -r UT_CORE_DIR="${TOP}/ut-core"
declare -r BUILD_DIR="${TOP}/build"
declare -r BUILD_INSTALL_PREFIX="${BUILD_DIR}/usr"
declare -r INSTALL_PATH="${BUILD_DIR}/bin"
declare -r MOCK_SERVICE_DIR="${TOP}/mockservice"
declare -r RDK_HAL_DIR="${TOP}/rdk-halif-aidl"
declare -r HALIF_OUT_DIR="${RDK_HAL_DIR}/out"
declare -r HALIF_LIB_DIR="${HALIF_OUT_DIR}/target/lib/rdk-halif-aidl"
declare -r HALIF_INCLUDE_DIR="${HALIF_OUT_DIR}/build/include"
declare -r DEFAULT_BINDER_TOOLCHAIN_ROOT="${RDK_HAL_DIR}/build-tools/linux_binder_idl"

RDK_HALIF_AIDL_VERSION="${RDK_HALIF_AIDL_VERSION:-main}"
UT_CORE_VERSION="${UT_CORE_VERSION:-}"
TARGET="${TARGET:-linux}"
BINDER_SOURCE_DIR="${BINDER_SOURCE_DIR:-${BINDER_TOOLCHAIN_ROOT:-${DEFAULT_BINDER_TOOLCHAIN_ROOT}}}"
BINDER_TOOLCHAIN_ROOT="${BINDER_TOOLCHAIN_ROOT:-${BINDER_SOURCE_DIR}}"
BINDER_SDK_DIR="${BINDER_SDK_DIR:-${HALIF_OUT_DIR}/target}"
BINDER_SDK_INCLUDE_DIR="${BINDER_SDK_INCLUDE_DIR:-${HALIF_OUT_DIR}/build}"
LINUX_BINDER_OUT_DIR="${LINUX_BINDER_OUT_DIR:-}"
export BINDER_SOURCE_DIR BINDER_TOOLCHAIN_ROOT BINDER_SDK_DIR BINDER_SDK_INCLUDE_DIR

declare -a HALIF_COMPONENTS=(common videodecoder panel)

checkout_halif_repo()
{
    if [ -x "${RDK_HAL_DIR}/build_binder.sh" ] &&
       [ -x "${RDK_HAL_DIR}/build_interfaces.sh" ]; then
        echo -e "${BOLD}${GREEN}Using existing HALIF source tree: ${RDK_HAL_DIR}${RESET}"
        return
    fi
    if [ -e "${RDK_HAL_DIR}" ]; then
        echo -e "${BOLD}${RED}Existing HALIF path is incomplete: ${RDK_HAL_DIR}${RESET}"
        echo "Remove or repair it before rebuilding."
        exit 1
    fi
    git clone --branch "${RDK_HALIF_AIDL_VERSION}" \
        https://github.com/rdkcentral/rdk-halif-aidl.git "${RDK_HAL_DIR}"
}

setup_binder_sdk()
{
    local aidl_ops="${BINDER_TOOLCHAIN_ROOT}/host/aidl_ops.py"
    local host_aidl="${BINDER_TOOLCHAIN_ROOT}/out/host/bin/aidl"
    local host_aidl_builder="${BINDER_TOOLCHAIN_ROOT}/build-aidl-generator-tool.sh"
    local sdk_marker="${BINDER_SDK_DIR}/.sdk_ready"

    if [ -n "${LINUX_BINDER_OUT_DIR}" ]; then
        if [ ! -d "${LINUX_BINDER_OUT_DIR}/lib" ] ||
           [ ! -d "${LINUX_BINDER_OUT_DIR}/include" ]; then
            echo -e "${BOLD}${RED}Invalid LINUX_BINDER_OUT_DIR: ${LINUX_BINDER_OUT_DIR}${RESET}"
            echo "Expected lib/ and include/ directories."
            exit 1
        fi

        echo -e "${BOLD}${GREEN}Staging target-compatible Binder SDK from ${LINUX_BINDER_OUT_DIR}...${RESET}"
        rm -rf "${BINDER_SDK_DIR}/lib/binder" \
            "${BINDER_SDK_INCLUDE_DIR}/include/binder_sdk"
        install -d "${BINDER_SDK_DIR}/lib/binder" \
            "${BINDER_SDK_INCLUDE_DIR}/include/binder_sdk"
        cp -a "${LINUX_BINDER_OUT_DIR}/lib/." "${BINDER_SDK_DIR}/lib/binder/"
        cp -a "${LINUX_BINDER_OUT_DIR}/include/." \
            "${BINDER_SDK_INCLUDE_DIR}/include/binder_sdk/"
        if [ -d "${LINUX_BINDER_OUT_DIR}/bin" ]; then
            install -d "${BINDER_SDK_DIR}/bin"
            cp -a "${LINUX_BINDER_OUT_DIR}/bin/." "${BINDER_SDK_DIR}/bin/"
        fi
        touch "${sdk_marker}" "${BINDER_SDK_INCLUDE_DIR}/.sdk_ready"
    fi

    if [ -f "${aidl_ops}" ] && [ -x "${host_aidl}" ] && [ -f "${sdk_marker}" ]; then
        echo -e "${BOLD}${GREEN}Binder toolchain and SDK are already staged.${RESET}"
        return
    fi

    if [ -d "${BINDER_TOOLCHAIN_ROOT}" ] &&
       [ ! -d "${BINDER_TOOLCHAIN_ROOT}/.git" ] &&
       [ ! -f "${aidl_ops}" ]; then
        rm -rf "${BINDER_TOOLCHAIN_ROOT}"
    fi

    if [ ! -f "${sdk_marker}" ]; then
        echo -e "${BOLD}${GREEN}Bootstrapping Binder toolchain and SDK...${RESET}"
        (
            cd "${RDK_HAL_DIR}"
            BINDER_SOURCE_DIR="${BINDER_SOURCE_DIR}" \
            BINDER_TOOLCHAIN_ROOT="${BINDER_TOOLCHAIN_ROOT}" \
            BINDER_SDK_DIR="${BINDER_SDK_DIR}" \
            BINDER_SDK_INCLUDE_DIR="${BINDER_SDK_INCLUDE_DIR}" \
                ./build_binder.sh
        )
    fi

    if [ ! -x "${host_aidl}" ]; then
        if [ ! -x "${host_aidl_builder}" ]; then
            echo -e "${BOLD}${RED}AIDL host-tool builder is missing: ${host_aidl_builder}${RESET}"
            exit 1
        fi
        echo -e "${BOLD}${GREEN}Building the AIDL host generator...${RESET}"
        "${host_aidl_builder}"
    fi

    if [ ! -f "${aidl_ops}" ]; then
        echo -e "${BOLD}${RED}Binder toolchain bootstrap did not create ${aidl_ops}.${RESET}"
        exit 1
    fi
    if [ ! -x "${host_aidl}" ]; then
        echo -e "${BOLD}${RED}Binder bootstrap did not create the AIDL host tool: ${host_aidl}${RESET}"
        exit 1
    fi
    if [ ! -f "${sdk_marker}" ]; then
        echo -e "${BOLD}${RED}Binder SDK bootstrap did not create ${sdk_marker}.${RESET}"
        exit 1
    fi
}

build_halif_modules()
{
    local component

    for component in "${HALIF_COMPONENTS[@]}"; do
        echo -e "${BOLD}${GREEN}Building HALIF component: ${component}${RESET}"
        (
            cd "${RDK_HAL_DIR}"
            ./build_interfaces.sh "${component}" --version current
        )
    done
}

find_component_library()
{
    local component="$1"
    local library

    library="${HALIF_LIB_DIR}/lib${component}-vcurrent-cpp.so"
    if [ ! -f "${library}" ]; then
        echo -e "${BOLD}${RED}Missing generated current ${component} HALIF library: ${library}${RESET}" >&2
        return 1
    fi
    printf '%s\n' "${library}"
}

create_halif_compatibility_files()
{
    local component
    local library
    local version
    local generated_mk="${HALIF_OUT_DIR}/halif_generated.mk"

    mkdir -p "${HALIF_LIB_DIR}"
    {
        echo "# Auto-generated by rdk-halif-aidl-test-panel/build.sh"
        echo -n "HALIF_INC_DIRS :="
        for component in "${HALIF_COMPONENTS[@]}"; do
            local include_dir
            include_dir="${HALIF_INCLUDE_DIR}/${component}/current/include"
            if [ ! -d "${include_dir}" ]; then
                echo -e "${BOLD}${RED}Missing generated current ${component} headers: ${include_dir}${RESET}" >&2
                return 1
            fi
            echo -n " ${include_dir}"
        done
        echo
        echo -n "HALIF_MODULE_LIBS :="
        for component in "${HALIF_COMPONENTS[@]}"; do
            library="$(find_component_library "${component}")"
            version="$(basename "${library}")"
            version="${version#lib${component}-v}"
            version="${version%-cpp.so}"
            ln -sfn "$(basename "${library}")" \
                "${HALIF_LIB_DIR}/lib${component}-cpp_vts.so"
            echo -n " -l${component}-cpp_vts"
        done
        echo
    } > "${generated_mk}"
}

install_linux_binder()
{
    local binder_lib_dir="${HALIF_OUT_DIR}/target/lib/binder"
    local binder_include_dir="${HALIF_OUT_DIR}/build/include/binder_sdk"
    local binder_bin_dir="${HALIF_OUT_DIR}/target/bin"

    if [ ! -d "${binder_lib_dir}" ] || [ ! -d "${binder_include_dir}" ]; then
        echo -e "${BOLD}${RED}Binder SDK was not generated by build_interfaces.sh.${RESET}"
        exit 1
    fi

    install -d "${BUILD_INSTALL_PREFIX}/bin" "${BUILD_INSTALL_PREFIX}/lib" \
        "${BUILD_INSTALL_PREFIX}/include"
    cp -a "${binder_lib_dir}/." "${BUILD_INSTALL_PREFIX}/lib/"
    cp -a "${binder_include_dir}/." "${BUILD_INSTALL_PREFIX}/include/"
    if [ -d "${binder_bin_dir}" ]; then
        cp -a "${binder_bin_dir}/." "${BUILD_INSTALL_PREFIX}/bin/"
    fi
}

build_ut_core()
{
    if [ ! -d "${UT_CORE_DIR}/.git" ]; then
        rm -rf "${UT_CORE_DIR}"
        git clone https://github.com/rdkcentral/ut-core.git "${UT_CORE_DIR}"
    fi

    (
        cd "${UT_CORE_DIR}"
        git fetch -q --tags origin
        if [ -z "${UT_CORE_VERSION}" ]; then
            UT_CORE_VERSION="$(git tag | sort -Vr | head -n1)"
        fi
        if [ -n "${UT_CORE_VERSION}" ]; then
            git checkout "${UT_CORE_VERSION}"
        fi
        ./build.sh TARGET="${TARGET}" VARIANT=CPP
    )
}

build_panel_test()
{
    rm -f "${TOP}"/src/*.o "${TOP}"/test/*.o "${MOCK_SERVICE_DIR}"/src/*.o
    make -j"$(nproc)" -C "${TOP}" TARGET="${TARGET}"
}

install_test()
{
    install -d "${INSTALL_PATH}"

    if compgen -G "${BUILD_INSTALL_PREFIX}/bin/*" > /dev/null; then
        install -m 755 "${BUILD_INSTALL_PREFIX}"/bin/* "${INSTALL_PATH}/"
    fi
    if compgen -G "${BUILD_INSTALL_PREFIX}/lib/*" > /dev/null; then
        install -m 755 "${BUILD_INSTALL_PREFIX}"/lib/* "${INSTALL_PATH}/"
    fi

    local component
    local library
    for component in "${HALIF_COMPONENTS[@]}"; do
        library="$(find_component_library "${component}")"
        install -m 755 "${library}" "${INSTALL_PATH}/$(basename "${library}")"
        ln -sfn "$(basename "${library}")" \
            "${INSTALL_PATH}/lib${component}-cpp_vts.so"
    done

    for component in "${HALIF_COMPONENTS[@]}"; do
        if [ ! -f "${INSTALL_PATH}/lib${component}-vcurrent-cpp.so" ]; then
            echo -e "${BOLD}${RED}Packaged current ${component} library is missing.${RESET}"
            exit 1
        fi
    done

    install -m 755 "${TOP}/run.sh" "${INSTALL_PATH}/"
    rm -rf "${INSTALL_PATH}/profiles"
    cp -a "${TOP}/profiles" "${INSTALL_PATH}/"

    tar -C "${INSTALL_PATH}" -czf "${BUILD_DIR}/vts-Panel.tar.gz" .
}

clean_test()
{
    rm -rf "${BUILD_DIR}"
    rm -f "${TOP}"/src/*.o "${TOP}"/test/*.o "${MOCK_SERVICE_DIR}"/src/*.o
    echo -e "${BOLD}${GREEN}Clean completed.${RESET}"
}

dist_clean_test()
{
    clean_test
    rm -rf "${RDK_HAL_DIR}" "${UT_CORE_DIR}"
    echo -e "${BOLD}${GREEN}Dist clean completed.${RESET}"
}

help()
{
    echo -e "${BOLD}${CYAN}Usage:${RESET}"
    echo -e "${BOLD}  $0 all                 ${RESET}${YELLOW}Build and package Panel VTS${RESET}"
    echo -e "${BOLD}  $0 TARGET=linux        ${RESET}${YELLOW}Build and package for Linux${RESET}"
    echo -e "${BOLD}  $0 clean               ${RESET}${YELLOW}Remove build output${RESET}"
    echo -e "${BOLD}  $0 dist_clean          ${RESET}${YELLOW}Remove build output and checkouts${RESET}"
    echo -e "${BOLD}  $0 help|-h|--help      ${RESET}${YELLOW}Show this help${RESET}"
}

validate_target()
{
    case "${TARGET}" in
        linux) ;;
        *)
            echo -e "${BOLD}${RED}Invalid target '${TARGET}'. This bootstrap supports linux.${RESET}"
            exit 2
            ;;
    esac
}

print_build_config()
{
    echo -e "${BOLD}${GREEN}VTS Version            : ${RESET}${YELLOW}$(git -C "${TOP}" describe --tags --always 2>/dev/null || echo unknown)${RESET}"
    echo -e "${BOLD}${GREEN}UT_CORE_VERSION        : ${RESET}${YELLOW}${UT_CORE_VERSION:-latest}${RESET}"
    echo -e "${BOLD}${GREEN}RDK_HALIF_AIDL_VERSION : ${RESET}${YELLOW}${RDK_HALIF_AIDL_VERSION}${RESET}"
    echo -e "${BOLD}${GREEN}TARGET                  : ${RESET}${YELLOW}${TARGET}${RESET}"
    echo -e "${BOLD}${GREEN}BINDER_TOOLCHAIN_ROOT   : ${RESET}${YELLOW}${BINDER_TOOLCHAIN_ROOT}${RESET}"
    echo -e "${BOLD}${GREEN}BINDER_SDK_DIR          : ${RESET}${YELLOW}${BINDER_SDK_DIR}${RESET}"
    echo -e "${BOLD}${GREEN}LINUX_BINDER_OUT_DIR    : ${RESET}${YELLOW}${LINUX_BINDER_OUT_DIR:-local bootstrap}${RESET}"
}

build_all()
{
    validate_target
    mkdir -p "${BUILD_DIR}"
    checkout_halif_repo
    setup_binder_sdk
    build_halif_modules
    create_halif_compatibility_files
    install_linux_binder
    build_ut_core
    build_panel_test
    install_test
    echo -e "${BOLD}${GREEN}Build completed: ${BUILD_DIR}/vts-Panel.tar.gz${RESET}"
}

command="${1:-help}"
if [[ "${command^^}" == TARGET=* ]]; then
    TARGET="${command#*=}"
    command="all"
fi

print_build_config

case "${command}" in
    all)
        build_all
        ;;
    clean)
        clean_test
        ;;
    dist_clean)
        dist_clean_test
        ;;
    help|-h|--help)
        help
        ;;
    *)
        echo -e "${BOLD}${RED}Invalid argument: ${command}${RESET}"
        help
        exit 2
        ;;
esac
