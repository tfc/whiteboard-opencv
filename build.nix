{ stdenv, opencv4, cmake, pkg-config, xorg, boost, gtk2, ffmpeg   }:

stdenv.mkDerivation {
  name = "whiteboard-opencv";
  src = ./.;
  VERBOSE=1;
  nativeBuildInputs = [ cmake pkg-config ];
  buildInputs = [ opencv4 ];

  installPhase = ''
    cp DisplayImage $out
  '';
}
