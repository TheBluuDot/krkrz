

#ifndef __CHARACTER_DATA_H__
#define __CHARACTER_DATA_H__

#include "tjsCommHead.h"
#include "tvpfontstruc.h"

/**
 * １グリフのメトリックを表す構造体
 */
struct tGlyphMetrics
{
	tjs_int CellIncX;		//!< 一文字進めるの必要なX方向のピクセル数
	tjs_int CellIncY;		//!< 一文字進めるの必要なY方向のピクセル数
};

//---------------------------------------------------------------------------
/**
 * A single shaped glyph produced by the complex text layout layer
 * (libraqm: bidi + harfbuzz/graphite2 shaping). Positions are pixels.
 */
struct tTVPShapedGlyph
{
	tjs_uint GlyphIndex; //!< FreeType glyph index (glyph ID)
	tjs_uint Cluster;    //!< index of first source codepoint in the shaped text
	tjs_int CellIncX;    //!< pen advance in X, pixels
	tjs_int CellIncY;    //!< pen advance in Y, pixels
	tjs_int OfsX;        //!< draw offset in X relative to pen position, pixels
	tjs_int OfsY;        //!< draw offset in Y relative to pen position, pixels
};

//---------------------------------------------------------------------------
/**
 * １グリフを表すクラス
 */
class tTVPCharacterData
{
	// character data holder for caching
private:
	tjs_uint8 * Data;
	tjs_int RefCount;

public:
	tjs_int OriginX; //!< 文字Bitmapを描画するascent位置との横オフセット
	tjs_int OriginY; //!< 文字Bitmapを描画するascent位置との縦オフセット
	tGlyphMetrics	Metrics; //!< メトリック、送り幅と高さを保持
	tjs_int Pitch; //!< 保持している画像ピッチ
	tjs_uint BlackBoxX; //!< 保持している画像幅
	tjs_uint BlackBoxY; //!< 保持している画像高さ
	tjs_int BlurLevel;
	tjs_int BlurWidth;
	tjs_uint Gray; // 階調

	bool Antialiased;
	bool Blured;
	bool FullColored;

public:
	tTVPCharacterData() : Gray(65), FullColored(false) { RefCount = 1; Data = NULL; }
	tTVPCharacterData( const tjs_uint8 * indata,
		tjs_int inpitch,
		tjs_int originx, tjs_int originy,
		tjs_uint blackboxw, tjs_uint blackboxh,
		const tGlyphMetrics & metrics,
		bool fullcolor = false );
	tTVPCharacterData(const tTVPCharacterData & ref);
	~tTVPCharacterData() { if(Data) delete [] Data; }

	void Alloc(tjs_int size) {
		if(Data) delete [] Data, Data = NULL;
		Data = new tjs_uint8[size];
	}

	tjs_uint8 * GetData() const { return Data; }

	void AddRef() { RefCount ++; }
	void Release() {
		if(RefCount == 1) {
			delete this;
		} else {
			RefCount--;
		}
	}

	void Expand();

	void Blur(tjs_int blurlevel, tjs_int blurwidth);
	void Blur();

	void Bold(tjs_int size);
	void Bold2(tjs_int size);

	void Resample4();
	void Resample8();

	/**
	 * 水平線を追加する(取り消し線、アンダーライン用)
	 * @param liney : ライン中心位置
	 * @param thickness : ライン太さ
	 * @param val : ライン値
	 */
	void AddHorizontalLine( tjs_int liney, tjs_int thickness, tjs_uint8 val );
};

//---------------------------------------------------------------------------
// Character Cache management
//---------------------------------------------------------------------------
struct tTVPFontAndCharacterData
{
	tTVPFont Font;
	tjs_uint32 FontHash;
	tjs_char Character;
	tjs_int BlurLevel;
	tjs_int BlurWidth;
	bool Antialiased;
	bool Blured;
	bool Hinting;
	// Complex-text (shaped) mode: the key addresses a font glyph directly
	// instead of a character code, carrying the metrics computed by the
	// shaper so they survive the glyph cache round-trip.
	bool GlyphIndexMode;
	tjs_uint GlyphIndex;
	tjs_int ShapedCellIncX;
	tjs_int ShapedCellIncY;
	tjs_int ShapedOfsX;
	tjs_int ShapedOfsY;

	tTVPFontAndCharacterData() :
		FontHash(0), Character(0), BlurLevel(0), BlurWidth(0),
		Antialiased(false), Blured(false), Hinting(true),
		GlyphIndexMode(false), GlyphIndex(0),
		ShapedCellIncX(0), ShapedCellIncY(0), ShapedOfsX(0), ShapedOfsY(0) {}

	bool operator == (const tTVPFontAndCharacterData &rhs) const {
		return Character == rhs.Character && Font == rhs.Font &&
			Antialiased == rhs.Antialiased && BlurLevel == rhs.BlurLevel &&
			BlurWidth == rhs.BlurWidth && Blured == rhs.Blured &&
			Hinting == rhs.Hinting &&
			GlyphIndexMode == rhs.GlyphIndexMode &&
			(!GlyphIndexMode || (
				GlyphIndex == rhs.GlyphIndex &&
				ShapedCellIncX == rhs.ShapedCellIncX &&
				ShapedCellIncY == rhs.ShapedCellIncY &&
				ShapedOfsX == rhs.ShapedOfsX &&
				ShapedOfsY == rhs.ShapedOfsY));
	}
};
//---------------------------------------------------------------------------
class tTVPFontHashFunc
{
public:
	static tjs_uint32 Make(const tTVPFontAndCharacterData &val)
	{
		tjs_uint32 v = val.FontHash;

		v ^= val.Antialiased?1:0;
		v ^= val.Character;
		v ^= val.Blured?1:0;
		v ^= val.BlurLevel ^ val.BlurWidth;
		if (val.GlyphIndexMode)
		{
			// domain-separate shaped glyph entries from plain characters
			v ^= val.GlyphIndex * 2654435769u;
			v ^= 0x5a5a5a5a;
		}
		return v;
	}
};





#endif // __CHARACTER_DATA_H__
