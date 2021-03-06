#
# Copyright (c) 2014, 2018, Oracle and/or its affiliates. All rights reserved.
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# This code is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License version 3 only, as
# published by the Free Software Foundation.
#
# This code is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# version 3 for more details (a copy is included in the LICENSE file that
# accompanied this code).
#
# You should have received a copy of the GNU General Public License version
# 3 along with this work; if not, write to the Free Software Foundation,
# Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
#
# Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
# or visit www.oracle.com if you need additional information or have any
# questions.
#

# Builds libR

ifneq (,$(wildcard $(TOPDIR)/platform.mk))
include $(TOPDIR)/platform.mk
else
ifneq ($(MAKECMDGOALS),clean)
$(error no platform.mk available)
endif
endif

.PHONY: all clean

C_LIBNAME := libR$(DYLIB_EXT)
C_LIB := $(FASTR_LIB_DIR)/$(C_LIBNAME)
R_LIBNAME := libR$(DYLIB_EXT)
R_LIBNAME_DUMMY := libR_dummy$(DYLIB_EXT)
R_LIB := $(FASTR_LIB_DIR)/$(R_LIBNAME)
R_LIB_DUMMY := $(FASTR_LIB_DIR)/$(R_LIBNAME_DUMMY)

ifeq ($(OS_NAME), Darwin)
VERSION_FLAGS := -current_version $(R_VERSION) -compatibility_version $(R_VERSION)
endif

BLAS_TARGET := $(FASTR_LIB_DIR)/libRblas$(DYLIB_EXT)
LAPACK_TARGET := $(FASTR_LIB_DIR)/libRlapack$(DYLIB_EXT)

all: $(R_LIB)

# use sentinels to avoid (usually unnecessary) rebuilds.
# N.B. if things change in the subdirs, a clean must be invoked
# to remove the sentinels

$(R_LIB): fficall.done
ifeq ($(FASTR_RFFI),managed)
	# nop
else
ifeq ($(OS_NAME),Darwin)
	FASTR_RFFI="nfi-only" $(DYLIB_LD) $(DYLIB_LDFLAGS) -Wl,-rpath,@loader_path/ -o $(R_LIB) $(wildcard lib/*.o) -L$(FASTR_LIB_DIR) -ldl -lRblas -lRlapack  -lpcre -lz $(VERSION_FLAGS)
ifeq ($(FASTR_RFFI),llvm)
	FASTR_RFFI="nfi-only" $(DYLIB_LD) $(DYLIB_LDFLAGS) -Wl,-rpath,@loader_path/ -o $(R_LIB_DUMMY) -L$(FASTR_LIB_DIR) -ldl -lRblas -lRlapack  -lpcre -lz $(VERSION_FLAGS)
	FASTR_RFFI="llvm-only" $(DYLIB_LD) $(DYLIB_LDFLAGS) -Wl,-rpath,@loader_path/ -o $(R_LIB) $(wildcard lib/*.bc) -L$(FASTR_LIB_DIR) -ldl -lRblas -lRlapack  -lpcre -lz $(VERSION_FLAGS)
endif
	install_name_tool -change libRblas.dylib @rpath/libRblas.dylib $(R_LIB)
	install_name_tool -change libRlapack.dylib @rpath/libRlapack.dylib $(R_LIB)
	install_name_tool -id @rpath/libR.dylib $(R_LIB)
# check if we captured libpcre/libz, rpath those in libR
	mx rupdatelib $(FASTR_LIB_DIR)
else
	FASTR_RFFI="nfi-only" $(DYLIB_LD) $(DYLIB_LDFLAGS) $(shell echo $(PKG_LDFLAGS_OVERRIDE)) -Wl,-rpath,'$$ORIGIN' -o $(R_LIB) $(wildcard lib/*.o) -L$(FASTR_LIB_DIR) -lRblas -lRlapack -ldl -lpcre -lz
ifeq ($(FASTR_RFFI),llvm)
	FASTR_RFFI="nfi-only" $(DYLIB_LD) $(DYLIB_LDFLAGS) $(shell echo $(PKG_LDFLAGS_OVERRIDE)) -Wl,-rpath,'$$ORIGIN' -o $(R_LIB_DUMMY) -L$(FASTR_LIB_DIR) -lRblas -lRlapack -ldl -lpcre -lz
	FASTR_RFFI="llvm-only" $(DYLIB_LD) $(DYLIB_LDFLAGS) $(shell echo $(PKG_LDFLAGS_OVERRIDE)) -Wl,-rpath,'$$ORIGIN' -o $(R_LIB) $(wildcard lib/*.bc) -L$(FASTR_LIB_DIR) -lRblas -lRlapack -ldl -lpcre -lz
endif
endif
endif # managed

ifeq ($(FASTR_RFFI),managed)
fficall.done: common.done
	touch fficall.done
else
ifeq ($(FASTR_RFFI),nfi)
fficall.done: common.done
	$(MAKE) -C src/truffle_nfi all
	touch fficall.done

else
ifeq ($(FASTR_RFFI),llvm)
export R_PACKAGE_DIR="lib"
fficall.done: common.done
	$(MAKE) FASTR_RFFI="nfi-only" -C src/truffle_nfi all
	$(MAKE) FASTR_RFFI="llvm-only" -C src/truffle_llvm all
	touch fficall.done
else
	$(error unknown value for FASTR_RFFI)
endif #llvm
endif #nfi
endif #managed

common.done:
	echo "src/common FASTR_RFFI: $(FASTR_RFFI)"
	$(MAKE) -C src/common all
	touch common.done

clean:
	$(MAKE) -C src/common clean
ifeq ($(FASTR_RFFI),nfi)
	$(MAKE) -C src/truffle_nfi clean
else
ifeq ($(FASTR_RFFI),llvm)
	$(MAKE) -C src/truffle_nfi clean
	$(MAKE) -C src/truffle_llvm clean
else
ifneq ($(FASTR_RFFI),managed)
	$(error unknown value for FASTR_RFFI)
endif
endif
endif
	rm -rf $(R_LIB)
	rm -rf $(R_LIB_DUMMY)
	rm -rf "$(R_LIB)l"
	rm -rf fficall.done
	rm -rf common.done
