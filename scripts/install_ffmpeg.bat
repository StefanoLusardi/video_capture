:: Download FFMPEG libraries
echo "Install FFMPEG libraries"
mkdir C:\ffmpeg\
powershell -executionpolicy bypass -Command "& {$cli=New-Object System.Net.WebClient; $cli.DownloadFile('https://www.gyan.dev/ffmpeg/builds/ffmpeg-release-full-shared.7z','C:\ffmpeg\ffmpeg.7z')}"

:: Unzip binaries and shared libraries
set PATH=%PATH%;C:\Program Files\7-Zip\
C:
cd C:\ffmpeg
7z x C:\ffmpeg\ffmpeg.7z -y

:: Rename bin directory
move C:\ffmpeg\ffmpeg-* C:\ffmpeg\ffmpeg
del C:\ffmpeg\ffmpeg.7z

:: Set FFMPEG_DIR environment variable
setx FFMPEG_DIR C:\ffmpeg\ffmpeg

:: Check installation
:: %FFMPEG_DIR%\bin\ffmpeg -version

echo "FFMPEG libraries installed"
