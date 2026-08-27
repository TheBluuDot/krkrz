
#define _USE_MATH_DEFINES
#include "FreeTypeFontRasterizer.h"
#include "LayerBitmapIntf.h"
#include "FreeType.h"
#include <cmath>
#include "MsgIntf.h"
#include "DebugIntf.h"

#ifdef TVP_ENABLE_RAQM
#include <raqm.h>
#endif

extern void TVPUninitializeFreeFont();
extern FontSystem* TVPFontSystem;

FreeTypeFontRasterizer::FreeTypeFontRasterizer() : RefCount(0), Face(NULL), LastBitmap(NULL) {
	AddRef();
}
FreeTypeFontRasterizer::~FreeTypeFontRasterizer() {
	if( Face ) delete Face;
	Face = NULL;
	TVPUninitializeFreeFont();
}
void FreeTypeFontRasterizer::AddRef() {
	RefCount++;
}
//---------------------------------------------------------------------------
void FreeTypeFontRasterizer::Release() {
	RefCount--;
	LastBitmap = NULL;
	if( RefCount == 0 ) {
		if( Face ) delete Face;
		Face = NULL;

		delete this;
	}
}
//---------------------------------------------------------------------------
void FreeTypeFontRasterizer::ApplyFont( class tTVPNativeBaseBitmap *bmp, bool force ) {
	if( bmp != LastBitmap || force ) {
		ApplyFont( bmp->GetFont() );
		LastBitmap = bmp;
	}
}
//---------------------------------------------------------------------------
void FreeTypeFontRasterizer::ApplyFont( const tTVPFont& font ) {
	CurrentFont = font;
	std::wstring stdname = TVPFontSystem->GetBeingFont(font.Face.AsStdString());
	// TVP_FACE_OPTIONS_NO_ANTIALIASING
	// TVP_FACE_OPTIONS_NO_HINTING
	// TVP_FACE_OPTIONS_FORCE_AUTO_HINTING
	tjs_uint32 opt = 0;
	opt |= (font.Flags & TVP_TF_ITALIC) ? TVP_TF_ITALIC : 0;
	opt |= (font.Flags & TVP_TF_BOLD) ? TVP_TF_BOLD : 0;
	opt |= (font.Flags & TVP_TF_UNDERLINE) ? TVP_TF_UNDERLINE : 0;
	opt |= (font.Flags & TVP_TF_STRIKEOUT) ? TVP_TF_STRIKEOUT : 0;
	opt |= (font.Flags & TVP_TF_FONTFILE) ? TVP_FACE_OPTIONS_FILE : 0;
	bool recreate = false;
	if( Face ) {
		if( Face->GetFontName() != stdname ) {
			delete Face;
			Face = new tFreeTypeFace( stdname, opt );
			recreate = true;
		}
	} else {
		Face = new tFreeTypeFace( stdname, opt );
		recreate = true;
	}
	Face->SetHeight( font.Height < 0 ? -font.Height : font.Height );
	if( recreate == false ) {
		if( font.Flags & TVP_TF_ITALIC ) {
			Face->SetOption(TVP_TF_ITALIC);
		} else {
			Face->ClearOption(TVP_TF_ITALIC);
		}
		if( font.Flags & TVP_TF_BOLD ) {
			Face->SetOption(TVP_TF_BOLD);
		} else {
			Face->ClearOption(TVP_TF_BOLD);
		}
		if( font.Flags & TVP_TF_UNDERLINE ) {
			Face->SetOption(TVP_TF_UNDERLINE);
		} else {
			Face->ClearOption(TVP_TF_UNDERLINE);
		}
		if( font.Flags & TVP_TF_STRIKEOUT ) {
			Face->SetOption(TVP_TF_STRIKEOUT);
		} else {
			Face->ClearOption(TVP_TF_STRIKEOUT);
		}
	}
	LastBitmap = NULL;
}
//---------------------------------------------------------------------------
void FreeTypeFontRasterizer::GetTextExtent(tjs_char ch, tjs_int &w, tjs_int &h) {
	if( Face ) {
		tGlyphMetrics metrics;
		if( Face->GetGlyphSizeFromCharcode( ch, metrics) ) {
			w = metrics.CellIncX;
			h = metrics.CellIncY;
		}
	}
}
//---------------------------------------------------------------------------
tjs_int FreeTypeFontRasterizer::GetAscentHeight() {
	if( Face ) return Face->GetAscent();
	return 0;
}
//---------------------------------------------------------------------------
tTVPCharacterData* FreeTypeFontRasterizer::GetBitmap( const tTVPFontAndCharacterData & font, tjs_int aofsx, tjs_int aofsy ) {
	if( font.Antialiased ) {
		Face->ClearOption( TVP_FACE_OPTIONS_NO_ANTIALIASING );
	} else {
		Face->SetOption( TVP_FACE_OPTIONS_NO_ANTIALIASING );
	}
	if( font.Hinting ) {
		Face->ClearOption( TVP_FACE_OPTIONS_NO_HINTING );
		//Face->SetOption( TVP_FACE_OPTIONS_FORCE_AUTO_HINTING );
	} else {
		Face->SetOption( TVP_FACE_OPTIONS_NO_HINTING );
		//Face->ClearOption( TVP_FACE_OPTIONS_FORCE_AUTO_HINTING );
	}
	if( font.GlyphIndexMode ) {
		// shaped glyph: metrics come from the shaper, glyph is addressed
		// by FreeType glyph index directly
		tTVPCharacterData* data = Face->GetGlyphFromGlyphIndex(
			font.GlyphIndex,
			font.ShapedCellIncX, font.ShapedCellIncY,
			font.ShapedOfsX, font.ShapedOfsY );
		if( data == NULL ) {
			data = Face->GetGlyphFromGlyphIndex( 0,
				font.ShapedCellIncX, font.ShapedCellIncY,
				font.ShapedOfsX, font.ShapedOfsY ); // .notdef fallback
		}
		if( data == NULL ) {
			TVPThrowExceptionMessage( TVPFontRasterizeError );
		}
		data->Antialiased = font.Antialiased;
		data->FullColored = false;
		data->Blured = font.Blured;
		data->BlurWidth = font.BlurWidth;
		data->BlurLevel = font.BlurLevel;
		data->OriginX += aofsx;
		if(font.Blured) data->Blur(); // nasty ...
		return data;
	}
	tTVPCharacterData* data = Face->GetGlyphFromCharcode(font.Character);
	if( data == NULL ) {
		data = Face->GetGlyphFromCharcode( Face->GetDefaultChar() );
	}
	if( data == NULL ) {
		data = Face->GetGlyphFromCharcode( Face->GetFirstChar() );
	}
	if( data == NULL ) {
		TVPThrowExceptionMessage( TVPFontRasterizeError );
	}

	int cx = data->Metrics.CellIncX;
	int cy = data->Metrics.CellIncY;
	if( font.Font.Angle == 0 ) {
		data->Metrics.CellIncX = cx;
		data->Metrics.CellIncY = 0;
	} else if(font.Font.Angle == 2700) {
		data->Metrics.CellIncX = 0;
		data->Metrics.CellIncY = cx;
	} else {
		double angle = font.Font.Angle * (M_PI/1800);
		data->Metrics.CellIncX = static_cast<tjs_int>(  std::cos(angle) * cx);
		data->Metrics.CellIncY = static_cast<tjs_int>(- std::sin(angle) * cx);
	}

	data->Antialiased = font.Antialiased;
	data->FullColored = false;
	data->Blured = font.Blured;
	data->BlurWidth = font.BlurWidth;
	data->BlurLevel = font.BlurLevel;

	// apply blur
	if(font.Blured) data->Blur(); // nasty ...
	return data;
}
//---------------------------------------------------------------------------
void FreeTypeFontRasterizer::GetGlyphDrawRect( const ttstr & text, tTVPRect& area ) {
	// アンチエイリアスとヒンティングは有効にする
	Face->ClearOption( TVP_FACE_OPTIONS_NO_ANTIALIASING );
	Face->ClearOption( TVP_FACE_OPTIONS_NO_HINTING );

	area.left = area.top = area.right = area.bottom = 0;
	tjs_int offsetx = 0;
	tjs_int offsety = 0;
	tjs_uint len = text.length();
	for( tjs_uint i = 0; i < len; i++ ) {
		tjs_char ch = text[i];
		tjs_int ax, ay;
		tTVPRect rt(0,0,0,0);
		bool result = Face->GetGlyphRectFromCharcode(rt,ch,ax,ay);
		if( result == false ) result = Face->GetGlyphRectFromCharcode(rt,Face->GetDefaultChar(),ax,ay);
		if( result == false ) result = Face->GetGlyphRectFromCharcode(rt,Face->GetFirstChar(),ax,ay);
		if( result ) {
			rt.add_offsets( offsetx, offsety );
			if( i != 0 ) {
				area.do_union( rt );
			} else {
				area = rt;
			}
		}
		offsetx += ax;
		offsety = 0;
	}
}


#ifdef TVP_ENABLE_RAQM
//---------------------------------------------------------------------------
/**
 * round a value in 1/64 pixel units to the nearest integer pixel
 */
static tjs_int TVPRoundFrom26_6( tjs_int v ) {
	return v >= 0 ? ((v + 32) >> 6) : -(((-v) + 32) >> 6);
}

/**
 * does this text contain characters that require complex layout?
 * (bidirectional reordering and/or context-dependent shaping)
 */
static bool TVPTextNeedsShaping( const ttstr & text ) {
	tjs_uint len = text.length();
	for( tjs_uint i = 0; i < len; i++ ) {
		tjs_char c = text[i];
		if( (c >= 0x0590 && c <= 0x08FF) || // Hebrew, Arabic, Syriac, Thaana, NKo, ...
			(c >= 0x0900 && c <= 0x109F) || // Indic through Myanmar
			(c >= 0x1200 && c <= 0x137F) || // Ethiopic
			(c >= 0x1780 && c <= 0x17FF) || // Khmer
			(c >= 0xFB50 && c <= 0xFDFF) || // Arabic Presentation Forms-A
			(c >= 0xFE70 && c <= 0xFEFF) )  // Arabic Presentation Forms-B
			return true;
	}
	return false;
}
#endif

bool FreeTypeFontRasterizer::ShapeText( const ttstr & text, std::vector<tTVPShapedGlyph> & glyphs ) {
#ifdef TVP_ENABLE_RAQM
	if(!Face) { TVPAddImportantLog(TJS_W("[shape-diag] no Face")); return false; }
	tjs_uint len = text.length();
	if(len == 0) return false;
	if(!TVPTextNeedsShaping(text)) return false; // keep legacy path for simple text

	static int shape_diag_count = 0;
	bool shape_diag = false;
	if(shape_diag_count < 25)
	{
		shape_diag_count += 1;
		shape_diag = true;
		TVPAddImportantLog(ttstr(TJS_W("[shape-diag] begin len=") + ttstr((int)len) + TJS_W(" face=") + Face->GetFontName()));
	}

	// UTF-16 -> UTF-32 (tjs_char is char16_t on non-Windows builds,
	// wchar_t with UTF-16 semantics on Windows builds)
	std::vector<uint32_t> u32;
	u32.reserve(len);
	for( tjs_uint i = 0; i < len; i++ ) {
		tjs_uint32 c = (tjs_uint32)(tjs_uint16)text[i];
		if( c >= 0xD800 && c <= 0xDBFF && i + 1 < len ) {
			tjs_uint32 lo = (tjs_uint32)(tjs_uint16)text[i+1];
			if( lo >= 0xDC00 && lo <= 0xDFFF ) {
				c = 0x10000 + ((c - 0xD800) << 10) + (lo - 0xDC00);
				i++;
			} else {
				c = 0xFFFD; // lone high surrogate
			}
		} else if( c >= 0xDC00 && c <= 0xDFFF ) {
			c = 0xFFFD; // lone low surrogate
		}
		u32.push_back(c);
	}

	FT_Face ftface = Face->GetFTFaceForShaping();
	if(!ftface) { if(shape_diag) TVPAddImportantLog(TJS_W("[shape-diag] ftface NULL -> legacy path")); return false; }

	raqm_t* rq = raqm_create();
	if(!rq) { if(shape_diag) TVPAddImportantLog(TJS_W("[shape-diag] raqm_create failed")); return false; }
	bool ok = true;
	ok &= raqm_set_text(rq, u32.data(), u32.size()) != 0;
	ok &= raqm_set_freetype_face(rq, ftface) != 0;
	ok &= raqm_set_par_direction(rq, RAQM_DIRECTION_DEFAULT) != 0;
	int load_flags = FT_LOAD_NO_BITMAP;
	if(Face->GetOption(TVP_FACE_OPTIONS_NO_HINTING))
		load_flags |= FT_LOAD_NO_HINTING | FT_LOAD_NO_AUTOHINT;
	if(Face->GetOption(TVP_FACE_OPTIONS_FORCE_AUTO_HINTING))
		load_flags |= FT_LOAD_FORCE_AUTOHINT;
	raqm_set_freetype_load_flags(rq, load_flags);
	ok &= raqm_layout(rq) != 0;
	if(!ok) {
		if(shape_diag) TVPAddImportantLog(TJS_W("[shape-diag] raqm steps failed -> legacy path"));
		raqm_destroy(rq);
		return false;
	}

	size_t count = 0;
	raqm_glyph_t* g = raqm_get_glyphs(rq, &count);
	glyphs.clear();
	glyphs.reserve(count);
	for(size_t gi = 0; gi < count; gi++) {
		tTVPShapedGlyph sg;
		sg.GlyphIndex = g[gi].index;
		sg.Cluster = g[gi].cluster;
		sg.CellIncX = TVPRoundFrom26_6(g[gi].x_advance);
		sg.CellIncY = TVPRoundFrom26_6(g[gi].y_advance);
		sg.OfsX = TVPRoundFrom26_6(g[gi].x_offset);
		sg.OfsY = TVPRoundFrom26_6(g[gi].y_offset);
		glyphs.push_back(sg);
	}
	raqm_destroy(rq);
	if(shape_diag) TVPAddImportantLog(ttstr(TJS_W("[shape-diag] OK glyphs=") + ttstr((int)count) + TJS_W(" textlen=") + ttstr((int)len)));
	return true;
#else
	return false;
#endif
}
