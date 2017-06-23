
MODULE_TOP = $(TOP)/frameworks/av/media/libcedarc

product = $(TARGET_BOARD_PLATFORM)

########## configure CONF_ANDROID_VERSION ##########
android_version = $(shell echo $(PLATFORM_VERSION) | cut -c 1)

config_file = null

ifeq ($(product), octopus)
    ifeq ($(android_version), 5)
        config_file = config_pad_A83_lollipop.mk
    else ifeq ($(android_version), 7)
	config_file = config_pad_A83_Nougat.mk
    endif
else ifeq ($(product), astar)
    ifeq ($(android_version), 7)
        config_file = config_pad_A33_Nougat.mk
    endif
else ifeq ($(product), tulip)
    ifeq ($(android_version), 5)
        config_file = config_pad_A64_lollipop.mk
    else ifeq ($(android_version), 6)
        config_file = config_pad_A64_Marshmallow.mk
    else ifeq ($(android_version), 7)
        config_file = config_pad_A64_Nougat.mk
    endif
else ifeq ($(product), dolphin)
    ifeq ($(android_version), 4)
        config_file = config_box_H3_kitkat.mk
    endif
else ifeq ($(product), rabbit)
    ifeq ($(android_version), 5)
        config_file = config_box_H64_lollipop.mk
    endif
else ifeq ($(product), cheetah)
    ifeq ($(android_version), 4)
        config_file = config_box_H5_kitkat.mk
    else ifeq ($(android_version), 5)
        config_file = config_box_H5_lollipop.mk
    endif
else ifeq ($(product), eagle)
    ifeq ($(android_version), 5)
        config_file = config_box_H8_kitkat.mk
    else ifeq ($(android_version), 7)
        config_file = config_box_H8_Nougat.mk
    endif
else ifeq ($(product), t3)
    ifeq ($(android_version), 6)
        config_file = config_cdr_T3_Marshmallow.mk
    endif
else ifeq ($(product), kylin)
    ifeq ($(android_version), 7)
        config_file = config_vr_A80_Nougat.mk
    endif
endif

ifeq ($(config_file), null)
    $(warning "can not find config_file: product=$(product), androd_version=$(android_version)")
else
    $(warning "config file: $(config_file)")
    include $(MODULE_TOP)/config/$(config_file)
endif
