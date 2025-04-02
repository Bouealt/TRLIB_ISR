# Driver 模块的 CMakeLists.txt

# 收集源文件
file(GLOB_RECURSE DRIVER_SOURCES "*.cpp")
file(GLOB_RECURSE DRIVER_HEADERS "*.h")

# 创建库
add_library(trlib_driver STATIC ${DRIVER_SOURCES} ${DRIVER_HEADERS})

# 设置库的属性
set_target_properties(trlib_driver PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION ${PROJECT_VERSION_MAJOR}
)

# 链接Base库
target_link_libraries(trlib_driver PUBLIC trlib_base)

# 安装规则（可选）
install(TARGETS trlib_driver
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)
install(FILES ${DRIVER_HEADERS} DESTINATION include/trlib/driver)