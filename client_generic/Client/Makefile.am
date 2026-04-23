## Process this file with automake to produce Makefile.in

if IS_LINUX_GNU
linux_CFLAGS=-DLINUX_GNU=1 -DSHAREDIR=\"$(prefix)/share/electricsheep/\"
endif

AM_CPPFLAGS = \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(datadir)/locale"\" \
	-DGL_GLEXT_PROTOTYPES \
	-I $(top_srcdir) \
	-iquote ../DisplayOutput/OpenGL \
	-I ../Common \
	-I ../DisplayOutput \
	-I ../DisplayOutput/Renderer \
	-I ../Common/Math \
	-I ../ContentDecoder \
	-I ../TupleStorage \
	-I ../ContentDownloader \
	-I ../lua5.1/src \
	-I ../Client \
	-I ../Networking \
	-I ../tinyXml

bin_PROGRAMS = electricsheep

electricsheep_SOURCES = \
Player.cpp \
main.cpp \
Voting.cpp \
Hud.cpp \
../Networking/Upload.cpp \
../Networking/Download.cpp \
../Networking/Networking.cpp \
../DisplayOutput/LoadDDS.cpp \
../DisplayOutput/Image.cpp \
../DisplayOutput/OpenGL/RendererGL.cpp \
../DisplayOutput/OpenGL/glx.cpp \
../DisplayOutput/OpenGL/egl.cpp \
../DisplayOutput/OpenGL/ShaderGL.cpp \
../DisplayOutput/OpenGL/TextureFlatGL.cpp \
../DisplayOutput/OpenGL/wgl.cpp \
../DisplayOutput/OpenGL/FontGL.cpp \
../DisplayOutput/OpenGL/mgl.cpp \
../DisplayOutput/Renderer/Shader.cpp \
../DisplayOutput/Renderer/Texture.cpp \
../DisplayOutput/Renderer/Font.cpp \
../DisplayOutput/Renderer/TextureFlat.cpp \
../DisplayOutput/Renderer/Renderer.cpp \
../DisplayOutput/LoadPNG.cpp \
../DisplayOutput/DisplayOutput.cpp \
../tinyXml/tinyxmlparser.cpp \
../tinyXml/tinyxml.cpp \
../tinyXml/tinystr.cpp \
../tinyXml/tinyxmlerror.cpp \
../TupleStorage/diriterator.cpp \
../TupleStorage/storage.cpp \
../TupleStorage/luastorage.cpp \
../ContentDecoder/ContentDecoder.cpp \
../ContentDownloader/SheepUploader.cpp \
../ContentDownloader/ContentDownloader.cpp \
../ContentDownloader/SheepGenerator.cpp \
../ContentDownloader/SheepDownloader.cpp \
../ContentDownloader/Sheep.cpp \
../ContentDownloader/Shepherd.cpp \
../Common/LuaState.cpp \
../Common/Common.cpp \
../Common/AlignedBuffer.cpp \
../Common/isaac.cpp \
../Common/pool.cpp \
../Common/Log.cpp \
../Common/Math/Rect.cpp \
../Common/luaxml.cpp \
../Common/Exception.cpp \
../Common/md5.c


if WITH_WAYLAND
# Wayland protocol files
WAYLAND_PROTOCOLS_SOURCES = \
	../DisplayOutput/OpenGL/xdg-shell-protocol.c \
	../DisplayOutput/OpenGL/xdg-shell-client-protocol.h \
	../DisplayOutput/OpenGL/wlr-layer-shell-unstable-v1-protocol.c \
	../DisplayOutput/OpenGL/wlr-layer-shell-unstable-v1-client-protocol.h \
	../DisplayOutput/OpenGL/xdg-decoration-unstable-v1-protocol.c \
	../DisplayOutput/OpenGL/xdg-decoration-unstable-v1-client-protocol.h

electricsheep_SOURCES += $(WAYLAND_PROTOCOLS_SOURCES)
BUILT_SOURCES = $(WAYLAND_PROTOCOLS_SOURCES)

endif

# Rules to generate the protocol files
%-protocol.c: %.xml
	$(WAYLAND_SCANNER) private-code < $< > $@

%-client-protocol.h: %.xml
	$(WAYLAND_SCANNER) client-header < $< > $@

electricsheep_LDADD = -lboost_system -lboost_thread -lboost_filesystem -lglut \
	$(AVCODEC_LIBS) $(AVFORMAT_LIBS) $(SWSCALE_LIBS) $(AVUTIL_LIBS) $(LUA_LIBS) $(GLU_LIBS) $(GLEE_LIBS) $(BOOST_LDADD) \
	$(CURL_LIBS) $(PNG_LIBS) $(XRENDER_LIBS) $(LIBGTOP_LIBS) $(WAYLAND_LIBS) $(LIBDECOR_LIBS) $(XKBCOMMON_LIBS)

AM_CXXFLAGS = $(linux_CFLAGS) $(AVCODEC_CFLAGS) $(AVFORMAT_CFLAGS) $(SWSCALE_CFLAGS) $(AVUTIL_CFLAGS) $(LIBGTOP_CFLAGS) \
	$(LUA_CFLAGS) $(GLU_CFLAGS) $(GLEE_CFLAGS) $(CURL_CFLAGS) $(PNG_CFLAGS) $(LUA_CFLAGS) $(XRENDER_CFLAGS) $(WAYLAND_CFLAGS) \
	$(LIBDECOR_CFLAGS) $(XKBCOMMON_CFLAGS) $(BOOST_CXXFLAGS) -lrt -lz -lGL \
	-D__STDC_CONSTANT_MACROS -Wno-write-strings $(AVC_DEFS)

CLEANFILES = $(BUILT_SOURCES)

