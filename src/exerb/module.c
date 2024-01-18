// $Id: module.cpp,v 1.11 2007/02/25 16:44:51 yuya Exp $

#include <ruby.h>
#include "exerb.h"
#include "module.h"
#include "utility.h"

////////////////////////////////////////////////////////////////////////////////

extern VALUE rb_mExerbRuntime;
extern VALUE rb_eExerbRuntimeError;

extern FILE_TABLE_HEADER *g_file_table_header;

extern int     g_loaded_resource_count;
extern HMODULE g_loaded_resource_table[];

////////////////////////////////////////////////////////////////////////////////

static VALUE rb_exerbruntime_s_filepath(VALUE self);
static VALUE rb_exerbruntime_s_filename(VALUE self);
static VALUE rb_exerbruntime_s_open(VALUE self, VALUE filename);
static VALUE rb_exerbruntime_s_load_string(VALUE self, VALUE id);
static VALUE rb_exerbruntime_s_load_icon(VALUE self, VALUE id);
static VALUE rb_exerbruntime_s_load_cursor(VALUE self, VALUE id);
static LPCTSTR exerb_convert_resource_id(VALUE value);

////////////////////////////////////////////////////////////////////////////////

#ifdef RUBY19
// need to export a path string in `filesystem` encoding
	#include <ruby/encoding.h>
	// For some early Ruby 1.9.0 (.1) versions, 'filesystem' encoding is not defined
	#ifndef rb_filesystem_encoding
		#define rb_filesystem_encoding rb_locale_encoding
	// For some even earlier Ruby 1.9.0 (.0) versions, 'locale' encoding is also not defined
	// Then there is nothing I can do about it... Just update to a higher version Ruby please
	#endif

	static VALUE rb_str_filesystem_encoding(const char*, long);
	static VALUE rb_str_filesystem_encoding(const char *strinput, long len)
	{
		rb_encoding *enc_fs = rb_filesystem_encoding();
		return rb_enc_str_new(strinput, len, enc_fs);
	}

	void Init_Locale_Encodings() // define 'locale', 'extern', and 'filesystem' encodings
	{
		VALUE encoding_loc = rb_enc_from_encoding(rb_locale_encoding()); // this will auto set 'locale' encoding; see `rb_locale_encoding` in `encoding.c`
		rb_enc_set_default_external(encoding_loc); // in addition to 'external' encoding, this will also auto set 'filesystem' encoding (they can be different); see `encoding.c`
	}
#else // no need for Ruby < 1.8
	#define rb_str_filesystem_encoding(chars, dummy) rb_str_new2(chars); \
					(void) dummy // this line avoids the 'unused variable' warning
// Ruby 1.8 does not process encoding for strings
#endif

void
Init_ExerbRuntime()
{
	static VALUE gv_exerb = Qtrue;
	rb_define_readonly_variable("$Exerb", &gv_exerb);

	rb_mExerbRuntime = rb_define_module("ExerbRuntime");

	rb_define_singleton_method(rb_mExerbRuntime, "filepath",    rb_exerbruntime_s_filepath, 0);
	rb_define_singleton_method(rb_mExerbRuntime, "filename",    rb_exerbruntime_s_filename, 0);
	rb_define_singleton_method(rb_mExerbRuntime, "open",        rb_exerbruntime_s_open, 1);
	rb_define_singleton_method(rb_mExerbRuntime, "load_string", rb_exerbruntime_s_load_string, 1);
	rb_define_singleton_method(rb_mExerbRuntime, "load_icon",   rb_exerbruntime_s_load_icon, 1);
	rb_define_singleton_method(rb_mExerbRuntime, "load_cursor", rb_exerbruntime_s_load_cursor, 1);

	rb_eExerbRuntimeError = rb_define_class_under(rb_mExerbRuntime, "Error", rb_eException);
}

static VALUE
rb_exerbruntime_s_filepath(VALUE self)
{
	char filepath[MAX_PATH] = "";
	const DWORD len = exerb_get_module_filepath(NULL, filepath, sizeof(filepath));
	return rb_str_filesystem_encoding(filepath, len);
}

static VALUE
rb_exerbruntime_s_filename(VALUE self)
{
	char filepath[MAX_PATH] = "";
	exerb_get_module_filepath(NULL, filepath, sizeof(filepath));
	const char *filename = exerb_get_filename(filepath);
	return rb_str_filesystem_encoding(filename, strlen(filename));
}

static VALUE
rb_exerbruntime_s_open(VALUE self, VALUE filename)
{
#ifdef RUBY19
        SafeStringValue(filename);
#else
	Check_SafeStr(filename);
#endif  
	rb_require("stringio");

	NAME_ENTRY_HEADER *name_entry = exerb_find_name_entry_by_filename(RSTRING_PTR(filename), NULL);
	if ( !name_entry ) rb_raise(rb_eLoadError, "No such file to load -- %s", RSTRING_PTR(filename));

	FILE_ENTRY_HEADER *file_entry = exerb_find_file_entry(name_entry->id);
	VALUE file = exerb_get_file_from_entry2(file_entry);

	return rb_funcall(rb_path2class("StringIO"), rb_intern("new"), 2, file, rb_str_new2("r"));
}

static VALUE
rb_exerbruntime_s_load_string(VALUE self, VALUE id)
{
	UINT res_id = FIX2INT(id); // unlike LoadIcon or LoadCursor where `res_id` can be either char* or int, `res_id` must be UINT here!
	char buffer[512] = "";

	for ( int i = 0; i < g_loaded_resource_count; i++ ) {
		if ( LoadString(g_loaded_resource_table[i], res_id, buffer, sizeof(buffer)) ) {
			return rb_str_new2(buffer);
		}
	}

	return Qnil;
}

static VALUE
rb_exerbruntime_s_load_icon(VALUE self, VALUE id)
{
	LPCSTR res_id = exerb_convert_resource_id(id);

	for ( int i = 0; i < g_loaded_resource_count; i++ ) {
		HICON icon = LoadIcon(g_loaded_resource_table[i], res_id);
		if ( icon ) return SIZET2NUM((uintptr_t)icon);
	}

	return Qnil;
}

static VALUE
rb_exerbruntime_s_load_cursor(VALUE self, VALUE id)
{
	LPCSTR res_id = exerb_convert_resource_id(id);

	for ( int i = 0; i < g_loaded_resource_count; i++ ) {
		HCURSOR icon = LoadCursor(g_loaded_resource_table[i], res_id);
		if ( icon ) return SIZET2NUM((uintptr_t)icon);
	}

	return Qnil;
}

static LPCTSTR
exerb_convert_resource_id(VALUE value)
{
	switch ( TYPE(value) ) {
	case T_FIXNUM:
		return MAKEINTRESOURCE(FIX2INT(value));
	case T_STRING:
		return RSTRING_PTR(value);
	default:
		rb_raise(rb_eArgError, "argument needs to be integer or string");
		return NULL;
	}
}

////////////////////////////////////////////////////////////////////////////////
