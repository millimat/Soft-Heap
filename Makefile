#
# A simple makefile for building project composed of C source files.
#
# Julie Zelenski, for CS107, Sept 2014
#

# It is likely that default C compiler is already gcc, but be explicit anyway
CC = gcc

# The line below controls the compiler settings for allocator.c
# Edit here to by applying gcc optimization flags (-Ox and -fxxx)
# Initially, the flags are configured for no optimization (to enable reliable
# debugging) but when ready, you can experiment with different compiler settings
# (e.g. different levels and enabling/disabling specific optimizations)
# When you are ready to submit, be sure these flags are configured to
# show your allocator in its best light!
ALLOCATOR_EXTRA_CFLAGS = -O2 -finline-functions

# The CFLAGS variable sets the flags for the compiler.  CS107 adds these flags:
#  -g          compile with debug information
#  -std=gnu99  use the C99 standard language definition with GNU extensions
#  -Wall       turn on optional warnings (warnflags configures specific diagnostic warnings)
# Do not edit here! Instead change ALLOCATOR_EXTRA_CFLAGS above
CFLAGS = -g -std=gnu99 -Wall $$warnflags
export warnflags = -Wfloat-equal -Wtype-limits -Wpointer-arith -Wlogical-op -Wshadow -fno-diagnostics-show-option

# The LDFLAGS variable sets flags for the linker and the LDLIBS variable lists
# additional libraries being linked. The standard libc is linked by default.
LDFLAGS = 
LDLIBS = -lm

# Configure build tools to emit code for IA32 architecture by adding the necessary
# flag to compiler and linker
# CFLAGS += -m32
# LDFLAGS += -m32

# The line below defines the variable 'PROGRAMS' to name all of the executables
# to be built by this makefile
PROGRAMS = softheap

# The line below defines a target named 'all', configured to trigger the
# build of everything named in the 'PROGRAMS' variable. The first target
# defined in the makefile becomes the default target. When make is invoked
# without any arguments, it builds the default target.
all:: $(PROGRAMS)

# The entry below is a pattern rule. It defines the general recipe to make
# the 'name.o' object file by compiling the 'name.c' source file.
%.o: %.c
	$(COMPILE.c) $< -o $@

# This pattern rule defines the general recipe to make the executable 'name'
# by linking the 'name.o' object file and any other .o prerequisites. The 
# rule is used for all executables listed in the PROGRAMS definition above.
$(PROGRAMS): %:%.o 
	$(LINK.o) $(filter %.o,$^) $(LDLIBS) -o $@

# Specific per-target customizations and prerequisites are listed here

$(PROGRAMS): %:%.o 

# Do not edit here! Instead change ALLOCATOR_EXTRA_CFLAGS above.
# Below are the default build settings for the other modules. In grading, we compile
# all modules other than your allocator with the default build settings from starter.
# Any changes you make here will be ignored in grading.  Changing these settings
# in development could cause your observed results to not match the grading results.
alloctest.o segment.o fcyc.o simple.o : CFLAGS += -O0
allocator.o: CFLAGS += $(ALLOCATOR_EXTRA_CFLAGS)
allocator.o: Makefile


# The line below defines the clean target to remove any previous build results
clean::
	rm -f $(PROGRAMS) *.o callgrind.out.* *~

# PHONY is used to mark targets that don't represent actual files/build products
.PHONY: clean all

# The line below tries to include our master Makefile, which we use internally.
# The - means that it is not an error if this file can't be found (which will
# normally be the case). You can just ignore this line.
-include Makefile.grading
