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

# Delete the gitkeep file if it exists
rm out/Encore.app/Contents/MacOS/.gitkeep

# Copy out the Songs folder
cp -r build_macos/Encore/Songs out/Songs
