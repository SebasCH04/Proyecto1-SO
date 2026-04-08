#!/bin/bash
# Corre el programa cp con distintas cantidades de hilos y registra el tiempo total
# Uso: ./benchmark.sh [directorio_fuente]
# Requiere que el binario C/cp ya esté compilado

SOURCE=${1:-"test_large"}
DEST_BASE="test_dest"
BINARY="./C/cp"
RESULTS_FILE="benchmark_results.csv"
THREADS=(1 2 4 8 16)

if [ ! -f "$BINARY" ]; then
    echo "Error: no se encontró el binario $BINARY"
    echo "Compilá primero con: gcc -o C/cp C/main.c C/fs_mgmt.c C/task_queue.c C/thread_pool.c C/logging.c -lpthread"
    exit 1
fi

if [ ! -d "$SOURCE" ]; then
    echo "Error: directorio fuente '$SOURCE' no existe"
    echo "Generalo con: ./gen_test_dir.sh $SOURCE"
    exit 1
fi

echo "hilos,tiempo_segundos" > "$RESULTS_FILE"
echo "=== Benchmark de rendimiento ==="
echo "Fuente: $SOURCE"
echo ""

for T in "${THREADS[@]}"; do
    DEST="${DEST_BASE}_${T}hilos"
    rm -rf "$DEST"
    mkdir -p "$DEST"

    echo -n "Hilos: $T ... "

    # Captura solo la línea de tiempo total, suprime el resto
    TIME_LINE=$("$BINARY" "$SOURCE" "$DEST" "$T" 2>/dev/null | grep "Tiempo total")
    SECONDS_VAL=$(echo "$TIME_LINE" | grep -oP '[0-9]+\.[0-9]+')

    echo "$TIME_LINE"
    echo "$T,$SECONDS_VAL" >> "$RESULTS_FILE"

    rm -rf "$DEST"
done

echo ""
echo "Resultados guardados en: $RESULTS_FILE"
echo ""
echo "=== Tabla de resultados ==="
echo "Hilos | Tiempo (s)"
echo "------|------------"
tail -n +2 "$RESULTS_FILE" | while IFS=, read -r hilos tiempo; do
    printf "  %-4s| %s\n" "$hilos" "$tiempo"
done
