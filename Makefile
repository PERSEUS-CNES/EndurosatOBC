

################################################################################

OBJ_DIR = obj
TARGET_DIR = out
TARGET_NAME = obc

CC=gcc

################################################################################

SRC_DIR = .
SRCS = \
main.c \
simpleEndurosat.c \
es_crc32.c

INCS_DIR = . 

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