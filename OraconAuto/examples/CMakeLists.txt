# OraconAuto Examples

add_executable(basic_agent basic_agent.cpp)
target_link_libraries(basic_agent OraconAuto)

add_executable(claude_demo claude_demo.cpp)
target_link_libraries(claude_demo OraconAuto)

add_executable(coding_assistant coding_assistant.cpp)
target_link_libraries(coding_assistant OraconAuto)
