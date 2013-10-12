require "table"
require "os"
require "io"
require "pq"
require "math"
--require "serialize"
--require "luaXML"
require "histogram"
require "os"

local dump = g_Log
--local dump = print
--local dump = function( _ ) end

context = {}

--	Set the rootpath for content, and the default fallback-sheep
function Init( _path, _numLoopIterations, _seamlessPlayback, _playEvenly, _medianlevel, _autoMedianLevel, _randomMedianLevel )

	dump( "Init: " .. _path )
	
	context.root = _path
	--	Add a trailing '/', if it's missing.. Something boost apparently only does under linux..
	local s = string.find( context.root, "/", -1, true )
	if s ~= context.root:len() then
		context.root = context.root .. "/"
	end

	context.sheep = {}
	context.allsheep = {}
	context.FirstLastCnt ={}
	--context.pq = pq:new()
	context.pq = {}
	context.histogram = histogram:new( 50 )
	context.CurrentSheep = nil
	context.LoopIterations = _numLoopIterations
	context.dirty = false
	context.SeamlessPlayback = _seamlessPlayback
	context.PlayEvenly = _playEvenly
	context.MaturityTreshold = 1.0
	context.AlertShown = false
	context.MedianLevel = _medianlevel
	
	if context.MedianLevel > 1.0 then
		context.MedianLevel = 1.0
	end
		
	if context.MedianLevel < 0.0 then
		context.MedianLevel = 0.0
	end
	context.AutoMedianLevel = _autoMedianLevel
	context.RandomMedianLevel = _randomMedianLevel

end

--	Add a sheep to the context, called from the directory-scanner in lua_playlist.h
function Add( _filepath, _file, _generation, _id, _first, _last, _atime )	
	
	local loopable = _first == _last;
	
	if context.allsheep[ _id ] == nil and (loopable == false or context.LoopIterations > 0) then	
		--dump ( "Adding " .. _id )
		context.dirty = true
		
		context.allsheep[ _id ] = {}
		context.allsheep[ _id ].generation = _generation
		context.allsheep[ _id ].playCount = g_PlayCount( _generation, _id ) or 0
		context.allsheep[ _id ].file = _file
		context.allsheep[ _id ].filepath = _filepath
		context.allsheep[ _id ].id = _id
		context.allsheep[ _id ].first = _first
		context.allsheep[ _id ].last = _last
		context.allsheep[ _id ].loopable = loopable
		context.allsheep[ _id ].atime = _atime
		context.allsheep[ _id ].rank = 0
		context.allsheep[ _id ].deleted = false
				
		if context.allsheep[ _id ].playCount > context.MaturityTreshold then
			context.allsheep[ _id ].maturity = 1.0
		else
			context.allsheep[ _id ].maturity = context.allsheep[ _id ].playCount / context.MaturityTreshold
		end
				
		context.sheep[ _id ] = context.allsheep[ _id ]
	end
	
end

--	Clear sheep context.
function Clear( _medianLevel )
    dump( "Clear" )
	context.sheep = {}
	context.allsheep = {}
	context.MedianLevel = _medianLevel
	
	if context.MedianLevel > 1.0 then
		context.MedianLevel = 1.0
	end
		
	if context.MedianLevel < 0.0 then
		context.MedianLevel = 0.0
	end
end

--
function	Override( _id )
    dump( "Override" )
	context.OverrideID = _id
end

--
function	Delete( _id )
	dump( tostring( _id ) .. ' marked for deletion...' )
	--	Create new .xxx files.

	--	Create a new empty file with ext .xxx so it wont get downloaded again.
	local newfile = string.gsub( context.sheep[_id].file, ".avi", ".xxx", 1 )
	local fh, msg = io.open( context.sheep[_id].filepath .. newfile, "w" )
	if fh == nil then
		dump( msg )
	else
		io.close( fh )
	end
	

	--	Add to deathrow
	context.deathrow = context.deathrow or {}
	table.insert( context.deathrow, _id )
	
	context.sheep[ _id ].deleted = true
end

function RebuildConnections()

	for k, v in pairs( context.FirstLastCnt ) do
		v.firstcnt = 0
		v.lastcnt = 0
	end

	for k, v in pairs( context.sheep ) do
		
		if 0.999 >= v.rank * context.PlayEvenly then
		
			if context.FirstLastCnt[ v.first ] == nil then
				context.FirstLastCnt[ v.first ] = {}
				context.FirstLastCnt[ v.first ].firstcnt = 1
				context.FirstLastCnt[ v.first ].lastcnt = 0
			else	
				context.FirstLastCnt[ v.first ].firstcnt = context.FirstLastCnt[ v.first ].firstcnt + 1
			end
			
			if context.FirstLastCnt[ v.last ] == nil then
				context.FirstLastCnt[ v.last ] = {}
				context.FirstLastCnt[ v.last ].firstcnt = 0
				context.FirstLastCnt[ v.last ].lastcnt = 1
			else
				context.FirstLastCnt[ v.last ].lastcnt = context.FirstLastCnt[ v.last ].lastcnt + 1
			end
		end
	end

end

function UpdateSheep()
	dump( "UpdateSheep" )
	
	local count = 0;
	
	for k, v in pairs( context.sheep ) do
		v.playCount = g_PlayCount( v.generation, v.id )
		
		if v.playCount > context.MaturityTreshold then
			v.maturity = 1.0
		else
			v.maturity = v.playCount / context.MaturityTreshold
		end

		v.atime = g_AccessTime( v.filepath .. v.file )
		
		count = count + 1
	end
	
	return count
end


function UpdateRanks()
	dump( "UpdateRanks" )
	
	a = {}
	
	for k, v in pairs( context.sheep ) do
		table.insert( a, v.playCount )
	end
	
	table.sort( a )
	local currentmedian = context.MedianLevel
	if context.RandomMedianLevel == true then
		currentmedian = context.MedianLevel + g_CRand() * (1. - context.MedianLevel)
		dump( "RandomMedian: " .. currentmedian)
	end
	local median = a[ math.floor( #a * currentmedian ) ]
	
	if median == nil then
		if #a > 0 then
			median = a[ 1 ]
		else
			median = 0
		end
	end
	
	dump ( "median: " .. median .. " - " .. math.floor( #a * currentmedian ) )
	g_ClearMedianSurvivorsStats()
	for k, v in pairs( context.sheep ) do
		if v.playCount > median then
			v.rank = 1.0
		else
			g_IncMedianCutSurvivors()
			v.rank = 0.0
		end
	end
end

function PlayEvenly( _rank )
		
	if g_CRand() >= _rank * context.PlayEvenly then
		return true
	end

	return false
end

function RemoveBrokenLoops()

	local change = true
	while change == true do
		change = false
		for k, v in pairs( context.sheep ) do
			local minimum
			
			if v.loopable then
				minimum = 2
			else
				minimum = 1
			end
			
			if context.FirstLastCnt[ v.first ] == nil or context.FirstLastCnt[ v.first ].lastcnt < minimum then
				--dump( "Broken: id=" .. tostring(v.id) .. ",first=" .. tostring(v.first) .. ",last=" .. tostring(v.last) .. ",atime=" .. tostring(v.atime) )
				change = true
				
				if context.FirstLastCnt[ v.first ] ~= nil then
					context.FirstLastCnt[ v.first ].firstcnt = context.FirstLastCnt[ v.first ].firstcnt - 1
				end
				
				if context.FirstLastCnt[ v.last ] ~= nil then
					context.FirstLastCnt[ v.last ].lastcnt = context.FirstLastCnt[ v.last ].lastcnt - 1
				end

				context.sheep[ v.id ] = nil
			end
			
			if context.FirstLastCnt[ v.last ] == nil or context.FirstLastCnt[ v.last ].firstcnt < minimum then
				--dump( "Broken: id=" .. tostring(v.id) .. ",first=" .. tostring(v.first) .. ",last=" .. tostring(v.last) .. ",atime=" .. tostring(v.atime) )
				change = true
				
				if context.FirstLastCnt[ v.first ] ~= nil then
					context.FirstLastCnt[ v.first ].firstcnt = context.FirstLastCnt[ v.first ].firstcnt - 1
				end

				if context.FirstLastCnt[ v.last ] ~= nil then
					context.FirstLastCnt[ v.last ].lastcnt = context.FirstLastCnt[ v.last ].lastcnt - 1
				end
				
				context.sheep[ v.id ] = nil
			end			
		end
	end
		
	local count = 0
	
	for k, v in pairs( context.sheep ) do
		g_IncDeadEndCutSurvivors()
		count = count + 1
		dump( "RemoveBrokenLoopsSurvivor: id=" .. tostring(v.id) .. ",first=" .. tostring(v.first) .. ",last=" .. tostring(v.last) .. ",atime=" .. tostring(v.atime) .. ",rank=" .. tostring(v.rank) )		
	end
	
	if count == 0 then
		dump( "RemoveBrokenLoops: There are no survivors - fallback to all sheep" )
		
		local i = 0
		
		local isloop = false
		
		for k, v in pairs( context.allsheep ) do
			context.sheep[ v.id ] = v
			
			i = i + 1
			
			if i == 1 then
				if v.first == v.last then
					isloop = true
				end
			end
		end
		
		if ( i ~= 1 or isloop == false ) and context.AlertShown == false then
			g_ErrorMessage( "Ignoring Seamless Playback. No closed loops found." )
			context.AlertShown = true
		end
	else
		context.AlertShown = false
	end
end

--
function Rebuild()

	dump( "Rebuild... " )
	
	if context.deathrow ~= nil then
		local preserveDeathrow = false
		for k, v in pairs( context.deathrow ) do
			--	Try to delete the file.
			local res, msg = os.remove( context.sheep[v].filepath .. context.sheep[v].file )
			if res ~= true then
				dump( msg )

				--	Sheep was currently playing or something like that, so lets preserve deathrow until next rebuild.
				preserveDeathrow = true
				dump( "Preserving deathrow" )
			else
				--	Remove from context list.
				if preserveDeathrow == false then
					dump( tostring( context.sheep[v].filepath ) .. tostring( context.sheep[v].file ) .. ' deleted due to negative vote...' )
					context.sheep[ v ] = nil
					context.allsheep[ v ] = nil
					context.dirty = true
				end
			end
		end

		--	At least one sheep wasnt removed, so keep the list until next rebuild.
		if preserveDeathrow == false then
			context.deathrow = nil
		end
	end
	
	local origcount = UpdateSheep()

	if origcount == 0 then
		return
	end	
	
	--if context.PlayEvenly > 0.0 then
		UpdateRanks()
	--end
	
	--if context.dirty == true then

		if context.SeamlessPlayback == true then
			g_ClearDeadEndSurvivorsStats()
			RebuildConnections()
			RemoveBrokenLoops()
		end
		
		context.dirty = false
	--end


	--	Fresh pq.
	--context.pq:clear()
	context.pq = {}

	local count = 0

	for k, v in pairs( context.sheep ) do
		--context.pq:insert( { prio = v.playCount, sheep = v } )
		--if v.loopable == true and v.rank < context.PlayEvenly then
		if 0.999 >= v.rank * context.PlayEvenly then
			table.insert( context.pq, 1, v )
			count = count + 1
		end
	end

	dump( "Found " .. count .. " sheep..." )
end

--	Always something returned from this playlist..
function Size()
	return 1
end

--
function GetRandomSheep()

   --    Make sure there is a queue...
--    if context.pq:empty() then
   if #context.pq == 0 then
       Rebuild()
   end

   local s = nil
   local evensheep = nil

   --    Return least played sheep.
   --if not context.pq:empty() then
   if #context.pq > 0 then

       retry = 0

       while retry < 10 do
           --local s = context.pq:remove()
           local rndnum = g_CRand()
           local rnd = math.floor( 1 + rndnum * #context.pq )
		   dump("rnd = " .. rnd);
           local s = context.pq[ rnd ]
		   
           if PlayEvenly( s.rank ) then
               evensheep = s
			   
			   if ( #context.pq < 20 ) or s.loopable or context.LoopIterations == 0 then
				dump( "Picked new sheep " .. s.id .. " from pq (playcount " .. s.playCount .. " ) rnd=" .. tostring(rnd) .. " rndnum=" .. tostring(rndnum) )
				return s
			   end
           end
           retry = retry + 1
       end 
   end
   
	if evensheep ~= nil then
		s = evensheep
	end

   if s == nil then
       dump( "Unable to grab a random sheep..." )
   else
       dump( "Picked non-evenly new sheep " .. s.id .. " from pq (playcount " .. s.playCount .. " )" )
   end

   return s
end

--
function check_for_eddy()

    local c = 0

	if #context.histogram.m_History == 0 then
		return false
	end

	--dump( #context.histogram.m_History .. " in history.." )

    for i=1, #context.histogram.m_History do

		local diff = 1
	    --dump( "edd checking " .. tostring(c) .. " " .. tostring(i) .. " " .. tostring(context.histogram.m_History[ i ].id) )

		for j=1, i do
			if i~=j then
				if context.histogram.m_History[ j ].id == context.histogram.m_History[ i ].id then
					diff = 0
				end
			end
		end

		c = c + diff

		if c <= i/3 then
			dump( "eddy " .. c .. "/" .. i )
			context.histogram:clear()
			return true
		end
    end

	--dump( "no eddy" )
    return false
end


--	Given a sheep, figure out where to go next.
function GraphAlgo( _currentSheep )

	--	User wants something else?
	if context.OverrideID then
		dump( "Override: " .. context.OverrideID )
		local s = context.sheep[ context.OverrideID ]
		context.OverrideID = nil
		return s
	end

	context.OverrideID = nil

	--	Nothing in, so nothing out...
	if not _currentSheep then
		dump( "GraphAlgo needs a starting point..." )
		return nil
	end

	dump( "GraphAlgo: CurrentSheep = " .. _currentSheep.id )

	local loops = pq:new()
	local edges = pq:new()
	local others = pq:new()

	local nextsheep = nil
	
	local tmpv = nil
	
	--	Build a list of loops & edges that the current sheep can jump to...
	for k, v in pairs( context.sheep ) do
		--	But not the same one since repeating loops are already dealt with above...
		if ( v.deleted == false ) and ( v.id ~= _currentSheep.id ) then
			if v.first == _currentSheep.last then --or v.last == _currentSheep.first then
				if v.loopable == true then
					loops:insert( { prio = v.atime * v.maturity, sheep = v } )
				else
					edges:insert( { prio = v.atime * v.maturity, sheep = v } )
				end
			else
				others:insert( { prio = v.atime * v.maturity, sheep = v } )
			end
		end
	end
	
	local firstedge = nil
	local firstloop = nil

	while not edges:empty() do
		--	Pick edge from pq.
		local edge = edges:remove().sheep
		
		if firstedge == nil then
			firstedge = edge
		end
		
		if PlayEvenly( edge.rank ) then
			dump( "picking edge!" )
			nextsheep = edge
			break
		end
	end
	
	if firstedge == nil then
		dump( "no edges" )
	end

	while not loops:empty() do
		--	Pick loop from pq.
		local loop = loops:remove().sheep
		
		if firstloop == nil then
			firstloop = loop
		end

		if PlayEvenly( loop.rank ) then
			dump( "picking loop!" )
			nextsheep = loop
			break
		end
	end

	if firstloop == nil then
		dump( "no loops" )
	end
	
	if nextsheep == nil and context.SeamlessPlayback == true then
		if firstloop ~= nil then
			nextsheep = firstloop
		else
			nextsheep = firstedge
		end
		dump( "choosing disabled sheep in emergency for seamless playback..." )
	end
	
	--while  nextsheep == nil and not others:empty() do
	--	local other = others:remove().sheep
	--	
	--	if PlayEvenly( other.rank ) then
	--		dump( "picking other!" )
	--		nextsheep = other
	--		break
	--	end
	--end
	
	--	Do we need to jump?
	if nextsheep ~= nil then

		dump( "Picked connection " .. nextsheep.id .. " from pq (playcount " .. nextsheep.playCount .. " )" )

		if check_for_eddy() == false then
			return nextsheep
		end
	end

	--	Will trigger a random sheep.
	dump( "No connections!" )
	return nil
end

--
function GetCurrentPlayingID()
	if not context.CurrentSheep then
		return 0
	end

	--dump( "GetCurrentPlayingID(): " .. tostring( context.CurrentSheep.id ) )

	return context.CurrentSheep.id
end

--
function GetOldestSheep()
	local atime = -1;
	local retval = nil;
	
	--	Build a list of loops & edges that the current sheep can jump to...
	for k, v in pairs( context.sheep ) do
		if ( v.deleted == false ) and ( atime == -1 or ( v.atime * v.maturity ) < atime ) and PlayEvenly( v.rank ) then
			atime = v.atime * v.maturity
			retval = v
		end
	end
	
	return retval
end


--	Return next sheep.
function Next( _curID, _startByRandom )
		
	local next = nil
	
	if context.sheep ~= nil then
		context.CurrentSheep = context.sheep[ _curID ]
	end 
	
	if context.CurrentSheep == nil then
		if _startByRandom == true then
			next = GetRandomSheep()
		else
			next = GetOldestSheep()
		end
	else
		next = GraphAlgo( context.CurrentSheep ) or GetRandomSheep()
	end
	
	if next then
		
		dump("Next sheep chosen: " .. next.id .. " played " .. next. playCount .. " times") 
		
		next.playCount = next.playCount + 1
		
		if next.playCount > context.MaturityTreshold then
			next.maturity = 1.0
		else
			next.maturity = next.playCount / context.MaturityTreshold
		end
		
		context.histogram:insert( next )

		next.atime = os.time( os.date('*t') )

		return next.filepath ..  next.file
	end

	dump( "No sheep chosen to play..." )
	context.CurrentSheep = nil
	return ""
end
