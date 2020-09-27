inherit bin_package

DESCRIPTION = "USB and ADB init scripts"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/\
${LICENSE};md5=89aea4e17d99a7cacdbeed46a0096b10"
PROVIDES = "usb-scripts"
MY_PN = "usb-scripts"
PR = "r0"
INSANE_SKIP_${PN} = "ldflags"
INHIBIT_PACKAGE_DEBUG_SPLIT = "1"
INHIBIT_PACKAGE_STRIP = "1"
SRC_URI="file://usb \
         file://adbd \
         file://find_partitions.sh"
S = "${WORKDIR}/"


do_install() {
      install -d ${D}/etc/default
      install -d ${D}/etc/init.d
      install -d ${D}/etc/rcS.d
      # Modem partition sits in firmware, but is linked in /lib/firmware/image
      # Since we cant expect the partition to be in place while building,
      # Just recreate the directory and link it
      install -d ${D}/firmware/image
      install -d ${D}/lib/firmware

      # Make a directory to mount the recovery image too, now that we're at it
      install -d ${D}/recovery

      cp  ${S}/usb ${D}/etc/init.d/
      cp  ${S}/adbd ${D}/etc/init.d/
      cp  ${S}/find_partitions.sh ${D}/etc/init.d/
      chmod +x ${D}/etc/init.d/usb
      chmod +x ${D}/etc/init.d/adbd
      chmod +x ${D}/etc/init.d/find_partitions.sh

      ln -sf -r ${D}/etc/init.d/usb ${D}/etc/rcS.d/S99usb
      ln -sf -r ${D}/etc/init.d/adbd ${D}/etc/rcS.d/S99adbd
      ln -sf -r ${D}/etc/init.d/adbd ${D}/etc/rcS.d/S80find_partitions.sh
      ln -sf -r ${D}/firmware/image ${D}/lib/firmware/image
      touch ${D}/etc/default/usb
      touch ${D}/etc/default/adbd
      touch ${D}/etc/default/find_partitions.sh

}
