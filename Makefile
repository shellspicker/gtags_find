# 编译参数
#CC := gcc
#CXX := g++
INCLUDE := -I. \
	-I/usr/include
LIBS := -L/usr/lib \
	-L/usr/lib64
LDFLAGS :=
DEFINES :=
CFLAGS := -g -Wall -O2 $(DEFINES) $(INCLUDE) $(LIBS)
CXXFLAGS := $(CFLAGS) -DHAVE_CONFIG_H

TARGET: all

# 自定义文件
TARGET_1 := ttt
SRCS_1 := c_shell.cpp
OBJS_1 := $(patsubst %.cpp, %.o, $(SRCS_1))
# 具体编译过程
sinclude $(OBJS_1:.o=.d)
$(TARGET_1): $(OBJS_1)
	@$(CXX) -o$(TARGET_1) $(LDFLAGS) $(OBJS_1)
# 所有目标合集
PROGRAM := $(TARGET_1)

# 这个大概不需要改
.PHONY: all
all: $(PROGRAM)
	@echo "make target: $(PROGRAM)"
.PHONY: clean
clean:
	@rm -f *.orig *~ *.o *.d $(PROGRAM)

# 约定俗成的生成头文件依赖关系%.d
%.d: %.c %.cpp
	@set -e;
	@rm -f $@;
	@$(CC) -MM $(CPPFLAGS) $< > $@.$$$$; \
		sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
		rm -f $@.$$$$
