module( ... )
messages = {

--
help = "Help",
apply = "Apply",
cancel = "Cancel",

--	Notify to restart.
restartLanguage = "Please reopen screensaver configuration to change language to ''${Language}''",

--	About tab.
aboutTab	= "About",

--	Player tab.
playerTab		= "Display",
playerDesc		= "These settings affect the visual presentation of the sheep on screen",
fullscreen		= "Fullscreen display (no need to change this except for debugging purposes)",
LoopIterations	= "The number of times to display a looping sheep",
PieceWiseLinear	= "Piecewise linear display. Produces smoother playback by interpolating across frames.",
player_fps		= "The number of times per second at which to decode a fresh frame from the sheep.\nLower this value if your machine isn't fast enough to keep up",
BufferLength	= "How many complete frames to buffer in advance.\nLower this value if ram is an issue.",


--	Content tab.
contentTab = "Server",
contentDesc = "These settings define how to communicate with the sheep server",
server = "The sheep server",
sheepdir = "Location of your sheep cache",
cache_size = "The amount of disk space the sheep may consume",
download_mode = "Enables the downloading of new sheep",
use_bittorrent = "Use BitTorrent peer-to-peer downloading (disabled in beta)",
registered = "Are you a registered user?",
password = "Your password",
unique_id = "Your unique client ID",
use_proxy = "Use a proxy server?",
proxy = "Proxy server ip address",
proxy_username = "Proxy server username",
proxy_password = "Proxy server password",

--	Generator tab.
generatorTab = "Renderer",
generatorDesc = "These settings affect how your client renders new sheep",
demo_mode = "Demo mode (disable rendering of sheep frames)",
all_cores = "Use all available processor cores to render sheep",
save_frames = "Save completed frames",
nickname = "Your user nickname",
user_url = "Link to your homepage",

--	Logs tab.
logsTab = "Logs",
logsDesc = "Logging information",

--	Statistics tab.
statsTab = "Statistics",
statsDesc = "Your sheep statistics",

--	Uninstall.
delSettings = "Delete settings",
delSheep = "Delete downloaded sheep",
delFrames = "Delete rendered frames"

}
