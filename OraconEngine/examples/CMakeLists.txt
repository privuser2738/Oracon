# OraconEngine Examples

add_executable(bouncing_ball bouncing_ball.cpp)
target_link_libraries(bouncing_ball OraconEngine)

add_executable(scripted_game scripted_game.cpp)
target_link_libraries(scripted_game OraconEngine)

add_executable(test_script_api test_script_api.cpp)
target_link_libraries(test_script_api OraconEngine)

add_executable(ai_npc_demo ai_npc_demo.cpp)
target_link_libraries(ai_npc_demo OraconEngine)

# Visual AI NPC demo (requires SDL2)
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(SDL2 sdl2)
    if(SDL2_FOUND)
        add_executable(visual_ai_npcs visual_ai_npcs.cpp)
        target_link_libraries(visual_ai_npcs OraconEngine ${SDL2_LIBRARIES})
        target_include_directories(visual_ai_npcs PRIVATE ${SDL2_INCLUDE_DIRS})
        target_compile_options(visual_ai_npcs PRIVATE ${SDL2_CFLAGS_OTHER})
        message(STATUS "Building visual_ai_npcs demo with SDL2")

        add_executable(village_of_oracon village_of_oracon.cpp)
        target_link_libraries(village_of_oracon OraconEngine ${SDL2_LIBRARIES})
        target_include_directories(village_of_oracon PRIVATE ${SDL2_INCLUDE_DIRS})
        target_compile_options(village_of_oracon PRIVATE ${SDL2_CFLAGS_OTHER})
        message(STATUS "Building village_of_oracon game with SDL2")
    endif()
endif()
