
// encontrar forma correta de contar milisegundos (talvez com o clock da placa)

// Milliseconds per clock of runtime
#define MILS_PER_CLOCK (1000/CLOCKS_PER_SEC)

// Intervalo entre lidas em milisegundos
#define QUANTUM 5

struct Gir_Lida;

// Inicializa mecanismo de leitura (CSV ou Giroscópio)
void INPUT_init();

// Lê valores de odometria do giroscópio
Gir_Lida Ler_Giroscopio();

#include "INPUT.cpp"
