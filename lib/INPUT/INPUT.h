
// encontrar forma correta de contar milisegundos (talvez com o clock da placa)

// Milliseconds per clock of runtime
#define MILS_PER_CLOCK (1000/CLOCKS_PER_SEC)

// Intervalo entre lidas em milisegundos
#define QUANTUM 5

struct Gir_Lida;

// Inicializa mecanismo de leitura (CSV ou Giroscópio)
void INPUT_init();

// Lê valores de odometria (CSV ou giroscópio)
Gir_Lida Ler_Giroscopio();

// Retorna média aritmetica dos HIS_TAM (16) ultimos valores recebidos
// Valor inicial da media é 0
Gir_Lida Moving_Avarage(Gir_Lida raw);

// Calcula medida do viés constante do giroscópio, supondo que está
// parado (w=0) e na posição de angulo = 0
// Teste demora um segundo
void Calc_Vies_Constante();

#include "INPUT.cpp"
