#top level makefile

CC=gcc
ROOT_DIR=$(shell pwd)
OUT_NAME=make
OUT_DIR=$(ROOT_DIR)/$(OUT_NAME)
OBJS_DIR=$(OUT_DIR)/obj
BIN_DIR=$(OUT_DIR)/bin
GDB=-g
INST_DIR=/usr/local/lib
ZIT_NAME=libzit.so
ZIT_VER=$(ZIT_NAME).0.0.0
CFLAGS='$(GDB) -fPIC -Wall -Werror -I$(ROOT_DIR)'

# **** export variable to sub makefiles ***
export CC CFLAGS OBJS_DIR BIN_DIR GDB

define make_obj
	@mkdir -p $(OUT_DIR)
	@mkdir -p $(OBJS_DIR)
	@mkdir -p $(BIN_DIR)
	@cp -u makeout.mk $(OBJS_DIR)/makefile
	@chmod 755 ./makeworker
	@./makeworker src $(OBJS_DIR) .c $(CC) $(CFLAGS)
endef

define install_zit
	@rm -f $(INST_DIR)/$(ZIT_NAME)* &&\
	cp $(BIN_DIR)/$(ZIT_VER) $(INST_DIR) &&\
	cd $(INST_DIR) &&\
	ldconfig -n ./ &&\
	ldconfig &&\
	ln -s $(ZIT_NAME).0 $(ZIT_NAME) &&\
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
	@./makeworker tests $(OBJS_DIR) .c $(CC) $(CFLAGS) &&\
	make -C $(OBJS_DIR) test

.PHONY:clean
clean:
	@rm -fr $(OUT_DIR)

.PHONY:install
install :
	$(install_zit)

.PHONY:uninstall
uninstall:
	@rm -f $(INST_DIR)/$(ZIT_NAME)* && ldconfig
