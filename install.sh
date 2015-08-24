#!/bin/sh

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
else
  ARCH="$(ls "${SOURCE}/out/${PLATFORM}" | head -n 1)"
fi

CONFIGURATION="$(ls "${SOURCE}/out/${PLATFORM}/${ARCH}" | grep -E "release|debug" | head -n 1)"

LIBDIR="${PREFIX}/lib"
INCLUDEDIR="${PREFIX}/include"

OCTB_STACK_DIR=resource/csdk/stack
OCTB_RANDOM_DIR=resource/csdk/ocrandom
OCTB_STACK_INCLUDEDIR=iotivity/${OCTB_STACK_DIR}/include
OCTB_RANDOM_INCLUDEDIR=iotivity/${OCTB_RANDOM_DIR}/include

# Compute where we will actually put the files. This part of the code honours DESTDIR. DESTDIR
# should not be used in the construction of any other variable such as LIBDIR or INCLUDEDIR.

ACTUAL_LIBDIR="${DESTDIR}/${LIBDIR##/}"
ACTUAL_INCLUDEDIR="${DESTDIR}/${INCLUDEDIR##/}"

# Paths have been established above. Copy the files and fill out the .pc file.

mkdir -p "${ACTUAL_LIBDIR}" || exit 1
if test "x${PLATFORM}x" = "xdarwinx"; then
  cp "${SOURCE}/out/${PLATFORM}/${ARCH}/${CONFIGURATION}"/*.a "${ACTUAL_LIBDIR}" || exit 1
else
  cp "${SOURCE}/out/${PLATFORM}/${ARCH}/${CONFIGURATION}/liboctbstack.so" "${ACTUAL_LIBDIR}" || exit 1
fi

mkdir -p "${ACTUAL_INCLUDEDIR}/iotivity/${OCTB_STACK_DIR}" || exit 1
cp -a "${SOURCE}/${OCTB_STACK_DIR}/include" "${ACTUAL_INCLUDEDIR}/${OCTB_STACK_INCLUDEDIR}" || exit 1

# FIXME - temporary solution for 0.9.2 - we need to touch logger.h
touch "${ACTUAL_INCLUDEDIR}/${OCTB_STACK_INCLUDEDIR}"/logger.h

mkdir -p "${ACTUAL_INCLUDEDIR}/iotivity/${OCTB_RANDOM_DIR}" || exit 1
cp -a "${SOURCE}/${OCTB_RANDOM_DIR}/include" "${ACTUAL_INCLUDEDIR}/${OCTB_RANDOM_INCLUDEDIR}" || exit 1

if test "x${NO_PC}x" = "xx"; then
  mkdir -p "${ACTUAL_LIBDIR}/pkgconfig"
  cat octbstack.pc.in | \
    sed \
      -e "s!@PREFIX@!${PREFIX}!g" \
      -e "s!@LIBDIR@!${LIBDIR}!g" \
      -e "s!@INCLUDEDIR@!${INCLUDEDIR}!g" \
      -e "s!@OCTB_STACK_INCLUDEDIR@!${OCTB_STACK_INCLUDEDIR}!g" \
      -e "s!@OCTB_RANDOM_INCLUDEDIR@!${OCTB_RANDOM_INCLUDEDIR}!g" \
    > "${ACTUAL_LIBDIR}/pkgconfig/octbstack.pc"
fi
