# =========================
# -Wl,--as-needed   ;only link needed *.so
# export LD_LIBRARY_PATH=/<libpath>  ;only for test
# -Wl,-rpaht=/<libpath>              ;app load file
# -------------------------
# #cp /<libpath>.lib*.so /usr/lib
# #chmod 755 /usr/lib/lib*.so
# #ldconfig
# =========================
# version control (solve dll hell)
# link-name     soname          real-name(lib<zit>.so.<major>.<minor>.<release>
# libzit.so ==> libzit.so.1 ==> libzit.so.1.1.1234
# <major>: not compatibility
# <minor>: downward compatibility
# <release>: compatible with each other
# gcc -shared -W,soname,libzit.so.0 -o llibzit.so.0.0.0 $^ ;make so
# ln -s libzit.so.0.0.0 libzit.so.0
# =============================
#$(BIN_DIR)/libzit.a : $(LIB_ZIT)
#	ar crv $@ $^
#	$(CC) -g -Wl,-rpath=. -L$(BIN_DIR) -lzit -lpthread -lrt -D_REENTRANT $^ -o $@

BIN_DIR=../$(BIN_NAME)
ZIT_LIB=atomic.o  jet.o  module.o  mutex.o  queue.o  ringbuf.o  semaphore.o  ssl.o  thread.o time.o traceconsole.o tracelog.o  trace.o \
tracering.o type.o list.o container.o framework.o socket.o filesys.o convert.o tracebkg.o rwlock.o dlfcn.o
ZIT_TST=tutility.o main.o tbase.o tthread.o
#ZIT_FLAGS='-lpthread -lrt -D_REENTRANT'

.PHONY : all
all : $(BIN_DIR)/$(ZIT_VER)
$(BIN_DIR)/$(ZIT_VER) : $(ZIT_LIB)
	$(CC) $(GDB) -shared -fPIC -Wl,-soname,$(ZIT_SONAME) -o $@ $^
ifeq ($(VER),release)
	@strip $@
endif

.PHONY : test
test : $(ZIT_TST)
	cd $(BIN_DIR) && rm -f $(ZIT_SONAME) $(ZIT_NAME)  && ln -s $(ZIT_VER) $(ZIT_SONAME) && ln -s $(ZIT_SONAME) $(ZIT_NAME)
	$(CC) $(GDB) -Wl,-rpath=.:make/$(VER) -L$(BIN_DIR) -o $(BIN_DIR)/zit_test $^ -lzit -pthread -ldl -lrt
	$(CC) $(GDB) -shared -o $(BIN_DIR)/libtstso.so tstso.o
# error follow, can not work on ubuntu.
#	$(CC) $(GDB) -lpthread -lrt -D_REENTRANT -lzit $^ -o $(BIN_DIR)/zit_test

.PHONY : arm_test
arm_test : $(ZIT_TST)
	$(CC) $(GDB) $^ -o $(BIN_DIR)/arm_zit_test -L $(BIN_DIR) -lzit -pthread -lrt
