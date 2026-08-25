
#include "tjsCommHead.h"

#include "FontSystem.h"
#include "StringUtil.h"
#include "MsgIntf.h"
#include <vector>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4819)
#endif
#include <ft2build.h>
#include FT_FREETYPE_H
#ifdef _MSC_VER
#pragma warning(pop)
#endif

extern void TVPInitializeFont();
extern void TVPGetAllFontList( std::vector<std::wstring>& list );
extern const tjs_char *TVPGetDefaultFontName();

void FontSystem::InitFontNames() {
	// enumlate all fonts
	if(FontNamesInit) return;

	std::vector<std::wstring> list;
	TVPGetAllFontList( list );
	size_t count = list.size();
	for( size_t i = 0; i < count; i++ ) {
		AddFont( list[i] );
	}

	FontNamesInit = true;
}
//---------------------------------------------------------------------------
void FontSystem::AddFont( const std::wstring& name ) {
	TVPFontNames.Add( name, 1 );
}
//---------------------------------------------------------------------------
bool FontSystem::FontExists( const std::wstring &name ) {
	// check existence of font
	InitFontNames();

	int * t = TVPFontNames.Find(name);
	return t != NULL;
}

FontSystem::FontSystem() : FontNamesInit(false), DefaultLOGFONTCreated(false) {
	ConstructDefaultFont();
}

void FontSystem::ConstructDefaultFont() {
	if( !DefaultLOGFONTCreated ) {
		DefaultLOGFONTCreated = true;
		DefaultFont.Height = -12;
		DefaultFont.Flags = 0;
		DefaultFont.Angle = 0;
		DefaultFont.Face = ttstr(TVPGetDefaultFontName());
	}
}

std::wstring FontSystem::GetBeingFont(std::wstring fonts) {
	// retrieve being font in the system.
	// font candidates are given by "fonts", separated by comma.

	bool vfont;

	if(fonts.c_str()[0] == TJS_W('@')) {     // for vertical writing
		fonts = fonts.c_str() + 1;
		vfont = true;
	} else {
		vfont = false;
	}

	bool prev_empty_name = false;
	while(fonts!=TJS_W("")) {
		std::wstring fontname;
		std::wstring::size_type pos = fonts.find_first_of(TJS_W(","));
		if( pos != std::string::npos ) {
			fontname = Trim( fonts.substr( 0, pos) );
			fonts = fonts.c_str()+pos+1;
		} else {
			fontname = Trim(fonts);
			fonts=TJS_W("");
		}

		// no existing check if previously specified font candidate is empty
		// eg. ",Fontname"

		if(fontname != TJS_W("") && (prev_empty_name || FontExists(fontname) ) ) {
			if(vfont && fontname.c_str()[0] != TJS_W('@')) {
				return  TJS_W("@") + fontname;
			} else {
				return fontname;
			}
		}

		prev_empty_name = (fontname == TJS_W(""));

	}
	if(vfont) {
		return std::wstring(TJS_W("@")) + std::wstring(TVPGetDefaultFontName());
	} else {
		return std::wstring(TVPGetDefaultFontName());
	}
}

//---------------------------------------------------------------------------
extern FT_Library FreeTypeLibrary; //!< FreeType ライブラリ (FreeType.cpp)
extern bool TVPEncodeUTF8ToUTF16( std::wstring &output, const std::string &source );

bool FontSystem::AddFontFromFile( const std::wstring & localpath, std::vector<std::wstring>* faces ) {
	// register privately with GDI so that the native FreeType rasterizer
	// (which opens fonts through GDI GetFontData) can access the file by
	// face name; FR_PRIVATE keeps it out of the system font list
	int added = AddFontResourceExW( localpath.c_str(), FR_PRIVATE, NULL );
	if( added == 0 ) return false;

	// retrieve the face name(s) through FreeType and register them
	FT_Face face = NULL;
	if( FT_New_Face( FreeTypeLibrary, std::string( localpath.begin(), localpath.end() ).c_str(), 0, &face ) == 0 ) {
		if( face->family_name ) {
			std::wstring family;
			TVPEncodeUTF8ToUTF16( family, face->family_name );
			AddFont( family );
			if( faces ) faces->push_back( family );
		}
		FT_Done_Face( face );
	}
	return true;
}
