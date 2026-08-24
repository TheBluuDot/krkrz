
#ifndef __FONT_RASTERIZER_H__
#define __FONT_RASTERIZER_H__

class FontRasterizer {

public:
	virtual ~FontRasterizer() {}
	virtual void AddRef() = 0;
	virtual void Release() = 0;
	virtual void ApplyFont( class tTVPNativeBaseBitmap *bmp, bool force ) = 0;
	virtual void ApplyFont( const struct tTVPFont& font ) = 0;
	virtual void GetTextExtent(tjs_char ch, tjs_int &w, tjs_int &h) = 0;
	virtual tjs_int GetAscentHeight() = 0;
	virtual class tTVPCharacterData* GetBitmap( const struct tTVPFontAndCharacterData & font, tjs_int aofsx, tjs_int aofsy ) = 0;
	virtual void GetGlyphDrawRect( const ttstr & text, struct tTVPRect& area ) = 0;
	/**
	 * Complex text layout: convert logical text into visual-order glyphs
	 * with bidirectional reordering and script shaping (libraqm:
	 * fribidi + harfbuzz, with graphite2 used for fonts carrying Silf
	 * tables such as Awami Nastaliq). Returns false when the
	 * implementation is unavailable or the text needs no shaping; the
	 * caller then falls back to plain per-character rendering.
	 */
	virtual bool ShapeText( const ttstr & text, std::vector<struct tTVPShapedGlyph> & glyphs ) { return false; }
};

#endif // __FREE_TYPE_FONT_RASTERIZER_H__
