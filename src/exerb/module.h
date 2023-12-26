// $Id: module.h,v 1.4 2005/04/25 16:40:45 yuya Exp $
#ifndef MODULE_H
#define MODULE_H 1 

void Init_ExerbRuntime();
#ifdef RUBY19
void Init_Locale_Encodings();
#endif
#endif /* MODULE_H */