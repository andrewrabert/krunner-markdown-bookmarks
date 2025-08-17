set(CMAKE_AUTOUIC ON)

kcoreaddons_add_plugin(markdownbookmarks SOURCES runner/markdownbookmarks.cpp core/Bookmark.cpp core/FileReader.cpp INSTALL_NAMESPACE "kf6/krunner")
target_link_libraries(markdownbookmarks
    KF6::Runner
    KF6::ConfigCore
    KF6::KIOWidgets
    Qt::Gui
    Qt::DBus
)

add_library(kcm_krunner_markdownbookmarks MODULE config/markdownbookmarks_config.ui)
target_sources(kcm_krunner_markdownbookmarks PRIVATE
    config/markdownbookmarks_config.cpp
    core/Bookmark.cpp
    core/FileReader.cpp
)
target_link_libraries(kcm_krunner_markdownbookmarks
    Qt::Core
    Qt::Gui
    KF6::CoreAddons
    KF6::KCMUtils
    KF6::ConfigCore
)
install(TARGETS kcm_krunner_markdownbookmarks DESTINATION ${KDE_INSTALL_QTPLUGINDIR}/)
