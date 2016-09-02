#top level makefile

CC=gcc
ROOT_DIR=$(shell pwd)
OUT_NAME=make
OUT_DIR=$(ROOT_DIR)/$(OUT_NAME)
OBJS_DIR=$(OUT_DIR)/obj
BIN_DIR=$(OUT_DIR)/bin
CFLAGS='-g -fPIC -Wall -Werror -I$(ROOT_DIR)'
export CC CFLAGS OBJS_DIR BIN_DIR

define make-dir
	@mkdir -p $(OUT_DIR)
	@mkdir -p $(OBJS_DIR)
	@mkdir -p $(BIN_DIR)
	@cp -u makeout.mk $(OBJS_DIR)/makefile
	@./makeworker src $(OBJS_DIR) .c gcc $(CFLAGS)
endef

.PHONY : all
all: makedir makeout
.PHONY:makedir
makedir:
	$(make-dir)
makeout:
	@make -C $(OBJS_DIR)

clean:
	@rm -fr $(OUT_DIR)