#top level makefile
ifeq ($(VER), debug)
	GDB=-g
else ifeq ($(VER), release)
	GDB=-O3
else
$(warning default release module)
$(warning useage: make VER={debug|release})
	VER=release
	GDB=-O3
endif

BIN_NAME=$(VER)
CC=gcc
#CC=arm-none-linux-gnueabi-gcc
ROOT_DIR=$(shell pwd)
OUT_NAME=make
OUT_DIR=$(ROOT_DIR)/$(OUT_NAME)
OBJS_DIR=$(OUT_NAME)/$(BIN_NAME)_obj
BIN_DIR=$(OUT_NAME)/$(BIN_NAME)
INST_DIR=/usr/local/lib
ZIT_NAME=libzit.so
ZIT_SONAME=$(ZIT_NAME).1
ZIT_VER=$(ZIT_SONAME).0.0
CFLAGS=$(GDB) -D_REENTRANT -fPIC -Wall -Werror -I.
CFLAGST = $(GDB) -D_REENTRANT -Wall -Werror -I$(ROOT_DIR)
# **** export variable to sub makefiles ***
export CC CFLAGS BIN_NAME GDB ZIT_VER ZIT_SONAME VER

define make_obj
	@mkdir -p $(OUT_DIR)
	@mkdir -p $(OBJS_DIR)
	@mkdir -p $(BIN_DIR)
	@cp -u makeout.mk $(OBJS_DIR)/makefile
	@./compiler.sh src $(OBJS_DIR) $(CC) "$(CFLAGS)"
endef
#	@gcc makeworker.c -o makeworker
#	@./makeworker src $(OBJS_DIR) .c $(CC) $(CFLAGS)

define install_zit
	@rm -f $(INST_DIR)/$(ZIT_NAME)* &&\
	rm -fr /usr/local/include/zit && \
	cp -r zit /usr/local/include/ && \
	cp $(BIN_DIR)/$(ZIT_VER) $(INST_DIR) &&\
	cd $(INST_DIR) &&\
	ldconfig -n ./ &&\
	ldconfig &&\
	ln -s $(ZIT_SONAME) $(ZIT_NAME) &&\
	cd -
endef

.PHONY : all
all: makezit makeout
.PHONY:makezit
makezit:
	$(make_obj)
makeout:
	@make -C $(OBJS_DIR)

.PHONY : test
test :
	@./compiler.sh tests $(OBJS_DIR) $(CC) "$(CFLAGST)" &&\
	make -C $(OBJS_DIR) test

#.PHONY : arm_test
#arm_test :
#	@./compiler tests $(OBJS_DIR) $(CC) "$(CFLAGST)" &&\
	make -C $(OBJS_DIR) arm_test
#	@./makeworker tests $(OBJS_DIR) .c $(CC) $(CFLAGST) &&\


.PHONY:clean
clean:
	@rm -fr $(OUT_DIR) &&\
	rm -f makeworker

.PHONY:install
install :
	$(install_zit)

.PHONY:uninstall
uninstall:
	@rm -f $(INST_DIR)/$(ZIT_NAME)* && ldconfig && rm -fr /usr/local/include/zit
