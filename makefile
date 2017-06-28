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
VERSION_MAJOR=1
VERSION_MINOR=0
VERSION_REVISION=0
VERSION_DATE=$(shell date '+20%y%m%d')
# base => alpha => beta => RC => relaease
VERSION_ALPHABET=beta
ZIT_SONAME=$(ZIT_NAME).$(VERSION_MAJOR)
ZIT_VER=$(ZIT_SONAME).$(VERSION_MINOR).$(VERSION_REVISION)
CFLAGS=$(GDB) -D_REENTRANT -fPIC -Wall -Werror -I.
CFLAGST = $(GDB) -D_REENTRANT -Wall -Werror -I$(ROOT_DIR)
# **** export variable to sub makefiles ***
export CC CFLAGS BIN_NAME GDB ZIT_VER ZIT_SONAME ZIT_NAME VER

AUTO_VERSION=src/auto_version.h
define auto_version
	@echo "#define VER_AUTO 1" > $(AUTO_VERSION) &&\
	echo "const int major_version=$(VERSION_MAJOR);" >> $(AUTO_VERSION) &&\
	echo "const int minor_version=$(VERSION_MINOR);" >> $(AUTO_VERSION) &&\
	echo "const int revision_version=$(VERSION_REVISION);" >> $(AUTO_VERSION) &&\
	echo "const char *version= \"V$(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_REVISION).$(VERSION_DATE)_$(VERSION_ALPHABET)\";" >> $(AUTO_VERSION) &&\
	echo "const char *build_date = \"BUILD_DATE:$(shell date '+20%y.%m.%d %k:%M')\";" >> $(AUTO_VERSION) &&\
	echo "const char *git_rev = \"GIT_REV:$(shell git rev-parse HEAD)\";" >> $(AUTO_VERSION)
endef

define make_obj
	$(auto_version) &&\
	mkdir -p $(OUT_DIR) &&\
	mkdir -p $(OBJS_DIR) &&\
	mkdir -p $(BIN_DIR) &&\
	cp -u makeout.mk $(OBJS_DIR)/makefile &&\
	./compiler.sh src $(OBJS_DIR) $(CC) "$(CFLAGS)" &&\
	rm -f $(AUTO_VERSION)
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
