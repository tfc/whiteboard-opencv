{
  description = "whiteboard-opencv";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs";
  };

  outputs = inputs@{ flake-parts, ... }:
    flake-parts.lib.mkFlake { inherit inputs; } {
      systems = [ "x86_64-linux" "aarch64-darwin" ];
      perSystem = { config, self', inputs', pkgs, system, ... }: {
        packages.default = pkgs.callPackage ./build.nix {
          opencv4 = pkgs.opencv4.override { enableGtk2 = true; };
        };
        devShells.default = pkgs.mkShell {
          inputsFrom = [ config.packages.default ];
        };
      };
    };
}
