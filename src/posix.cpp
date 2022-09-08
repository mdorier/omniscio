/******************************************************************************
 Copyright (c) 2014 ENS Rennes, Inria Rennes Bretagne Atlantique
 All rights reserved.
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
 * Neither the name of the University of California, Berkeley nor the
   names of its contributors may be used to endorse or promote products
   derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND ANY
 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

#pragma GCC diagnostic ignored "-fpermissive"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <set>
#define __GNU_SOURCE
#ifndef __USE_GNU
#define __USE_GNU
#endif
#include <dlfcn.h>

#include "omniscio.h"

static std::set<int> allowed_fds;
static std::set<FILE*> allowed_streams;

#define IS_ALLOWED_FD(fd) allowed_fds.count(fd)

#define IS_ALLOWED_FILE(f) allowed_streams.count(f)

#define offset64_t __off64_t
#define offset_t off_t

extern "C" {

#define FORWARD_DECL(name,ret,args) \
	ret (*_libc_ ## name)args = NULL;

#define MAP_OR_FAIL(func) \
	if(omniscio_tracing_enabled_p == 0) { \
		omniscio_tracing_enabled_p \
		= dlsym(RTLD_DEFAULT, "omniscio_tracing_enabled"); \
		if(omniscio_tracing_enabled_p == 0) { \
			omniscio_tracing_enabled_p = \
				&omniscio_tracing_disabled;\
		}\
	} \
	if (!(_libc_ ## func) && \
			(omniscio_tracing_enabled_p != \
			 &omniscio_tracing_disabled)) { \
		_libc_ ## func = dlsym(RTLD_NEXT, #func); \
		if(!(_libc_ ## func)) { \
			omniscio_tracing_enabled = 0; \
			fprintf(stderr, "Failed to map symbol: %s\n", #func); \
			exit(1); \
		} \
	}

int* omniscio_tracing_enabled_p;
int omniscio_tracing_disabled = 0;
#define omniscio_tracing_enabled *omniscio_tracing_enabled_p

FORWARD_DECL(open64, int, (const char *path, int flags, ...))
FORWARD_DECL(open, int, (const char *path, int flags, ...))
FORWARD_DECL(creat, int, (const char* path, mode_t mode))
FORWARD_DECL(creat64, int, (const char* path, mode_t mode))
FORWARD_DECL(close, int, (int fd))
FORWARD_DECL(write, ssize_t, (int fd, const void *buf, size_t count))
FORWARD_DECL(read, ssize_t, (int fd, void *buf, size_t count))
FORWARD_DECL(pread, ssize_t, (int fd, void *buf, size_t count, offset_t offset))
FORWARD_DECL(pread64, ssize_t, (int fd, void *buf, size_t count, offset_t offset))
FORWARD_DECL(pwrite, ssize_t, (int fd, const void *buf, size_t count, offset_t offset))
FORWARD_DECL(pwrite64, ssize_t, (int fd, const void *buf, size_t count, offset_t offset))

FORWARD_DECL(fopen, FILE*, (const char *path, const char *mode))
FORWARD_DECL(fopen64, FILE*, (const char *path, const char *mode))
FORWARD_DECL(fclose, int, (FILE *fp))
FORWARD_DECL(fread, size_t, (void *ptr, size_t size, size_t nmemb, FILE *stream))
FORWARD_DECL(fwrite, size_t, (const void *ptr, size_t size, size_t nmemb, FILE *stream))

FILE *fopen(const char *path, const char *mode) 
{
	MAP_OR_FAIL(fopen);
	bool is_allowed = false;
	if(strstr(path,"/dev/") != path) {
		is_allowed = true;
	}

	if(omniscio_tracing_enabled && is_allowed) {
		OMNISCIO_UNTRACED_START;
		omniscio_open_start(path,OMNISCIO_LIBC);
		FILE* result = _libc_fopen(path,mode);
		omniscio_file ofile;
		omniscio_file_from_libc(&ofile,result);
		omniscio_open_end((result != 0 ? 0 : -1),ofile);
		allowed_streams.insert(result);
		OMNISCIO_UNTRACED_END;
		return result;
	} else {
		return _libc_fopen(path,mode);
	}
}

FILE *fopen64(const char *path, const char *mode)
{
	MAP_OR_FAIL(fopen64);
	bool is_allowed = false;
	if(strstr(path,"/dev/") != path) {
		is_allowed = true;
	}
	if(omniscio_tracing_enabled && is_allowed) {
		OMNISCIO_UNTRACED_START;
		omniscio_open_start(path,OMNISCIO_LIBC);
		FILE* result = _libc_fopen64(path,mode);
		omniscio_file ofile;
		omniscio_file_from_libc(&ofile,result);
		omniscio_open_end((result != 0 ? 0 : -1),ofile);
		allowed_streams.insert(result);
		OMNISCIO_UNTRACED_END;
		return result;
	} else {
		return _libc_fopen64(path,mode);
	}
}

int open(const char *pathname, int flags, ...)
{
	MAP_OR_FAIL(open);
	bool is_allowed = false;
	if(strstr(pathname,"/dev/") != pathname) {
		is_allowed = true;
	}
	if(omniscio_tracing_enabled && is_allowed) {
		OMNISCIO_UNTRACED_START;
		omniscio_open_start(pathname,OMNISCIO_POSIX);
		int result;
		if(flags & O_CREAT) {
			va_list arg;
			va_start(arg, flags);
			int mode = va_arg(arg, int);
			va_end(arg);
			result = _libc_open(pathname,flags,mode);
		} else {
			result = _libc_open(pathname,flags);
		}
		omniscio_file ofile;
		omniscio_file_from_posix(&ofile,result);
		omniscio_open_end((result != -1 ? 0 : -1),ofile);
		allowed_fds.insert(result);
		OMNISCIO_UNTRACED_END;
		return result;
	} else {
		if(flags & O_CREAT) {
			va_list arg;
			va_start(arg, flags);
			int mode = va_arg(arg, int);
			va_end(arg);
			return _libc_open(pathname,flags,mode);
		} else {
		 	return _libc_open(pathname,flags);
		}
	}
}

int open64(const char *pathname, int flags, ...)
{
	MAP_OR_FAIL(open64);
	bool is_allowed = false;
	if(strstr(pathname,"/dev/") != pathname) {
		is_allowed = true;
	}
	if(omniscio_tracing_enabled && is_allowed) {
		OMNISCIO_UNTRACED_START;
		omniscio_open_start(pathname,OMNISCIO_POSIX);
		int result;
		if(flags & O_CREAT) {
			va_list arg;
			va_start(arg, flags);
			int mode = va_arg(arg, int);
			va_end(arg);
			result = _libc_open64(pathname,flags,mode);
		} else {
			result = _libc_open64(pathname,flags);
		}
		omniscio_file ofile;
		omniscio_file_from_posix(&ofile,result);
		omniscio_open_end((result != -1 ? 0 : -1),ofile);
		allowed_fds.insert(result);
		OMNISCIO_UNTRACED_END;
		return result;
	} else {
		if(flags & O_CREAT) {
			va_list arg;
			va_start(arg, flags);
			int mode = va_arg(arg, int);
			va_end(arg);
			return _libc_open64(pathname,flags,mode);
		} else {
			return _libc_open64(pathname,flags);
		}
	}
}

int creat(const char* path, mode_t mode)
{
	MAP_OR_FAIL(creat);
	bool is_allowed = false;
	if(strstr(path,"/dev/") != path) {
		is_allowed = true;
	}
	if(omniscio_tracing_enabled && is_allowed) {
		OMNISCIO_UNTRACED_START;
		omniscio_open_start(path,OMNISCIO_POSIX);
		int result = _libc_creat(path,mode);
		omniscio_file ofile;
		omniscio_file_from_posix(&ofile,result);
		omniscio_open_end((result != -1 ? 0 : -1),ofile);
		allowed_fds.insert(result);
		OMNISCIO_UNTRACED_END;
		return result;
	} else {
		return _libc_creat(path,mode);
	}
}

int creat64(const char* path, mode_t mode)
{
	MAP_OR_FAIL(creat64);
	bool is_allowed = false;
	if(strstr(path,"/dev/") != path) {
		is_allowed = true;
	}
	if(omniscio_tracing_enabled && is_allowed) {
		OMNISCIO_UNTRACED_START;
		omniscio_open_start(path,OMNISCIO_POSIX);
		int result = _libc_creat64(path,mode);
		omniscio_file ofile;
		omniscio_file_from_posix(&ofile,result);
		omniscio_open_end((result != -1 ? 0 : -1),ofile);
		allowed_fds.insert(result);
		OMNISCIO_UNTRACED_END;
		return result;
	} else {
		return _libc_creat64(path,mode);
	}
}

ssize_t pread(int fd, void *buf, size_t count, offset_t offset)
{
	MAP_OR_FAIL(pread);
	struct stat s;
	fstat(fd, &s);
	if(omniscio_tracing_enabled 
	&& S_ISREG(s.st_mode) 
	&& fd > 2 && IS_ALLOWED_FD(fd)) {
		OMNISCIO_UNTRACED_START;
		omniscio_file ofile;
		omniscio_file_from_posix(&ofile,fd);
		omniscio_read_start(ofile,offset,count);
		ssize_t result = _libc_pread(fd,buf,count,offset);
		omniscio_read_end(((size_t)result == count) ? 0 : -1);
		OMNISCIO_UNTRACED_END;
		return result;
	} else {
		return _libc_pread(fd,buf,count,offset);
	}
}

ssize_t pread64(int fd, void *buf, size_t count, offset64_t offset)
{
	MAP_OR_FAIL(pread64);
	struct stat s;
	fstat(fd, &s);
	if(omniscio_tracing_enabled 
	&& S_ISREG(s.st_mode) 
	&& fd > 2 && IS_ALLOWED_FD(fd)) {
		OMNISCIO_UNTRACED_START;
		omniscio_file ofile;
		omniscio_file_from_posix(&ofile,fd);
		omniscio_read_start(ofile,offset,count);
		ssize_t result = _libc_pread64(fd,buf,count,offset);
		omniscio_read_end(((size_t)result == count) ? 0 : -1);
		OMNISCIO_UNTRACED_END;
		return result;
	} else {
		return _libc_pread64(fd,buf,count,offset);
	}
}

ssize_t pwrite(int fd, const void *buf, size_t count, offset_t offset)
{
	MAP_OR_FAIL(pwrite);
	struct stat s;
	fstat(fd, &s);
	if(omniscio_tracing_enabled 
	&& S_ISREG(s.st_mode) 
	&& fd > 2 && IS_ALLOWED_FD(fd)) {
		OMNISCIO_UNTRACED_START;
		omniscio_file ofile;
		omniscio_file_from_posix(&ofile,fd);
		omniscio_write_start(ofile,offset,count);
		ssize_t result = _libc_pwrite(fd,buf,count,offset);
		omniscio_write_end(((size_t)result == count) ? 0 : -1);
		OMNISCIO_UNTRACED_END;
		return result;
	} else {
		return _libc_pwrite(fd,buf,count,offset);
	}
}

ssize_t pwrite64(int fd, const void *buf, size_t count, offset64_t offset)
{
	MAP_OR_FAIL(pwrite64);
	struct stat s;
	fstat(fd, &s);
	if(omniscio_tracing_enabled 
	&& S_ISREG(s.st_mode) 
	&& fd > 2 && IS_ALLOWED_FD(fd)) {
		OMNISCIO_UNTRACED_START;
		omniscio_file ofile;
		omniscio_file_from_posix(&ofile,fd);
		omniscio_write_start(ofile,offset,count);
		ssize_t result = _libc_pwrite64(fd,buf,count,offset);
		omniscio_write_end(((size_t)result == count) ? 0 : -1);
		OMNISCIO_UNTRACED_END;
		return result;
	} else {
		return _libc_pwrite64(fd,buf,count,offset);
	}
}

size_t fwrite(const void *ptr, size_t size, size_t count, FILE *stream)
{
	MAP_OR_FAIL(fwrite);
	if(omniscio_tracing_enabled && IS_ALLOWED_FILE(stream)) {
		OMNISCIO_UNTRACED_START;
		long int pos = ftell(stream);
		omniscio_file ofile;
		omniscio_file_from_libc(&ofile,stream);
		omniscio_write_start(ofile,pos,(size*count));
		size_t result = _libc_fwrite(ptr,size,count,stream);
		omniscio_write_end((result == count) ? 0 : -1);
		OMNISCIO_UNTRACED_END;
		return result;
	} else {
		return _libc_fwrite(ptr,size,count,stream);
	}
}

ssize_t write(int fd, const void *buf, size_t count)
{
	MAP_OR_FAIL(write);
	struct stat s;
	fstat(fd, &s);
	if(omniscio_tracing_enabled 
	&& S_ISREG(s.st_mode) 
	&& fd > 2 && IS_ALLOWED_FD(fd)) {
		OMNISCIO_UNTRACED_START;
		off_t pos = lseek(fd, 0, SEEK_CUR);
		omniscio_file ofile;
		omniscio_file_from_posix(&ofile,fd);
		omniscio_write_start(ofile,pos,count);
		size_t result = _libc_write(fd,buf,count);
		omniscio_write_end((result == count) ? 0 : -1);
		OMNISCIO_UNTRACED_END;
		return result;
	} else {
		return _libc_write(fd,buf,count);
	}
}

size_t fread(void *ptr, size_t size, size_t count, FILE *stream) 
{
	MAP_OR_FAIL(fread);
	if(omniscio_tracing_enabled && IS_ALLOWED_FILE(stream)) {
		OMNISCIO_UNTRACED_START;
		long int pos = ftell(stream);
		omniscio_file ofile;
		omniscio_file_from_libc(&ofile,stream);
		omniscio_read_start(ofile,pos,size*count);
		size_t result = _libc_fread(ptr,size,count,stream);
		omniscio_read_end((result > 0) ? 0 : -1);
		OMNISCIO_UNTRACED_END;
		return result;
	} else {
		return _libc_fread(ptr,size,count,stream);
	}
}

ssize_t read(int fd, void *buf, size_t count)
{
	MAP_OR_FAIL(read);
	struct stat s;
	fstat(fd, &s);
	if(omniscio_tracing_enabled 
	&& S_ISREG(s.st_mode) 
	&& fd > 2 && IS_ALLOWED_FD(fd)) {
		OMNISCIO_UNTRACED_START;
		off_t pos = lseek(fd, 0, SEEK_CUR);
		omniscio_file ofile;
		omniscio_file_from_posix(&ofile,fd);
		omniscio_read_start(ofile,pos,count);
		size_t result = _libc_read(fd,buf,count);
		omniscio_read_end((result > 0) ? 0 : -1);
		OMNISCIO_UNTRACED_END;
		return result;
	} else {
		return _libc_read(fd,buf,count);
	}
}

int fclose(FILE* stream)
{
	MAP_OR_FAIL(fclose);
	if(omniscio_tracing_enabled && IS_ALLOWED_FILE(stream)) {
		OMNISCIO_UNTRACED_START;
		omniscio_file ofile;
		omniscio_file_from_libc(&ofile,stream);
		omniscio_close_start(ofile);
		int result = _libc_fclose(stream);
		omniscio_close_end(result);
		allowed_streams.erase(stream);
		OMNISCIO_UNTRACED_END;
		return result;
	} else {
		return _libc_fclose(stream);
	}
}

int close(int fd) 
{
	MAP_OR_FAIL(close);
	struct stat s;
	fstat(fd, &s);
	if(omniscio_tracing_enabled 
	&& S_ISREG(s.st_mode) 
	&& fd > 2 && IS_ALLOWED_FD(fd)) {
		OMNISCIO_UNTRACED_START;
		omniscio_file ofile;
		omniscio_file_from_posix(&ofile,fd);
		omniscio_close_start(ofile);
		int result = _libc_close(fd);
		omniscio_close_end(result);
		allowed_fds.erase(fd);
		OMNISCIO_UNTRACED_END;
		return result;
	} else {
		return _libc_close(fd);
	}
}

}
