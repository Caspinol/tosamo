RM		= /bin/rm

CC		= gcc
SIZE		= size

BIN_NAME	= tosamo
TEST_BIN_NAME	= tosamo_test

BUILD		= obj
BIN_DIR		= bin
TEST_DIR	= tests
SRC_DIR 	= src

SRC		= main.c config.c tcp.c settings.c
SRC		+= master.c slave.c timed.c
SRC		+= utils.c log.c crc.c lists.c

INC		= -I$(SRC_DIR)/include

DEFINE		=

LFLAGS 		= 

CFLAGS 		= -Wall -g -ggdb -std=gnu99
CFLAGS		+= -pthread
CFLAGS		+= $(INC) $(DEFINE)

TCFLAGS		= -Wall -g -ggdb -std=gnu9
TCFLAGS		+= $(INC) $(DEFINE)

VPATH		= src

OBJS		= $(addprefix $(BUILD)/, $(SRC:.c=.o))

.PHONY: all dir clean show maketest

all: $(BIN_DIR)/$(BIN_NAME) maketest

$(BUILD)/%.o: %.c | dir
	@echo "CC 	-	$<"
	$(CC) $(CFLAGS) -c -o $@ $<

$(BIN_DIR)/$(BIN_NAME): $(OBJS)
	@echo "LD	-	$(BIN_NAME)"
	@$(CC) $(LFLAGS) $^ -o $@
	@echo "SIZE	OF	$(BIN_NAME)"
	@$(SIZE) $(BIN_DIR)/$(BIN_NAME)

dir: $(BUILD) $(TEST_DIR)

$(BUILD):
	@echo "MK DIR	-	 $@"
	@mkdir -p $@

$(TEST_DIR):
	@echo "MK DIR	-	 $@"
	@mkdir -p $@


clean:
	@echo "Deleting *.o files"
	@$(RM) $(OBJS) | true
	@echo "Deleting $(BIN_NAME) binary"
	@$(RM) $(BIN_DIR)/$(BIN_NAME) | true
	@echo "Deleting test binary"
	@$(RM) $(TEST_DIR)/$(TEST_BIN_NAME) | true
	@echo "Cleanup complete!"

show:
	@echo "Source dir: $(SRC)"
	@echo "Build dir: $(BUILD)"
	@echo "Bin dir: $(BIN_DIR)"
	@echo "obj files: $(OBJS)"
