#!/bin/sh
svn export https://svn.sourceforge.jp/svnroot/samba-jp/trunc
if [ $? -ne 0 ]
then
   echo svn get error
   exit 1
fi
cp -pvr trunc/docs-xml/manpages-3/* docs-xml/manpages-3
cp -pvr trunc/docs-xml/smbdotconf/* docs-xml/smbdotconf
cp -pvr trunc/docs-xml/Samba3-HOWTO/* docs-xml/Samba3-HOWTO
cp -pvr trunc/docs-xml/Samba3-Developers-Guide/* docs-xml/Samba3-Developers-Guide
cp -pv  trunc/docs-xml/build/DTD/samba.entities docs-xml/build/DTD/
cp -pv  trunc/docs-xml/xslt/* docs-xml/xslt/
cat $(pwd)/docs-xml/build/catalog.xml.in | sed -e "s|@BUILDDIR@|$(pwd)/docs-xml|g" > $(pwd)/docs-xml/build/catalog.xml
mv  $(pwd)/docs-xml/build/catalog.xml.in  $(pwd)/docs-xml/build/catalog.xml.in.org
mv  $(pwd)/docs-xml/build/catalog.xml     $(pwd)/docs-xml/build/catalog.xml.in

