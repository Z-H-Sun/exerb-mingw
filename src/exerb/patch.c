#include <stdio.h>

#include "ruby.h"
#include "exerb.h"

VALUE __real_rb_require(const char *fname);
#ifdef RUBY19
VALUE __real_rb_require_safe(VALUE, int);
#endif

VALUE
__wrap_rb_require(const char *fname)
{
        VALUE fn = rb_str_new2(fname);
        OBJ_FREEZE(fn);
        return exerb_require_safe(fn, rb_safe_level());
        // return __real_rb_require(fname);
}

#ifdef RUBY19

VALUE
__wrap_rb_require_safe(VALUE fname, int safe)
{
        return exerb_require_safe(fname, safe);
}

#endif
