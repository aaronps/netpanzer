/*
Copyright (C) 2003 Matthias Braun <matze@braunis.de>
                                                                                
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


#include <stdio.h>
#include <string.h>
#include "Exception.hpp"
#include "Log.hpp"
#include "FileSystem.hpp"
#include "physfs/physfs.h"

namespace filesystem
{

bool initialize( const char* argv0, const char* application )
{
    if( !PHYSFS_init(argv0) )
        throw Exception("failure while initialising physfs: %s",
                        PHYSFS_getLastError());

    const char* basedir = PHYSFS_getBaseDir();
    const char* userdir = PHYSFS_getUserDir();
    const char* dirsep = PHYSFS_getDirSeparator();
    
    std::string appdir(".");
    appdir += application;

    std::string writedir(userdir);
    writedir += appdir;
    
    if ( ! PHYSFS_setWriteDir(writedir.c_str()) )
    {
        // try to create the directory...
        if ( ! PHYSFS_setWriteDir(userdir) || ! PHYSFS_mkdir(appdir.c_str()) )
        {
            throw Exception("failed creating configuration directory: '%s': %s",
                            writedir.c_str(), PHYSFS_getLastError());
        }

        if ( ! PHYSFS_setWriteDir(writedir.c_str()) )
        {
            throw Exception("couldn't set configuration directory to '%s': %s",
                            writedir.c_str(), PHYSFS_getLastError());
        }
    }

    PHYSFS_addToSearchPath(writedir.c_str(), 0);
    PHYSFS_addToSearchPath(basedir, 1);

    /* Root out archives, and add them to search path... */
    const char* archiveExt = "zip";
    if (archiveExt != NULL) {
        char **rc = PHYSFS_enumerateFiles("/");
        char **i;
        size_t extlen = strlen(archiveExt);
        char *ext;

        for (i = rc; *i != NULL; i++) {
            size_t l = strlen(*i);
            if ((l > extlen) && ((*i)[l - extlen - 1] == '.')) {
                ext = (*i) + (l - extlen);
                if (strcasecmp(ext, archiveExt) == 0) {
                    LOGGER.warning("==== Adding zip: '%s'", *i);
                    const char *d = PHYSFS_getRealDir(*i);
                    char* str = new char[strlen(d) + strlen(dirsep) + l + 1];
                    sprintf(str, "%s%s%s", d, dirsep, *i);
                    PHYSFS_addToSearchPath(str, 0);
                    delete[] str;
                } /* if */
            } /* if */
        } /* for */

        PHYSFS_freeList(rc);
    } /* if */

#ifdef NP_DATADIR
    try {
	filesystem::addToSearchPath(NP_DATADIR);
    } catch(...)
    { }
#endif

#ifdef __APPLE__
    // Mac OS X puts the data files into NetPanzer.app/Contents/Resources
    try {
      std::ostringstream dir;
      dir << PHYSFS_getBaseDir() << "/NetPanzer.app/Contents/Resources/";
      filesystem::addToSearchPath(dir.str().c_str());
    } catch(...)
    { }
#endif
    
    PHYSFS_removeFromSearchPath(writedir.c_str());
    PHYSFS_addToSearchPath(writedir.c_str(), 0);
    return true;
}

void shutdown()
{
    PHYSFS_deinit();
}

void addToSearchPath(const char* directory, bool append)
{
    if(!PHYSFS_addToSearchPath(directory, append))
        throw Exception("Couldn't add '%s' to searchpath: %s", directory,
                        PHYSFS_getLastError());
}

void removeFromSearchPath(const char* directory)
{
    if(!PHYSFS_removeFromSearchPath(directory))
        throw Exception("Couldn't remove '%s' from searchpath: %s", directory,
                        PHYSFS_getLastError());
}

const char* getRealDir(const char* filename)
{
    return PHYSFS_getRealDir(filename);
}

std::string getRealName(const char* filename)
{
    const char* dir = PHYSFS_getRealDir(filename);
    if (dir == 0) {
        throw Exception("no such path '%s'", filename);
    }
    std::string realname = dir;
    realname += PHYSFS_getDirSeparator();
    realname += filename;
    return realname;
}

std::string getRealWriteName(const char* filename)
{
    const char* dir = PHYSFS_getWriteDir();
    if (dir == 0) {
        throw Exception("no writedir defined");
    }
    std::string realname = dir;
    realname += PHYSFS_getDirSeparator();
    realname += filename;
    return realname;
}

char** enumerateFiles(const char* directory)
{
    return PHYSFS_enumerateFiles(directory);
}

void freeList(char** list)
{
    PHYSFS_freeList(list);
}

WriteFile* openWrite(const char* filename)
{
    PHYSFS_file* file = PHYSFS_openWrite(filename);
    if(!file)
        throw Exception("couldn't open file '%s' for writing: %s", filename,
                        PHYSFS_getLastError());

    return new WriteFile(file);
}

WriteFile* openAppend(const char* filename)
{
    PHYSFS_file* file = PHYSFS_openAppend(filename);
    if(!file)
        throw Exception("couldn't open file '%s' for writing(append): %s",
                        filename, PHYSFS_getLastError());

    return new WriteFile(file);
}

void mkdir(const char* directory)
{
    if(!PHYSFS_mkdir(directory))
        throw Exception("couldn't create directory '%s': %s", directory,
                        PHYSFS_getLastError());
}

void remove(const char* filename)
{
    if(!PHYSFS_delete(filename))
        throw Exception("couldn't remove file '%s': %s", filename,
                        PHYSFS_getLastError());
}

bool exists(const char* filename)
{
    return PHYSFS_exists(filename);
}

bool isDirectory(const char* filename)
{
    return PHYSFS_isDirectory(filename);
}

bool isSymbolicLink(const char* filename)
{
    return PHYSFS_isSymbolicLink(filename);
}

int64_t getLastModTime(const char* filename)
{
    int64_t modtime = PHYSFS_getLastModTime(filename);
    if(modtime < 0)
        throw Exception("couldn't determine modification time of '%s': %s",
                        filename, PHYSFS_getLastError());

    return modtime;
}

//---------------------------------------------------------------------------

bool File::eof()
{
    return PHYSFS_eof(file);
}

int64_t File::tell()
{
    return PHYSFS_tell(file);
}

void File::seek(uint64_t position)
{
    if(!PHYSFS_seek(file, position))
        throw Exception("Seek operation failed: %s", PHYSFS_getLastError());
}

int64_t File::fileLength()
{
    return PHYSFS_fileLength(file);
}

void File::setBuffer(uint64_t bufsize)
{
    if(!PHYSFS_setBuffer(file, bufsize))
        throw Exception("couldn't adjust buffer size: %s",
                        PHYSFS_getLastError());
}

void File::flush()
{
    PHYSFS_flush(file); // no exception - what should an app do if flush fails?
}

//---------------------------------------------------------------------------

ReadFile::ReadFile(PHYSFS_file* file)
        : File(file)
{}

ReadFile::ReadFile(const std::string& name)
{
    PHYSFS_file* newfile = PHYSFS_openRead(name.c_str());
    if ( !newfile )
    {
		int fn_length = name.length()+1;
		char fn[fn_length];
		memcpy(fn, name.c_str(), fn_length); // includes \0;

		char * folder_sep = strrchr(fn, '/');
		char * fn_start = fn;
		char ** filelist = 0;
		if ( folder_sep )
		{
			*folder_sep = 0;
			filelist = enumerateFiles(fn);
			*folder_sep = '/';
			fn_start = folder_sep+1;
		}
		else
		{
			filelist = enumerateFiles(".");
			folder_sep = fn;
		}

		if ( filelist )
		{
			for(char** curfile = filelist; *curfile != 0; curfile++)
			{
				if ( strcasecmp(*curfile, fn_start) == 0 )
				{
					memcpy(fn_start, *curfile, fn_length-(folder_sep-fn));
					newfile = PHYSFS_openRead(fn);
					break;
				}
			}
			freeList(filelist);
		}
    }
    file = newfile;
}


size_t ReadFile::_read(void* buffer, size_t objsize, size_t objcount)
{
    return (size_t) PHYSFS_read(file, buffer, objsize, objcount);
}

void ReadFile::read(void* buffer, size_t objsize, size_t objcount)
{
    PHYSFS_sint64 objsread = PHYSFS_read(file, buffer, objsize, objcount);
    if(objsread != (PHYSFS_sint64) objcount)
	throw FileReadException(objsread, objcount, "eof while reading");
}

bool ReadFile::isEOF()
{
    return isOpen() && PHYSFS_eof(file);
}

int8_t ReadFile::read8()
{
    int8_t val;
    if(PHYSFS_read(file, &val, 1, 1) != 1)
        throw Exception("read error: %s", PHYSFS_getLastError());
    return val;
}

int16_t ReadFile::readSLE16()
{
    int16_t val;
    if(!PHYSFS_readSLE16(file, &val))
        throw Exception("read error: %s", PHYSFS_getLastError());
    return val;
}

uint16_t ReadFile::readULE16()
{
    uint16_t val;
    if(!PHYSFS_readULE16(file, &val))
        throw Exception("read error: %s", PHYSFS_getLastError());
    return val;
}

int16_t ReadFile::readSBE16()
{
    int16_t val;
    if(!PHYSFS_readSBE16(file, &val))
        throw Exception("read error: %s", PHYSFS_getLastError());
    return val;
}

uint16_t ReadFile::readUBE16()
{
    uint16_t val;
    if(!PHYSFS_readUBE16(file, &val))
        throw Exception("read error: %s", PHYSFS_getLastError());
    return val;
}

int32_t ReadFile::readSLE32()
{
    int32_t val;
    if(!PHYSFS_readSLE32(file, &val))
        throw Exception("read error: %s", PHYSFS_getLastError());
    return val;
}

uint32_t ReadFile::readULE32()
{
    uint32_t val;
    if(!PHYSFS_readULE32(file, &val))
        throw Exception("read error: %s", PHYSFS_getLastError());
    return val;
}

int32_t ReadFile::readSBE32()
{
    int32_t val;
    if(!PHYSFS_readSBE32(file, &val))
        throw Exception("read error: %s", PHYSFS_getLastError());
    return val;
}

uint32_t ReadFile::readUBE32()
{
    uint32_t val;
    if(!PHYSFS_readUBE32(file, &val))
        throw Exception("read error: %s", PHYSFS_getLastError());
    return val;
}

int64_t ReadFile::readSLE64()
{
    PHYSFS_sint64 val;
    if(!PHYSFS_readSLE64(file, &val))
        throw Exception("read error: %s", PHYSFS_getLastError());
    return val;
}

uint64_t ReadFile::readULE64()
{
    PHYSFS_uint64 val;
    if(!PHYSFS_readULE64(file, &val))
        throw Exception("read error: %s", PHYSFS_getLastError());
    return val;
}

int64_t ReadFile::readSBE64()
{
    PHYSFS_sint64 val;
    if(!PHYSFS_readSBE64(file, &val))
        throw Exception("read error: %s", PHYSFS_getLastError());
    return val;
}

uint64_t ReadFile::readUBE64()
{
    PHYSFS_uint64 val;
    if(!PHYSFS_readUBE64(file, &val))
        throw Exception("read error: %s", PHYSFS_getLastError());
    return val;
}

void ReadFile::readLine(std::string& buffer)
{
    if(isEOF())
        throw Exception("end of file while reading line");

    char c;
    buffer = "";

    while(!isEOF() && (c = read8()) != '\n') {
        buffer += c;
    }
}

//---------------------------------------------------------------------------

WriteFile::WriteFile(PHYSFS_file* file)
        : File(file)
{}

void WriteFile::write(const void* buffer, size_t objsize, size_t objcount)
{
    PHYSFS_sint64 objswritten = PHYSFS_write(file, buffer, objsize, objcount);
    if(objswritten != (PHYSFS_sint64) objcount)
	throw FileReadException(objswritten, objcount,
		"write not possible (disk full)?");
}

void WriteFile::write8(int8_t val)
{
    if(PHYSFS_write(file, &val, 1, 1) != 1)
        throw Exception("couldn't write: %s", PHYSFS_getLastError());
}

void WriteFile::writeSLE16(int16_t val)
{
    if(!PHYSFS_writeSLE16(file, val))
        throw Exception("couldn't write: %s", PHYSFS_getLastError());
}

void WriteFile::writeULE16(uint16_t val)
{
    if(!PHYSFS_writeULE16(file, val))
        throw Exception("couldn't write: %s", PHYSFS_getLastError());
}

void WriteFile::writeSBE16(int16_t val)
{
    if(!PHYSFS_writeSBE16(file, val))
        throw Exception("couldn't write: %s", PHYSFS_getLastError());
}

void WriteFile::writeUBE16(uint16_t val)
{
    if(!PHYSFS_writeUBE16(file, val))
        throw Exception("couldn't write: %s", PHYSFS_getLastError());
}

void WriteFile::writeSLE32(int32_t val)
{
    if(!PHYSFS_writeSLE32(file, val))
        throw Exception("couldn't write: %s", PHYSFS_getLastError());
}

void WriteFile::writeULE32(uint32_t val)
{
    if(!PHYSFS_writeULE32(file, val))
        throw Exception("couldn't write: %s", PHYSFS_getLastError());
}

void WriteFile::writeSBE32(int32_t val)
{
    if(!PHYSFS_writeSBE32(file, val))
        throw Exception("couldn't write: %s", PHYSFS_getLastError());
}

void WriteFile::writeUBE32(uint32_t val)
{
    if(!PHYSFS_writeUBE32(file, val))
        throw Exception("couldn't write: %s", PHYSFS_getLastError());
}

void WriteFile::writeSLE64(int64_t val)
{
    if(!PHYSFS_writeSLE64(file, val))
        throw Exception("couldn't write: %s", PHYSFS_getLastError());
}

void WriteFile::writeULE64(uint64_t val)
{
    if(!PHYSFS_writeULE64(file, val))
        throw Exception("couldn't write: %s", PHYSFS_getLastError());
}

void WriteFile::writeSBE64(int64_t val)
{
    if(!PHYSFS_writeSBE64(file, val))
        throw Exception("couldn't write: %s", PHYSFS_getLastError());
}

void WriteFile::writeUBE64(uint64_t val)
{
    if(!PHYSFS_writeUBE64(file, val))
        throw Exception("couldn't write: %s", PHYSFS_getLastError());
}

void WriteFile::writeLine(const std::string& buffer)
{
    write(buffer.c_str(), buffer.size(), 1);
    write8('\n');
}

//---------------------------------------------------------------------------

FileReadException::FileReadException(size_t newobjread, size_t newobjrequested,
	const std::string& msg) throw()
    : Exception(msg.c_str()), objread(newobjread),
	objrequested(newobjrequested)
{
}

FileReadException::~FileReadException() throw()
{
}

}
