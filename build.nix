{ stdenv, opencv4, cmake }:

stdenv.mkDerivation {
  name = "whiteboard-opencv";
  src = ./.;
  nativeBuildInputs = [ cmake ];
  buildInputs = [ opencv4 ];

  installPhase = ''
    cp DisplayImage $out
  '';
}
