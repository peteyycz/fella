{
  description = "Fella - Clay UI application with Raylib renderer";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
      in
      {
        packages.default = pkgs.stdenv.mkDerivation {
          pname = "fella";
          version = "0.1.0";
          src = ./.;

          nativeBuildInputs = with pkgs; [
            pkg-config
          ];

          buildInputs = with pkgs; [
            raylib
          ] ++ pkgs.lib.optionals pkgs.stdenv.hostPlatform.isLinux [
            libx11
            libxrandr
            libxinerama
            libxcursor
            libxi
            libGL
            wayland
            wayland-protocols
            libxkbcommon
          ] ++ pkgs.lib.optionals pkgs.stdenv.hostPlatform.isDarwin [
            darwin.apple_sdk.frameworks.Cocoa
            darwin.apple_sdk.frameworks.IOKit
            darwin.apple_sdk.frameworks.CoreVideo
          ];

          buildPhase = ''
            cc -O2 -o fella main.c \
              $(pkg-config --cflags --libs raylib) \
              -lm -lpthread -ldl
          '';

          installPhase = ''
            mkdir -p $out/bin $out/share/fella
            cp fella $out/bin/
            cp -r resources $out/share/fella/
          '';
        };

        devShells.default = pkgs.mkShell {
          nativeBuildInputs = with pkgs; [
            pkg-config
          ];

          buildInputs = with pkgs; [
            # C toolchain
            gcc
            gnumake
            gdb
            valgrind
            clang-tools

            # Clay dependency
            raylib
          ] ++ pkgs.lib.optionals pkgs.stdenv.hostPlatform.isLinux [
            libx11
            libxrandr
            libxinerama
            libxcursor
            libxi
            libGL
            wayland
            wayland-protocols
            libxkbcommon
          ] ++ pkgs.lib.optionals pkgs.stdenv.hostPlatform.isDarwin [
            darwin.apple_sdk.frameworks.Cocoa
            darwin.apple_sdk.frameworks.IOKit
            darwin.apple_sdk.frameworks.CoreVideo
          ];

          # On non-NixOS, the host system's GPU drivers aren't visible to Nix-built programs.
          # Add /usr/lib64 so GLFW/EGL can find the actual GPU driver implementation.
          LD_LIBRARY_PATH = "/usr/lib64";

          shellHook = ''
            echo "fella dev shell"
            echo "  build:  make"
            echo "  run:    make run"
            echo "  clean:  make clean"
          '';
        };
      }
    );
}
