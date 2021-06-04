:: Download FFMPEG libraries
echo "Install FFMPEG libraries"

mkdir C:\ffmpeg\
powershell -executionpolicy bypass -Command "& {$cli=New-Object System.Net.WebClient; $cli.DownloadFile('https://www.gyan.dev/ffmpeg/builds/ffmpeg-release-full-shared.7z','C:\ffmpeg\ffmpeg.7z')}"

:: Unzip
set PATH=%PATH%;C:\Program Files\7-Zip\
cd C:\ffmpeg
7z x C:\ffmpeg\ffmpeg.7z -y

:: Rename
move ffmpeg-* ffmpeg
del ffmpeg.7z

:: Set PATH
setx FFMPEG_DIR C:\ffmpeg\ffmpeg
%FFMPEG_DIR%\bin\ffmpeg -version