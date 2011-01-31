// $Id: luazlib.c 1048 2006-05-22 09:24:53Z kaos $

#include <stdlib.h>
#include <zlib.h>
#include "luazlib.h"

// TODO: all file handling routines

static int zlib_compress( lua_State * L )
{
	int				rc;
	uLong			dstL, srcL;
	Byte *			dst;
	const Byte *	src;

	src = (Byte *)luaL_checklstring( L, 1, (size_t*)&srcL );
	dstL = (uLong)( 1.1 * srcL + 12 );
	dst = (Byte *)malloc( dstL );
	if( !dst )
	{
		lua_pushnil( L );
		lua_pushliteral( L, "failed to allocate output buffer" );
		return 2;
	}

	rc = compress( dst, &dstL, src, srcL );
	switch( rc )
	{
		case Z_OK:
			lua_pushlstring( L, (const char *)dst, dstL );
			lua_pushnumber( L, dstL );
			break;

		case Z_MEM_ERROR:
			lua_pushnil( L );
			lua_pushliteral( L, "not enough memory" );
			break;

		case Z_BUF_ERROR:
			lua_pushnil( L );
			lua_pushliteral( L, "output buffer error" );
			break;
	}

	free( dst );
	return 2;
}

static int dynamic_uncompress( Bytef ** dst, uLongf * dstLen, const Bytef * src, uLong srcLen );

static int zlib_uncompress( lua_State * L )
{
	int				rc;
	uLong			dstL, srcL;
	Byte *			dst;
	const Byte *	src;

	src = (Byte *)luaL_checklstring( L, 1, (size_t *)&srcL );
	dstL = srcL;
	rc = dynamic_uncompress( &dst, &dstL, src, srcL );
	switch( rc )
	{
		case Z_OK:
			lua_pushlstring( L, (const char *)dst, dstL );
			lua_pushnumber( L, dstL );
			break;

		case Z_MEM_ERROR:
			lua_pushnil( L );
			lua_pushliteral( L, "not enough memory" );
			break;

		case Z_BUF_ERROR:
			lua_pushnil( L );
			lua_pushliteral( L, "output buffer error" );
			break;

		case Z_DATA_ERROR:
			lua_pushnil( L );
			lua_pushliteral( L, "input data corrupted" );
			break;
	}

	free( dst );
	return 2;
}

/////////////////////////////
// gzip specific - based on zlib's gzio.c
static int const gz_magic[2] = {0x1f, 0x8b}; /* gzip magic header */

/* gzip flag byte */
#define ASCII_FLAG   0x01 /* bit 0 set: file probably ascii text */
#define HEAD_CRC     0x02 /* bit 1 set: header CRC present */
#define EXTRA_FIELD  0x04 /* bit 2 set: extra field present */
#define ORIG_NAME    0x08 /* bit 3 set: original file name present */
#define COMMENT      0x10 /* bit 4 set: file comment present */
#define RESERVED     0xE0 /* bits 5..7: reserved */

typedef struct gz_stream {
    z_stream stream;
    int      z_err;   /* error code for last stream operation */
    uLong    crc;     /* crc32 of uncompressed data */
} gz_stream;


static int get_byte( gz_stream *s )
{
    if (s->stream.avail_in == 0) {
		return EOF;
    }

    s->stream.avail_in--;
    return *(s->stream.next_in)++;
}

static uLong getLong( gz_stream *s)
{
    uLong x = (uLong)get_byte(s);
    int c;

    x += ((uLong)get_byte(s))<<8;
    x += ((uLong)get_byte(s))<<16;
    c = get_byte(s);
    if (c == EOF) s->z_err = Z_DATA_ERROR;
    x += ((uLong)c)<<24;
	//printf( "zlib: getLong() == %.4lx (%d)\n", x, s->z_err );
    return x;
}

static void check_header( gz_stream *s )
{
    int method; /* method byte */
    int flags;  /* flags byte */
    uInt len;
    int c;

    /* Assure two bytes in the buffer so we can peek ahead -- handle case
       where first byte of header is at the end of the buffer after the last
       gzip segment */
    len = s->stream.avail_in;
    if (len < 2) {
        s->z_err = Z_ERRNO;
		return;
    }

    /* Peek ahead to check the gzip magic header */
    if (s->stream.next_in[0] != gz_magic[0] ||
        s->stream.next_in[1] != gz_magic[1]) {
        s->z_err = Z_ERRNO;
        return;
    }

    s->stream.avail_in -= 2;
    s->stream.next_in += 2;

    /* Check the rest of the gzip header */
    method = get_byte(s);
    flags = get_byte(s);
    if (method != Z_DEFLATED || (flags & RESERVED) != 0) {
        s->z_err = Z_DATA_ERROR;
        return;
    }

    /* Discard time, xflags and OS code: */
    for (len = 0; len < 6; len++) (void)get_byte(s);

    if ((flags & EXTRA_FIELD) != 0) { /* skip the extra field */
        len  =  (uInt)get_byte(s);
        len += ((uInt)get_byte(s))<<8;
        /* len is garbage if EOF but the loop below will quit anyway */
        while (len-- != 0 && get_byte(s) != EOF) ;
    }
    if ((flags & ORIG_NAME) != 0) { /* skip the original file name */
        while ((c = get_byte(s)) != 0 && c != EOF) ;
    }
    if ((flags & COMMENT) != 0) {   /* skip the .gz file comment */
        while ((c = get_byte(s)) != 0 && c != EOF) ;
    }
    if ((flags & HEAD_CRC) != 0) {  /* skip the header crc */
        for (len = 0; len < 2; len++) (void)get_byte(s);
    }

    s->z_err = Z_OK;
}

static int gz_uncompress( Bytef ** dst, uLongf * dstLen, const Bytef * src, uLong srcLen )
{
    int err;
    gz_stream *s;
	Bytef * buf = NULL;
    Bytef * start; /* starting point for crc computation */

	*dst = NULL;
    s = (gz_stream *)malloc(sizeof(gz_stream));
    if (!s) return Z_MEM_ERROR;

    s->stream.zalloc = (alloc_func)0;
    s->stream.zfree = (free_func)0;
    s->stream.opaque = (voidpf)0;
    s->stream.next_in = Z_NULL;
    s->stream.next_out = Z_NULL;
    s->stream.avail_in = s->stream.avail_out = 0;
    s->z_err = Z_OK;
    s->crc = crc32(0L, Z_NULL, 0);

	err = inflateInit2(&(s->stream), -MAX_WBITS);
	/* windowBits is passed < 0 to tell that there is no zlib header.
	 * Note that in this case inflate *requires* an extra "dummy" byte
	 * after the compressed stream in order to complete decompression and
	 * return Z_STREAM_END. Here the gzip CRC32 ensures that 4 bytes are
	 * present after the compressed stream.
	 */
	if (err != Z_OK ) {
		printf( "zlib: inflateInit2() failed: %d\n", err );
		free(s);
		return err;
	}

	s->stream.next_in  = (Bytef*)src;
	s->stream.avail_in = srcLen;

	check_header(s); /* skip the .gz header */
	if( s->z_err != Z_OK )
	{
		err = s->z_err;
		free(s);
		printf( "zlib: check_header() failed: %d\n", err );
		return err;
	}

	do
	{
		buf = (Bytef *)realloc( buf, (2 * s->stream.avail_in) + s->stream.total_out );
		start = s->stream.next_out = buf + s->stream.total_out;
	    s->stream.avail_out = 2 * s->stream.avail_in;

		s->z_err = inflate(&(s->stream), Z_NO_FLUSH);
		//printf( "zlib: inflate total out: %ld (%d)\n", s->stream.total_out, s->z_err );
        s->crc = crc32(s->crc, start, (uInt)(s->stream.next_out - start));
		if( s->z_err == Z_STREAM_END )
		{
			s->z_err = Z_OK;
            if (getLong(s) == s->crc)
				break;

			printf( "zlib: crc32 check failed (%.4lx)\n", s->crc );
            s->z_err = Z_DATA_ERROR;
			break;
		}

	    if (s->z_err != Z_OK )
		{
			printf( "zlib: inflate() failed: %d\n", s->z_err );
			break;
		}

	}while( s->stream.avail_in );

	*dst = buf;
    *dstLen = s->stream.total_out;
    err = inflateEnd(&(s->stream));
	//printf( "zlib: inflate %d, inflate end %d\n", err, s->z_err );
	err = s->z_err == Z_OK ? err : s->z_err;
	free(s);

	return err;
}

/////////////////////////////////////////////////////////////////////
static int zlib_gzuncompress( lua_State * L )
{
	int				rc;
	uLong			dstL, srcL;
	Byte *			dst;
	const Byte *	src;

	src = (Byte *)luaL_checklstring( L, 1, (size_t *)&srcL );
	dstL = srcL;
	rc = gz_uncompress( &dst, &dstL, src, srcL );
	switch( rc )
	{
		case Z_OK:
			lua_pushlstring( L, (const char *)dst, dstL );
			lua_pushnumber( L, dstL );
			break;

		case Z_MEM_ERROR:
			lua_pushnil( L );
			lua_pushliteral( L, "not enough memory" );
			break;

		case Z_BUF_ERROR:
			lua_pushnil( L );
			lua_pushliteral( L, "output buffer error" );
			break;

		case Z_DATA_ERROR:
			lua_pushnil( L );
			lua_pushliteral( L, "input data corrupted" );
			break;
	}

	free( dst );
	return 2;
}



/////////////////////////////////////////////////////////////////////

static const luaL_Reg zlib_funcs[] =
{
	{ "compress", zlib_compress },
	{ "uncompress", zlib_uncompress },
	{ "gzuncompress", zlib_gzuncompress },
	{ NULL, NULL }
};

LUAZLIB_API int luaopen_zlib( lua_State * L )
{
	luaL_openlib( L, LUA_ZLIBNAME, zlib_funcs, 0 );

	lua_pushliteral (L, "_COPYRIGHT");
	lua_pushliteral (L, "Copyright (C) 2006 Andreas Stenius, Boxcom AB");
	lua_settable (L, -3);
	lua_pushliteral (L, "_DESCRIPTION");
	lua_pushliteral (L, "Binding to the zlib library");
	lua_settable (L, -3);
	lua_pushliteral (L, "_VERSION");
	lua_pushliteral (L, "LuaZlib 1.0.0");
	lua_settable (L, -3);

	return 1;
}

// modifed uncompress from zlib sources
static int dynamic_uncompress( Bytef ** dst, uLongf * dstLen, const Bytef * src, uLong srcLen )
{
    z_stream stream;
    int err;
	Bytef *	buf = NULL;

	*dst = NULL;
    stream.next_in = (Bytef*)src;
    stream.avail_in = (uInt)srcLen;

    stream.zalloc = (alloc_func)0;
    stream.zfree = (free_func)0;

    err = inflateInit(&stream);
    if (err != Z_OK) return err;

	do
	{
		buf = (Bytef *)realloc( buf, (2 * stream.avail_in) + stream.total_out );
		stream.next_out = buf + stream.total_out;
	    stream.avail_out = 2 * stream.avail_in;

		err = inflate(&stream, Z_SYNC_FLUSH);
		if( err == Z_STREAM_END ) break;
	    if (err != Z_OK )
		{
			inflateEnd( &stream );
			return err;
		}
	}while( stream.avail_in );

	*dst = buf;
    *dstLen = stream.total_out;
    err = inflateEnd(&stream);
    return err;
}
