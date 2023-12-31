// $Id: gui.cpp,v 1.15 2008/12/19 12:17:50 arton Exp $

#include <ruby.h>
#include "exerb.h"
#include "resource.h"

////////////////////////////////////////////////////////////////////////////////

int exerb_main(int argc, char** argv, void (*on_init)(VALUE, VALUE, VALUE), void (*on_fail)(VALUE)); // exerb.cpp

int WINAPI WinMain(HINSTANCE current_instance, HINSTANCE prev_instance, LPSTR cmd_line, int show_cmd);
static void on_init(VALUE io_stdin, VALUE io_stdout, VALUE io_stderr);
static void on_fail(VALUE errinfo);

static void exerb_replace_io_methods(const VALUE io);
static VALUE rb_exerbio_write(VALUE self, VALUE str);
static VALUE rb_exerbio_return_nil(int argc, VALUE *argv, VALUE self);

static LRESULT CALLBACK dialog_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

static void checkFonts();
static int font_size = 0;
static BOOL have_font_consolas = FALSE, have_font_segoe_ui = FALSE;
static HFONT font = NULL, font_dlg = NULL;
static HICON icon = NULL;
const char * const ConsolasFontName = "Consolas";
const char * const SegoeUIFontName = "Segoe UI";

////////////////////////////////////////////////////////////////////////////////

int WINAPI
WinMain(HINSTANCE current_instance, HINSTANCE prev_instance, LPSTR cmd_line, int show_cmd)
{
#ifdef _DEBUG
	__asm { int 3 }
#endif
	return exerb_main(0, NULL, on_init, on_fail);
}

static void
on_init(VALUE io_stdin, VALUE io_stdout, VALUE io_stderr)
{
	exerb_replace_io_methods(io_stdin);
	exerb_replace_io_methods(io_stdout);
	exerb_replace_io_methods(io_stderr);

	rb_define_global_function("gets",      rb_exerbio_return_nil, -1);
	rb_define_global_function("readline",  rb_exerbio_return_nil, -1);
	rb_define_global_function("getc",      rb_exerbio_return_nil, -1);
	rb_define_global_function("readlines", rb_exerbio_return_nil, -1);
}

static void
on_fail(VALUE errinfo)
{
	DialogBoxParam(NULL, MAKEINTRESOURCE(IDD_EXCEPTION), NULL, (DLGPROC)dialog_proc, (LPARAM)errinfo);
}

////////////////////////////////////////////////////////////////////////////////

static void
exerb_replace_io_methods(const VALUE io)
{
	rb_define_singleton_method(io, "reopen", rb_exerbio_return_nil, -1);
	rb_define_singleton_method(io, "each", rb_exerbio_return_nil, -1);
	rb_define_singleton_method(io, "each_line", rb_exerbio_return_nil, -1);
	rb_define_singleton_method(io, "each_byte", rb_exerbio_return_nil, -1);
	rb_define_singleton_method(io, "syswrite", rb_exerbio_return_nil, -1);
	rb_define_singleton_method(io, "sysread", rb_exerbio_return_nil, -1);
	rb_define_singleton_method(io, "fileno", rb_exerbio_return_nil, -1);
	rb_define_singleton_method(io, "to_i", rb_exerbio_return_nil, -1);
	rb_define_singleton_method(io, "to_io", rb_exerbio_return_nil, -1);
	rb_define_singleton_method(io, "fsync", rb_exerbio_return_nil, -1);
	rb_define_singleton_method(io, "sync", rb_exerbio_return_nil, -1);
	rb_define_singleton_method(io, "sync=", rb_exerbio_return_nil, -1);
	rb_define_singleton_method(io, "lineno", rb_exerbio_return_nil, -1);
	rb_define_singleton_method(io, "lineno=", rb_exerbio_return_nil, -1);
	rb_define_singleton_method(io, "readlines", rb_exerbio_return_nil, -1);
	rb_define_singleton_method(io, "read", rb_exerbio_return_nil, -1);
	rb_define_singleton_method(io, "write", rb_exerbio_write, 1);
	rb_define_singleton_method(io, "gets", rb_exerbio_return_nil, -1);
	rb_define_singleton_method(io, "readline", rb_exerbio_return_nil, -1);
	rb_define_singleton_method(io, "getc", rb_exerbio_return_nil, -1);
	rb_define_singleton_method(io, "readchar", rb_exerbio_return_nil, -1);
	rb_define_singleton_method(io, "ungetc", rb_exerbio_return_nil, -1);
	rb_define_singleton_method(io, "flush", rb_exerbio_return_nil, -1);
	rb_define_singleton_method(io, "tell", rb_exerbio_return_nil, -1);
	rb_define_singleton_method(io, "seek", rb_exerbio_return_nil, -1);
	rb_define_singleton_method(io, "rewind", rb_exerbio_return_nil, -1);
	rb_define_singleton_method(io, "pos", rb_exerbio_return_nil, -1);
	rb_define_singleton_method(io, "pos=", rb_exerbio_return_nil, -1);
	rb_define_singleton_method(io, "eof", rb_exerbio_return_nil, -1);
	rb_define_singleton_method(io, "eof?", rb_exerbio_return_nil, -1);
	rb_define_singleton_method(io, "close", rb_exerbio_return_nil, -1);
	rb_define_singleton_method(io, "closed?", rb_exerbio_return_nil, -1);
	rb_define_singleton_method(io, "close_read", rb_exerbio_return_nil, -1);
	rb_define_singleton_method(io, "close_write", rb_exerbio_return_nil, -1);
	rb_define_singleton_method(io, "isatty", rb_exerbio_return_nil, -1);
	rb_define_singleton_method(io, "tty?", rb_exerbio_return_nil, -1);
	rb_define_singleton_method(io, "binmode", rb_exerbio_return_nil, -1);
	rb_define_singleton_method(io, "sysseek", rb_exerbio_return_nil, -1);
	rb_define_singleton_method(io, "ioctl", rb_exerbio_return_nil, -1);
	rb_define_singleton_method(io, "fcntl", rb_exerbio_return_nil, -1);
	rb_define_singleton_method(io, "pid", rb_exerbio_return_nil, -1);
}

static VALUE
rb_exerbio_write(VALUE self, VALUE str)
{
	rb_secure(1);
	if ( TYPE(str) != T_STRING ) str = rb_obj_as_string(str);
	if ( RSTRING_LEN(str) > 0 ) OutputDebugString(RSTRING_PTR(str));
	return Qnil;
}

static VALUE
rb_exerbio_return_nil(int argc, VALUE *argv, VALUE self)
{
	return Qnil;
}

////////////////////////////////////////////////////////////////////////////////

static LRESULT CALLBACK
dialog_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{

	switch ( message ) {
		case WM_INITDIALOG:
			if ( lparam ) {
				const VALUE errinfo       = (VALUE)lparam;
				const VALUE message       = rb_funcall(errinfo, rb_intern("message"), 0);
				const VALUE message_str   = rb_funcall(message, rb_intern("gsub"), 2, rb_str_new2("\n"), rb_str_new2("\r\n"));
				const VALUE backtrace     = rb_funcall(errinfo, rb_intern("backtrace"), 0);
				const VALUE backtrace_str = rb_str_concat(rb_ary_join(backtrace, rb_str_new2("\r\n")), rb_str_new2("\r\n"));

				SetDlgItemText(hwnd, IDC_EDIT_TYPE,      rb_obj_classname(errinfo));
#ifdef RUBY19
				SetDlgItemText(hwnd, IDC_EDIT_MESSAGE,   rb_string_value_ptr((volatile VALUE*) &message_str));
				SetDlgItemText(hwnd, IDC_EDIT_BACKTRACE, rb_string_value_ptr((volatile VALUE*) &backtrace_str));
#else
				SetDlgItemText(hwnd, IDC_EDIT_MESSAGE,   STR2CSTR(message_str));
				SetDlgItemText(hwnd, IDC_EDIT_BACKTRACE, STR2CSTR(backtrace_str));
#endif        
			}

			{
				char self_filename[MAX_PATH]       = "";
				char window_title_format[MAX_PATH] = "";
				char window_title[MAX_PATH]        = "";
				GetModuleFileName(NULL, self_filename, sizeof(self_filename));
				GetWindowText(hwnd, window_title_format, sizeof(window_title_format));
				wsprintf(window_title, window_title_format, self_filename);
				SetWindowText(hwnd, window_title);
			}

			icon = LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_RUBY), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR | LR_DEFAULTSIZE | LR_SHARED);
			SendMessage(hwnd, WM_SETICON, (WPARAM)ICON_BIG, (LPARAM)icon);

			checkFonts();
			font = CreateFont(font_size, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY, FIXED_PITCH | FF_MODERN, have_font_consolas ?  ConsolasFontName : "Courier New");
			SendDlgItemMessage(hwnd, IDC_EDIT_TYPE,      WM_SETFONT, (WPARAM)font, (LPARAM)FALSE);
			SendDlgItemMessage(hwnd, IDC_EDIT_MESSAGE,   WM_SETFONT, (WPARAM)font, (LPARAM)FALSE);
			SendDlgItemMessage(hwnd, IDC_EDIT_BACKTRACE, WM_SETFONT, (WPARAM)font, (LPARAM)FALSE);
			if (have_font_segoe_ui) {
				font_dlg = CreateFont(font_size, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY, FF_DONTCARE, SegoeUIFontName);
				SendDlgItemMessage(hwnd, ID_CLOSE,             WM_SETFONT, (WPARAM)font_dlg, (LPARAM)FALSE);
				SendDlgItemMessage(hwnd, IDC_STATIC_TITLE,     WM_SETFONT, (WPARAM)font_dlg, (LPARAM)FALSE);
				SendDlgItemMessage(hwnd, IDC_STATIC_TYPE,      WM_SETFONT, (WPARAM)font_dlg, (LPARAM)FALSE);
				SendDlgItemMessage(hwnd, IDC_STATIC_MESSAGE,   WM_SETFONT, (WPARAM)font_dlg, (LPARAM)FALSE);
				SendDlgItemMessage(hwnd, IDC_STATIC_BACKTRACE, WM_SETFONT, (WPARAM)font_dlg, (LPARAM)FALSE);
			}

			MessageBeep(MB_ICONHAND);
			
			return TRUE;
		case WM_DESTROY:
			DeleteObject(icon);
			DeleteObject(font);
			if (font_dlg)
				DeleteObject(font_dlg);
			return TRUE;
		case WM_CLOSE:
			EndDialog(hwnd, ID_CLOSE);
			return TRUE;
		case WM_COMMAND:
			if ( LOWORD(wparam) == ID_CLOSE ) {
				EndDialog(hwnd, ID_CLOSE);
				return TRUE;
			}
			break;
	}

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////

static int CALLBACK EnumFontFamExProc(
	ENUMLOGFONTEXA *lpelfe,
	NEWTEXTMETRICEXA *lpntme,
	DWORD FontType,
	LPARAM lParam
)
{
	if (strnicmp((char *) lpelfe->elfFullName, ConsolasFontName, strlen(ConsolasFontName)+1) == 0)
		have_font_consolas=TRUE;
	if (strnicmp((char *) lpelfe->elfFullName, SegoeUIFontName, strlen(SegoeUIFontName)+1) == 0)
		have_font_segoe_ui=TRUE;

	if (have_font_consolas && have_font_segoe_ui)
		return 0;

	return 1;
}

static void checkFonts()
{
	LOGFONTA lf = {0};
	HDC hDC = GetDC(NULL);
	EnumFontFamiliesExA(hDC, &lf, (FONTENUMPROCA)EnumFontFamExProc, 0, 0);
	font_size = -MulDiv(DLG_FONT_SIZE, GetDeviceCaps(hDC, LOGPIXELSY), 72); // need to accurately calculate font size according to screen DPI; see the comments for the `lfHeight` param on MSDN: https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-logfonta#members
	ReleaseDC(NULL, hDC);
}