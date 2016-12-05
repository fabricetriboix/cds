# Copyright (c) 2016  Fabrice Triboix
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

SHELL := /bin/sh

# Check existence of tools
CCACHE := $(shell which ccache 2> /dev/null)
DOXYGEN := $(shell which doxygen 2> /dev/null)
DOT := $(shell which dot 2> /dev/null)

MODULES = $(TOPDIR)/src/plf/$(PLF) $(TOPDIR)/src/list \
			$(TOPDIR)/src/binarytree $(TOPDIR)/src/map

# Path for make to search for source files
VPATH = $(foreach i,$(MODULES),$(i)/src) $(foreach i,$(MODULES),$(i)/test) \
		$(TOPDIR)/src/cds_vs_stl/list $(TOPDIR)/src/cds_vs_stl/map

# Output libraries
OUTPUT_LIBS = libcds.a

# Include paths for compilation
INCS = $(foreach i,$(MODULES),-I$(i)/include)

# List public header files
HDRS = $(foreach i,$(MODULES),$(wildcard $(i)/include/*.h))

# List of object files for various targets
LIBCDS_OBJS = cdscommon.o cdslist.o cdsbinarytree.o cdsmap.o
RTTEST_MAIN_OBJ = rttestmain.o
CDS_TEST_OBJS = test-list.o test-binarytree.o test-map.o

# Libraries to link against when building test programs
LINKLIBS = -lcds -lrttest -lrtsys
ifeq ($(V),debug)
LINKLIBS += -lflloc
endif

# CDS vs STL executables
CDS_VS_STL = cdslistperf stllistperf cdsmapperf stlmapperf mkrnd


# Standard targets

all: $(OUTPUT_LIBS) doc cds_unit_tests $(CDS_VS_STL)

doc: doc/html/index.html


# Rules to build object files, libraries, programs, etc.

define RUN_CC
set -eu; \
cmd="$(CCACHE) $(CC) $(CFLAGS) $(INCS) -o $(1) -c $(2)"; \
if [ $(D) == 1 ]; then \
	echo "$$cmd"; \
else \
	echo "CC    $(1)"; \
fi; \
$$cmd || (echo "Command line was: $$cmd"; exit 1)
endef

define RUN_CXX
set -eu; \
cmd="$(CCACHE) $(CXX) $(CXXFLAGS) $(INCS) -o $(1) -c $(2)"; \
if [ $(D) == 1 ]; then \
	echo "$$cmd"; \
else \
	echo "CXX   $(1)"; \
fi; \
$$cmd || (echo "Command line was: $$cmd"; exit 1)
endef

define RUN_AR
set -eu; \
cmd="$(AR) crs $(1) $(2)"; \
if [ $(D) == 1 ]; then \
	echo "$$cmd"; \
else \
	echo "AR    $(1)"; \
fi; \
$$cmd || (echo "Command line was: $$cmd"; exit 1)
endef

define RUN_LINK
set -eu; \
cmd="$(CC) -L. $(LINKFLAGS) -o $(1) $(2) $(3)"; \
if [ $(D) == 1 ]; then \
	echo "$$cmd"; \
else \
	echo "LINK  $(1)"; \
fi; \
$$cmd || (echo "Command line was: $$cmd"; exit 1)
endef

libcds.a: $(LIBCDS_OBJS)

cds_unit_tests: $(CDS_TEST_OBJS) $(RTTEST_MAIN_OBJ) $(OUTPUT_LIBS)
	@$(call RUN_LINK,$@,$(filter %.o,$^),$(LINKLIBS))

cdslistperf: cdslistperf.o $(OUTPUT_LIBS)
	@$(call RUN_LINK,$@,$(filter %.o,$^),$(LINKLIBS))

stllistperf: stllistperf.o $(OUTPUT_LIBS)
	@$(call RUN_LINK,$@,$(filter %.o,$^),$(LINKLIBS) $(CXXLIB))

cdsmapperf: cdsmapperf.o $(OUTPUT_LIBS)
	@$(call RUN_LINK,$@,$(filter %.o,$^),$(LINKLIBS))

stlmapperf: stlmapperf.o $(OUTPUT_LIBS)
	@$(call RUN_LINK,$@,$(filter %.o,$^),$(LINKLIBS) $(CXXLIB))

mkrnd: mkrnd.o $(OUTPUT_LIBS)
	@$(call RUN_LINK,$@,$(filter %.o,$^),$(LINKLIBS) $(CXXLIB))


doc/html/index.html: $(filter-out %_private.h,$(HDRS))
ifeq ($(DOXYGEN),)
	@echo "Doxygen not found, documentation will not be built"
else
	@set -eu; \
	if [ -z "$(DOT)" ]; then \
		cp $(TOPDIR)/Doxyfile doxy1; \
	else \
		cat $(TOPDIR)/Doxyfile | sed -e 's/HAVE_DOT.*/HAVE_DOT = YES/' > doxy1;\
	fi; \
	cat doxy1 | sed -e 's:INPUT[	 ]*=.*:INPUT = $^:' > doxy2; \
	cmd="doxygen doxy3"; \
	if [ $(D) == 1 ]; then \
		cp doxy2 doxy3; \
		echo "$$cmd"; \
	else \
		cat doxy2 | sed -e 's/QUIET.*/QUIET = YES/' > doxy3; \
		echo "DOXY  $@"; \
	fi; \
	$$cmd
endif

test: cds_unit_tests
	@set -eu; \
	if ! which rttest2text.py > /dev/null 2>&1; then \
		echo "ERROR: rttest2text.py not found in PATH"; \
		exit 1; \
	fi; \
	./$< > cds.rtt; \
	find $(TOPDIR) -name 'test-*.c' | xargs rttest2text.py cds.rtt


define INSTALL
set -eu; \
[ -x $(1) ] && mode=755 || mode=644; \
dst=$(2)/`basename $(1)`; \
cmd="mkdir -p `dirname $$dst` && cp -rf $(1) $$dst && chmod $$mode $$dst"; \
if [ $(D) == 1 ]; then \
	echo "$$cmd"; \
else \
	echo "INST  $(1)"; \
fi; \
eval $$cmd;
endef


install: install_lib install_inc install_doc

install_lib: $(OUTPUT_LIBS)
	@$(foreach i,$^,$(call INSTALL,$(i),$(LIBDIR)))

install_inc:
	@$(foreach i,$(HDRS),$(call INSTALL,$(i),$(INCDIR)))

install_doc: doc
	@$(call INSTALL,doc/html,$(DOCDIR))


lib%.a:
	@$(call RUN_AR,$@,$^)

%.o: %.c
	@$(call RUN_CC,$@,$<)

%.o: %.cpp
	@$(call RUN_CXX,$@,$<)

dbg:
	@echo "Platform = $(PLF)"
	@echo "VPATH = $(VPATH)"
	@echo "HDRS = $(HDRS)"
	@echo "PATH = $(PATH)"


# Automatic header dependencies

# XXX OBJS = $(LIBCDS_OBJS) $(RTTEST_MAIN_OBJ) $(CDS_TEST_OBJS)
OBJS = $(LIBCDS_OBJS)

-include $(OBJS:.o=.d)

%.d: %.c
	@set -eu; \
	cmd="$(CC) $(CFLAGS) $(INCS) -MT $(@:.d=.o) -MM -MF $@ $<"; \
	if [ $(D) == 1 ]; then \
		echo "$$cmd"; \
	else \
		echo "DEP   $@"; \
	fi; \
	$$cmd

%.d: %.cpp
	@set -eu; \
	cmd="$(CXX) $(CXXFLAGS) $(INCS) -MT $(@:.d=.o) -MM -MF $@ $<"; \
	if [ $(D) == 1 ]; then \
		echo "$$cmd"; \
	else \
		echo "DEPXX $@"; \
	fi; \
	$$cmd
