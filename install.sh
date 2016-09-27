#!/bin/sh

# Copyright 2016 Intel Corporation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set -x

# Default prefix is /usr
if test "x${PREFIX}x" = "xx"; then
	PREFIX="/usr"
fi

# Default source root is the pwd
if test "x${SOURCE}x" = "xx"; then
	SOURCE="$(pwd)"
fi

# Make sure SOURCE, PREFIX, and DESTDIR do not end in a "/"
SOURCE="${SOURCE%%/}"
PREFIX="${PREFIX%%/}"
DESTDIR="${DESTDIR%%/}"

# Install the first platform/arch/configuration we find
PLATFORM="$(ls "${SOURCE}/out" | head -n 1)"

if test "${PLATFORM}" = "darwin"; then
  ARCH=x86_64
  PLATFORM_LIBS="-lconnectivity_abstraction -lcoap -lc_common -locsrm -lroutingmanager"
else
  ARCH="$(ls "${SOURCE}/out/${PLATFORM}" | head -n 1)"
  PLATFORM_LIBS=""
fi

CONFIGURATION="$(ls "${SOURCE}/out/${PLATFORM}/${ARCH}" | grep -E "release|debug" | head -n 1)"

LIBDIR="${PREFIX}/lib"
INCLUDEDIR="${PREFIX}/include"
SHAREDIR="${PREFIX}/share/iotivity"

OCTB_STACK_DIR=resource/csdk/stack
OCTB_CCOMMON_DIR=resource/c_common
OCTB_STACK_INCLUDEDIR=iotivity/${OCTB_STACK_DIR}/include
OCTB_CCOMMON_INCLUDEDIR=iotivity/${OCTB_CCOMMON_DIR}

# Compute where we will actually put the files. This part of the code honours DESTDIR. DESTDIR
# should not be used in the construction of any other variable such as LIBDIR or INCLUDEDIR.

ACTUAL_LIBDIR="${DESTDIR}/${LIBDIR##/}"
ACTUAL_INCLUDEDIR="${DESTDIR}/${INCLUDEDIR##/}"
ACTUAL_SHAREDIR="${DESTDIR}/${SHAREDIR##/}"

# Paths have been established above. Copy the files and fill out the .pc file.

mkdir -p "${ACTUAL_SHAREDIR}" || exit 1
cp "${SOURCE}/resource/csdk/security/provisioning/sample/oic_svr_db_server_justworks.json" ${ACTUAL_SHAREDIR}

mkdir -p "${ACTUAL_LIBDIR}" || exit 1
if test "x${PLATFORM}x" = "xdarwinx"; then
  cp "${SOURCE}/out/${PLATFORM}/${ARCH}/${CONFIGURATION}"/*.a "${ACTUAL_LIBDIR}" || exit 1
else
  cp "${SOURCE}/out/${PLATFORM}/${ARCH}/${CONFIGURATION}/liboctbstack.so" "${ACTUAL_LIBDIR}" || exit 1
  cp "${SOURCE}/out/${PLATFORM}/${ARCH}/${CONFIGURATION}/libconnectivity_abstraction.so" "${ACTUAL_LIBDIR}" || exit 1
fi

mkdir -p "${ACTUAL_INCLUDEDIR}/iotivity/${OCTB_STACK_DIR}" || exit 1
cp -a "${SOURCE}/${OCTB_STACK_DIR}/include" "${ACTUAL_INCLUDEDIR}/${OCTB_STACK_INCLUDEDIR}" || exit 1

# FIXME - temporary solution for 0.9.2 - we need to touch logger.h
touch "${ACTUAL_INCLUDEDIR}/${OCTB_STACK_INCLUDEDIR}"/logger.h

mkdir -p "${ACTUAL_INCLUDEDIR}/iotivity/${OCTB_CCOMMON_DIR}" || exit 1
cp -a "${SOURCE}/${OCTB_CCOMMON_DIR}/platform_features.h" "${ACTUAL_INCLUDEDIR}/${OCTB_CCOMMON_INCLUDEDIR}" || exit 1
cp -a "${SOURCE}/${OCTB_CCOMMON_DIR}/iotivity_config.h" "${ACTUAL_INCLUDEDIR}/${OCTB_CCOMMON_INCLUDEDIR}" || exit 1

if test "x${INSTALL_PC}x" = "xtruex"; then
  mkdir -p "${ACTUAL_LIBDIR}/pkgconfig"
  cat ./octbstack.pc.in | \
    sed \
      -e "s!@PREFIX@!${PREFIX}!g" \
      -e "s!@LIBDIR@!${LIBDIR}!g" \
      -e "s!@INCLUDEDIR@!${INCLUDEDIR}!g" \
      -e "s!@OCTB_STACK_INCLUDEDIR@!${OCTB_STACK_INCLUDEDIR}!g" \
      -e "s!@PLATFORM_LIBS@!${PLATFORM_LIBS}!g" \
      -e "s!@OCTB_CCOMMON_INCLUDEDIR@!${OCTB_CCOMMON_INCLUDEDIR}!g" \
    > "${ACTUAL_LIBDIR}/pkgconfig/octbstack.pc"
fi
