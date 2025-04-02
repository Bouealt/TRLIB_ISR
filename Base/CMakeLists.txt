# Base 模块的 CMakeLists.txt

# 收集源文件
file(GLOB_RECURSE BASE_SOURCES "*.cpp")
file(GLOB_RECURSE BASE_HEADERS "*.h")

# 创建库
add_library(trlib_base STATIC ${BASE_SOURCES} ${BASE_HEADERS})

# 设置库的属性
set_target_properties(trlib_base PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION ${PROJECT_VERSION_MAJOR}
)

# 链接必要的库
target_link_libraries(trlib_base PUBLIC pthread)

# 安装规则（可选）
install(TARGETS trlib_base
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)
install(FILES ${BASE_HEADERS} DESTINATION include/trlib/base)