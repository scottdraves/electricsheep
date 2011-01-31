require"logging"

function logging.null()
	return logging.new(	function( self, level, message )
							return message
						end, true )
end