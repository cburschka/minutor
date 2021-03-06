/*
   Copyright (c) 2013, Sean Kasun
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "chunkcache.h"
#include "chunkloader.h"

#if defined(__unix__) || defined(__unix) || defined(unix)
 #include <unistd.h>
#elif defined(_WIN32) || defined(WIN32)
 #include <windows.h>
#endif

ChunkID::ChunkID(int x,int z) : x(x),z(z)
{
}
bool ChunkID::operator==(const ChunkID &other) const
{
	return (other.x==x) && (other.z==z);
}
uint qHash(const ChunkID &c)
{
	return (c.x<<16)^(c.z&0xffff); // safe way to hash a pair of integers
}

ChunkCache::ChunkCache()
{
	long chunks=10000; // as default 10000 chunks, or 10% more than 1920x1200 blocks
#if defined(__unix__) || defined(__unix) || defined(unix)
	long pages = sysconf(_SC_AVPHYS_PAGES);
	long page_size = sysconf(_SC_PAGE_SIZE);
	chunks = (pages*page_size) / (sizeof(Chunk) + 16*sizeof(ChunkSection));
	cache.setMaxCost(chunks);
#elif defined(_WIN32) || defined(WIN32)
	MEMORYSTATUSEX status;
	status.dwLength = sizeof(status);
	GlobalMemoryStatusEx(&status);
    DWORDLONG available = std::min<DWORDLONG>(status.ullAvailPhys, status.ullAvailVirtual);
	chunks = available / (sizeof(Chunk) + 16*sizeof(ChunkSection));
#endif
	cache.setMaxCost(chunks);
	maxcache = 2*chunks; //most chunks are only filled less than half with sections
}

ChunkCache::~ChunkCache()
{
}

void ChunkCache::clear()
{
	QThreadPool::globalInstance()->waitForDone();
	mutex.lock();
	cache.clear();
	mutex.unlock();
}

void ChunkCache::setPath(QString path)
{
	this->path=path;
}
QString ChunkCache::getPath()
{
	return path;
}

Chunk *ChunkCache::fetch(int x, int z)
{
	ChunkID id(x,z);
	mutex.lock();
	Chunk *chunk=cache[id];
	mutex.unlock();
	if (chunk!=NULL)
	{
		if (chunk->loaded)
			return chunk;
		return NULL; //we're loading this chunk, or it's blank.
	}
	// launch background process to load this chunk
	chunk=new Chunk();
	mutex.lock();
	cache.insert(id,chunk);
	mutex.unlock();
	ChunkLoader *loader=new ChunkLoader(path,x,z,cache,mutex);
	connect(loader,SIGNAL(loaded(int,int)),
			this,SLOT(gotChunk(int,int)));
	QThreadPool::globalInstance()->start(loader);
	return NULL;
}

void ChunkCache::gotChunk(int x,int z)
{
	emit chunkLoaded(x,z);
}

void ChunkCache::adaptCacheToWindow(int x,int y)
{
	int chunks=((x+15)>>4)*((y+15)>>4); // number of chunks visible in window
	chunks *= 1.10; // add 10%
	cache.setMaxCost(std::min<int>(chunks,maxcache));
}
