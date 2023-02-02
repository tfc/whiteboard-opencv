{ stdenv, opencv4, cmake }:

stdenv.mkDerivation {
  name = "whiteboard-opencv";
  src = ./.;
  VERBOSE=1;
  nativeBuildInputs = [ cmake ];
  buildInputs = [ opencv4 ];

  installPhase = ''
    cp DisplayImage $out
  '';
}
