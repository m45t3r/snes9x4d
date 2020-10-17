#!/bin/sh

cd $(dirname $0)

OPK_NAME=snes9x4d.opk

echo Building ${OPK_NAME}...

# create opk
FLIST="../snes9x4d default.gcw0.desktop sfc.png"

rm -f ${OPK_NAME}
mksquashfs ${FLIST} ${OPK_NAME} -all-root -no-xattrs -noappend -no-exports

cat default.gcw0.desktop
