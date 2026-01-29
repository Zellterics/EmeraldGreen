#!/bin/bash
set -e

GLSLC=glslc
SHADERS_DIR=.

VERT="$SHADERS_DIR/basic.vert"
FRAG="$SHADERS_DIR/basic.frag"
VERT_OUT="$SHADERS_DIR/basicVert.spv"
FRAG_OUT="$SHADERS_DIR/basicFrag.spv"

echo "Compilando vertex shader..."
$GLSLC "$VERT" -o "$VERT_OUT"

echo "Compilando fragment shader..."
$GLSLC "$FRAG" -o "$FRAG_OUT"

echo
echo "✅ Compilación exitosa."
