/*
 * zlib.c - An interface for zlib.
 *
 *   Copyright (C) UENO Katsuhiro 2000-2003
 *
 * $Id$
 */
/*
This file was modified from Ruby's souce code repository (1.8.7-p334).
When compared with the original file, the irrelevant methods not used in
this project have been truncated. Type disagreements have been corrected.
03022c9d9f686b87d66443eaf38fd7b0e7db5b69 is the commit hash when the file
was copied. Its latest version can be found in the following URL:
https://github.com/ruby/ruby/blob/master/ext/zlib/zlib.c

Ruby is copyrighted free software by Yukihiro Matsumoto <matz@netlab.jp>.
You can redistribute it and/or modify it under either the terms of the
2-clause BSD License:

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS “AS IS”
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/

#include <ruby.h>
#include <zlib.h>
#include <time.h>

#define RUBY_ZLIB_VERSION  "0.6.0"

#define OBJ_IS_FREED(val)  (RBASIC(val)->flags == 0)

/* from zutil.h */
#ifndef DEF_MEM_LEVEL
#if MAX_MEM_LEVEL >= 8
#define DEF_MEM_LEVEL  8
#else
#define DEF_MEM_LEVEL  MAX_MEM_LEVEL
#endif
#endif

/*--------- Prototypes --------*/

static voidpf zlib_mem_alloc _((voidpf, uInt, uInt));
static void zlib_mem_free _((voidpf, voidpf));

struct zstream;
struct zstream_funcs;
static void zstream_init _((struct zstream*, const struct zstream_funcs *));
static void zstream_expand_buffer _((struct zstream*));
static VALUE zstream_detach_buffer _((struct zstream*));
static void zstream_append_input _((struct zstream*, const Bytef *, unsigned int));
static void zstream_reset_input _((struct zstream*));
static void zstream_reset _((struct zstream*));
static VALUE zstream_end _((struct zstream*));
static void zstream_run _((struct zstream*, Bytef*, uInt, int));

static VALUE inflate_run _((VALUE));
static VALUE rb_inflate_s_inflate _((VALUE, VALUE));

void Init_zlib _((void));

/*--------- Exceptions --------*/

static VALUE cZError, cStreamEnd, cNeedDict;
static VALUE cStreamError, cDataError, cMemError, cBufError, cVersionError;

static void
raise_zlib_error(err, msg)
    int err;
    const char *msg;
{
    VALUE exc;

    if (!msg) {
	msg = zError(err);
    }

    switch(err) {
      case Z_STREAM_END:
	exc = rb_exc_new2(cStreamEnd, msg);
	break;
      case Z_NEED_DICT:
	exc = rb_exc_new2(cNeedDict, msg);
	break;
      case Z_STREAM_ERROR:
	exc = rb_exc_new2(cStreamError, msg);
	break;
      case Z_DATA_ERROR:
	exc = rb_exc_new2(cDataError, msg);
	break;
      case Z_BUF_ERROR:
	exc = rb_exc_new2(cBufError, msg);
	break;
      case Z_VERSION_ERROR:
	exc = rb_exc_new2(cVersionError, msg);
	break;
      case Z_MEM_ERROR:
	exc = rb_exc_new2(cMemError, msg);
	break;
      case Z_ERRNO:
	rb_sys_fail(msg);
	/* no return */
      default:
      {
	  char buf[BUFSIZ];
	  snprintf(buf, BUFSIZ, "unknown zlib error %d: %s", err, msg);
	  exc = rb_exc_new2(cZError, buf);
      }
    }

    rb_exc_raise(exc);
}

/*-------- zstream - internal APIs --------*/

struct zstream {
    unsigned long flags;
    VALUE buf;
    long buf_filled;
    VALUE input;
    z_stream stream;
    const struct zstream_funcs {
	int (*reset) _((z_streamp));
	int (*end) _((z_streamp));
	int (*run) _((z_streamp, int));
    } *func;
};

#define ZSTREAM_FLAG_READY      0x1
#define ZSTREAM_FLAG_IN_STREAM  0x2
#define ZSTREAM_FLAG_FINISHED   0x4
#define ZSTREAM_FLAG_CLOSING    0x8
#define ZSTREAM_FLAG_UNUSED     0x10

#define ZSTREAM_READY(z)       ((z)->flags |= ZSTREAM_FLAG_READY)
#define ZSTREAM_IS_READY(z)    ((z)->flags & ZSTREAM_FLAG_READY)
#define ZSTREAM_IS_FINISHED(z) ((z)->flags & ZSTREAM_FLAG_FINISHED)
#define ZSTREAM_IS_CLOSING(z)  ((z)->flags & ZSTREAM_FLAG_CLOSING)

/* I think that more better value should be found,
   but I gave up finding it. B) */
#define ZSTREAM_INITIAL_BUFSIZE       1024
#define ZSTREAM_AVAIL_OUT_STEP_MAX   16384
#define ZSTREAM_AVAIL_OUT_STEP_MIN    2048

static const struct zstream_funcs deflate_funcs = {
    deflateReset, deflateEnd, deflate,
};

static const struct zstream_funcs inflate_funcs = {
    inflateReset, inflateEnd, inflate,
};


static voidpf
zlib_mem_alloc(opaque, items, size)
    voidpf opaque;
    uInt items, size;
{
    return xmalloc(items * size);
}

static void
zlib_mem_free(opaque, address)
    voidpf opaque, address;
{
    free(address);
}

static void
zstream_init(z, func)
    struct zstream *z;
    const struct zstream_funcs *func;
{
    z->flags = 0;
    z->buf = Qnil;
    z->buf_filled = 0;
    z->input = Qnil;
    z->stream.zalloc = zlib_mem_alloc;
    z->stream.zfree = zlib_mem_free;
    z->stream.opaque = Z_NULL;
    z->stream.msg = Z_NULL;
    z->stream.next_in = Z_NULL;
    z->stream.avail_in = 0;
    z->stream.next_out = Z_NULL;
    z->stream.avail_out = 0;
    z->func = func;
}

#define zstream_init_inflate(z)   zstream_init((z), &inflate_funcs)

static void
zstream_expand_buffer(z)
    struct zstream *z;
{
    long inc;

    if (NIL_P(z->buf)) {
	    /* I uses rb_str_new here not rb_str_buf_new because
	       rb_str_buf_new makes a zero-length string. */
	z->buf = rb_str_new(0, ZSTREAM_INITIAL_BUFSIZE);
	z->buf_filled = 0;
	z->stream.next_out = (Bytef*)RSTRING(z->buf)->ptr;
	z->stream.avail_out = ZSTREAM_INITIAL_BUFSIZE;
	RBASIC(z->buf)->klass = 0;
	return;
    }

    if (RSTRING(z->buf)->len - z->buf_filled >= ZSTREAM_AVAIL_OUT_STEP_MAX) {
	/* to keep other threads from freezing */
	z->stream.avail_out = ZSTREAM_AVAIL_OUT_STEP_MAX;
    }
    else {
	inc = z->buf_filled / 2;
	if (inc < ZSTREAM_AVAIL_OUT_STEP_MIN) {
	    inc = ZSTREAM_AVAIL_OUT_STEP_MIN;
	}
	rb_str_resize(z->buf, z->buf_filled + inc);
	z->stream.avail_out = (inc < ZSTREAM_AVAIL_OUT_STEP_MAX) ?
	    inc : ZSTREAM_AVAIL_OUT_STEP_MAX;
    }
    z->stream.next_out = (Bytef*)RSTRING(z->buf)->ptr + z->buf_filled;
}

static VALUE
zstream_detach_buffer(z)
    struct zstream *z;
{
    VALUE dst;

    if (NIL_P(z->buf)) {
	dst = rb_str_new(0, 0);
    }
    else {
	dst = z->buf;
	rb_str_resize(dst, z->buf_filled);
	RBASIC(dst)->klass = rb_cString;
    }

    z->buf = Qnil;
    z->buf_filled = 0;
    z->stream.next_out = 0;
    z->stream.avail_out = 0;
    return dst;
}

static void
zstream_append_input(z, src, len)
    struct zstream *z;
    const Bytef *src;
    unsigned int len;
{
    if (len <= 0) return;

    if (NIL_P(z->input)) {
	z->input = rb_str_buf_new(len);
	rb_str_buf_cat(z->input, (const char*)src, len);
	RBASIC(z->input)->klass = 0;
    }
    else {
	rb_str_buf_cat(z->input, (const char*)src, len);
    }
}

#define zstream_append_input2(z,v)\
    zstream_append_input((z), RSTRING(v)->ptr, RSTRING(v)->len)

static void
zstream_reset_input(z)
    struct zstream *z;
{
    z->input = Qnil;
}

static void
zstream_reset(z)
    struct zstream *z;
{
    int err;

    err = z->func->reset(&z->stream);
    if (err != Z_OK) {
	raise_zlib_error(err, z->stream.msg);
    }
    z->flags = ZSTREAM_FLAG_READY;
    z->buf = Qnil;
    z->buf_filled = 0;
    z->stream.next_out = 0;
    z->stream.avail_out = 0;
    zstream_reset_input(z);
}

static VALUE
zstream_end(z)
    struct zstream *z;
{
    int err;

    if (!ZSTREAM_IS_READY(z)) {
	rb_warning("attempt to close uninitialized zstream; ignored.");
	return Qnil;
    }
    if (z->flags & ZSTREAM_FLAG_IN_STREAM) {
	rb_warning("attempt to close unfinished zstream; reset forced.");
	zstream_reset(z);
    }

    zstream_reset_input(z);
    err = z->func->end(&z->stream);
    if (err != Z_OK) {
	raise_zlib_error(err, z->stream.msg);
    }
    z->flags = 0;
    return Qnil;
}

static void
zstream_run(z, src, len, flush)
    struct zstream *z;
    Bytef *src;
    uInt len;
    int flush;
{
    uInt n;
    int err;
    volatile VALUE guard;

    if (NIL_P(z->input) && len == 0) {
	z->stream.next_in = (Bytef*)"";
	z->stream.avail_in = 0;
    }
    else {
	zstream_append_input(z, src, len);
	z->stream.next_in = (Bytef*)RSTRING(z->input)->ptr;
	z->stream.avail_in = RSTRING(z->input)->len;
	/* keep reference to `z->input' so as not to be garbage collected
	   after zstream_reset_input() and prevent `z->stream.next_in'
	   from dangling. */
	guard = z->input;
    }

    if (z->stream.avail_out == 0) {
	zstream_expand_buffer(z);
    }

    for (;;) {
        /* VC allocates err and guard to same address.  accessing err and guard
           in same scope prevents it. */
        RB_GC_GUARD(guard);
	n = z->stream.avail_out;
	err = z->func->run(&z->stream, flush);
	z->buf_filled += n - z->stream.avail_out;
	rb_thread_schedule();

	if (err == Z_STREAM_END) {
	    z->flags &= ~ZSTREAM_FLAG_IN_STREAM;
	    z->flags |= ZSTREAM_FLAG_FINISHED;
	    break;
	}
	if (err != Z_OK) {
	    if (flush != Z_FINISH && err == Z_BUF_ERROR
		&& z->stream.avail_out > 0) {
		z->flags |= ZSTREAM_FLAG_IN_STREAM;
		break;
	    }
	    zstream_reset_input(z);
	    if (z->stream.avail_in > 0) {
		zstream_append_input(z, z->stream.next_in, z->stream.avail_in);
	    }
	    raise_zlib_error(err, z->stream.msg);
	}
	if (z->stream.avail_out > 0) {
	    z->flags |= ZSTREAM_FLAG_IN_STREAM;
	    break;
	}
	zstream_expand_buffer(z);
    }

    zstream_reset_input(z);
    if (z->stream.avail_in > 0) {
	zstream_append_input(z, z->stream.next_in, z->stream.avail_in);
        guard = Qnil; /* prevent tail call to make guard effective */
    }
}

/* ------------------------------------------------------------------------- */

/*
 * Document-class: Zlib::Inflate
 *
 * Zlib:Inflate is the class for decompressing compressed data.  Unlike
 * Zlib::Deflate, an instance of this class is not able to duplicate (clone,
 * dup) itself.
 */

static VALUE
inflate_run(args)
    VALUE args;
{
    struct zstream *z = (struct zstream *)((VALUE *)args)[0];
    VALUE src = ((VALUE *)args)[1];

    zstream_run(z, (Bytef*)RSTRING(src)->ptr, RSTRING(src)->len, Z_SYNC_FLUSH);
    zstream_run(z, (Bytef*)"", 0, Z_FINISH);  /* for checking errors */
    return zstream_detach_buffer(z);
}

/*
 * call-seq: Zlib::Inflate.inflate(string)
 *
 * Decompresses +string+. Raises a Zlib::NeedDict exception if a preset
 * dictionary is needed for decompression.
 *
 * This method is almost equivalent to the following code:
 *
 *   def inflate(string)
 *     zstream = Zlib::Inflate.new
 *     buf = zstream.inflate(string)
 *     zstream.finish
 *     zstream.close
 *     buf
 *   end
 *
 */
static VALUE
rb_inflate_s_inflate(obj, src)
    VALUE obj, src;
{
    struct zstream z;
    VALUE dst, args[2];
    int err;

    StringValue(src);
    zstream_init_inflate(&z);
    err = inflateInit(&z.stream);
    if (err != Z_OK) {
	raise_zlib_error(err, z.stream.msg);
    }
    ZSTREAM_READY(&z);

    args[0] = (VALUE)&z;
    args[1] = src;
    dst = rb_ensure(inflate_run, (VALUE)args, zstream_end, (VALUE)&z);

    OBJ_INFECT(dst, src);
    return dst;
}

void Init_zlib()
{
    VALUE mZlib, cZStream, cInflate;

    mZlib = rb_define_module("Zlib");

    cZError = rb_define_class_under(mZlib, "Error", rb_eStandardError);
    cStreamEnd    = rb_define_class_under(mZlib, "StreamEnd", cZError);
    cNeedDict     = rb_define_class_under(mZlib, "NeedDict", cZError);
    cDataError    = rb_define_class_under(mZlib, "DataError", cZError);
    cStreamError  = rb_define_class_under(mZlib, "StreamError", cZError);
    cMemError     = rb_define_class_under(mZlib, "MemError", cZError);
    cBufError     = rb_define_class_under(mZlib, "BufError", cZError);
    cVersionError = rb_define_class_under(mZlib, "VersionError", cZError);

    cZStream = rb_define_class_under(mZlib, "ZStream", rb_cObject);
    rb_undef_alloc_func(cZStream);
    cInflate = rb_define_class_under(mZlib, "Inflate", cZStream);
    rb_define_singleton_method(cInflate, "inflate", rb_inflate_s_inflate, 1);
}

/* Code added by Z-H-Sun
   IMPORTANT! Everytime `zlib.c` is updated, need to add the following lines to the replaced file
*/
extern VALUE ruby_zlib_inflate(VALUE); // export for use in utility.c: exerb_get_file_from_entry2
VALUE ruby_zlib_inflate(VALUE src)
{ return rb_inflate_s_inflate(rb_cObject, src); }
