#!/bin/bash

set -eu

if [ ! -e './build/x64-linux/release/cds_vs_stl/list/stllistperf' ]; then
    echo "Executables for linux 64-bit not found"
    echo "This shell script only works for Linux x64"
    exit 1
fi

export TIME='%S %U %M'
tmpfile=`mktemp cdsperf-XXXXXX.txt`
rndfile=`mktemp rnd-XXXXXX`

cleanup()
{
    rm -f "$tmpfile" "$rndfile"
}

trap cleanup 0


count=10000000
printf "Testing lists: insert, walk and delete %'d items\n" $count

/usr/bin/time -o "$tmpfile" \
    ./build/x64-linux/release/cds_vs_stl/list/cdslistperf "$count" \
    > /dev/null
read cdslistkernel_s cdslistuser_s cdslistmem_KiB < "$tmpfile"
cdslistkernel_ms=`echo "$cdslistkernel_s" 1000 \* p | dc`
cdslistuser_ms=`echo "$cdslistuser_s" 1000 \* p | dc`
cdslisttime_ms=`echo "$cdslistkernel_ms" "$cdslistuser_ms" + p | dc`
cdslistmem_MiB=`echo "$cdslistmem_KiB" 1024 / p | dc`
echo "  cds list: $cdslisttime_ms ms  $cdslistmem_MiB MiB"

/usr/bin/time -o "$tmpfile" \
    ./build/x64-linux/release/cds_vs_stl/list/stllistperf "$count" \
    > /dev/null
read stllistkernel_s stllistuser_s stllistmem_KiB < "$tmpfile"
stllistkernel_ms=`echo "$stllistkernel_s" 1000 \* p | dc`
stllistuser_ms=`echo "$stllistuser_s" 1000 \* p | dc`
stllisttime_ms=`echo "$stllistkernel_ms" "$stllistuser_ms" + p | dc`
stllistmem_MiB=`echo "$stllistmem_KiB" 1024 / p | dc`
echo "  stl list: $stllisttime_ms ms  $stllistmem_MiB MiB"

tmp1=`echo "$cdslisttime_ms" | cut -d. -f1`
tmp2=`echo "$stllisttime_ms" | cut -d. -f1`
if [ "$tmp1" -lt "$tmp2" ]; then
    tmp=`echo "$stllisttime_ms" "$cdslisttime_ms" - 100 \* "$cdslisttime_ms" / p | dc`
    echo "  stl took ${tmp}% more time than cds"
else
    tmp=`echo "$cdslisttime_ms" "$stllisttime_ms" - 100 \* "$stllisttime_ms" / p | dc`
    echo "  cds took ${tmp}% more time than stl"
fi

if [ "$cdslistmem_MiB" -lt "$stllistmem_MiB" ]; then
    tmp=`echo "$stllistmem_MiB" "$cdslistmem_MiB" - 100 \* "$cdslistmem_MiB" / p | dc`
    echo "  stl used ${tmp}% more memory than cds"
else
    tmp=`echo "$cdslistmem_MiB" "$stllistmem_MiB" - 100 \* "$stllistmem_MiB" / p | dc`
    echo "  cds used ${tmp}% more memory than stl"
fi


count=500000
printf "Testing maps: insert and delete %'d items\n" $count

./build/x64-linux/release/cds_vs_stl/map/mkrnd "$count" "$rndfile"

/usr/bin/time -o "$tmpfile" \
    ./build/x64-linux/release/cds_vs_stl/map/cdsmapperf \
        "$count" "$rndfile" > /dev/null
read cdsmapkernel_s cdsmapuser_s cdsmapmem_KiB < "$tmpfile"
cdsmapkernel_ms=`echo "$cdsmapkernel_s" 1000 \* p | dc`
cdsmapuser_ms=`echo "$cdsmapuser_s" 1000 \* p | dc`
cdsmaptime_ms=`echo "$cdsmapkernel_ms" "$cdsmapuser_ms" + p | dc`
cdsmapmem_MiB=`echo "$cdsmapmem_KiB" 1024 / p | dc`
echo "  cds map: $cdsmaptime_ms ms  $cdsmapmem_MiB MiB"

/usr/bin/time -o "$tmpfile" \
    ./build/x64-linux/release/cds_vs_stl/map/stlmapperf \
        "$count" "$rndfile" > /dev/null
read stlmapkernel_s stlmapuser_s stlmapmem_KiB < "$tmpfile"
stlmapkernel_ms=`echo "$stlmapkernel_s" 1000 \* p | dc`
stlmapuser_ms=`echo "$stlmapuser_s" 1000 \* p | dc`
stlmaptime_ms=`echo "$stlmapkernel_ms" "$stlmapuser_ms" + p | dc`
stlmapmem_MiB=`echo "$stlmapmem_KiB" 1024 / p | dc`
echo "  stl map: $stlmaptime_ms ms  $stlmapmem_MiB MiB"

tmp1=`echo "$cdsmaptime_ms" | cut -d. -f1`
tmp2=`echo "$stlmaptime_ms" | cut -d. -f1`
if [ "$tmp1" -lt "$tmp2" ]; then
    tmp=`echo "$stlmaptime_ms" "$cdsmaptime_ms" - 100 \* "$cdsmaptime_ms" / p | dc`
    echo "  stl took ${tmp}% more time than cds"
else
    tmp=`echo "$cdsmaptime_ms" "$stlmaptime_ms" - 100 \* "$stlmaptime_ms" / p | dc`
    echo "  cds took ${tmp}% more time than stl"
fi

if [ "$cdsmapmem_MiB" -lt "$stlmapmem_MiB" ]; then
    tmp=`echo "$stlmapmem_MiB" "$cdsmapmem_MiB" - 100 \* "$cdsmapmem_MiB" / p | dc`
    echo "  stl used ${tmp}% more memory than cds"
else
    tmp=`echo "$cdsmapmem_MiB" "$stlmapmem_MiB" - 100 \* "$stlmapmem_MiB" / p | dc`
    echo "  cds used ${tmp}% more memory than stl"
fi
