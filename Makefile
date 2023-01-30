

################################################################################

OBJ_DIR = obj
TARGET_DIR = out
TARGET_NAME = obc

CC=gcc

################################################################################
DIR_1 =  ./sbgECom/src
DIR_2 =  ./sbgECom/src/binaryLogs 
DIR_3 =  ./sbgECom/src/commands/transfer
DIR_4 =  ./sbgECom/src/commands 
DIR_5 =  ./sbECom/common
DIR_6 =  ./sbECom/common/crc
DIR_7 =  ./sbECom/common/interfaces
DIR_8 =  ./sbECom/common/network
DIR_9 =  ./sbECom/common/platform
DIR_10 =  ./sbECom/common/splitBuffer
DIR_11 =  ./sbECom/common/streamBuffer
DIR_12 =  ./sbECom/common/swap
DIR_13 =  ./sbECom/common/version

SRC_DIR = .
SRCS = \
obc.c \
ftdi_manager.c\
storage_manager.c\
endurosat_manager.c \
es_crc32.c

INCS_DIR = $(DIR_1) $(DIR_2) $(DIR_3) $(DIR_4) $(DIR_5) $(DIR_6) $(DIR_7) $(DIR_8) $(DIR_9) $(DIR_10) $(DIR_11) $(DIR_12) $(DIR_13) 

LIBS_DIR = .
LIBS = \
ftd2xx

################################################################################

DEPENDENCIES = \
pthread rt dl

CFLAGS = \
${INCS_DIR:%=-I%} \
-Wall -Wextra

LDFLAGS = \
$(LIBS_DIR:%=-L%) 

################################################################################

OBJS := $(SRCS:%.c=$(OBJ_DIR)/%.o)

################################################################################

.PHONY: all clean

$(TARGET_DIR)/$(TARGET_NAME): $(OBJS)
	@echo "Linking $(notdir $@) ..."
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS:%=-l%) $(DEPENDENCIES:%=-l%)


$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@echo "Compiling $(notdir $<) ..."
	$(CC) $(CFLAGS) -x c -c -o $@ $<

$(OBJ_DIR) $(TARGET_DIR):
	mkdir -p $@

all: $(OBJ_DIR) $(TARGET_DIR) $(TARGET_DIR)/$(TARGET_NAME)
	
clean:
	rm -rf $(TARGET_DIR)/* $(OBJ_DIR)/*