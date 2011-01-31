#include <iostream>
#include <vector>
#include <string>

#include "../tinyXml/tinyxml.h"
#include "base.h"

extern "C" {
	#include "lua.h"
	#include "lauxlib.h"
	#include "lualib.h"
}

/*
*/
void LuaXML_ParseNode( lua_State *L, TiXmlNode* pNode)
{
	if( !pNode )
		return;

	//	Resize stack if neccessary.
	luaL_checkstack( L, 5, "LuaXML_ParseNode : recursion too deep" );

	TiXmlElement* pElem = pNode->ToElement();
	if( pElem )
	{
		//	Element name.
		lua_pushstring( L,"name" );
		lua_pushstring( L,pElem->Value() );
		lua_settable( L,-3 );

		//	Parse attributes.
		TiXmlAttribute* pAttr = pElem->FirstAttribute();
		if( pAttr )
		{
			lua_pushstring( L,"attr" );
			lua_newtable( L );

			for( ;pAttr; pAttr = pAttr->Next() )
			{
				lua_pushstring( L,pAttr->Name() );
				lua_pushstring( L,pAttr->Value() );
				lua_settable( L, -3 );
			}

			lua_settable( L, -3 );
		}
	}

	//	Children.
	TiXmlNode *pChild = pNode->FirstChild();
	if( pChild )
	{
		int iChildCount = 0;

		for( ;pChild; pChild = pChild->NextSibling() )
		{
			switch( pChild->Type() )
			{
				case TiXmlNode::TINYXML_ELEMENT:
					//	Normal element, parse recursive.
					lua_newtable( L );
					LuaXML_ParseNode( L, pChild );
					lua_rawseti( L, -2, ++iChildCount );
				break;

				case TiXmlNode::TINYXML_TEXT:
					//	Plaintext, push raw.
					lua_pushstring( L, pChild->Value() );
					lua_rawseti( L, -2, ++iChildCount );
				break;

				case TiXmlNode::TINYXML_DECLARATION:
				case TiXmlNode::TINYXML_UNKNOWN:
				case TiXmlNode::TINYXML_COMMENT:
				case TiXmlNode::TINYXML_DOCUMENT:
				default:
					break;
			};
		}

		lua_pushstring( L,"n" );
		lua_pushnumber( L, iChildCount );
		lua_settable( L, -3 );
	}
}

/*
*/
static int LuaXML_ParseFile( lua_State *L )
{
	const char* sFileName = luaL_checkstring( L, 1 );
	TiXmlDocument doc( sFileName );
	doc.LoadFile();
	lua_newtable( L );
	LuaXML_ParseNode( L, &doc );
	return 1;
}

/*
*/
static int LuaXML_ParseData( lua_State *L )
{
	const char  *sData = luaL_checkstring( L, 1 );

    TiXmlDocument doc;
	doc.Parse( (const char *)sData, 0, TIXML_ENCODING_UTF8 );
	lua_newtable( L );
	LuaXML_ParseNode( L, &doc );
	return 1;
}

//
static const luaL_Reg luaxmllib[] = {
	{	"parseData",    LuaXML_ParseData },
	{	"parseFile",	LuaXML_ParseFile },
	{	NULL,	NULL	}
};

/*
** Open math library
*/
extern int luaopen_xml( lua_State *L )
{
	luaL_register( L, "luaXML",  luaxmllib );
	return 1;
}
