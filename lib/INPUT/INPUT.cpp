// Funções, Tipos, Variáveis independentes do modo ===========================
#include <iostream>
struct Gir_Lida {
    double pos;
    double vel;
    
    void print() {
        printf("%lf, %lf\n", pos, vel);
    }
};

// Gir_Lida {grau, grau/s} vira {rad, rad/s}
#define ToRad(v) ((Gir_Lida){v.pos*0.01745329, v.vel*0.01745329})

Gir_Lida Moving_Avarage(Gir_Lida raw)
{
    // Tamanho do buffer de historico de lidas
    #define HIS_TAM 16

    static Gir_Lida MAvarage = {}, oldest;
    static Gir_Lida historia[HIS_TAM] = {};
    static int presente = 0;
    
    // achata valor lido para calculo da media
    raw.pos /= HIS_TAM,
    raw.vel /= HIS_TAM;

    // remove valor mais antigo lido da media
    oldest = historia[presente];
    MAvarage.pos -= oldest.pos, 
    MAvarage.vel -= oldest.vel;

    // contabiliza valor mais recente lido na media
    MAvarage.pos += raw.pos,
    MAvarage.vel += raw.vel;
    // e sobrescreve valor mais antigo na historia com o mais recente
    historia[presente] = raw;
    
    // incremento modular
    presente++; if(presente==HIS_TAM)presente=0;

    return MAvarage;
}

Gir_Lida VIES_C;

/* Escolhe entre a leitura de dados do arquivo csv ou o sensor MPU-6050 */

#if defined(CSV_MODE) // Funções, Tipos, Variáveis do modo CSV ===============

#include <time.h>

typedef Gir_Lida DATA_point; 

vector<DATA_point> _DATA;

// Lê valores do arquivo CSV para que possam ser utilizados
void INPUT_init()
{
    FILE *DATA_file = fopen("files/log.csv", "r");

    if(DATA_file == NULL)
        printf("ERRO NA ABERTURA DO ARQUIVO"), exit(0);

    while(getc(DATA_file) != '\n'); //pula primeira linha

    DATA_point buffer_data;
    while ( 
        fscanf (
            DATA_file, 
            "%lf,%lf\n", 
            &(buffer_data.pos), &(buffer_data.vel)
        ) != EOF
    ) 
        _DATA.push_back(buffer_data);
    
    Calc_Vies_Constante();

}

// Lê valor da tabela lida do CSV
Gir_Lida Ler_Giroscopio()
{
    static int t = 0;
    static Gir_Lida read;
    printf("% 5i: ", t); 
    // sem comentários pq aqui é igual à outra
    read = Moving_Avarage(_DATA[ t++ ]);
    read.pos -= VIES_C.pos, read.vel -= VIES_C.vel;
    read = ToRad(read);
    return read;
}

void Calc_Vies_Constante() 
{
    // sabemos que nesse CSV o giroscopio está parado até o frame 3500
    // (e mais um pouco depois, mas decidi parar nesse ponto)
    for(int i = 0; i < 3500; i++) 
        VIES_C = Ler_Giroscopio(), printf("\n");
}

// printa valores lidos do csv no intervalo [from, till] para teste
void _DATA_print(int from, int till)
{
    static unsigned int i;
    if(till < 0)
        for(i = 0; i < _DATA.size(); i++)
            _DATA[i].print();
    else
        for(i = from; i < _DATA.size() && i < (unsigned int)till; i++)
            _DATA[i].print();
}

// roda Moving_Avarage varias vezes para debug
void _Teste_MA() 
{
    for(int i = 0; i < 16; i++)
        Moving_Avarage((DATA_point){0.0, 0.0});
    for(int i = 0; i < 32; i++)
        Moving_Avarage((DATA_point){(double)i, -(double)i/10.0}).print();
    for(int i = 0; i < 32; i++)
        Moving_Avarage((DATA_point){(double)32, -(double)32/10.0}).print();
}

#else // Funções, Tipos, Variáveis do modo giroscópio ========================

#include "../MPU6050/MPU6050.cpp"

MPU6050 GIROSCOPIO(PF_0, PF_1);

// tempo transcorrido em milisegundos
int64_t time_milli()
{
    // *obg usuarios de forums
    // https://forums.mbed.com/t/how-to-get-the-runtime-in-microseconds-in-mbed-os-6/12020/3

    using namespace std::chrono;
    // Convert time_point to one in *millisecond accuracy
    auto now_ms = time_point_cast<milliseconds>(Kernel::Clock::now()); 
    return now_ms.time_since_epoch().count();
}

Gir_Lida Ler_Giroscopio()
{
    static double buffer[4];
    static Gir_Lida rawread, read;

    // recebe valores do giroscópio
    GIROSCOPIO.readGyro (&(buffer[1])); // buffer = {--, ax, ay, az};
    GIROSCOPIO.readAccel(&(buffer[0])); // buffer = {wx, wy, wz, az};
    rawread = (Gir_Lida){buffer[3], buffer[2]}; //           2^  3^

    // limpeza dos valores com moving avarage
    read = Moving_Avarage(rawread);
    
    // remoção de viés constante precalculado
    read.pos -= VIES_C.pos, read.vel -= VIES_C.vel;

    // converte valores de graus para radianas
    read = ToRad(read);

    // espera a passagem de 5 ms
    thread_sleep_for( QUANTUM - (time_milli() % QUANTUM) );

    // retorna valor lido
    return read;
}

void Calc_Vies_Constante()
{
    while(time_milli() < 1000)
        VIES_C = Ler_Giroscopio();
}

void INPUT_init()
{
    GIROSCOPIO.initialize();

    Calc_Vies_Constante();
}

#endif
//============================================================================