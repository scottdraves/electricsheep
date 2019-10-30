 sudo yum install $(cat rpm/required-packages.yum.list)

./autogen.sh

LUA_LIBS="-llua-5.1 -lm -ldl" ./configure

make CXXFLAGS+="-DGL_GLEXT_PROTOTYPES=1"

sudo make install