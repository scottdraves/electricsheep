## Process this file with automake to produce Makefile.in

if IS_LINUX_GNU
linux_CFLAGS=-DLINUX_GNU=1 -DSHAREDIR=\"$(prefix)/share/electricsheep/\"
endif




INCLUDES = \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(datadir)/locale"\" \
	-I $(top_srcdir) \
	-I ../../DisplayOutput/OpenGL \
	-I ../../Common \
	-I ../../DisplayOutput \
	-I ../../DisplayOutput/Renderer \
	-I ../../Common/Math \
	-I ../../ContentDecoder \
	-I ../../TupleStorage \
	-I ../../ContentDownloader \
	-I ../../lua5.1/src \
	-I ../../Client \
	-I ../../Networking \
	-I ../../tinyXml \
	-I /usr/include/libavcodec \
	-I /usr/include/libavformat \
	-I /usr/include/libswscale \
	-I /usr/include/libavutil

bin_PROGRAMS = electricsheep-preferences

electricsheep_preferences_SOURCES = \
	config.cpp \
	electricsheepguiMyDialog2.cpp \
	main.cpp \
	../../tinyXml/tinyxmlparser.cpp \
	../../tinyXml/tinyxml.cpp \
	../../tinyXml/tinystr.cpp \
	../../tinyXml/tinyxmlerror.cpp \
	../../TupleStorage/diriterator.cpp \
	../../TupleStorage/luastorage.cpp \
	../../TupleStorage/storage.cpp \
	../../Common/AlignedBuffer.cpp \
	../../Common/Common.cpp \
	../../Common/Exception.cpp \
	../../Common/Log.cpp \
	../../Common/LuaState.cpp \
	../../Common/luaxml.cpp \
	../../Common/md5.c \
	../../Common/pool.cpp

electricsheep_preferences_LDADD = -lboost_thread -lboost_filesystem $(CURL_LIBS) $(LUA_LIBS) -ltinyxml @WX_LDADD@

AM_CXXFLAGS = $(linux_CFLAGS) @WX_CXXFLAGS@


