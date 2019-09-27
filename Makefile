# 编译参数
#CC := gcc
#CXX := g++
INCLUDE := -I. \
	-I/usr/include
LIBS := -L/usr/lib \
	-L/usr/lib64
LDFLAGS := 
DEFINES := 
CFLAGS := -g -Wall -O2 $(INCLUDE) $(LIBS) $(LDFLAGS) $(DEFINES)
CXXFLAGS := $(CFLAGS) -std=c++11 -DHAVE_CONFIG_H
# make工具, Makefile指定
#MAKE = make
#MAKEFILE = Makefile

# default target
default: all

# 自定义文件
TARGET_1 := ttt
SRCS_1 := c_shell.cpp
OBJS_1 := $(patsubst %.cpp, %.o, $(SRCS_1))
sinclude $(OBJS_1:.o=.d)
# 具体编译过程
$(TARGET_1): $(OBJS_1)
	$(CXX) -o$(TARGET_1) $(OBJS_1)
# 所有目标合集
PROGRAM := $(TARGET_1)

# 以下大概不需要改
.PHONY: all
all:
	$(MAKE) $(PROGRAM)
.PHONY: clean
clean:
	rm -f *.orig *~ *.o *.d $(PROGRAM)

%.d: %.cpp
	@set -e
	@rm -f $@
	@$(CXX) -MM $< | sed 's:^\(.*\):$@ \1:g' > $@

# 以下是生成.d文件的4种方法.
# 形如%.d %.o: %.c something.h...
# 生成.d的原因是.h里面增加或减少包含其他.h文件, .d也能同步更新.
#@$(CC) -MM $< | awk '{print "$@ " $$0}' > $@
#@$(CC) -MM $< | awk '{printf "%s %s\n", "$@", $$0}' > $@
#@$(CXX) -MM $< | sed 's:^\(.*\):$@ \1:g' > $@
#@$(CC) -MM $(CPPFLAGS) $< > $@.$$$$; \
#	sed 's,\($*\)\.o[ :]*,\1.o $@: ,g' < $@.$$$$ > $@; \
#	rm -f $@.$$$$
