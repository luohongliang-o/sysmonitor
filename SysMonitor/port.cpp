/*

*/

#include "port.h"
#include <string.h>

/*
* Find the first occurrence of find in s, where the search is limited to the
* first slen characters of s.
*/
char * strstr_s(const char *s, size_t slen, const char *find) {
	char c, sc;  size_t len;
	if ((c = *find++) != '\0') {
		len = strlen(find);
		do {
			do {
				if (slen-- < 1 || (sc = *s++) == '\0')
					return (NULL);
			} while (sc != c);
			if (len > slen)
				return (NULL);
		} while (strncmp(s, find, len) != 0);
		s--;
	}
	return ((char *)s);
}

char * strchr_s(const char * string, size_t slen, int ch) {

	size_t idx = 0;
	while (*string && *string != (char)ch && idx < slen) {
		string++; ++idx;
	}

	if (*string == (char)ch)
		return((char *)string);
	return(NULL);
}

/*
* Find the first occurrence of find in s, where the search is limited to the
* first slen characters of s.
*/
char * strncpy_s_(char * _Dst, size_t dlen, const char * src, size_t slen) {
	if (dlen <= slen) {
		strncpy(_Dst, src, dlen - 1);
		_Dst[dlen - 1] = 0;
	}
	else {
		strncpy(_Dst, src, slen);
		_Dst[slen] = 0;
	}
	return _Dst;
}
