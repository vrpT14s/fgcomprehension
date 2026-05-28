{
  description = "imgui dev shell";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  };

  outputs = { self, nixpkgs }:
    let
      system = "x86_64-linux";

      pkgs = import nixpkgs {
        inherit system;
      };
      llvmPkgs = pkgs.llvmPackages_20;
    in {
      devShells.${system}.default = pkgs.mkShell {
        packages = [
	  #llvm.llvm
          #llvm.clang

          ##cmake
          #gdb

          #pkg-config

          #sdl3

          #libGL
          #mesa

          #xorg.libX11
          #xorg.libXcursor
          #xorg.libXi
          #xorg.libXext
          #xorg.libXrandr
          #xorg.libXfixes
          #xorg.libXinerama
          #xorg.libXxf86vm
          llvmPkgs.clang
          llvmPkgs.llvm

          pkgs.gdb
          pkgs.pkg-config

          pkgs.sdl3

          pkgs.libGL
          pkgs.mesa

          pkgs.xorg.libX11
          pkgs.xorg.libXcursor
          pkgs.xorg.libXi
          pkgs.xorg.libXext
          pkgs.xorg.libXrandr
          pkgs.xorg.libXfixes
          pkgs.xorg.libXinerama
          pkgs.xorg.libXxf86vm

          pkgs.perf
        ];

        shellHook = ''
          echo "SDL3/OpenGL installed"
        '';
      };
    };
}
