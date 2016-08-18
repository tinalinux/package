BuildPath      = ./build
ObjectPath     = $(BuildPath)/obj
OutputPath     = .
DependFilePath = $(BuildPath)/dep

Target         = $(OutputPath)/my-parrot   ## output target.

ifneq ($(BuildPath),wildcard($(BuildPath)))
a := $(shell mkdir -p $(BuildPath))
endif
ifneq ($(ObjectPath),wildcard($(ObjectPath)))
a := $(shell mkdir -p $(ObjectPath))
endif
ifneq ($(DependFilePath),wildcard($(DependFilePath)))
a := $(shell mkdir -p $(DependFilePath))
endif

################################################################################
## set the source files, object files and dependency files
################################################################################
## set the source path to VPATH.
SourcePath = $(shell find ./ -type d)
SourcePath := $(filter-out $(BuildPath) $(ObjectPath) $(DependFilePath), $(SourcePath))
VPATH := $(SourcePath)

## set the include path.
SourceIncludePath = $(patsubst %,-I%,$(subst :, ,$(VPATH)))

## set the source files.
SourceFiles  = $(foreach dir,$(SourcePath),$(shell find $(dir) -maxdepth 1 -name "*.cpp"))

## set the object files.
ObjectFiles = $(addprefix $(ObjectPath)/, $(addsuffix .o ,$(basename $(notdir $(SourceFiles)))))

## set the dependency files.
DependFiles = $(addprefix $(DependFilePath)/, $(addsuffix .d ,$(notdir $(basename $(SourceFiles)))))

################################################################################
## set flags for compile and link
################################################################################
## set the include path for compile flags.

## set compile flags
CompileFlags = $(CFLAGS) $(SourceIncludePath) -g -O0

## set link flags
TinaPlayerLoadFlags +=   -ltinaplayer -lxplayer -lcdc_vdecoder -lcdc_adecoder -lcdc_sdecoder -lcdc_base -lcdc_ve -lcdc_memory \
               -lcdx_parser  -lplayer -lcdx_stream -lcdx_base  -lpostprocess \
               -law_plugin  -lpthread -ldl -lstdc++ -lrt -lm -lc -lz -lasound
#LoadFlags = $(LDFLAGS) $(TinaPlayerLoadFlags) -lpthread -lev -lwifimg -lsmartlinkd_client -ljson-c -g -O0
LoadFlags = $(LDFLAGS) $(TinaPlayerLoadFlags) \
            -lpthread -lev -lcurl -lssl -lcrypto -ljson-c -lmad \
            -lwifimg -lsmartlinkd_client -lbtapp -lbsa -ltinymp3 \
            -g -O0

################################################################################
## make commands, all/clean/cleanall
################################################################################
## define commands for make, sush as all, clean
.PHONY: all clean cleanall install
all:$(Target) install

install:
	-mkdir -p $(CONFIG_PREFIX)/usr/bin
	-cp $(Target) $(CONFIG_PREFIX)/usr/bin
clean:
	-rm -rf $(ObjectFiles)
	-rm -f $(Target)

cleanall: clean
	-rm -rf $(DependFiles)
	-rm -rf $(BuildPath)

################################################################################
## define target dependencies.
################################################################################
## compile source files to object files.
$(ObjectFiles):$(ObjectPath)/%.o:%.cpp
	@$(CXX) $(CompileFlags) -o $@ -c $<
	@echo "CC    $<"

## link object files to the target share library.
$(Target):$(ObjectFiles)
	$(CXX) -o $@ $^ $(LoadFlags)
	@echo "LD    $@"

## set rules to generate .d files.
$(DependFilePath)/%.d:%.cpp
	@set -e; rm -f $@; \
	$(CXX) -MM $(CompileFlags) $< > $@.$$$$; \
	sed 's,.*\.o[:]*,$(ObjectPath)\/$*\.o $@: ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$
	@echo "updating    $@"

## include the .d files to set dependency rules.
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),cleantarget)
ifneq ($(MAKECMDGOALS),cleanall)
#-include $(DependFiles)
endif
endif
endif
