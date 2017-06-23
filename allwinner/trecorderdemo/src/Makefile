Target = trecorderdemo
## set the include path for compile flags.
SourceIncludePath := -I$(STAGING_DIR)/usr/include/allwinner/include -I$(STAGING_DIR)/usr/include/allwinner
CompileFlags = $(CFLAGS) $(SourceIncludePath)

LoadFlags +=   -lawrecorder -lcdc_vencoder -laencoder \
               -lcdx_base -lcdx_muxer -lcdc_base -lcdc_ve \
               -lcdc_memory -lpthread -ldl -lstdc++ -lrt -lz

$(Target): trecorderdemo.c RecorderWriter.c RecorderWriter.h
	$(CC) -o $@ $^ $(CompileFlags) $(LDFLAGS) $(LoadFlags)
