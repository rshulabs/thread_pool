# 定义SRC_LIST 包含所有目录源文件
aux_source_directory(. SRC_LIST)

# 指定生成可执行文件
add_executable(ThreadPoolv1 ${SRC_LIST})