Target = realtinaplayerdemo

ifeq ($(TARGET_BOARD),$(filter $(TARGET_BOARD),azalea tulip sitar cello banjo))
LoadFlags +=  -ltinaplayer -lxplayer -lcdc_vdecoder -ladecoder -lcdc_sdecoder -lcdc_base -lcdc_ve -lcdc_memory \
               -lcdx_parser  -lcdx_playback -lcdx_stream -lcdx_base \
               -lpthread -ldl -lstdc++ -lrt -lm -lc -lz -lasound
else ifeq ($(TARGET_BOARD),$(filter $(TARGET_BOARD),astar octopus))
LoadFlags +=  -ltinaplayer -lxplayer -lcdc_vdecoder -lcdc_adecoder -lcdc_sdecoder -lcdc_base -lcdc_ve -lcdc_memory \
               -lcdx_parser  -lplayer -lcdx_stream -lcdx_base -law_plugin \
               -lpthread -ldl -lstdc++ -lrt -lm -lc -lz -lasound
endif

$(Target): tinaplayerdemo.cpp
	$(CXX) -o $@ $^ $(CFLAGS) $(LDFLAGS) $(LoadFlags) -g
