#!/bin/bash
# Genera un directorio de prueba con archivos de distintos tamaños y subdirectorios
# Uso: ./gen_test_dir.sh [directorio_destino]

DIR=${1:-"test_large"}

echo "Generando directorio de prueba en: $DIR"
rm -rf "$DIR"
mkdir -p "$DIR/subA/subA1"
mkdir -p "$DIR/subA/subA2"
mkdir -p "$DIR/subB/subB1"
mkdir -p "$DIR/subC"

# Genera N archivos de tamaño aleatorio en un directorio dado
gen_files() {
    local dest=$1
    local count=$2
    local size_kb=$3
    for i in $(seq 1 $count); do
        dd if=/dev/urandom of="$dest/file_${size_kb}k_$i.bin" bs=1024 count=$size_kb 2>/dev/null
    done
}

# raiz: 20 archivos de 512 KB
gen_files "$DIR" 20 512

# subA: 30 archivos de 256 KB
gen_files "$DIR/subA" 30 256

# subA/subA1: 25 archivos de 128 KB
gen_files "$DIR/subA/subA1" 25 128

# subA/subA2: 25 archivos de 64 KB
gen_files "$DIR/subA/subA2" 25 64

# subB: 20 archivos de 1 MB
gen_files "$DIR/subB" 20 1024

# subB/subB1: 30 archivos de 512 KB
gen_files "$DIR/subB/subB1" 30 512

# subC: 50 archivos de 256 KB
gen_files "$DIR/subC" 50 256

TOTAL=$(find "$DIR" -type f | wc -l)
SIZE=$(du -sh "$DIR" | cut -f1)
echo "Directorio generado: $TOTAL archivos, $SIZE en disco"
