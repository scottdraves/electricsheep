require "class"

module( ..., package.seeall )

local _mt = class.create( _M )

--	Return new instance.
function _M:new( _len )
	return setmetatable( { m_Len = _len, m_History = {} }, _mt )
end

--  Insert into queue.
function _M:insert( _v )
	--	Insert first.
	table.insert( self.m_History, 1, _v )

	--	Remove 'oldest' element.
	if #self.m_History > self.m_Len then
		table.remove( self.m_History )
	end
end

--
function _M:clear()
	--	yay for gc...
	self.m_History = {}
end

--	Check if an element exists in the histogram
function _M:contains( _v )
	for k, v in ipairs( self.m_History ) do
		if v == _v then
			return true
		end
	end
	return false
end
