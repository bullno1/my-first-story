# --- cute_framework ---
set(CF_FRAMEWORK_STATIC $<NOT:$<BOOL:${RELOADABLE}>> CACHE BOOL "")
set(CF_FRAMEWORK_BUILD_SAMPLES OFF CACHE BOOL "")
set(CF_FRAMEWORK_BUILD_TESTS OFF CACHE BOOL "")

add_subdirectory("cute_framework" EXCLUDE_FROM_ALL)

# Make the include path system so that targets linking to cute framework can
# use a stricter warning settings.
target_include_directories(cute SYSTEM PUBLIC BEFORE "cute_framework/include")
target_include_directories(cute SYSTEM PUBLIC BEFORE "cute_framework/libraries")
target_include_directories(cute SYSTEM PUBLIC BEFORE "cute_framework/libraries/cimgui")
target_include_directories(cute SYSTEM PUBLIC BEFORE "cute_framework/libraries/cimgui/imgui")

# --- remodule --
add_library(remodule INTERFACE)
target_include_directories(remodule INTERFACE remodule)

# --- blibs --
add_library(blibs INTERFACE)
target_include_directories(blibs INTERFACE blibs)

# --- clay ---
add_library(clay INTERFACE)
target_include_directories(clay SYSTEM INTERFACE clay)

# --- pico_headers ---
add_library(pico_headers INTERFACE)
target_include_directories(pico_headers SYSTEM INTERFACE pico_headers)
