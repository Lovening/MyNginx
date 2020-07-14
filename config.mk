
#项目编译的根目录
#BUILD_ROOT = /mnt/hgfs/linux/nginx
export BUILD_ROOT = $(shell pwd)

#头文件的路径变量
export INCLUDE_PATH = $(BUILD_ROOT)/_include/

#我们要编译的目录
BUILD_DIR = $(BUILD_ROOT)/signal/ \
			$(BUILD_ROOT)/proc/  \
			$(BUILD_ROOT)/net/  \
			$(BUILD_ROOT)/app/ 

#编译时是否生成调试信息
export DEBUG = true

