module( ... )
vars = {

--	player
--fullscreen		= { type="bool" },
LoopIterations	= { type="int", min=0, max=8 },
PieceWiseLinear	= { type="bool" },
player_fps		= { type="int", min=5, max=60 },
BufferLength	= { type="int", min=1, max=200 },

--	content
server = { type="string" },
sheepdir = { type="string" },
download_mode = { type="bool" },
--use_bittorrent = { type="bool" },
registered = { type="bool" },
password = { type="pass" },
use_proxy = { type="bool" },
proxy = { type="string" },
proxy_username = { type="string" },
proxy_password = { type="pass" },

--	generator
demo_mode = { type="bool" },
all_cores = { type="bool" },
save_frames = { type="bool" },
nickname = { type="string" },
user_url = { type="string" },

}
