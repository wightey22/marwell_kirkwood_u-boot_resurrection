#! /bin/sh
cd output/htmldocs/manpages-3
for file in *.html
do
   echo $file
   php ../../../htmlconv.php <$file >$file.new
   mv $file $file.old
   mv $file.new $file
done

