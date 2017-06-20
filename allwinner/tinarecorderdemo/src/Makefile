Target = tinarecorderdemo
## set the include path for compile flags.
SourceIncludePath := -I$(STAGING_DIR)/usr/include/allwinner/include -I$(STAGING_DIR)/usr/include/allwinner
CompileFlags = $(CFLAGS) $(SourceIncludePath)

ifeq ($(TARGET_BOARD),$(filter $(TARGET_BOARD),azalea tulip sitar cello banjo))
LoadFlags +=   -lawrecorder -lcdc_vencoder -laencoder \
               -lcdx_base -lcdx_muxer -lcdc_base -lcdc_ve \
               -lcdc_memory -lpthread -ldl -lstdc++ -lrt -lz

else ifeq ($(TARGET_BOARD),$(filter $(TARGET_BOARD),astar octopus))
LoadFlags +=   -lawrecorder -lcdc_vencoder -lcdc_aencoder \
               -lcdx_base -lcdx_muxer -lcdc_base -lcdc_ve \
               -lcdc_memory -lpthread -ldl -lstdc++ -lrt -lz
endif

ifeq ($(TARGET_BOARD),$(filter $(TARGET_BOARD),azalea tulip sitar cello banjo))
$(Target): tinarecorderdemo_new.c RecorderWriter_new.c RecorderWriter_new.h
	$(CC) -o $@ $^ $(CompileFlags) $(LDFLAGS) $(LoadFlags)

else ifeq ($(TARGET_BOARD),$(filter $(TARGET_BOARD),astar octopus))
$(Target): tinarecorderdemo_old.c RecorderWriter_old.c RecorderWriter_old.h
	$(CC) -o $@ $^ $(CompileFlags) $(LDFLAGS) $(LoadFlags)

endif
