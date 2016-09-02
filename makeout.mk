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
# ls -s libzit.so.0.0.0 libzit.so.0

LIB_ZIT=atomic.o  jet.o  module.o  mutex.o  queue.o  ringbuf.o  semaphore.o  ssl.o  thread.o	time.o	traceconsole.o	tracelog.o  trace.o  tracering.o  type.o

.PHONY : all
all : $(BIN_DIR)/libzit.a $(BIN_DIR)/libzit.so.0.0.0
$(BIN_DIR)/libzit.a : $(LIB_ZIT)
	ar crv $@ $^
$(BIN_DIR)/libzit.so.0.0.0 : $(LIB_ZIT)
	$(CC) -shared -Wl,-soname,libzit.so.0 -o $@ $^
