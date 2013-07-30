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
    $$PWD/iconsets/light-gray/README \
    $$PWD/iconsets/light-gray/remove-poet.png \
    $$PWD/iconsets/light-gray/right.png \
    $$PWD/iconsets/light-gray/search.png \
    $$PWD/iconsets/light-gray/search-options.png \
    $$PWD/iconsets/light-gray/select-mask.png \
    $$PWD/iconsets/light-gray/settings.png \
    $$PWD/iconsets/light-gray/up.png


exists( $$PWD/fonts/*.ttf ) {
    fonts.path = $${RESOURCES_PATH}/fonts
    fonts.files = $$quote($$PWD/fonts/XB Sols.ttf) \
        $$PWD/fonts/license.txt
mac {
    QMAKE_BUNDLE_DATA += fonts
}
else {
    INSTALLS += fonts
}
}
