for f in `ls *.c *.pkg *.h makefile`
do
    if [ -f $f ];then
        echo convert $f
        iconv -f UTF-8 -t GBK $f > gbk/$f
    fi
done
