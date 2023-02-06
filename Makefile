

################################################################################

OBJ_DIR = obj
OBJ_DIR_SBG = sbgECom/obj
TARGET_DIR = out
TARGET_DIR_SBG = sbgECom/out
TARGET_NAME = obc
TARGET_NAME_SBG = sbg

CC=gcc

################################################################################

SUBDIR = sbgECom/src sbgECom/src/protocol sbgECom/src/commands sbgECom/src/commands/transfer \
sbgECom/src/binaryLogs sbgECom/common sbgECom/common/version sbgECom/common/swap \
sbgECom/common/streamBuffer sbgECom/common/splitBuffer sbgECom/common/platform \
sbgECom/common/network sbgECom/common/interfaces sbgECom/common/crc 

SRC_DIR = .
SRCS = \
obc.c \
ftdi_manager.c\
storage_manager.c\
endurosat_manager.c \
es_crc32.c \
centrale.c

SRCS_SBG = \
$(wildcard **/*.c $(foreach fd, $(SUBDIR), $(fd)/*.c))

INCS_DIR = .
#$(wildcard **/*.c $(foreach fd, $(SUBDIR), $(fd)/*.c))

LIBS_DIR = . \
$(TARGET_DIR_SBG)

LIBS = \
ftd2xx \
$(TARGET_NAME_SBG)

################################################################################

DEPENDENCIES = \
pthread rt dl

CFLAGS = \
${INCS_DIR:%=-I%} \
$(addprefix -I, $(SUBDIR)) \
-Wall -Wextra

CFLAGS_SBG = \
$(addprefix -I, $(SUBDIR)) \
-Wall -Wextra

LDFLAGS = \
$(LIBS_DIR:%=-L%)

################################################################################

OBJS := $(SRCS:%.c=$(OBJ_DIR)/%.o)
OBJS_SBG := $(SRCS_SBG:%.c=$(OBJ_DIR_SBG)/%.o)

################################################################################

.PHONY: all clean

.DEFAULT_GOAL := all

$(TARGET_DIR_SBG)/$(TARGET_NAME_SBG): $(OBJS_SBG)
	@echo "Making SBG static library lib$(notdir $@).a ..."
	ar rcs $(TARGET_DIR_SBG)/lib$(TARGET_NAME_SBG).a $^

$(TARGET_DIR)/$(TARGET_NAME): $(OBJS)
	@echo "Linking $(notdir $@) ..."
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS:%=-l%) $(DEPENDENCIES:%=-l%)

$(OBJ_DIR_SBG)/%.o: %.c
	mkdir -p $(@D)
	@echo "Compiling SBG $(notdir $<) ..."
	$(CC) $(CFLAGS_SBG) -x c -c -o $@ $<

$(OBJ_DIR)/%.o: %.c
	@echo "Compiling $(notdir $<) ..."
	$(CC) $(CFLAGS) -x c -c -o $@ $<

$(OBJ_DIR) $(TARGET_DIR) $(OBJ_DIR_SBG) $(TARGET_DIR_SBG):
	mkdir -p $@
	
#PHONY += echoes
#echoes:
#	@echo "SRC files: $(SRCS_SBG)"
#	@:

all: $(OBJ_DIR) $(OBJ_DIR_SBG) $(TARGET_DIR) $(TARGET_DIR_SBG) $(TARGET_DIR_SBG)/$(TARGET_NAME_SBG) $(TARGET_DIR)/$(TARGET_NAME) 
	
clean:
	rm -rf $(TARGET_DIR)/* $(OBJ_DIR)/* $(TARGET_DIR_SBG)/* $(OBJ_DIR_SBG)/* 