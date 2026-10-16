{ pkgs ? import <nixpkgs> { }, }:
(pkgs.buildFHSEnv {
  name = "encore-build-env";
  targetPkgs =
    pkgs: with pkgs; [
      gdb
      cmake
      ninja
      pkg-config
      wayland-scanner
      gcc
      fish
      gmp
      icu
      ffmpeg
      dbus
      udev
      alsa-lib
      pipewire
      sdl3
      wayland
      wayland-protocols
      libdecor
      libxkbcommon
      mesa
      libGL
      libx11
      libxext
      libxrandr
      libxcursor
      libxfixes
      libxi
      libxscrnsaver
      libxrender
    ];
  extraOutputsToInstall = [ "dev" "lib" ];

  profile = ''
    export PKG_CONFIG_PATH="/usr/lib/pkgconfig:/usr/share/pkgconfig:$PKG_CONFIG_PATH"
  '';

  runScript = "fish";
}).env
