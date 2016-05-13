# A simple makefile for building project composed of C source files.
#
# Julie Zelenski, for CS107, Sept 2014
#

# It is likely that default C compiler is already gcc, but be explicit anyway
CC = gcc

# The CFLAGS variable sets the flags for the compiler. 
CFLAGS = -g -O3 -std=gnu99 -Wall $$warnflags
export warnflags = -Wfloat-equal -Wtype-limits -Wpointer-arith -Wlogical-op -Wshadow -fno-diagnostics-show-option

# The LDFLAGS variable sets flags for the linker and the LDLIBS variable lists
# additional libraries being linked. The standard libc is linked by default
# We additionally require the library for CVector/CMap, so it is noted here
LDFLAGS = -L.
LDLIBS = -lsoftheap -lm

# Configure build tools to emit code for IA32 architecture by adding the necessary
# flag to compiler and linker
# CFLAGS += -m32
# LDFLAGS += -m32

# The line below defines the variable 'PROGRAMS' to name all of the executables
# to be built by this makefile.  If you write additional client programs,
# add them to the list below so they can be built using make. The programs
# named in this list will be compiled from a similarly-named .c file (i.e.
# the program vectest is built from client program vectest.c)
PROGRAMS = run-tests

# The line below defines a target named 'all', configured to trigger the
# build of everything named in the 'PROGRAMS' variable. The first target
# defined in the makefile becomes the default target. When make is invoked
# without any arguments, it builds the default target.
all:: $(PROGRAMS)

# The entry below is a pattern rule. It defines the general recipe to make
# the 'name.o' object file by compiling the 'name.c' source file. It also
# lists cvector.h and cmap.h to be treated as prerequisites.
%.o: %.c softheap.h
	$(COMPILE.c) -I. $< -o $@

# This pattern rule defines the general recipe to make the executable 'name'
# by linking the 'name.o' object file and any other .o prerequisites. The
# rule is used for all executables listed in the PROGRAMS definition above.
# The client programs need to be rebuilt if library is updated, so
# add as a prerequisite. 
$(PROGRAMS): %:%.o libsoftheap.a
	$(LINK.o) $(filter %.o,$^) $(LDLIBS) -o $@

# These pattern rules disable implicit rules for executables
# by supplying empty recipe. Accidentally attempting to build
# cvector gives confusing failure from implicit rules, if disabled
# build stops right away and reports "No rule"
%: %.c
%: %.o

# Specific per-target customizations and prerequisites are listed here

# Custom rule to build library (Make has no implicit rule for .a) from our .o files
# marking the object files as intermediate will discard them after folding into library.
# Use D flag for "deterministic" mode, internal timestamps are zeros, library binary 
# will be unchanged from recompile if no source change
ARFLAGS = rvD
libsoftheap.a: softheap.o
	$(AR) $(ARFLAGS) $@ $?
.INTERMEDIATE: softheap.o

# The line below defines the clean target to remove any previous build results
clean::
	rm -f $(PROGRAMS) $(SOLN_PROGRAMS) libsoftheap.a core *.o callgrind.out.* *~

# PHONY is used to mark targets that don't represent actual files/build products
.PHONY: clean all soln
