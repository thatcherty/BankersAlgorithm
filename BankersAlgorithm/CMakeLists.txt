cmake_minimum_required(VERSION 3.16)
project(BankersAlgorithm LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# --- FTXUI is bundled in the project folder as ./FTXUI ---
add_subdirectory(FTXUI)


add_executable(BankersAlgorithm
  Source.cpp
  state.cpp
  InputHandler.cpp
  "node.h" "node.cpp" "BankersFTXApp.h")


target_link_libraries(BankersAlgorithm PRIVATE
  ftxui::screen
  ftxui::dom
  ftxui::component
)

# Optional: warnings (safe)
if (MSVC)
  target_compile_options(BankersAlgorithm PRIVATE /W4)
else()
  target_compile_options(BankersAlgorithm PRIVATE -Wall -Wextra -Wpedantic)
endif()