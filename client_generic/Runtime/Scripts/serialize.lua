require "table"
require "io"
require "string"

--
local function auto( tab, key )
	return setmetatable( {}, {
		__index = auto,
		__newindex = assign,
		parent = tab,
		key = key
	})
end

local meta = { __index = auto }

--	The if statement below prevents the table from being created if the value assigned is nil.
--	This is, I think, technically correct but it might be desirable to use assignment to nil to force a table into existence.
function assign( tab, key, val )
	-- if val ~= nil then
	local oldmt = getmetatable( tab )
	oldmt.parent[ oldmt.key ] = tab
	setmetatable( tab, meta )
	tab[ key ] = val
	-- end
end

--
function AutoTable( _tab )
	return setmetatable( _tab, meta )
end




--	exportstring( string )
--	returns a "Lua" portable version of the string
local function exportstring( s )
  s = string.format( "%q",s )
  -- to replace
  s = string.gsub( s,"\\\n","\\n" )
  s = string.gsub( s,"\r","\\r" )
  s = string.gsub( s,string.char(26),"\"..string.char(26)..\"" )
  return s
end

--	The Save Function
function table.save(  tbl,filename )

   local charS,charE = "   ","\n"
   local file,err
   -- create a pseudo file that writes to a string and return the string
   if not filename then
      file =  { write = function( self,newstr ) self.str = self.str..newstr end, str = "" }
      charS,charE = "",""
   -- write table to tmpfile
   elseif filename == true or filename == 1 then
      charS,charE,file = "","",io.tmpfile()
   -- write table to file
   -- use io.open here rather than io.output, since in windows when clicking on a file opened with io.output will create an error
   else
      file,err = io.open( filename, "w" )
      if err then return _,err end
   end
   -- initiate variables for save procedure
   local tables,lookup = { tbl },{ [tbl] = 1 }
   file:write( "return {"..charE )
   for idx,t in ipairs( tables ) do
      if filename and filename ~= true and filename ~= 1 then
         file:write( "-- Table: {"..idx.."}"..charE )
      end
      file:write( "{"..charE )
      local thandled = {}
      for i,v in ipairs( t ) do
         thandled[i] = true
         -- escape functions and userdata
         if type( v ) ~= "userdata" then
            -- only handle value
            if type( v ) == "table" then
               if not lookup[v] then
                  table.insert( tables, v )
                  lookup[v] = #tables
               end
               file:write( charS.."{"..lookup[v].."},"..charE )
            elseif type( v ) == "function" then
               file:write( charS.."loadstring("..exportstring(string.dump( v )).."),"..charE )
            else
               local value =  ( type( v ) == "string" and exportstring( v ) ) or tostring( v )
               file:write(  charS..value..","..charE )
            end
         end
      end
      for i,v in pairs( t ) do
         -- escape functions and userdata
         if (not thandled[i]) and type( v ) ~= "userdata" then
            -- handle index
            if type( i ) == "table" then
               if not lookup[i] then
                  table.insert( tables,i )
                  lookup[i] = #tables
               end
               file:write( charS.."[{"..lookup[i].."}]=" )
            else
               local index = ( type( i ) == "string" and "["..exportstring( i ).."]" ) or string.format( "[%d]",i )
               file:write( charS..index.."=" )
            end
            -- handle value
            if type( v ) == "table" then
               if not lookup[v] then
                  table.insert( tables,v )
                  lookup[v] = #tables
               end
               file:write( "{"..lookup[v].."},"..charE )
            elseif type( v ) == "function" then
               file:write( "loadstring("..exportstring(string.dump( v )).."),"..charE )
            else
               local value =  ( type( v ) == "string" and exportstring( v ) ) or tostring( v )
               file:write( value..","..charE )
            end
         end
      end
      file:write( "},"..charE )
   end
   file:write( "}" )
   -- Return Values
   -- return stringtable from string
   if not filename then
      -- set marker for stringtable
      return file.str.."--|"
   -- return stringttable from file
   elseif filename == true or filename == 1 then
      file:seek ( "set" )
      -- no need to close file, it gets closed and removed automatically
      -- set marker for stringtable
      return file:read( "*a" ).."--|"
   -- close file and return 1
   else
      file:close()
      return 1
   end
end

--// The Load Function
function table.load( sfile )
   -- catch marker for stringtable
   if string.sub( sfile,-3,-1 ) == "--|" then
      tables,err = loadstring( sfile )
   else
      tables,err = loadfile( sfile )
   end
   if err then return _,err
   end
   tables = tables()
   
   if tables == nil then
      return _, "Config file is corrupted"
   end
   
   for idx = 1,#tables do
      local tolinkv,tolinki = {},{}
      for i,v in pairs( tables[idx] ) do
         if type( v ) == "table" and tables[v[1]] then
            table.insert( tolinkv, { i,tables[v[1]] } )
         end
         if type( i ) == "table" and tables[i[1]] then
            table.insert( tolinki, { i,tables[i[1]] } )
         end
      end

      -- we need to set AutoTable on every level, otherwise we get asserts for uknown settings
      tables[idx] = AutoTable(tables[idx])

      -- link values, first due to possible changes of indices
      for _,v in ipairs( tolinkv ) do
         tables[idx][v[1]] = v[2]
      end
      -- link indices
      for _,v in ipairs( tolinki ) do
         tables[idx][v[2]],tables[idx][v[1]] =  tables[idx][v[1]],nil
      end
   end
   return AutoTable(tables[1])
end