# Template copied from my article about Makefiles, at:
#     http://ttsiodras.github.io/makefile.html

#CXX=clang++

# The directories containing the source files, separated by ':'
VPATH=src

# To make "release" the default configuration if invoked with just "make":
ifeq ($(CFG),)
CFG=release
endif

# The source files: regardless of where they reside in the source tree,
# VPATH will locate them...
Group0_SRC = \
    bishops.cpp

# Build a Dependency list and an Object list, by replacing the .cpp
# extension to .d for dependency files, and .o for object files.
Group0_DEP = $(patsubst %.cpp, deps.$(CFG)/Group0_%.d, ${Group0_SRC})
Group0_OBJ = $(patsubst %.cpp, objs.$(CFG)/Group0_%.o, ${Group0_SRC})

# Your final binary
TARGET=swapBishops

# What compiler to use for generating dependencies: 
# it will be invoked with -MM -MP
CXXDEP = g++ -std=c++11

# What include flags to pass to the compiler
INCLUDEFLAGS= -I src

CXXFLAGS += -std=c++11

# Separate compile options per configuration
ifeq ($(CFG),debug)
CXXFLAGS += -g -Wall -D_DEBUG ${INCLUDEFLAGS}
else
CXXFLAGS += -O2 -Wall ${INCLUDEFLAGS}
endif

# A common link flag for all configurations
LDFLAGS +=

all:	inform bin.$(CFG)/${TARGET}

inform:
ifneq ($(CFG),release)
ifneq ($(CFG),debug)
	@echo "Invalid configuration "$(CFG)" specified."
	@echo "You must specify a configuration when running make, e.g."
	@echo  "make CFG=debug"
	@echo  
	@echo  "Possible choices for configuration are 'release' and 'debug'"
	@exit 1
endif
endif
	@echo "Configuration "$(CFG)
	@echo "------------------------"

bin.$(CFG)/${TARGET}: ${Group0_OBJ} | inform
	@mkdir -p $(dir $@)
	$(CXX) -g -o $@ $^ ${LDFLAGS}

objs.$(CFG)/Group0_%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) -c $(CXXFLAGS) -o $@ $<

deps.$(CFG)/Group0_%.d: %.cpp
	@mkdir -p $(dir $@)
	@echo Generating dependencies for $<
	@set -e ; $(CXXDEP) -MM -MP $(INCLUDEFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,objs.$(CFG)\/Group0_\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

.PHONY:	clean release debug test benchmark

clean:
	@rm -rf \
	deps.debug objs.debug bin.debug \
	deps.release objs.release bin.release

release:
	$(MAKE) CFG=release

debug:
	$(MAKE) CFG=debug

test:	| release
	bin.release/$(TARGET)

benchmark:
	@$(MAKE) CFG=release || exit 1
	@./contrib/benchmark.sh

# Unless "make clean" is called, include the dependency files
# which are auto-generated. Don't fail if they are missing
# (-include), since they will be missing in the first invocation!
ifneq ($(MAKECMDGOALS),clean)
-include ${Group0_DEP}
endif
