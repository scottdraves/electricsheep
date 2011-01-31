#include	"FontGL.h"
#include	"Log.h"
#include	"Settings.h"
#include	<fstream>
#include	<iostream>

#ifdef LINUX_GNU
#include <endian.h>
#if __BYTE_ORDER == __LITTLE_ENDIAN
#define __LITTLE_ENDIAN__ __LITTLE_ENDIAN
#undef __BIG_ENDIAN__
#else
#undef __LITTLE_ENDIAN__
#define __BIG_ENDIAN__ __BIG_ENDIAN
#endif
#endif

#if defined( WIN32 ) || defined( __LITTLE_ENDIAN__ )
	#define SWAP_LONG_(x) ((x << 24) | ((x << 8) & 0x00FF0000) |  ((x >> 8) & 0x0000FF00) | (x >> 24))
	#define SWAP_SHORT_(x) ((x << 8) | (x >> 8))
#else
	#define SWAP_LONG_(x) x
	#define SWAP_SHORT_(x) x
#endif

// Helper function to read a piece of data from a stream.
template<class T, class S>
void Read_Object(T& to_read, S& in)
{
	in.read(reinterpret_cast<char*>(&to_read), sizeof(T));
}

// This is how glyphs are stored in the file.
struct Glyph_Buffer
{
	unsigned char ascii, width;
	unsigned short x, y;
};


namespace	DisplayOutput
{

/*
	CFontGL().

*/
CFontGL::CFontGL( spCTextureFlat textTexture  ) : CBaseFont(), m_glyphs(NULL)
{
	m_spTextTexture = textTexture;

	for (uint32 i = 0; i != 256; ++i)
		m_table[i] = NULL;

}


/*
	~CFontGL().

*/
CFontGL::~CFontGL()
{		
	SAFE_DELETE_ARRAY(m_glyphs);
}

/*
*/
bool	CFontGL::Create()
{
  // Open the file and check whether it is any good (a font file
  // starts with "F0")
#ifndef LINUX_GNU
	std::ifstream input((g_Settings()->Get( "settings.app.InstallDir", std::string(".\\") ) + "TrebuchetMS-20.glf").c_str(), std::ios::binary);
#else
	std::ifstream input((g_Settings()->Get( "settings.app.InstallDir", std::string("") ) + "TrebuchetMS-20.glf").c_str(), std::ios::binary);
#endif
	if (input.fail() || input.get() != 'F' || input.get() != '0')
		return false;
 
	// Get the texture size, the number of glyphs and the line height.
	uint32 width, height, n_chars, line_height, tmp;
	
	//multibytes fields are stored as big endian
	Read_Object(tmp, input);
	width = SWAP_LONG_(tmp);
	
	Read_Object(tmp, input);
	height = SWAP_LONG_(tmp);
	
	Read_Object(tmp, input);
	line_height = SWAP_LONG_(tmp);
	
	Read_Object(tmp, input);
	n_chars = SWAP_LONG_(tmp);
	
	m_lineHeight = line_height;
	m_texLineHeight = static_cast<fp4>(m_lineHeight) / (fp4)height;
 
	// Make the glyph table.
	m_glyphs = new Glyph[n_chars];
	for (uint32 i = 0; i != 256; ++i)
		m_table[i] = NULL;
 
	// Read every glyph, store it in the glyph array and set the right
	// pointer in the table.
	Glyph_Buffer buffer;
	for (uint32 i = 0; i < n_chars; ++i){
		Read_Object(buffer, input);
		
		buffer.x = SWAP_SHORT_(buffer.x);
		buffer.y = SWAP_SHORT_(buffer.y);
		
		m_glyphs[i].tex_x1 = static_cast<fp4>(buffer.x) / (fp4)width;
		m_glyphs[i].tex_x2 = static_cast<fp4>(buffer.x + buffer.width) / (fp4)width;
		m_glyphs[i].tex_y1 = static_cast<fp4>(buffer.y) / (fp4)height;
		m_glyphs[i].advance = buffer.width;

		m_table[buffer.ascii] = m_glyphs + i;
	}
	
	// All chars that do not have their own glyph are set to point to
	// the default glyph.
	Glyph* default_glyph = m_table[(unsigned char)'\xFF'];
	// We must have the default character (stored under '\xFF')
	if (default_glyph == NULL)
		return false;
		
	for (uint32 i = 0; i != 256; ++i){
		if (m_table[i] == NULL)
			m_table[i] = default_glyph;
	}
	
	m_spTextImage = new DisplayOutput::CImage();
		
	m_spTextImage->Create( width, height, eImage_RGBA8, 0, false );
	
	if (height != m_spTextImage->GetHeight() || m_spTextImage->GetPitch(0) != width * 4)
		return false;

 	input.read(reinterpret_cast<char*>(m_spTextImage->GetData(0)), m_spTextImage->GetPitch(0) * m_spTextImage->GetHeight());
		
	m_spTextTexture->Upload(m_spTextImage);

	return( true );
}

fp4 CFontGL::LineHeight() const
{
	return m_lineHeight;
}

fp4 CFontGL::TexLineHeight() const
{
	return m_texLineHeight;
}

fp4 CFontGL::CharWidth(uint8 c) const
{
	if (m_table[c])
		return m_table[c]->advance;
	else
		return 0.0;
}
 
fp4 CFontGL::StringWidth(const std::string& str) const
{
	fp4 total = 0.0;
	for (uint32 i = 0; i != str.size(); ++i)
		total += CharWidth(str[i]);
	return total;
}

CFontGL::Glyph *CFontGL::GetGlyph(uint8 c)
{
	return m_table[c];
}

spCTextureFlat CFontGL::GetTexture( void )
{
	return m_spTextTexture;
}
	
void	CFontGL::Reupload()
{
	m_spTextTexture->Reupload();
}
	
};
