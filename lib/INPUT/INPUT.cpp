// Funções, Tipos, Variáveis independentes do modo ===========================

// Intervalo entre lidas em milisegundos 
#define QUANTUM 5

// Frequência de leituras em Hz
#define FREQ 200

// obs: QUANTUM*FREQ = 1000 sempre!!!

// Converte v de graus (ou grau/s) para radianas (ou rad/s)
#define ToRad(v) (v*0.01745329)

double Moving_Avarage(double raw)
{
    // Tamanho do buffer de historico de lidas
    #define HIS_TAM 16

    static double MAvarage = 0.0, oldest;
    static double historia[HIS_TAM] = {};
    static int presente = 0;
    
    // achata valor lido para calculo da media
    raw /= HIS_TAM,

    // remove valor mais antigo lido da media
    oldest = historia[presente];
    MAvarage -= oldest;

    // contabiliza valor mais recente lido na media
    MAvarage += raw;
    // e sobrescreve valor mais antigo na historia com o mais recente
    historia[presente] = raw;
    
    // incremento modular
    presente++; if(presente==HIS_TAM)presente=0;

    return MAvarage;
}

double VIES_C; // Vies Constante

double theta = 0.0;
double GetTheta() {return theta;}

/* Escolhe entre a leitura de dados do arquivo csv ou o sensor MPU-6050 */

#if defined(CSV_MODE) // Funções, Tipos, Variáveis do modo CSV ===============

#include <time.h>

vector<vector<double>> _DATA;

// Lê valores do arquivo CSV para que possam ser utilizados
void INPUT_init()
{
    FILE *DATA_file = fopen("files/log.csv", "r");

    if(DATA_file == NULL)
        printf("ERRO NA ABERTURA DO ARQUIVO\n"), exit(0);

    while(getc(DATA_file) != '\n'); //pula primeira linha

    vector<double> buffer_data(2);
    while ( 
        fscanf (
            DATA_file, 
            "%lf,%lf\n", 
            &(buffer_data[0]), &(buffer_data[1])
        ) != EOF
    ) 
        _DATA.push_back(buffer_data);
    
    Calc_Vies_Constante();

}

// Lê valor da tabela lida do CSV
double Ler_Giroscopio()
{
    static int t = 3000;
    static double read;
    // sem comentários pq aqui é igual à outra
    read = Moving_Avarage(_DATA[ t ][0]);
    read -= VIES_C;
    read = ToRad(read);
    if(-0.01 < read && read < 0.01) read = 0;

    theta += read/FREQ;
    while(theta > +3.14159265) theta -= 2*3.14159265;
    while(theta < -3.14159265) theta += 2*3.14159265;
    
    printf("% 5i: { % 11.8lf | % 11.8lf }\n", t, theta, _DATA[ t ][1]); 
    
    t++; return read;
}

// Calcula viés com base no intervalo em que o giroscópio está sabidamente parado
void Calc_Vies_Constante() 
{
    VIES_C = 0.0;
    for(int i = 0; i < 3500; i++)
        VIES_C += _DATA[i][0]/3500;
}

// printa valores lidos do csv no intervalo [from, till] para teste
void _DATA_print(int from, int till)
{
    static unsigned int i;
    if(till < 0)
        for(i = 0; i < _DATA.size(); i++)
            cout<<_DATA[i][0]<<", "<<_DATA[i][1]<<"\n";
    else
        for(i = from; i < _DATA.size() && i < (unsigned int)till; i++)
            cout<<_DATA[i][0]<<", "<<_DATA[i][1]<<"\n";
}

// roda Moving_Avarage varias vezes para debug
void _Teste_MA() 
{
    for(int i = 0; i < 16; i++)
        cout<<Moving_Avarage(0.0)<<"\n";
    for(int i = 0; i < 32; i++)
        cout<<Moving_Avarage((double)i)<<"\n";
    for(int i = 0; i < 32; i++)
        cout<<Moving_Avarage((double)32)<<"\n";
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

double Ler_Giroscopio()
{
    static double buffer[3];
    static double rawread, read;

    // recebe valores do giroscópio
    GIROSCOPIO.readGyro (&(buffer[0]));
    rawread = buffer[2];

    // limpeza dos valores com moving avarage
    read = Moving_Avarage(rawread);
    
    // remoção de viés constante precalculado
    read -= VIES_C;

    // converte valores de graus para radianas
    read = ToRad(read);

    // aproxima a 0
    if(-0.01 < read && read < 0.01) read = 0;

    // contabiliza w no ângulo
    theta += read/FREQ;

    // mantem angulo em [-pi, pi]
    while(theta > +3.14159265) theta -= 2*3.14159265;
    while(theta < -3.14159265) theta += 2*3.14159265;

    // espera a passagem de 5 ms
    thread_sleep_for( QUANTUM - (time_milli() % QUANTUM) );

    // retorna valor lido
    return read;
}

void Calc_Vies_Constante()
{
    double buffer[3];

    VIES_C = 0;
    for(int i = 0; i < 20; i++)
        GIROSCOPIO.readGyro (&(buffer[0])),
        VIES_C += buffer[2]/20,
        thread_sleep_for( QUANTUM - (time_milli() % QUANTUM) );
}

void INPUT_init()
{
    GIROSCOPIO.initialize();

    Calc_Vies_Constante();
}

#endif
//============================================================================