#DEPENDPATH += . build
#INCLUDEPATH += .

RESOURCES += $$PWD/saaghar.qrc

win32 {
    RC_FILE = $$PWD/win.rc
    utilities.files += $$PWD/saaghar.ico
}

mac {
    ICON = $$PWD/saaghar.icns
    QMAKE_INFO_PLIST = $$PWD/Info.plist
}

unix:!macx {
    desktop.files += $$PWD/Saaghar.desktop
    icon.files = $$PWD/images/saaghar.png
}

poetsImage.path = $${RESOURCES_PATH}/poets_images
poetsImage.files = \
    $$PWD/poets_images/10.png \
    $$PWD/poets_images/11.png \
    $$PWD/poets_images/12.png \
    $$PWD/poets_images/13.png \
    $$PWD/poets_images/14.png \
    $$PWD/poets_images/15.png \
    $$PWD/poets_images/16.png \
    $$PWD/poets_images/17.png \
    $$PWD/poets_images/18.png \
    $$PWD/poets_images/19.png \
    $$PWD/poets_images/2.png \
    $$PWD/poets_images/20.png \
    $$PWD/poets_images/21.png \
    $$PWD/poets_images/22.png \
    $$PWD/poets_images/23.png \
    $$PWD/poets_images/24.png \
    $$PWD/poets_images/25.png \
    $$PWD/poets_images/26.png \
    $$PWD/poets_images/27.png \
    $$PWD/poets_images/28.png \
    $$PWD/poets_images/29.png \
    $$PWD/poets_images/3.png \
    $$PWD/poets_images/30.png \
    $$PWD/poets_images/31.png \
    $$PWD/poets_images/32.png \
    $$PWD/poets_images/33.png \
    $$PWD/poets_images/34.png \
    $$PWD/poets_images/35.png \
    $$PWD/poets_images/37.png \
    $$PWD/poets_images/38.png \
    $$PWD/poets_images/39.png \
    $$PWD/poets_images/4.png \
    $$PWD/poets_images/40.png \
    $$PWD/poets_images/41.png \
    $$PWD/poets_images/42.png \
    $$PWD/poets_images/43.png \
    $$PWD/poets_images/44.png \
    $$PWD/poets_images/45.png \
    $$PWD/poets_images/46.png \
    $$PWD/poets_images/47.png \
    $$PWD/poets_images/5.png \
    $$PWD/poets_images/501.png \
    $$PWD/poets_images/502.png \
    $$PWD/poets_images/503.png \
    $$PWD/poets_images/504.png \
    $$PWD/poets_images/505.png \
    $$PWD/poets_images/6.png \
    $$PWD/poets_images/600.png \
    $$PWD/poets_images/602.png \
    $$PWD/poets_images/7.png \
    $$PWD/poets_images/8.png \
    $$PWD/poets_images/9.png \
    $$PWD/poets_images/48.png \
    $$PWD/poets_images/49.png \
    $$PWD/poets_images/50.png \
    $$PWD/poets_images/506.png \
    $$PWD/poets_images/51.png \
    $$PWD/poets_images/610.png

backgrounds.path = $${RESOURCES_PATH}/themes/backgrounds
backgrounds.files = \
    $$PWD/backgrounds/bgpatterns_1.png \
    $$PWD/backgrounds/bgpatterns_2.png \
    $$PWD/backgrounds/bgpatterns_3.png \
    $$PWD/backgrounds/bgpatterns_4.png \
    $$PWD/backgrounds/grav-rand_1.png \
    $$PWD/backgrounds/grav-rand_2.png \
    $$PWD/backgrounds/grav-rand_3.png \
    $$PWD/backgrounds/grav-rand_4.png \
    $$PWD/backgrounds/saaghar-pattern_1.png \
    $$PWD/backgrounds/woodw-blue_1.png \
    $$PWD/backgrounds/woodw-golden_1.png \
    $$PWD/backgrounds/woodw-golden_2.png \
    $$PWD/backgrounds/woodw-green_1.png \
    $$PWD/backgrounds/woodw_3d_1.png

lightGrayIcons.path = $${RESOURCES_PATH}/themes/iconsets/light-gray
lightGrayIcons.files = \
    $$PWD/iconsets/light-gray/browse_net.png \
    $$PWD/iconsets/light-gray/cancel.png \
    $$PWD/iconsets/light-gray/check-updates.png \
    $$PWD/iconsets/light-gray/clear-left.png \
    $$PWD/iconsets/light-gray/clear-right.png \
    $$PWD/iconsets/light-gray/close-tab.png \
    $$PWD/iconsets/light-gray/copy.png \
    $$PWD/iconsets/light-gray/down.png \
    $$PWD/iconsets/light-gray/exit.png \
    $$PWD/iconsets/light-gray/export.png \
    $$PWD/iconsets/light-gray/export-pdf.png \
    $$PWD/iconsets/light-gray/faal.png \
    $$PWD/iconsets/light-gray/filter.png \
    $$PWD/iconsets/light-gray/fullscreen.png \
    $$PWD/iconsets/light-gray/home.png \
    $$PWD/iconsets/light-gray/import-to-database.png \
    $$PWD/iconsets/light-gray/left.png \
    $$PWD/iconsets/light-gray/new_tab.png \
    $$PWD/iconsets/light-gray/new_window.png \
    $$PWD/iconsets/light-gray/next.png \
    $$PWD/iconsets/light-gray/no-fullscreen.png \
    $$PWD/iconsets/light-gray/ocr-verification.png \
    $$PWD/iconsets/light-gray/previous.png \
    $$PWD/iconsets/light-gray/print.png \
    $$PWD/iconsets/light-gray/print-preview.png \
    $$PWD/iconsets/light-gray/qt-logo.png \
    $$PWD/iconsets/light-gray/random.png \
    $$PWD/iconsets/light-gray/remove-poet.png \
    $$PWD/iconsets/light-gray/right.png \
    $$PWD/iconsets/light-gray/search.png \
    $$PWD/iconsets/light-gray/search-options.png \
    $$PWD/iconsets/light-gray/select-mask.png \
    $$PWD/iconsets/light-gray/settings.png \
    $$PWD/iconsets/light-gray/up.png \
    $$PWD/iconsets/light-gray/README

classicIcons.path = $${RESOURCES_PATH}/themes/iconsets/classic
classicIcons.files = \
    $$PWD/iconsets/classic/add-tab.png \
    $$PWD/iconsets/classic/album.png \
    $$PWD/iconsets/classic/arrow-down.png \
    $$PWD/iconsets/classic/bookmark-folder.png \
    $$PWD/iconsets/classic/bookmark-off.png \
    $$PWD/iconsets/classic/bookmark-on.png \
    $$PWD/iconsets/classic/bookmarks-import.png \
    $$PWD/iconsets/classic/browse_net.png \
    $$PWD/iconsets/classic/cancel.png \
    $$PWD/iconsets/classic/check-updates.png \
    $$PWD/iconsets/classic/clear-left.png \
    $$PWD/iconsets/classic/clear-right.png \
    $$PWD/iconsets/classic/close-tab.png \
    $$PWD/iconsets/classic/copy.png \
    $$PWD/iconsets/classic/down.png \
    $$PWD/iconsets/classic/exit.png \
    $$PWD/iconsets/classic/export-pdf.png \
    $$PWD/iconsets/classic/export.png \
    $$PWD/iconsets/classic/faal.png \
    $$PWD/iconsets/classic/filter.png \
    $$PWD/iconsets/classic/fullscreen.png \
    $$PWD/iconsets/classic/home.png \
    $$PWD/iconsets/classic/import-to-database.png \
    $$PWD/iconsets/classic/left.png \
    $$PWD/iconsets/classic/music-player.png \
    $$PWD/iconsets/classic/new_tab.png \
    $$PWD/iconsets/classic/new_window.png \
    $$PWD/iconsets/classic/next.png \
    $$PWD/iconsets/classic/no-fullscreen.png \
    $$PWD/iconsets/classic/no-photo.png \
    $$PWD/iconsets/classic/ocr-verification.png \
    $$PWD/iconsets/classic/one-hemistich-line.png \
    $$PWD/iconsets/classic/outline.png \
    $$PWD/iconsets/classic/previous.png \
    $$PWD/iconsets/classic/print-preview.png \
    $$PWD/iconsets/classic/print.png \
    $$PWD/iconsets/classic/qt-logo.png \
    $$PWD/iconsets/classic/random.png \
    $$PWD/iconsets/classic/redo.png \
    $$PWD/iconsets/classic/refresh.png \
    $$PWD/iconsets/classic/remove-poet.png \
    $$PWD/iconsets/classic/right.png \
    $$PWD/iconsets/classic/search-options.png \
    $$PWD/iconsets/classic/search.png \
    $$PWD/iconsets/classic/select-mask.png \
    $$PWD/iconsets/classic/settings.png \
    $$PWD/iconsets/classic/stepped-hemistich-line.png \
    $$PWD/iconsets/classic/two-hemistich-line.png \
    $$PWD/iconsets/classic/un-bookmark.png \
    $$PWD/iconsets/classic/undo.png \
    $$PWD/iconsets/classic/up.png \
    $$PWD/iconsets/classic/README

iconicCyanIcons.path = $${RESOURCES_PATH}/themes/iconsets/iconic-cyan
iconicCyanIcons.files = \
    $$PWD/iconsets/iconic-cyan/add-tab.png \
    $$PWD/iconsets/iconic-cyan/album.png \
    $$PWD/iconsets/iconic-cyan/arrow-down.png \
    $$PWD/iconsets/iconic-cyan/bookmark-folder.png \
    $$PWD/iconsets/iconic-cyan/bookmark-off.png \
    $$PWD/iconsets/iconic-cyan/bookmark-off1.png \
    $$PWD/iconsets/iconic-cyan/bookmark-on-0.png \
    $$PWD/iconsets/iconic-cyan/bookmark-on.png \
    $$PWD/iconsets/iconic-cyan/cancel.png \
    $$PWD/iconsets/iconic-cyan/check-updates.png \
    $$PWD/iconsets/iconic-cyan/clear-left.png \
    $$PWD/iconsets/iconic-cyan/clear-right.png \
    $$PWD/iconsets/iconic-cyan/close-tab.png \
    $$PWD/iconsets/iconic-cyan/copy.png \
    $$PWD/iconsets/iconic-cyan/down.png \
    $$PWD/iconsets/iconic-cyan/download-sets-repositories.png \
    $$PWD/iconsets/iconic-cyan/exit.png \
    $$PWD/iconsets/iconic-cyan/export-pdf.png \
    $$PWD/iconsets/iconic-cyan/faal.png \
    $$PWD/iconsets/iconic-cyan/fullscreen.png \
    $$PWD/iconsets/iconic-cyan/help-contents.png \
    $$PWD/iconsets/iconic-cyan/home.png \
    $$PWD/iconsets/iconic-cyan/left.png \
    $$PWD/iconsets/iconic-cyan/lock-toolbars.png \
    $$PWD/iconsets/iconic-cyan/music-player.png \
    $$PWD/iconsets/iconic-cyan/new_tab.png \
    $$PWD/iconsets/iconic-cyan/new_window.png \
    $$PWD/iconsets/iconic-cyan/next.png \
    $$PWD/iconsets/iconic-cyan/next1.png \
    $$PWD/iconsets/iconic-cyan/no-fullscreen.png \
    $$PWD/iconsets/iconic-cyan/no-photo.png \
    $$PWD/iconsets/iconic-cyan/ocr-verification.png \
    $$PWD/iconsets/iconic-cyan/one-hemistich-line.png \
    $$PWD/iconsets/iconic-cyan/outline.png \
    $$PWD/iconsets/iconic-cyan/previous.png \
    $$PWD/iconsets/iconic-cyan/previous1.png \
    $$PWD/iconsets/iconic-cyan/qt-logo.png \
    $$PWD/iconsets/iconic-cyan/random.png \
    $$PWD/iconsets/iconic-cyan/redo.png \
    $$PWD/iconsets/iconic-cyan/refresh.png \
    $$PWD/iconsets/iconic-cyan/right.png \
    $$PWD/iconsets/iconic-cyan/search-options.png \
    $$PWD/iconsets/iconic-cyan/search.png \
    $$PWD/iconsets/iconic-cyan/select-mask.png \
    $$PWD/iconsets/iconic-cyan/settings.png \
    $$PWD/iconsets/iconic-cyan/stepped-hemistich-line.png \
    $$PWD/iconsets/iconic-cyan/two-hemistich-line.png \
    $$PWD/iconsets/iconic-cyan/undo.png \
    $$PWD/iconsets/iconic-cyan/up.png \
    $$PWD/iconsets/iconic-cyan/README


exists( $$PWD/fonts/*.ttf ) {
    fonts.path = $${RESOURCES_PATH}/fonts
    fonts.files = $$files($$PWD/fonts/*)
mac {
    QMAKE_BUNDLE_DATA += fonts
}
else {
    INSTALLS += fonts
}
}
