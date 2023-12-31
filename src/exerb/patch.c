#include <stdio.h>

#include "ruby.h"
#include "exerb.h"

VALUE __real_rb_require(const char *);
VALUE __real_rb_require_safe(VALUE, int);

VALUE
__wrap_rb_require(const char *fname)
{
        VALUE fn = rb_str_new2(fname);
        OBJ_FREEZE(fn);
        return exerb_require(fn);
}


VALUE
__wrap_rb_require_safe(VALUE fname, int safe)
{
        return exerb_require(fname);
}

