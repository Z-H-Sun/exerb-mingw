// $Id: utility.h,v 1.13 2005/04/17 11:50:55 yuya Exp $

#ifndef UTILITY_H
#define UTILITY_H 1 

#include <windows.h>

//////////
// deal with pointer-sized integers
#include <stdint.h> // DWORD -> uintptr_t
// use Ruby defined macro definitions
#if !defined(NUM2SIZET) // NUM2UINT -> NUM2SIZET
# if defined(HAVE_LONG_LONG) && SIZEOF_SIZE_T > SIZEOF_LONG
#  define NUM2SIZET(x) ((size_t)NUM2ULL(x))
# else
#  define NUM2SIZET(x) NUM2ULONG(x)
# endif
#endif
#if !defined(SIZET2NUM) // INT2NUM -> SIZET2NUM
# if defined(HAVE_LONG_LONG) && SIZEOF_SIZE_T > SIZEOF_LONG
#  define SIZET2NUM(v) ULL2NUM(v)
# elif SIZEOF_SIZE_T == SIZEOF_LONG
#  define SIZET2NUM(v) ULONG2NUM(v)
# else
#  define SIZET2NUM(v) UINT2NUM(v)
# endif
#endif
//////////

char* exerb_strdup(const char* str);
HANDLE exerb_fopen_for_read(const char *filepath);
HANDLE exerb_fopen_for_write(const char *filepath);
BOOL exerb_fclose(const HANDLE file);
DWORD exerb_fseek(const HANDLE file, const LONG pos, const DWORD method);
DWORD exerb_fread(const HANDLE file, void *buffer, const int size);
/* char* exerb_fread(const HANDLE file); */
DWORD exerb_fwrite(const HANDLE file, const void *buffer, const int size);
DWORD exerb_fsize(const HANDLE file);
DWORD exerb_get_module_filepath(const HMODULE handle, char *filepath, const int size);
const char* exerb_get_filename(const char *filepath);
int exerb_cmp_filename_with_ext(const char *name1, const char *name2, const char *ext);
void exerb_create_tmpfile(const char *prefix, char *filepath, const void *buffer, const int size);
void exerb_raise_runtime_error(const DWORD error_no);
NAME_ENTRY_HEADER* exerb_get_first_name_entry();
FILE_ENTRY_HEADER* exerb_get_first_file_entry();
char* exerb_get_name_from_entry(const NAME_ENTRY_HEADER *name_entry);
char* exerb_get_file_from_entry1(const FILE_ENTRY_HEADER *file_entry);
VALUE exerb_get_file_from_entry2(const FILE_ENTRY_HEADER *file_entry);
NAME_ENTRY_HEADER* exerb_find_name_entry(const WORD id);
NAME_ENTRY_HEADER* exerb_find_name_entry_by_filename(const char *filename, const char *ext);
FILE_ENTRY_HEADER* exerb_find_file_entry(const WORD id);
FILE_ENTRY_HEADER* exerb_find_file_entry_by_filename(const char *filename, const char *ext);

#endif /* UTILITY_H */