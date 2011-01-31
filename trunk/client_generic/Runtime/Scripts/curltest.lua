require "table"
require "os"
require "io"
require "math"
require "string"

require "serialize"
require	"luaXML"
require "luacurl"
require "luazlib"


function	newCurl()

	local c = curl.new()

	--	Set all password etc stuff here

	--c:setopt( curl.OPT_PROXY, "yadda.org:1234" )

	return c
end



--	Download compressed file, return data.
function DownloadCompressed( _url )

	print( "Downloading " .. _url .. "\n" )

	local c = newCurl()

	c:setopt( curl.OPT_WRITEFUNCTION,	function ( stream, buffer )
											print( tostring(stream) )
											--if stream:write( buffer ) then
												--return string.len( buffer )
											--end
										end );

	local instream = "->" --io.open( "data.gzip", "wb" )

	c:setopt( curl.OPT_WRITEDATA, instream )

	c:setopt( curl.OPT_NOPROGRESS, false )
	c:setopt( curl.OPT_PROGRESSFUNCTION,	function ( _, dltotal, dlnow, uptotal, upnow )
												print( dltotal, dlnow, uptotal, upnow )
											end )

	c:setopt( curl.OPT_HTTPHEADER, "Connection: Keep-Alive", "Accept-Language: en-us" )
	c:setopt( curl.OPT_URL, _url )
	c:setopt( curl.OPT_CONNECTTIMEOUT, 15 )

	c:perform()
	c:close()

	--instream:close()

	local f = assert( io.open( "data.gzip", "rb" ) )
	local data = f:read( "*a" )
	f:close()

	local out = assert(io.open( "temp.gzip", "wb" ) )
	out:write( data )
	out:close()


	local res, l = zlib.gzuncompress( data )
	if not res then	print( l )	end

	print( "uncompressed size: " .. tostring(l) .. "\n" )

	--	Return uncompressed data.
	return res
end


local serverName = "v2d7c.sheepserver.net"
local CLIENT_VERSION = "WIN_2.7b11"
local clientID = g_GenerateID()


local xmldata = DownloadCompressed( "http://" .. serverName .. "/cgi/list?v=".. CLIENT_VERSION .."&u=" .. clientID )

--	Convert xml to regular lua table...
local xmltable = luaXML.parseData( xmldata )


table.save( xmltable, "test.lua" )
