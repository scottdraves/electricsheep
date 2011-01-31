require( "table" )
require( "os" )
require( "math" )
require( "string" )
require "logging.null"
require "logging.localized"
require "desc"

local normalFont = "HELVETICA_NORMAL_10"
local boldFont = "HELVETICA_NORMAL_10"
local largeBoldFont = "HELVETICA_NORMAL_12"


local information =
[[
Electric Sheep is a distributed screen-saver.
During normal operation it uses the internet to contact the server (sheepserver.net) to download AVI animations, both with HTTP on port 80 and 8080, and also by BitTorrent(disabled in beta), which has been integrated into the client.

Until it downloads its first animation, the screen-saver displays just a black screen and a logo. This takes several minutes, and depending on network conditions and server load, perhaps much longer. Please be patient, and let it run overnight before concluding there is a problem.

The sheep (as the AVI animations are called) that you get have been rendered by users of the screen-saver all over the world. They are distributed under a Creative Commons license, which means you can reuse and remix them as long as you give credit to the Electric Sheep as their origin.

Your computer too will participate in this rendering effort.  So not only will your computer download sheep, but also download sheep genomes, render them into JPEG/PNG frames, and upload the results to the server.  No other information is sent to the server: the Electric Sheep respect your privacy.

The shepherds (those who wrote the software and run the server) use the sheep for commercial purposes in order to support the network and develop it further.  For example there's the Spotworks DVD, and "Dreams in High Fidelity", a painting that evolves.  Some jobs rendered by the network may be for images or animations which are not sheep at all, and will not appear in the screen-saver.

While you are watching the screen-saver, if you see a sheep you like (or dislike) you may vote for it by pushing the up (or down) arrow key.  These votes are transmitted to the server and used to drive the evolution of the sheep: sheep that receive more votes live longer and have sex with each other, producing offspring with a family resemblance.

If you are interested you can have more direct influence over the sheep by downloading additional software to design your own sheep and post them into the gene pool.  They will then be rendered by the network and eventually appear on screens all over the world.  If they prove popular, they will interbreed with the rest of the population,and variations on them will appear too. ]]


--	Create an iterator that iterates in alphabetical order...
function pairsByKeys( t, f )
	local a = {}

	for n in pairs( t ) do
		table.insert( a, n )
	end

	table.sort( a, f )

	local i = 0					-- iterator variable
	local iter = function ()	-- iterator function
		i = i + 1
		if a[i] == nil then
			return nil
		else
			return a[i], t[ a[i] ]
		end
	end

	return iter
end

--	Main func.
function CreateUI()

	--	Language settings.
	local languageName = "English"
	local localized = logging.localized( 'Dictionary.' .. languageName, 'Dictionary.English', logging.null() )
	function localize( _what )	return localized:info( _what )	end

	--	Create a string thing.
	function CreateString( _name, _value, _parent, _isPassword )
		return	iup.hbox
				{
					GAP = 100,
					iup.label{ title = localize( {id=_name} ) .. ": " },
					iup.text{ nc = 32, size = 132, value = _value, password = _isPassword, action = function( self, _c, _data ) _parent[_name] = _data end },
					iup.fill{},
				}
	end

	--	Create a checkbox toggler.
	function CreateToggle( _name, _value, _parent )
		local state = "OFF"
		if _value == 1 then state = "ON" end
		return	iup.hbox { GAP = 100, iup.toggle {	title = localize( {id=_name} ),	value = state,	action = function( self, _state )	_parent[ _name ] = _state	end }, iup.fill{}, }
	end

	--	Create a slider between _min & _max.
	function CreateNumber( _name, _value, _parent, _min, _max )
		local indicator = iup.label{ title=_value, ALIGNMENT="ALEFT:ABOTTOM" }
		local hbox = iup.hbox	{
									GAP = 10,
									indicator,
									iup.val { "HORIZONTAL"; RASTERSIZE="124x28", min=_min, max=_max, value=math.floor(_value), mousemove_cb = function( self, _val ) local v = math.floor(_val) indicator.title = v; _parent[ _name ] = v end },
									iup.label{ title = localize( {id=_name} ) },
								}
		return hbox
	end

	--	Create a tab.
	function CreateTabBox( _key, _value )
		local entries = {}
		for k, v in pairsByKeys( _value ) do
			local vardesc = desc.vars[ k ]	--	Only add variable if there's a description for it.
			if vardesc then
				if vardesc.type == "string" then table.insert( entries, CreateString( k, v, _value, "no" ) )	end
				if vardesc.type == "pass" then table.insert( entries, CreateString( k, v, _value, "yes" ) )	end
				if vardesc.type == "bool" then table.insert( entries, CreateToggle( k, v, _value ) )	end
				if vardesc.type == "int" then table.insert( entries, CreateNumber( k, v, _value, vardesc.min, vardesc.max ) )	end
			end
		end

		local box = iup.vbox{ gap = 10, iup.label{ title = localize( {id=_key .. "Desc"}), FONT=largeBoldFont },  unpack( entries ) }
		box.tabtitle = localize( {id=_key .. "Tab"} )
		return box
	end

	--	Build a sorted list of tabs from the settings.
	boxes = {}
	for k,v in pairsByKeys( g_Settings.settings ) do
		if k ~= "app" then
			table.insert( boxes, CreateTabBox( k, v ) )
		end
	end

	-- Creates dialog with tabs.
	dlg = iup.dialog {
						title = "Electric Sheep Config", size="600x280", RESIZE="NO", MAXBOX="NO", MINBOX="NO", MENUBOX="NO", MODAL="YES",
						iup.vbox
						{
							iup.tabs
							{
								--	First the about-tab, since it's not part of the settings...
								iup.hbox{ tabtitle = localize( { id="aboutTab" } ), iup.multiline{ font=normalFont, wordwrap="YES", size="420x170", value = information, READONLY="YES" }, iup.fill{}},
								--	... then unpack the table of tabs built above...
								unpack(boxes)
							},

							iup.fill{},
							iup.hbox
							{
								iup.label{ title = g_ClientVersion or 'Unknown version!', font=boldFont, ALIGNMENT="ACENTER:ACENTER" },
								iup.fill{},
								iup.hbox
								{
									iup.button{ title = localize( {id="help"} ), action = function() os.execute( 'start ' .. g_HelpLink ) end },
									iup.button{ title = localize( {id="apply"} ), action = function() table.save( g_Settings, g_Root .. "settings.lua" ) os.exit() end },
									iup.button{ title = localize( {id="cancel"} ), action = function() os.exit() end  },
								},
							}
						},
					}

	-- Shows dialog in the center of the screen
	dlg:showxy( iup.CENTER, iup.CENTER )
end

--	Build everything.
local status, err = pcall( CreateUI )
if status == false then iup.Message( "Failure", err )	end

--	Spin here until we're complete.
iup.MainLoop()
