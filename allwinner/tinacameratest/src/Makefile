TARGET			=	tinacameratest
INCLUDES		+=	-I$(STAGING_DIR)/usr/include/allwinner/include -I$(STAGING_DIR)/usr/include/allwinner

ifeq ($(TARGET_BOARD),$(filter $(TARGET_BOARD),azalea tulip sitar cello banjo))
LIBS			+=	-lawrecorder -lcdc_vencoder -laencoder \
					-lcdx_base -lcdx_muxer -lcdc_base -lcdc_ve \
					-lcdc_memory -lpthread -ldl -lstdc++ -lrt -lz \
					-lcdc_vdecoder
CompileFlags    +=  -DTARGET_BOARD=$(TARGET_BOARD)
else ifeq ($(TARGET_BOARD),$(filter $(TARGET_BOARD),astar octopus))
LIBS			+=	-lawrecorder -lcdc_vencoder -lcdc_aencoder \
					-lcdx_base -lcdx_muxer -lcdc_base -lcdc_ve \
					-lcdc_memory -lpthread -ldl -lstdc++ -lrt -lz \
					-lcdc_vdecoder
endif
ifeq ($(TARGET_BOARD),$(filter $(TARGET_BOARD),azalea tulip sitar cello banjo))
SRCS	= tinacameratest.c RecorderWriter.c common.c fifo.c disp.c\
		  camera_detect.c camera_capture.c camera_encoder.c camera_decoder.c camera_mux.c
else ifeq ($(TARGET_BOARD),$(filter $(TARGET_BOARD),astar octopus))
SRCS	= tinacameratest.c RecorderWriter_old.c common.c fifo.c disp.c\
		  camera_detect.c camera_capture.c camera_encoder.c camera_decoder.c camera_mux.c
endif

OBJS	= $(SRCS:.c=.o)

%.o: %.c
	$(CC) $(CFLAGS) $(CompileFlags) $(INCLUDES) -c -o $@ $<

ifeq ($(TARGET_BOARD),$(filter $(TARGET_BOARD),azalea tulip sitar cello banjo))
$(TARGET): $(OBJS)
	$(CC) -o $@ $(CompileFlags) $(OBJS) $(LIBS) $(LDFLAGS)
else ifeq ($(TARGET_BOARD),$(filter $(TARGET_BOARD),astar octopus))
$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LIBS) $(LDFLAGS)
endif

all:$(TARGET)


clean:
	rm -rf $(TARGET) *.o *.a *~
	cd common && rm -f *.o *.a *.bak *~ .depend
