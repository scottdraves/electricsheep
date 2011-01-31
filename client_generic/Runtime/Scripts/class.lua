module( ..., package.seeall )

--
function create( _members )

		members = _members or {}

		local mt = {
			__metatable = members;
			__index     = members;
		}

		local function new( _, _init )
			return setmetatable( _init or {}, mt )
		end

		local function copy( _obj, ... )
			local newobj = _obj:new( unpack( arg ) )

			for n,v in pairs( _obj ) do
				newobj[n] = v
			end

			return newobj
		end

		members.new  = members.new  or new
		members.copy = members.copy or copy

		return mt
end
