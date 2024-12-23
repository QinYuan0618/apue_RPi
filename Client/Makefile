CC=gcc
AR=ar

a11: dynamic lib static_lib
	@make clear
	@make install
	
# 1.执行生成动态库
dynamic_lib:
	${CC} -shared -fPIc *.c -o lib${LIBNAME}.so

# 2.执行生成静态库
static_lib:
	${CC} -c *.c
	${AR} -rcs lib${LIBNAME}.a *.o

# 4.头文件和库复制到别的路径下（../install）
install:
	cp -rf lib${LIBNAME}.* ${INSTPATH}
	cp -rf *.h ${INSTPATH}
	
uninstall:
	rm -f ${INSTPATH}/lib${LIBNAME}.*
	rm -f ${INSTPATH}/*.h
	
# 3.删除.o目标文件
clear:
	rm -f *.o
	
clean: clear
	rm -f lib${LIBNAME}.*