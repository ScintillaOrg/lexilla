// Scintilla source code edit control
// Converter.h - Encapsulation of iconv
// Copyright 2004 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#include <iconv.h>
const iconv_t iconvhBad = (iconv_t)(-1);
// Since various versions of iconv can not agree on whether the src argument
// is char ** or const char ** provide a templatised adaptor.
template<typename T>
size_t iconv_adaptor(size_t(*f_iconv)(iconv_t, T, size_t *, char **, size_t *),
		iconv_t cd, char** src, size_t *srcleft,
		char **dst, size_t *dstleft) {
	return f_iconv(cd, (T)src, srcleft, dst, dstleft);
}
/**
 * Encapsulate iconv safely and avoid iconv_adaptor complexity in client code.
 */
class Converter {
	iconv_t iconvh;
public:
	Converter() {
		iconvh = iconvhBad;
	}
	Converter(const char *charSetDestination, const char *charSetSource) {
		iconvh = iconv_open(charSetDestination, charSetSource);
	}
	~Converter() {
		Close();
	}
	operator bool() const {
		return iconvh != iconvhBad;
	}
	void Open(const char *charSetDestination, const char *charSetSource) {
		Close();
		if (*charSetSource) {
			iconvh = iconv_open(charSetDestination, charSetSource);
		}
	}
	void Close() {
		if (iconvh != iconvhBad) {
			iconv_close(iconvh);
			iconvh = iconvhBad;
		}
	}
	size_t Convert(char** src, size_t *srcleft, char **dst, size_t *dstleft) const {
		if (iconvh == iconvhBad) {
			return (size_t)(-1);
		} else {
			return iconv_adaptor(iconv, iconvh, src, srcleft, dst, dstleft);
		}
	}
};
