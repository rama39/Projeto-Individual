
// Inicializa mecanismo de leitura (CSV ou Giroscópio)
void INPUT_init();

// Lê valores de odometria (CSV ou giroscópio)
double Ler_Giroscopio();

// Retorna média aritmetica dos HIS_TAM (16) últimos valores recebidos
// Valor inicial da media é 0
double Moving_Avarage(double raw);

// Calcula medida do viés constante do giroscópio, supondo que está
// parado (w=0) e na posição de angulo = 0
// Teste demora um segundo
void Calc_Vies_Constante();

#include "INPUT.cpp"
