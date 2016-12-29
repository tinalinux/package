TARGET			=	tinacameratest
INCLUDES		+=	-I$(STAGING_DIR)/usr/include/allwinner/include -I$(STAGING_DIR)/usr/include/allwinner
LIBS			+=	-lawrecorder -lcdc_vencoder -lcdc_aencoder \
					-lcdx_base -lcdx_muxer -lcdc_base -lcdc_ve \
					-lcdc_memory -lpthread -ldl -lstdc++ -lrt -lz \
					-lcdc_vdecoder

SRCS	= tinacameratest.c RecorderWriter.c common.c fifo.c \
		  camera_detect.c camera_capture.c camera_encoder.c camera_decoder.c camera_mux.c
OBJS	= $(SRCS:.c=.o)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LIBS) $(LDFLAGS)

all:$(TARGET)


clean:
	rm -rf $(TARGET) *.o *.a *~
	cd common && rm -f *.o *.a *.bak *~ .depend
