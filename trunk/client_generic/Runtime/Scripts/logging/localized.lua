require"logging"

--
function logging.localized( _language, _fallback, _output )

	local concat = table.concat
	local function check( _template, _logtable ) 
		local used = { id = true }

		for key in _template:gmatch( '${(%w+)}' ) do
			used[key] = true
		end

		local buf = {}
		for k, v in pairs( _logtable ) do
			if not used[k] then
				buf[ #buf + 1 ] = k .. ': ' .. v
			end
		end

		if #buf > 0 then
			return ' (' .. concat( buf, ', ' ) .. ')'
		else
			return ''
		end
	end

	--
	function	TranslateMessage( _lingo, _logTab )

		--	If it's not a table, make it a string.
		if type( _logTab ) ~= "table" then
			_logTab = { id = logging.tostring( _logTab ) }
		end

		--	If it is a table, but no id, the it has nothing to do with localization.
		if not _logTab.id then
			return _logTab
		end
		
		local template = _lingo.messages[ _logTab.id ] or _logTab.id

 		local extra = check( template, _logTab )
		return template:gsub( '${(%w+)}', _logTab ) .. extra
	end

	local logger = _output

	return logging.new(  function( self, level, message )
							local lingo = require( _language )

							--	Set metatable to fallback.
							if _fallback ~= _language then
								local fallback = require( _fallback )
								local langinherit_mt = {}
								setmetatable( lingo.messages, langinherit_mt )
								langinherit_mt.__index = function( _t, _key ) return fallback.messages[ _key ] end
								langinherit_mt.__mode = "k"
							end

							return logger:info( TranslateMessage( lingo, message ) )
						end, true )
end