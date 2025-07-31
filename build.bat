@SET "PATH=%PATH%;E:\Qt\5.15.0\msvc2019_64\bin"
@call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x64
cmake Dependencies/obs-studio/ -B Dependencies/obs-studio/build -DCMAKE_BUILD_TYPE=RelWithDebInfo -DENABLE_FRONTEND:BOOL=OFF -DENABLE_UI:BOOL=OFF -DENABLE_SCRIPTING:BOOL=OFF -DENABLE_HEVC:BOOL=OFF -DENABLE_BROWSER:BOOL=OFF -DENABLE_AJA:BOOL=OFF -DENABLE_VST:BOOL=OFF -DENABLE_VLC:BOOL=OFF -DENABLE_VIRTUALCAM:BOOL=OFF -DENABLE_DECKLINK:BOOL=OFF -DENABLE_NVAFX:BOOL=OFF -DENABLE_NVVFX:BOOL=OFF -DENABLE_NVENC:BOOL=OFF -DENABLE_QSV11:BOOL=OFF -DENABLE_WEBRTC:BOOL=OFF -DENABLE_WEBSOCKET:BOOL=OFF -DENABLE_SERVICE_UPDATES:BOOL=OFF -DENABLE_FREETYPE:BOOL=OFF
cmake --build Dependencies/obs-studio/build --config RelWithDebInfo
cmake --install Dependencies/obs-studio/build --config RelWithDebInfo --prefix Dependencies/libobs/
qmake OpenRGBEffectsPlugin.pro CONFIG-=debug_and_release CONFIG+=release CONFIG+=force_debug_info
jom
