
struct Gir_Lida {
    double pos;
    double vel;
};

// Retorna média aritmetica dos HIS_TAM (16) ultimos valores recebidos
// Valor inicial da media é 0
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


/*
    Escolhe entre a leitura de dados do arquivo csv ou o sensor MPU-6050
*/

#if defined(CSV_MODE)

/*
    Essa versão da biblioteca foi feita para utilizar os dados do csv

    A seleção entre _INPUT.cpp e INPUT.cpp é feita em projeto.h
    
*/

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

}

// Lê valor da tabela lida do CSV em função do tempo transcorrido do programa
Gir_Lida Ler_Giroscopio()
{
    static int t;
    t = ( (clock()*MILS_PER_CLOCK) / QUANTUM ) % _DATA.size();
    t = t % _DATA.size();
    return Moving_Avarage(_DATA[ t ]);
}

// printa valores lidos do csv no intervalo [from, till]
void _DATA_print(int from, int till)
{
    static unsigned int i;
    if(till < 0)
        for(i = 0; i < _DATA.size(); i++)
            printf("%lf, %lf\n", _DATA[i].pos, _DATA[i].vel);
    else
        for(i = from; i < _DATA.size() && i < (unsigned int)till; i++)
            printf("%lf, %lf\n", _DATA[i].pos, _DATA[i].vel);
}

#else // GYROSCOPE MODE

/*
    Essa versão da biblioteca foi feita para utilizar o sensor MPU-6050

    A seleção entre _INPUT.cpp e INPUT.cpp é feita em projeto.h
    
*/

#include "../MPU6050/MPU6050.cpp"

MPU6050 GIROSCOPIO(PF_0, PF_1);

void INPUT_init() 
{
    GIROSCOPIO.initialize();
}

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
    GIROSCOPIO.readGyro (&(buffer[0]));
    GIROSCOPIO.readAccel(&(buffer[1]));
    rawread = (Gir_Lida){buffer[0], buffer[1]};

    // limpeza dos valores com moving avarage
    read = Moving_Avarage(rawread);

    // TODO correção de viés

    // espera a passagem de 5 ms
    thread_sleep_for( 5 - (time_milli() % 5) );

    // retorna valor lido
    return read;
}

#endif
