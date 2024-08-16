#! /bin/zsh

export RACK_DIR=/Volumes/Daten/DEV/Rack-SDK

cd /Volumes/Daten/DEV/projects/VCV/PW
make

rm -r ~/Library/Application\ Support/Rack2/plugins-mac-arm64/PW
cp -R /Volumes/Daten/DEV/projects/VCV/PW ~/Library/Application\ Support/Rack2/plugins-mac-arm64/PW

open -F /Applications/VCV\ Rack\ 2\ Free.app
#open /Users/paulwollstadt/Library/Application\ Support/Rack2/patches/PW_Build_Test.vcv -e /Applications/VCV\ Rack\ 2\ Free.app
