# TRLIB_ISR 模块的 CMakeLists.txt

# 收集源文件
file(GLOB_RECURSE ISR_SOURCES "*.cpp")
file(GLOB_RECURSE ISR_HEADERS "*.h")

# 创建库
add_library(trlib_isr STATIC ${ISR_SOURCES} ${ISR_HEADERS})

# 从库中排除main.cpp
list(FILTER ISR_SOURCES EXCLUDE REGEX "main.cpp$")

# 设置库的属性
set_target_properties(trlib_isr PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION ${PROJECT_VERSION_MAJOR}
)

# 链接依赖库
target_link_libraries(trlib_isr PUBLIC trlib_base trlib_driver)

# 创建可执行文件
add_executable(ISR main.cpp)
target_link_libraries(ISR PRIVATE trlib_isr trlib_driver trlib_base)

# 安装规则（可选）
install(TARGETS trlib_isr
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)
install(FILES ${ISR_HEADERS} DESTINATION include/trlib/isr)
install(TARGETS ISR DESTINATION bin)