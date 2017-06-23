
一、linux相关说明:

1. 每套linux so对应的编译工具链如下：

so_dir: arm926-uclibc                 host: arm-linux              toolchain_dir: uclibc_arm926_static
so_dir: arm-linux-gnueabi             host: arm-linux-gnueabi      toolchain_dir: external-toolchain
so_dir: arm-linux-gnueabihf           host: arm-linux-gnueabihf    toolchain_dir: 系统自带
so_dir: arm-none-linux-gnueabi        host: arm-none-linux-gnueabi toolchain_dir: 系统自带
so_dir: arm-openwrt-linux-muslgnueabi host: arm-openwrt-linux      toolchain_dir: OpenWrt-Toolchain-sunxi_gcc-5.2.0_musl-1.1.12_eabi.Linux-x86_64
so_dir: arm-openwrt-linux-uclibc      host: arm-openwrt-linux      toolchain_dir: toolchain_arm_uClibc
so_dir: arm-linux-gnueabihf-linaro    host: arm-linux-gnueabihf    toolchain_dir: gcc-linaro-arm-linux-gnueabihf-4.8-2014.01_linux
so_dir: arm-aarch64-openwrt-linux     host: aarch64-openwrt-linux  toolchain_dir: toolchain-sunxi-tina2.0-64
so_dir: arm-openwrt-linux-muslgnueabi-v5  host: arm-openwrt-linux-muslgnueabi    toolchain_dir: linux-x86

2.编译步骤如下（以arm926-uclibc为例）：

2.1 export编译工具链：
                     TOOLS_CHAIN=/home/user/workspace/tools_chain/
                     export PATH=${TOOLS_CHAIN}/uclibc_arm926_static/bin:$PATH

2.2. 运行automake的相关工具：./bootstrap

2.3. 配置makefile：
2.3.1 模式：./configure --prefix=INSTALL_PATH --host=HOST_NAME LDFLAGS="-LSO_PATH"
2.3.2 示例：./configure --prefix=/home/user/workspace/libcedarc/install --host=arm-linux LDFLAGS="-L/home/user/workspace/libcedarc/lib/arm926-uclibc"
2.3.3 特别说明：如果内核用的是linux3.10, 则必须加上flag：CFLAGS="-DCONF_KERNEL_VERSION_3_10" CPPFLAGS="-DCONF_KERNEL_VERSION_3_10"
	  即：./configure --prefix=/home/user/workspace/libcedarc/install --host=arm-linux CFLAGS="-DCONF_KERNEL_VERSION_3_10" CPPFLAGS="-DCONF_KERNEL_VERSION_3_10" LDFLAGS="-L/home/user/workspace/libcedarc/lib/arm926-uclibc"

2.4 编译：make ; make install

二、android相关说明：

1). CedarC-v1.0.4

1. 特别说明：

1.1 camera模块相关：( >=android7.0的平台需要进行如下修改,其他平台保持之前的做法)
    修改的原因：a.omx和android framework都没有对NV21和NV12这两种图像格式进行细分，都是用OMX_COLOR_FormatYUV420SemiPlanar
	            进行表示，OMX_COLOR_FormatYUV420SemiPlanar即可以表示NV21，也可以表示NV12；
                b.而ACodec将OMX_COLOR_FormatYUV420SemiPlanar用作NV12，camera将OMX_COLOR_FormatYUV420SemiPlanar用于NV21，
                之前的做法是omx_venc通过进程的包名进行区分兼容，若调用者是ACodec，则用作NV12，若调用者是camera，则用作
				NV21；
				c.android7.0因为权限管理的原因，无法获取到进程的包名，所以无法在omx_venc进行兼容，只能在上层caller层进行
				兼容，而cts会对ACodec的接口进行测试，若改动ACodec，则会响应到cts测试，所以只能修改camera
				d.修改的原则为：扩展图像格式的枚举类型的成员变量OMX_COLOR_FormatYVU420SemiPlanar，用于表示NV21；
				  即 OMX_COLOR_FormatYUV420SemiPlanar --> NV12
				     OMX_COLOR_FormatYVU420SemiPlanar --> NV21

	修改的地方：
	a. 同步头文件：同步openmax/omxcore/inc/OMX_IVCommon.h 到./native/include/media/openmax/OMX_IVCommon.h
	b. camera模块在调用openmax/venc编码接口时需要进行修改：
	   修改前 NV21 --> OMX_COLOR_FormatYUV420SemiPlanar，
	   修改后 NV21 --> OMX_COLOR_FormatYVU420SemiPlanar;

2. 改动点如下：
2.1 openmax:venc add p_skip interface
2.2 h265:fix the HevcDecodeNalSps and HevcInitialFBM
2.3 h264:refactor the H264ComputeOffset
2.4 mjpeg scale+rotate的修正
2.5 vdcoder/h265: add the code of parser HDR info
2.6 vdecoder/h265: add the process of error-frame
2.7 vdecoder/h264: make sure pMbNeighborInfoBuf is 16K-align to fix mbaff function
2.8 openmax/vdec: remove cts-flag
2.9 openmax/venc: remove cts-flag
2.10 detection a complete frame bitstream and crop the stuff zero data
2.11 vdecoder/h265:fix the bug that the pts of keyFrame is error when seek
2.12 openmax/inc: adjust the define of struct
2.13 vencoder: add lock for vencoderOpen()
2.14 vdecoder/ALMOST decoders:fix rotate and scaledown
2.15 vdecoder/h265:fix the process of searching the start-code when sbm cycles
2.16 vdecoder/h265:fix the bug when request fbm fail after fbm initial
2.17 vdecoder/h265:improve the process when poc is abnormal
2.18 cedarc: unify the release of android and linux
2.19 vdecoder/avs: make mbInfoBuf to 16K align

2).CedarC-v1.0.5

1. 改动点如下：
1.1.configure.ac:fix the config for linux compiling
1.2.openmax/venc: revert mIsFromCts on less than android_7.0 platfrom
1.3.vdecoder/h265soft:make h265soft be compatible with AndroidN
1.4.cedarc: merge the submit of cedarc-release
1.5.vdecoder/h265:use the flag "bSkipBFrameIfDelay"
1.6.vdecoder:fix the buffer size for thumbnail mode
1.7.cedarc: fix compile error of linux
1.8.omx:venc add fence for androidN
1.9.openmax:fix some erros
1.10.omx_venc: add yvu420sp for omx_venc init
1.11.videoengine:add 2k limit for h2
1.12.cedarc: add the toolschain of arm-openwrt-linux-muslgnueabi for v5
1.13.cedarc: 解决0x1663 mpeg4 播放花屏的问题
1.14.vdecoder: fix compile error of soft decoder for A83t
1.15.omx_vdec: fix the -1 bug for cts
1.16.修改mpeg2 获取ve version的方式
1.17.vencoder: fix for input addrPhy check
1.18.cedarc: merge the submit of cedarc-release