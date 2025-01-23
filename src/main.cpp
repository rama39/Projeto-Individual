
#define CSV_MODE // Código será compilado no modo de leitura de CSV

#include "projeto.h"


int main() 
{
    INPUT_init();

    Gir_Lida temp;
    
    for(int i = 0; i < 10000; i++)
        temp = Ler_Giroscopio(),
        printf("%lf, %lf\n", temp.pos, temp.vel);

    #ifdef CSV_MODE
    printf("Lidos %i valores de posicao e velocidade angular\n", _DATA.size());
    _DATA_print(10, 20);
    #endif
}


/*
notes:

    sussy value: -488.9024390243903 
    happens repeatedly, doesn't make sense
    seems to happen at completely random intervals

    5 ms

    Podem usar aquela biblioteca: MPU6050.h

*/