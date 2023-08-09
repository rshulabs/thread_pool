# 定义SRC_LIST 包含所有目录源文件
aux_source_directory(. MAIN)

# 指定生成可执行文件
add_executable(ThreadPool ${MAIN})