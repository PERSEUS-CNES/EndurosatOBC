

################################################################################

OBJ_DIR = obj
OBJ_DIR_SBG = sbgECom/obj
TARGET_DIR = out
TARGET_DIR_SBG = sbgECom/out
TARGET_NAME = obc
TARGET_NAME_SBG = sbg

CC=gcc

################################################################################

SUBDIR = Centrale/sbgECom/src Centrale/sbgECom/src/protocol Centrale/sbgECom/src/commands Centrale/sbgECom/src/commands/transfer \
Centrale/sbgECom/src/binaryLogs Centrale/sbgECom/common Centrale/sbgECom/common/version Centrale/sbgECom/common/swap \
Centrale/sbgECom/common/streamBuffer Centrale/sbgECom/common/splitBuffer Centrale/sbgECom/common/platform \
Centrale/sbgECom/common/network Centrale/sbgECom/common/interfaces Centrale/sbgECom/common/crc

SRC_DIR = .
OUTILS := FileDeMessage.c EnvoieDataFileMessage.c ReceptionDataFileMessage.c
FILS   := FilsCentrale.c FilsSauvegarde.c FilsTransmission.c FilsSS.c FilsEnergie.c
STRUCT :=  Structure.c  VariableGlobale.c

SRCS := OBC.c $(OUTILS) $(STRUCT) $(FILS)

SRCS_SBG = \
$(wildcard **/*.c $(foreach fd, $(SUBDIR), $(fd)/*.c))

INCS_DIR = . \
Librairies
#$(wildcard **/*.c $(foreach fd, $(SUBDIR), $(fd)/*.c))

LIBS_DIR = . \
$(TARGET_DIR_SBG)

LIBS = \
ftd2xx \
$(TARGET_NAME_SBG)

################################################################################

DEPENDENCIES = \
pthread rt dl 

GCCFLAGS = -g -W -Wall -Wextra #-Wconversion -Werror -mtune=native  -march=native  -std=c99


CFLAGS = \
${INCS_DIR:%=-I%} \
$(addprefix -I, $(SUBDIR)) \
$(GCCFLAGS)

CFLAGS_SBG = \
$(addprefix -I, $(SUBDIR)) \
$(GCCFLAGS)

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