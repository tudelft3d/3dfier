{
  description = "3dfier development shell and package";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-25.05";

  outputs =
    { self, nixpkgs }:
    let
      systems = [
        "x86_64-linux"
        "aarch64-linux"
        "x86_64-darwin"
        "aarch64-darwin"
      ];

      forAllSystems = nixpkgs.lib.genAttrs systems;

      pkgsFor = system: import nixpkgs { inherit system; };
    in
    {
      packages = forAllSystems (
        system:
        let
          pkgs = pkgsFor system;
        in
        {
          default = pkgs.stdenv.mkDerivation {
            pname = "3dfier";
            version = "1.5.0";

            src = self;

            nativeBuildInputs = with pkgs; [
              cmake
              pkg-config
            ];

            buildInputs = with pkgs; [
              boost
              cgal
              gdal
              gmp
              mpfr
              pdal
              yaml-cpp
            ];

            cmakeFlags = [
              "-DCMAKE_BUILD_TYPE=Release"
            ];

            meta = {
              description = "Open-source tool for creating 3D models from 2D GIS datasets";
              homepage = "https://github.com/tudelft3d/3dfier";
              license = pkgs.lib.licenses.gpl3Only;
              mainProgram = "3dfier";
              platforms = pkgs.lib.platforms.unix;
            };
          };
        }
      );

      devShells = forAllSystems (
        system:
        let
          pkgs = pkgsFor system;
        in
        {
          default = pkgs.mkShell {
            inputsFrom = [
              self.packages.${system}.default
            ];

            packages = with pkgs; [
              gdb
              ninja
            ];

            shellHook = ''
              echo "3dfier dev shell"
              echo "Configure: cmake -S . -B build -DCMAKE_BUILD_TYPE=Release"
              echo "Build:     cmake --build build"
            '';
          };
        }
      );
    };
}
