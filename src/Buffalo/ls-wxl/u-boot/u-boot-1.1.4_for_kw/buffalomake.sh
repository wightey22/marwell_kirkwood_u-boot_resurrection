#!/bin/sh

. /opt/cross.conf

CROSS_ENV_PATH=${MARVELL_ARM_ENV2}
export PATH=${CROSS_ENV_PATH}/bin:${PATH}
IMAGE_DIR=../images


build_lsxhl()
{
	echo "*** LS-XHL/LS-CHL/LS-XL ***"
	OUTPUT=buffalo_mvlsxh_6281
	RULE=${OUTPUT}_config
	
	make mrproper
	make OTHERCFLAGS="" ${RULE} SPIBOOT=1 BOOTROM=1 LE=1
	make -s
	if [ -f u-boot-${OUTPUT}.bin ]; then
		./tools/doimage -T flash -D 0x600000 -E 0x660000 -R dramregs_400mvlsxh_A.txt \
		u-boot-${OUTPUT}.bin u-boot_lsxh.bin
		cp u-boot_lsxh.bin $IMAGE_DIR
		./tools/doimage -T flash -D 0x600000 -E 0x660000 -R dramregs_300xl_A.txt \
		u-boot-${OUTPUT}.bin u-boot_lsxl.bin
		cp u-boot_lsxl.bin $IMAGE_DIR
	fi
}

build_lswxl()
{
	echo "*** LS-WXL/LS-WSX ***"
	OUTPUT=buffalo_mvwxl_6281
	RULE=${OUTPUT}_config

	make mrproper
	make OTHERCFLAGS="" ${RULE} SPIBOOT=1 BOOTROM=1 LE=1 NAND=1
	make -s
	if [ -f u-boot-${OUTPUT}.bin ]; then
		./tools/doimage -T flash -D 0x600000 -E 0x660000 -R dramregs_300wxl_A.txt \
		u-boot-${OUTPUT}.bin u-boot_lswxl.bin
		cp u-boot_lswxl.bin $IMAGE_DIR
	fi
}


[ -d $IMAGE_DIR ] || mkdir $IMAGE_DIR

case $1 in
    lsxhl|lschl|lsxl)
	build_lsxhl
	;;
    lswxl|lswsxl|lsavl)
	build_lswxl
	;;
    all)
	build_lsxhl
	build_lswxl
	;;
    *)
	echo "Usage: `basename $0` <target>"
esac
