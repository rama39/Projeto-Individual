
struct Gir_Lida {
    double pos;
    double vel;
};




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
    return _DATA[ t ];
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
    static Gir_Lida rawread, finalread;

    //recebe valores do giroscópio no buffer
    GIROSCOPIO.readGyro (&(buffer[0]));
    GIROSCOPIO.readAccel(&(buffer[1]));
    rawread = (Gir_Lida){buffer[0], buffer[1]};

    // TODO limpeza dos valores
    // TODO correção de viés
    finalread = rawread;

    // espera a passagem de 5 ms
    thread_sleep_for( 5 - (time_milli() % 5) );

    // retorna valor lido
    return finalread;
}

#endif
