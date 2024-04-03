myArgs = -Wall -g -lpthread -std=c++20 #参数
.PHONY:clean ALL    #伪目标
#两个函数
src = $(wildcard *.cpp)  #找到当前目录下所有.cpp文件 赋值给src
obj = $(patsubst %.cpp,%.o,$(src))   #将参数3( $(src) )里所有包含参数1(%.cpp)的文件替换成参数3(%.o)
ALL:demo

demo:$(obj)
	g++ $(myArgs) $^ -o $@
$(obj): %.o: %.cpp
	g++ -c $< -o $@ $(myArgs)
	
clean:  #使用clean :先使用make clean -n 模拟删除(提示删除内容) 再make clean
	-rm -rf $(obj) demo   
