# Required - cmake complains about 3.1 being deprecated, so bump to 3.10
cmake_minimum_required(VERSION 3.10)
# Yay! :D
set(PROJECT_NAME "bluray_info")

# Display what compiler is doing
set(CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL "ON")

# Don't check for c++ compiler
project(bluray_info LANGUAGES C)

# Look for include files and libraries outside of baseline /usr, but respecting install prefix first
# This is needed on FreeBSD where non-system packages are in /usr/local
include_directories(AFTER "${CMAKE_INSTALL_PREFIX}/include")
link_directories(AFTER "${CMAKE_INSTALL_PREFIX}/lib")

# Get fancy variables
include(GNUInstallDirs)

# Use pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(BLURAY REQUIRED libbluray>=1.2.0)
# libmpv doesn't have a specific version needed
pkg_check_modules(MPV mpv)

# Build bluray_info, requires math library as well (-lm)
add_executable(bluray_info bluray_info.c bluray_open.c bluray_chapter.c bluray_video.c bluray_audio.c bluray_pgs.c bluray_time.c)
target_link_libraries(bluray_info PRIVATE bluray)
target_link_libraries(bluray_info PRIVATE m)
INSTALL(FILES bluray_info PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE DESTINATION bin)
INSTALL(FILES bluray_info.1 DESTINATION "${CMAKE_INSTALL_MANDIR}/man1")

# Build bluray_copy
add_executable(bluray_copy bluray_copy.c bluray_open.c bluray_time.c)
target_link_libraries(bluray_copy PRIVATE bluray)
target_link_libraries(bluray_copy PRIVATE m)
INSTALL(FILES bluray_copy PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE DESTINATION bin)
INSTALL(FILES bluray_copy.1 DESTINATION "${CMAKE_INSTALL_MANDIR}/man1")

# Build bluray_backup
add_executable(bluray_backup bluray_backup.c bluray_open.c bluray_time.c)
target_link_libraries(bluray_backup PRIVATE bluray)
target_link_libraries(bluray_backup PRIVATE m)
INSTALL(FILES bluray_backup PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE DESTINATION bin)
# bluray_backup doesn't have a man page
# INSTALL(FILES bluray_backup.1 DESTINATION "${CMAKE_INSTALL_MANDIR}/man1")

# Build bluray_id, which is a standalone app from other bluray_info code
add_executable(bluray_id bluray_id.c)
target_link_libraries(bluray_id PRIVATE bluray)
INSTALL(FILES bluray_id PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE DESTINATION bin)
# bluray_id doesn't have a man page
# INSTALL(FILES bluray_id.1 DESTINATION "${CMAKE_INSTALL_MANDIR}/man1")

# bluray_rip is a shell script, so just copy it over
INSTALL(FILES bluray_rip PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE DESTINATION bin)
INSTALL(FILES bluray_rip.1 DESTINATION "${CMAKE_INSTALL_MANDIR}/man1")

# Documentation
INSTALL(FILES README LICENSE PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ DESTINATION share/doc/${PROJECT_NAME})

# If libmpv was found by pkgconfig, then build bluray_player
if(MPV_FOUND)
	add_executable(bluray_player bluray_player.c bluray_open.c bluray_video.c bluray_audio.c bluray_pgs.c bluray_time.c)
	target_link_libraries(bluray_player PRIVATE bluray)
	target_link_libraries(bluray_player PRIVATE m)
	target_link_libraries(bluray_player PRIVATE mpv)
	INSTALL(FILES bluray_player PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE DESTINATION bin)
	INSTALL(FILES bluray_player.1 DESTINATION man/man1)
endif()

# cmake doesn't have a native uninstall option
add_custom_target("uninstall")
add_custom_command(
	TARGET "uninstall"
	POST_BUILD
	COMMAND xargs < install_manifest.txt rm -v
	COMMAND rmdir -v ${CMAKE_INSTALL_PREFIX}/share/doc/${PROJECT_NAME}
)

set(CPACK_PACKAGE_CONTACT "Steve Dibb <steve.dibb@gmail.com>")
set(CPACK_PACKAGE_DESCRIPTION "Blu-ray utilities")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Blu-ray utilities")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/beandog/bluray_info")

set(CPACK_DEBIAN_PACKAGE_NAME "blurayinfo")
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "x86_64")
set(CPACK_RPM_PACKAGE_DESCRIPTION "Blu-ray utilities")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libbluray2, libmpv2")
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
set(CPACK_DEBIAN_PACKAGE_SECTION "video")
