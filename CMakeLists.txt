cmake_minimum_required(VERSION 3.16)

project(markdownbookmarks)
set(CMAKE_PROJECT_VERSION "3.0.5")

set(QT_MAJOR_VERSION 6)
set(QT_MIN_VERSION 6.7.0)
set(KF_MIN_VERSION 5.105.0)
set(KDE_COMPILERSETTINGS_LEVEL 5.82.0)

find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDEClangFormat)
include(KDEGitCommitHooks)
include(FeatureSummary)
include(ECMDeprecationSettings)

# Find the required Libaries
find_package(Qt6 ${QT_MIN_VERSION} REQUIRED CONFIG COMPONENTS Widgets Core DBus)
find_package(KF6 ${KF_MIN_VERSION} REQUIRED COMPONENTS Runner KCMUtils KIO)

ecm_set_disabled_deprecation_versions(
	KF ${KF_MIN_VERSION}
	QT ${QT_MIN_VERSION}
)
add_subdirectory(src)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

# CPack build
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "KRunner plugin for markdown bookmarks")
set(CPACK_PACKAGE_NAME ${CMAKE_PROJECT_NAME})
set(CPACK_PACKAGE_VERSION "${CMAKE_PROJECT_VERSION}")
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Andrew Rabert")
set(CPACK_RPM_PACKAGE_ARCHITECTURE "x86_64")
set(CPACK_RPM_PACKAGE_LICENSE "LGPL 3")

# Then names of dependencies and paths might differ
set(KRUNNER_VERSION "${KF6Runner_VERSION}")
set(CPACK_PACKAGE_FILE_NAME ${CMAKE_PROJECT_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_OS}-plasma6)
if (CPACK_OS STREQUAL "ubuntu")
	set(CPACK_DEBIAN_PACKAGE_DEPENDS "libkf6runner6 (>=${KRUNNER_VERSION})")
	set(CPACK_GENERATOR "DEB")
elseif(CPACK_OS STREQUAL "fedora")
	set(CPACK_RPM_PACKAGE_REQUIRES "kf6-krunner >= ${KRUNNER_VERSION}")
	set(CPACK_GENERATOR "RPM")
elseif(CPACK_OS STREQUAL "opensuse")
	set(CPACK_RPM_PACKAGE_REQUIRES "kf6-krunner >= ${KRUNNER_VERSION}")
	set(CPACK_GENERATOR "RPM")
else()
  unset(CPACK_PACKAGE_FILE_NAME)
endif()

# These scripts will kill krunner and when it is invoked next time the plugins will be loaded
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_SOURCE_DIR}/postinst;${CMAKE_CURRENT_SOURCE_DIR}/postrm")
set(CPACK_RPM_POST_INSTALL_SCRIPT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/postinst")
set(CPACK_RPM_POST_UNINSTALL_SCRIPT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/postrm")

INCLUDE(CPack)

# add clang-format target for all our real source files
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})
kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
