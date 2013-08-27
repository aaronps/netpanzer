/*
Copyright (C) 1998 Pyrosoft Inc. (www.pyrosoftgames.com), Matthew Bogue
 
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
 
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
 
You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


#include <iostream>
#include <string>
#include <algorithm>
#include <memory>

#include "Util/TimerInterface.hpp"
#include "Util/FileSystem.hpp"
#include "Util/Exception.hpp"
#include "Util/Endian.hpp"
#include "Util/StringUtil.hpp"
#include "PackedSurface.hpp"
#include "Surface.hpp"

#ifdef MSVC
#pragma pack (1)
#endif
struct SpanHead
{
    unsigned short x1;
    unsigned short len;
}
__attribute__((packed));
#ifdef MSVC
#pragma pack ()
#endif

#define TRANSPIX 0

int PackedSurface::totalSurfaceCount = 0;  // The number of surfaces alive.
int PackedSurface::totalByteCount    = 0;  // The number of bytes of the surfaces alive.
int PackedSurface::totalDrawCount    = 0;  // The number of bytes of the surfaces alive.

// PAK file format revision history:
//--------------------------------------
// 1 MB Initial version

const int CURRENT_PAK_VERSION = 1;

//--------------------------------------------------------------------------
inline void bltBlendSpan(PIX *dRow, const PIX *sRow, size_t pixelsPerRow, const Uint8 *table)
{
    for( size_t idx=0; idx<pixelsPerRow; idx++ )
    {
        dRow[idx] = table[ (dRow[idx]<<8)+sRow[idx] ];
    }
}

//--------------------------------------------------------------------------
void PackedSurface::setOffset(const iXY &offset)
{
    PackedSurface::offset = offset;
}

//--------------------------------------------------------------------------
void PackedSurface::setOffsetCenter()
{
    PackedSurface::offset = pix / -2;
}

//--------------------------------------------------------------------------
void PackedSurface::free()
{
    if (myMem && rowOffsetTable != 0) {
        totalByteCount -= (pix.y * frameCount + 1) * sizeof(*rowOffsetTable);

        ::free(rowOffsetTable);

        //assert(totalByteCount >= 0);
    }
    if (myMem && packedDataChunk != 0) {
        totalByteCount -= pix.y * frameCount;

        ::free(packedDataChunk);

        //assert(totalByteCount >= 0);
    }

    reset();
}

//--------------------------------------------------------------------------
void PackedSurface::reset()
{
    pix.zero();
    rowOffsetTable  = 0;
    packedDataChunk = 0;
    frameCount      = 0;
    curFrame        = 0.0;
    fps             = 30.0;
    myMem           = true;

    center.zero();
    offset.zero();
}

//--------------------------------------------------------------------------
void PackedSurface::pack(const PtrArray<Surface>& source)
{
    free();

    pix.x      = source.get(0)->getWidth();
    pix.y      = source.get(0)->getHeight();
    frameCount = source.size();
    fps        = 0; // source.getFPS();

    rowOffsetTable = (int *) malloc((pix.y*frameCount + 1) * sizeof(*rowOffsetTable));
    if (rowOffsetTable == 0) {
        throw Exception("ERROR: Unable to allocate rowTableOffset for PackedSurface.");
    }

    int bytesAlloced = 0;
    packedDataChunk = 0;
    int curByteOffset = 0;

    for (int frame = 0 ; frame < frameCount ; ++frame)
    {
        for (int y = 0 ; y < pix.y ; ++y)
        {
            rowOffsetTable[frame*pix.y + y] = curByteOffset;
            const PIX *rowPtr = source.get(frame)->pixPtr(0,y);

            int x = 0;
            while (x < pix.x) {

                // transparent pixels until first non-transparent pixel

                if (rowPtr[x] == TRANSPIX) {
                    x++;
                    continue;
                }

                // Now scan forward until we hit the end of the span,
                // either the right side of the bitmap or a transparent pixel

                int spanX1 = x;
                do {
                    x++;
                } while((x < pix.x) && (rowPtr[x] != TRANSPIX));
                int spanLen = x - spanX1;

                // Check if we need to grow buffer to add the span

                int newSize = (curByteOffset + sizeof(SpanHead) + spanLen*sizeof(PIX) + 3) & ~3;
                if (newSize > bytesAlloced) {
                    bytesAlloced = newSize + 16*1024;
                    packedDataChunk = (Uint8 *)realloc(packedDataChunk, bytesAlloced);
                    if (packedDataChunk == 0) {
                        throw Exception("ERROR: Out of memory for packedDataChunk for PackedSurface.");
                    }
                }

                // Now write the data into the chunk.

                SpanHead *span = (SpanHead *)(packedDataChunk + curByteOffset);
                span->x1 = htol16(spanX1);
                span->len = htol16(spanLen);

                // to make same files as old pak files, has to memset to 'cd'
                memset(span+1,0xcd, newSize-curByteOffset-sizeof(SpanHead));
                memcpy(span + 1, &rowPtr[spanX1], spanLen * sizeof(PIX));
                curByteOffset = newSize;
            }
        }
    }

    // Shove in final offset table entry, which we need to know the
    // length of data for the last row of the last frame

    rowOffsetTable[frameCount*pix.y] = curByteOffset;

    // Shrink buffer to the size we really need

    packedDataChunk = (Uint8 *) realloc(packedDataChunk, curByteOffset);
    if (packedDataChunk == 0) throw Exception("Hell froze");

}

//--------------------------------------------------------------------------
void PackedSurface::load(const std::string& filename)
{
    filesystem::ReadFile file(filename);
    if ( file.isOpen() )
    {
        free();
        Sint32 version = file.readSLE32();
        
        if (version < 1)
        {
            throw Exception("Invalid PAK file version: %d", version);
        }
        
        if (version > CURRENT_PAK_VERSION)
        {
            throw Exception("PAK file version of '%s' is newer(%d) than "
                "the currently supported version(%d)",
            filename.c_str(), version, CURRENT_PAK_VERSION);
        }
        
        pix.x = file.readSLE32();
        pix.y = file.readSLE32();

        center = pix / 2;

        frameCount = file.readSLE32(); 
        // should be done like following but this isn't backward compatible to
        // the existing files :-/
        //fps = float(file->readSLE32()) / 65536;
        // XXX is this correct?!?
        Sint32 fpsint = file.readSLE32();
        fps = * ((float*) (void*) &fpsint);
        offset.x = file.readSLE32();
        offset.y = file.readSLE32();
        
        rowOffsetTable = (int *) malloc((pix.y * frameCount + 1) * sizeof(*rowOffsetTable));
        if (rowOffsetTable == 0)
        {
            throw Exception("ERROR: Unable to allocate rowTableOffset for PackedSurface.");
        }
        
        for(int i=0;i<(pix.y * frameCount+1);i++)
        {
            rowOffsetTable[i] = file.readSLE32();
        }
        
        packedDataChunk = (Uint8 *)malloc(rowOffsetTable[pix.y*frameCount]);
        if (packedDataChunk == 0)
        {
            throw Exception("ERROR: Unable to allocate packedDataChunk for PackedSurface.");
        }
        file.read(packedDataChunk, rowOffsetTable[pix.y*frameCount], 1);

        // Add size of rowTableOffset.
        totalByteCount += (pix.y * frameCount + 1) * sizeof(*rowOffsetTable);

        // Add size of packedDataChunk.
        totalByteCount += pix.y * frameCount;
    }
    else
    {
        throw Exception("Pak file not found '%s'", filename.c_str());
    }
}

//--------------------------------------------------------------------------
void PackedSurface::save(const std::string& filename) const
{
    try {
	std::auto_ptr<filesystem::WriteFile> file(
                filesystem::openWrite(filename));

	Sint32 version = CURRENT_PAK_VERSION;
	file->writeSLE32(version);
	file->writeSLE32(pix.x);
	file->writeSLE32(pix.y);
    
        file->writeSLE32(frameCount);

	// is this correct?!?
	file->writeSLE32( *((Uint32*) (void*) (&fps)) );

	file->writeSLE32(offset.x);
	file->writeSLE32(offset.y);
    
	for(int i=0;i<(pix.y*frameCount+1); i++) {
	    file->writeSLE32(rowOffsetTable[i]);
	}
    
	file->write(packedDataChunk, rowOffsetTable[pix.y*frameCount], 1);
    } catch(std::exception& e) {
	throw Exception("Error while writing pakfile '%s': %s",
		filename.c_str(), e.what());
    }
}

//--------------------------------------------------------------------------
void PackedSurface::blt(Surface &dest, int destX, int destY) const
{
    totalDrawCount++;

    destX += offset.x;
    destY += offset.y;

    // Clip and trivial reject

    int needClipX = 0;

    iXY srcMax;
    srcMax.x = dest.getWidth() - destX;
    if (srcMax.x <= 0) return; // off right
    srcMax.y = dest.getHeight() - destY;
    if (srcMax.y <= 0) return; // off bottom

    iXY srcMin;
    if (destX < 0) {
        srcMin.x = -destX;
        if (srcMin.x >= pix.x) return; // off left
        needClipX = 1;
    } else {
        srcMin.x = 0;
    }
    if (destY < 0) {
        srcMin.y = -destY;
        if (srcMin.y >= pix.y) return; // off right
    } else {
        srcMin.y = 0;
    }

    if (srcMax.x > pix.x) {
        srcMax.x = pix.x;
    } else {
        needClipX = 1;
    }
    if (srcMax.y > pix.y) {
        srcMax.y = pix.y;
    }

    const int *table = &rowOffsetTable[int(curFrame)*pix.y];

    if (needClipX) {
        const Uint8 *rowData = packedDataChunk + table[srcMin.y];
        PIX *destRowPtr = dest.pixPtr(0, destY + srcMin.y) + destX;
        for (int y = srcMin.y ; y < srcMax.y ; ++y) {

            const Uint8 *rowEnd = packedDataChunk + table[y+1];

            // Search for first span which is not completely off to the left

            int x1, x2;
            SpanHead *span;
            const PIX *data;
            for (;;) {
                if (rowData >= rowEnd) goto nextRow;
                span = (SpanHead *)rowData;
                x1 = ltoh16(span->x1);
                x2 = x1 + ltoh16(span->len);
                rowData += (sizeof(*span) + ltoh16(span->len)*sizeof(PIX) + 3) & ~3;
                if (x2 > srcMin.x) break;
            }

            // Clip against left edge

            data = (const PIX *)(span + 1);
            if (x1 < srcMin.x) {
                data += srcMin.x - x1;
                x1 = srcMin.x;
            }

            // Output spans to the screen until we hit the first
            // span which is partially or completely out to the
            // right

            for (;;) {
                if (x1 >= srcMax.x) {
                    rowData = rowEnd;
                    goto nextRow;
                }

                // Clip current span against right edge
                if (x2 > srcMax.x) {
                    memcpy(destRowPtr + x1, data, (srcMax.x-x1) * sizeof(PIX));
                    rowData = rowEnd;
                    goto nextRow;
                }

                memcpy(destRowPtr + x1, data, (x2-x1) * sizeof(PIX));

                if (rowData >= rowEnd) goto nextRow;
                span = (SpanHead *)rowData;
                x1 = ltoh16(span->x1);
                if (x1 >= srcMax.x) {
                    rowData = rowEnd;
                    goto nextRow;
                }
                x2 = x1 + ltoh16(span->len);
                data = (const PIX *)(span + 1);
                rowData += (sizeof(*span) + ltoh16(span->len)*sizeof(PIX) + 3) & ~3;
            }

nextRow:
            assert(rowData == rowEnd);
            destRowPtr += dest.getPitch();
        }
    } else {
        const Uint8 *rowData = packedDataChunk + table[srcMin.y];
        PIX *destRowPtr = dest.pixPtr(0,destY + srcMin.y) + destX;
        for (int y = srcMin.y ; y < srcMax.y ; ++y) {

            const Uint8 *rowEnd = packedDataChunk + table[y+1];

            while (rowData < rowEnd) {
                SpanHead *span = (SpanHead *)rowData;
                memcpy(destRowPtr + ltoh16(span->x1), span + 1,
                    ltoh16(span->len) * sizeof(PIX));
                rowData += (sizeof(*span) + ltoh16(span->len)*sizeof(PIX) + 3) & ~3;
            }
            assert(rowData == rowEnd);
            destRowPtr += dest.getPitch();
        }
    }

}

// setTo
//---------------------------------------------------------------------------
// Purpose: Maps the calling PackedSurface to some specified coordinates of
//          the another PackedSurface.
//---------------------------------------------------------------------------
void PackedSurface::setTo(const PackedSurface &source)
{
    assert(this != 0);

    free();

    frameCount      = source.getFrameCount();
    fps             = source.getFPS();
    offset          = source.getOffset();
    center          = source.getCenter();
    pix             = source.getPix();
    rowOffsetTable  = source.getRowOffsetTable();
    curFrame        = source.getCurFrame();
    packedDataChunk = source.getPackedDataChunk();
    myMem           = false;

} // end Surface::setTo

// nextFrame
//---------------------------------------------------------------------------
// Purpose: Move to the next frame based off time and the fps.  Returns a 0 if it
//          was the last frame and 1 otherwise.
//---------------------------------------------------------------------------
int PackedSurface::nextFrame()
{
    curFrame += TimerInterface::getTimeSlice() * fps;

    if (curFrame >= frameCount) {
        curFrame = 0.0;
        return 0;
    }

    return 1;
}

// bltBlend
//--------------------------------------------------------------------------
void PackedSurface::bltBlend(Surface &dest, int destX, int destY, const uint8_t * blendTable) const
{
    totalDrawCount++;

    destX += offset.x;
    destY += offset.y;

    // Clip and trivial reject

    int needClipX = 0;

    iXY srcMax;
    srcMax.x = dest.getWidth() - destX;
    if (srcMax.x <= 0) return; // off right
    srcMax.y = dest.getHeight() - destY;
    if (srcMax.y <= 0) return; // off bottom

    iXY srcMin;
    if (destX < 0) {
        srcMin.x = -destX;
        if (srcMin.x >= pix.x) return; // off left
        needClipX = 1;
    } else {
        srcMin.x = 0;
    }
    if (destY < 0) {
        srcMin.y = -destY;
        if (srcMin.y >= pix.y) return; // off right
    } else {
        srcMin.y = 0;
    }

    if (srcMax.x > pix.x) {
        srcMax.x = pix.x;
    } else {
        needClipX = 1;
    }
    if (srcMax.y > pix.y) {
        srcMax.y = pix.y;
    }

    const int  *table  = &rowOffsetTable[int(curFrame)*pix.y];

    if (needClipX) {
        const Uint8 *rowData = packedDataChunk + table[srcMin.y];
        PIX *destRowPtr = dest.pixPtr(0, destY + srcMin.y) + destX;
        for (int y = srcMin.y ; y < srcMax.y ; ++y) {

            const Uint8 *rowEnd = packedDataChunk + table[y+1];

            // Search for first span which is not completely off to the left

            int x1, x2;
            SpanHead *span;
            const PIX *data;
            for (;;) {
                if (rowData >= rowEnd) goto nextRow;
                span = (SpanHead *)rowData;
                x1 = ltoh16(span->x1);
                x2 = x1 + ltoh16(span->len);
                rowData += (sizeof(*span) + ltoh16(span->len)*sizeof(PIX) + 3) & ~3;
                if (x2 > srcMin.x) break;
            }

            // Clip against left edge

            data = (const PIX *)(span + 1);
            if (x1 < srcMin.x) {
                data += srcMin.x - x1;
                x1 = srcMin.x;
            }

            // Output spans to the screen until we hit the first
            // span which is partially or completely out to the
            // right

            for (;;) {
                if (x1 >= srcMax.x) {
                    rowData = rowEnd;
                    goto nextRow;
                }

                // Clip current span against right edge
                if (x2 > srcMax.x) {
                    bltBlendSpan(destRowPtr + x1, data, srcMax.x - x1, blendTable);

                    rowData = rowEnd;
                    goto nextRow;
                }

                bltBlendSpan(destRowPtr + x1, data, x2 - x1, blendTable);

                if (rowData >= rowEnd) goto nextRow;
                span = (SpanHead *)rowData;
                x1 = ltoh16(span->x1);
                if (x1 >= srcMax.x) {
                    rowData = rowEnd;
                    goto nextRow;
                }
                x2 = x1 + ltoh16(span->len);
                data = (const PIX *)(span + 1);
                rowData += (sizeof(*span) + ltoh16(span->len)*sizeof(PIX) + 3) & ~3;
            }

nextRow:
            assert(rowData == rowEnd);
            destRowPtr += dest.getPitch();
        }
    } else {
        const Uint8 *rowData = packedDataChunk + table[srcMin.y];
        PIX *destRowPtr = dest.pixPtr(0, destY + srcMin.y) + destX;
        for (int y = srcMin.y ; y < srcMax.y ; ++y) {

            const Uint8 *rowEnd = packedDataChunk + table[y+1];

            while (rowData < rowEnd) {
                SpanHead *span = (SpanHead *)rowData;

                bltBlendSpan(destRowPtr + ltoh16(span->x1),
                    (const PIX *)(span + 1), ltoh16(span->len), blendTable);

                rowData += (sizeof(*span) + ltoh16(span->len)*sizeof(PIX) + 3) & ~3;
            }
            assert(rowData == rowEnd);
            destRowPtr += dest.getPitch();
        }
    }

} // end PackedSurface::bltBlend

// loadAllPAKInDirectory
//---------------------------------------------------------------------------
int loadAllPAKInDirectory(const char *path, PackedSurfaceList& paklist)
{
    char** list = filesystem::enumerateFiles(path);

    PtrArray<char> fnames(20);

    for ( char** file = list; *file != 0; file++ )
    {
        if ( strstr(*file, ".pak") )
        {
            fnames.push_back(*file);
        }
    }
   
    std::sort(fnames.begin(), fnames.end(), StringUtil::cstr_sorter());

    char fil[512];
    int len = strlen(path);
    memcpy(fil, path, len);

    // Now load in the sorted PAK names.
    for (size_t i = 0; i < fnames.size(); i++)
    {
        PackedSurface* surface = new PackedSurface();
        
        strcpy(fil+len, fnames[i]);
        surface->load(fil);
        
        paklist.push_back(surface);
    }

    filesystem::freeList(list);

    return fnames.size();
} // end loadAllPAKInDirectory

