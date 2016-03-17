RM		= /bin/rm

CC		= gcc
SIZE		= size

BIN_NAME	= tosamod
TEST_BIN_NAME	= tosamo_test

BUILD		= obj
BIN_DIR		= bin
TEST_DIR	= tests
SRC_DIR 	= src

INSTALL_PFX	= /usr/local
UNAME		= $(shell uname)

SRC		= main.c config.c tcp.c settings.c
SRC		+= master.c slave.c timed.c
SRC		+= utils.c log.c crc.c lists.c

TSRC		= CuTest.c runTests.c setTests.c

INC		= -I$(SRC_DIR)/include

DEFINE		=

LFLAGS 		= -pthread

CFLAGS 		= -Wall -g -ggdb -std=gnu99
CFLAGS		+= $(INC) $(DEFINE)

TCFLAGS		= -Wall -g -ggdb -std=gnu9
TCFLAGS		+= $(INC) $(DEFINE)

VPATH		= src

OBJS		= $(addprefix $(BUILD)/, $(SRC:.c=.o))
# Build objects out of test files
TOBJS		= $(addprefix $(TEST_DIR)/, $(TSRC:.c=.o))

.PHONY: all dir clean maketest install uninstall upgrade

all: $(BIN_DIR)/$(BIN_NAME) $(TEST_DIR)/$(TEST_BIN_NAME)

$(BUILD)/%.o: %.c | dir
	@echo "CC 	-	$<"
	$(CC) $(CFLAGS) -c -o $@ $<

$(BIN_DIR)/$(BIN_NAME): $(OBJS)
	@echo "LD	-	$(BIN_NAME)"
	@$(CC) $(LFLAGS) $^ -o $@
	@echo "SIZE	OF	$(BIN_NAME)"
	@$(SIZE) $(BIN_DIR)/$(BIN_NAME)

dir: $(BUILD) $(BIN_DIR)

$(BUILD):
	@echo "MK DIR	-	 $@"
	@mkdir -p $@

$(BIN_DIR):
	@echo "MK DIR	-	 $@"
	@mkdir -p $@

$(TEST_DIR)/%.o: %.c
	@echo "CC	-	$<"
	$(CC) $(CFLAGS) -c -o $@ $<

$(TEST_DIR)/$(TEST_BIN_NAME): $(TOBJS) $(filter-out $(BUILD)/main.o, $(OBJS))
	@echo "LD	-	$(TEST_BIN_NAME)"
	@$(CC) $(LFLAGS) $^ -o $@
	@echo "Tests build - run: make test"

test:
	$(TEST_DIR)/$(TEST_BIN_NAME)

install:
	@echo "Installing config file"
	@install etc/tosamo.cfg $(INSTALL_PFX)/etc/
	@echo "Installing binaries"
	@install $(BIN_DIR)/$(BIN_NAME) $(INSTALL_PFX)/sbin
	@echo "Installing init script"
ifeq ($(UNAME), Darwin)
	@echo "On OSX"
	@install scripts/osx/net.catdamnit.tosamod.plist /Library/LaunchDaemons/
else
	@echo "On Linux"
	@install scripts/linux/tosamod /etc/init.d/
endif

uninstall:
	@echo "Un-installing"
	@echo "Deleting binaries"
	@rm -f $(INSTALL_PFX)/sbin/$(BIN_NAME)
	@echo "Removing init script"
ifeq ($(UNAME), Darwin)
	@echo "On OSX"
	@rm -f /Library/LaunchDaemons/net.catdamnit.tosamod.plist
else
	@echo "On Linux"
	@rm -f /etc/init.d/tosamod
endif

upgrade:
	@echo "Cleaning up old binaries"
	rm -f $(INSTALL_PFX)/sbin/$(BIN_NAME)
	@echo "Installing new binaries"
	@install $(BIN_DIR)/$(BIN_NAME) $(INSTALL_PFX)/sbin
clean:
	@echo "Deleting *.o files"
	@$(RM) $(OBJS) $(TOBJS) | true
	@echo "Deleting $(BIN_NAME) binary"
	@$(RM) $(BIN_DIR)/$(BIN_NAME) | true
	@echo "Deleting test binary"
	@$(RM) $(TEST_DIR)/$(TEST_BIN_NAME) | true
	@echo "Cleanup complete!"
