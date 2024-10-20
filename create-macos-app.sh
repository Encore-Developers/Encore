#!/bin/bash
# Encore.app building+packing script, made by Emma/InvoxiPlayGames 2024
# Requires you to have run the build in build_macos

# Remove all existing artifacts
rm -rf out
mkdir out

# Copy the app template to the .app folder
cp -r Encore.app_template out/Encore.app

# Copy the game assets into the Resources folder
cp -r build_macos/Encore/Assets out/Encore.app/Contents/Resources

# Copy out the game executable and any used dynamic libraries
cp build_macos/Encore/Encore out/Encore.app/Contents/MacOS/Encore
cp build_macos/Encore/*.dylib out/Encore.app/Contents/MacOS

# Fix executable so dynamic libraries work from the executable path
install_name_tool -add_rpath @executable_path out/Encore.app/Contents/MacOS/Encore

# Delete the gitkeep file if it exists
rm out/Encore.app/Contents/MacOS/.gitkeep

# Copy out the Songs folder
cp -r build_macos/Encore/Songs out/Songs

# -- !! TEMPORARY INDEV HACK !! --
cp build_macos/Encore/players.json out/players.json
# -- !! TEMPORARY INDEV HACK !! --

echo "Copy the Encore app and the Songs folder to a folder on your computer." > out/Copy_to_your_computer!.txt
