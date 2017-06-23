Target = realtplayerdemo

SourceIncludePath := -I$(STAGING_DIR)/usr/include/allwinner/include -I$(STAGING_DIR)/usr/include/allwinner
CompileFlags = $(CFLAGS) $(SourceIncludePath)

LoadFlags +=  -ltplayer -lxplayer -lcdc_vdecoder -ladecoder -lcdc_sdecoder -lcdc_base -lcdc_ve -lcdc_memory \
               -lcdx_parser  -lcdx_playback -lcdx_stream -lcdx_base \
               -lpthread -ldl -lstdc++ -lrt -lm -lc -lz -lasound

$(Target): tplayerdemo.c
	$(CC) -o $@ $^ $(CompileFlags) $(LDFLAGS) $(LoadFlags)
