@call ndk-build
@call ant -quiet debug

@echo copying to directory...
@copy bin\classes.jar ..\%1\Assets\Plugins\Android\vrlib.jar
@copy libs\armeabi-v7a\libOculusplugin.so ..\%1\Assets\Plugins\Android\.
@xcopy res\raw\. ..\%\Assets\Plugins\Android\res\raw\. /s /y
