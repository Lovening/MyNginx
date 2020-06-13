#.PHONY:all clean 

ifeq ($(DEBUG), true)
#CC = gcc -g
CC = g++ -g
VERSION = debug
else
#CC = gcc
CC = g++ 
VERSION = release
endif

#扫描当前文件夹下所有的.cpp文件
SRCS = $(wildcard *.cpp) 

OBJS = $(SRCS:.cpp=.o)

DEPS = $(SRCS:.cpp=.d)

# := 在解析阶段直接赋值常量字符串【立即展开】，而 = 在运行阶段，实际使用变量时再进行求值【延迟展开】
BIN := $(addprefix $(BUILD_ROOT)/, $(BIN))

LINK_OBJ_DIR = $(BUILD_ROOT)/app/link_obj
DEP_DIR = $(BUILD_ROOT)/app/dep

#创建临时目录，链接目录
$(shell mkdir -p $(LINK_OBJ_DIR))
$(shell mkdir -p $(DEP_DIR))

#增加绝对路径
OBJS := $(addprefix $(LINK_OBJ_DIR)/, $(OBJS))
DEPS := $(addprefix $(DEP_DIR)/, $(DEPS))

LINK_OBJ = $(wildcard $(LINK_OBJ_DIR)/*.o)
LINK_OBJ += $(OBJS)

all:$(DEPS) $(OBJS) $(BIN)

ifneq ("$(wildcard $(DESP))", "")
include $(DEPS)
endif

#一些变量：$@：目标，     $^：所有目标依赖
$(BIN):$(LINK_OBJ)
	@echo "-----------------------build $(VERSION) mode------------------"
	$(CC) -o $@ $^

$(LINK_OBJ_DIR)/%.o:%.cpp
	$(CC) -I$(INCLUDE_PATH) -o $@ -c $(filter %.cpp,$^)


#修改.h重新编译
$(DEP_DIR)/%.d:%.cpp
	echo -n $(LINK_OBJ_DIR)/ > $@
	gcc -I$(INCLUDE_PATH) -MM $^ >> $@
