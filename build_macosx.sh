#!/bin/bash
QTDIR=/usr/local/qt3
VERSION="0.9.5"
APPNAME=kYDPdict

../configure  --without-x --with-qt-dir=$QTDIR 
make || exit

cd src

echo "Removing..." && rm -rf ${APPNAME}.app

echo "Contents..."
mkdir -p ${APPNAME}.app/Contents/MacOS
cp kydpdict ${APPNAME}.app/Contents/MacOS/${APPNAME}
mkdir -p ${APPNAME}.app/Contents/Resources
cp ../icons/babelfish.icns ${APPNAME}.app/Contents/Resources

echo "Frameworks..."
mkdir -p ${APPNAME}.app/Contents/Frameworks
cp $QTDIR/lib/libqt-mt.3.dylib ${APPNAME}.app/Contents/Frameworks
install_name_tool \
        -id @executable_path/../Frameworks/libqt-mt.3.dylib \
        ${APPNAME}.app/Contents/Frameworks/libqt-mt.3.dylib
install_name_tool \
        -change libqt-mt.3.dylib \
        @executable_path/../Frameworks/libqt-mt.3.dylib \
        ${APPNAME}.app/Contents/MacOS/${APPNAME}

echo "Info..."
echo "<?xml version='1.0' encoding='UTF-8'?>" > ${APPNAME}.app/Contents/Info.plist
echo "<!DOCTYPE plist SYSTEM 'file://localhost/System/Library/DTDs/PropertyList.dtd'>" >> ${APPNAME}.app/Contents/Info.plist
echo "<plist version='0.9'>" >> ${APPNAME}.app/Contents/Info.plist
echo "<dict>" >> ${APPNAME}.app/Contents/Info.plist
echo "        <key>CFBundleIdentifier</key>" >> ${APPNAME}.app/Contents/Info.plist
echo "        <string>com.komsoft.kydpdict</string>" >> ${APPNAME}.app/Contents/Info.plist
echo "        <key>CFBundlePackageType</key>" >> ${APPNAME}.app/Contents/Info.plist
echo "        <string>APPL</string>" >> ${APPNAME}.app/Contents/Info.plist
echo "        <key>CFBundleExecutable</key>" >> ${APPNAME}.app/Contents/Info.plist
echo "        <string>${APPNAME}</string>" >> ${APPNAME}.app/Contents/Info.plist
echo "        <key>CFBundleIconFile</key>" >> ${APPNAME}.app/Contents/Info.plist
echo "        <string>babelfish.icns</string>" >> ${APPNAME}.app/Contents/Info.plist
echo "        <key>CFBundleGetInfoString</key>" >> ${APPNAME}.app/Contents/Info.plist
echo "        <string>Pawel Piecuch - MacOSX build</string>" >> ${APPNAME}.app/Contents/Info.plist
echo "        <key>CFBundleSignature</key>" >> ${APPNAME}.app/Contents/Info.plist
echo "        <string>ttxt</string>" >> ${APPNAME}.app/Contents/Info.plist
echo "        <key>NOTE</key>" >> ${APPNAME}.app/Contents/Info.plist
echo "        <string>YDP Dictionary interface - MacOSX build</string>" >> ${APPNAME}.app/Contents/Info.plist
echo "</dict>" >> ${APPNAME}.app/Contents/Info.plist
echo "</plist>" >> ${APPNAME}.app/Contents/Info.plist
echo -n "APPL???? " > ${APPNAME}.app/Contents/PkgInfo

echo "Disk image.."
destdir=`pwd`"/pkg_dist"
name=${APPNAME}-${VERSION}

rm -rf ${name}.dmg

mkdir -p ${destdir}
appSize="$(($(du -k -s "${APPNAME}.app" | awk '{print $1}')+3))"

echo "--> Creating Disc Image"
## From hdiutil man page, a sector is 512k.  Allocate an extra 5% for
## directories and partition tables.
sectorsAlloced=`echo 2.1*${appSize}|bc`
hdiutil create -ov ${destdir}/discRW -sectors ${sectorsAlloced}
## Need to format the disc image before mounting
mountLoc=`hdid -nomount ${destdir}/discRW.dmg | grep HFS | cut -f1`
/sbin/newfs_hfs -v "$name" ${mountLoc}
hdiutil eject ${mountLoc}
echo "Copying Package to Disc Image"
hdid ${destdir}/discRW.dmg
		
if [ ! -d "/Volumes/${name}" ]; then
	echo "Could not create disc image.  The $name installer package in this directory should be correct.  Please use the Disc Copy program to create a disc image." >&2
	exit 0
fi
		
cp -R ${APPNAME}.app "/Volumes/${name}"
		
## Converting Disk Image to read-only
echo 'Converting Disc Image to read-only'
hdiutil eject ${mountLoc}
hdiutil resize ${destdir}/discRW.dmg -sectors min
hdiutil convert ${destdir}/discRW.dmg -format UDZO -imagekey zlib-level=2 -o "${name}.dmg"
		
echo "Cleaning"
rm -rf "$destdir"

cd ..
