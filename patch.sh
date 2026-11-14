#!/bin/bash
set -e

BUILDROOT_OUT="$(dirname "$0")/buildroot/output/build"

# zstd 1.5.7 / programs/fileio.c

# what this changes (diff):

# -    prefs.frameInfo.contentChecksumFlag = (contentChecksum_t)checksumFlag;                                                  
# +    prefs.frameInfo.contentChecksumFlag = (LZ4F_contentChecksum_t)checksumFlag;  


ZSTD_FILEIO="$BUILDROOT_OUT/host-zstd-1.5.7/programs/fileio.c"

if [ -f "$ZSTD_FILEIO" ]; then
    if grep -q '(contentChecksum_t)checksumFlag' "$ZSTD_FILEIO"; then
        sed -i 's/(contentChecksum_t)checksumFlag/(LZ4F_contentChecksum_t)checksumFlag/' "$ZSTD_FILEIO"
        echo "[patch 1 applied] zstd: contentChecksum_t -> LZ4F_contentChecksum_t"
    else
        echo "[patch 1 skipped] zstd: already patched or not needed"
    fi
else
    echo "[patch 1 skipped] zstd source not yet extracted (run make once to extract, then re-run this script)"
fi
