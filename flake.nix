{
  description = "whiteboard-opencv";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/4095b8232c4fb44aefce7aef8b3f106982e2b428";
    pre-commit-hooks.url = "github:cachix/pre-commit-hooks.nix";
  };

  outputs = inputs@{ flake-parts, pre-commit-hooks, ... }:
    flake-parts.lib.mkFlake { inherit inputs; } {
      systems = [ "x86_64-linux" "aarch64-darwin" ];
      perSystem = { config, self', inputs', pkgs, system, ... }: {
        packages.default = pkgs.callPackage ./build.nix {
          opencv4 = pkgs.opencv4.override { enableGtk2 = true; };
        };
        devShells.default = pkgs.mkShell {
          inputsFrom = [ config.packages.default ];
          inherit (config.checks.pre-commit-check) shellHook;
        };
        checks = {
          pre-commit-check = pre-commit-hooks.lib.${system}.run {
            src = ./.;
            hooks = {
              clang-format.enable = true;
              nixpkgs-fmt.enable = true;
              statix.enable = true;
            };
          };
        };
      };
    };
}
