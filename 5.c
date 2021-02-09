#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <locale.h>

//clock_t begin = clock(); here, do your time-consuming job clock_t end = clock(); double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;

// JUEGO
// https://www.playok.com/es/damas/#209

// SITIOS DE INTERES
// https://github.com/NikolaiT/incolumitas/blob/master/content/C/the-art-of-cheating-making-a-chess-com-chess-bot-using-a-unusual-approach.md#chap_preface
// https://turbo.net/grid/selenium/python-ide?&title=Python%20IDE&merge-user=&verb=try&web-url=https%3A%2F%2Fturbo.net&hub-url=https%3A%2F%2Fturbo.net%2Fio
// https://geekflare.com/run-linux-from-a-web-browser/
// auceLabs, CrossBrowserTesting, endtest

// WEB CLAUDI
// claudilleyda.github.io

// NORMAS: https://en.wikipedia.org/wiki/Brazilian_draughts
// NORMA QUE NO SE SI HE TENIDO EN CUENTA O ES QUE ES UNA SITUACION IMPOSIBLE: si una dama esta entre dos piezas enemigas, tras comerse una puede cambiar se dentido para comerse la otra ? si es que  si que puede guay, si no puede hay que cambiarlo 

// Crear un archivo damas.h con las declaraciones de todas las funciones que usamos (ej: int abs(int x);)
// e incluir #include "damas.h" en la cabeza de este fichero.
// Compilar el programa como una libreria (gcc -shared -o damas.so -fPIC damas.c)
// En el .py: ctypes import * y declarar un objeto
// libDamas = CDLL("./damas.so"). Entonces podemos llamar las funciones como libDamas.Funcion().

#define n 8//longitud del lado del tablero. esto se tiene que declarar de esta forma porque sino dara error
int depth=12;
#define engine_plays_as 5 //1=blanco 3=negro 5=ambos.  cualquier otro no juega
#define show_best_move 1 // 1 en caso afirmativo 0 en caso negativo
#define show_engine_process 0 // 1 en caso afirmativo 0 en caso negativo
#define max_depth_showing 5// indica el numero maximo de niveles de profundidad que mostraran su min/max
#define mostrar_tablero 1 // 1 si se quiere que se imprima el tablero tras cada jugada o la frase que indica quien ha ganado. es mejor poner aqui un 0 cuando se llama a la funcion DAMAS() dentro d eun bucle.
#define MAX_MOV 40 // numero maximo de movimientos legales que espero de una posicion
char Pns[2][3]={"no","si"};// Esta variable global se usa para los Prints y sirve para que en lugar de printear un 0 o un 1 printee un no o un si. esto es util para hacer un print que la frase tenga sentido independientemente si es si o no
char BN[4][10]={" ","blancas"," ","negras"};
int printeado_error=0;
long unsigned int contador=0;

int NUM=0;

int veces_encontrado=0;
int veces_podado_primera=0;

typedef struct {int casilla_destino[2]; int casilla_origen[2]; int mata; int casilla_victima[2]; double valor;} mov;// haria falta que guardara quien fue la victica (ficha normal o dama) y si corona o no

// TVP
typedef struct {unsigned long long hash_key; mov movimiento;} TVP_pos;
typedef struct {TVP_pos *tabla; int num_entradas; int num_guardadas;} TVP;

int abs(int x){
    if(x>0){return x;}
    return -x;
}

void print_movimiento(char *cabezal, mov datos_movimiento, int ev){
    // Comprobacion de que es un movimiento inicializado
    if(datos_movimiento.casilla_origen[0]==99 || datos_movimiento.casilla_origen[1]==99 || datos_movimiento.casilla_destino[0]==99 || datos_movimiento.casilla_destino[1]==99){
        printf("print_movimiento: movimiento no inicializado!\n");
    }
    char letras[8]="abcdefgh";
    char mata[2]="-x";
    printf("pm: %s:",cabezal);
    if(ev==0){printf(" ev:--");}
    else{
      printf(" ev: %.1lf",datos_movimiento.valor);
      if(datos_movimiento.valor>100){
        printf(" victoria blanca en %d movimientos",(int)datos_movimiento.valor-99-depth);
      }
      else if(datos_movimiento.valor<-100){
        printf(" victoria negra en %d movimientos",-(int)datos_movimiento.valor-99-depth);
      }
    }
    printf("\t (%d,%d) -> (%d,%d).",datos_movimiento.casilla_origen[0],datos_movimiento.casilla_origen[1],datos_movimiento.casilla_destino[0],datos_movimiento.casilla_destino[1]);
    printf(" %c%d%c%c%d ",letras[datos_movimiento.casilla_origen[1]],8-datos_movimiento.casilla_origen[0],mata[datos_movimiento.mata],letras[datos_movimiento.casilla_destino[1]],8-datos_movimiento.casilla_destino[0]);
    printf(" %s mata",Pns[datos_movimiento.mata]);
    if(datos_movimiento.mata==1){printf(", victima en (%d,%d)",datos_movimiento.casilla_victima[0],datos_movimiento.casilla_victima[1]);}
    printf("\n");
}

void imprimir_tablero(int tablero[n][n]){
    //char piezas[5][6]={" ","⛀","⛁","⛂","⛃"};// http://xahlee.info/comp/unicode_games_cards.html
    char piezas[6][6]={" ","1","2","3","4","5"};
    char num[10]={" 1234567"};
    int ancho=6*n+1;
    int alto=2*n+1;
    char tablero_para_imprimir[alto][ancho];// Este tablero estará lleno de espacios y caracteres ascii etc
    int k,i;
    printf("\n");
    for(k=0;k<alto;k++){// la k es vertical
        for(i=0;i<ancho;i++){// la i es horizontal
            if(k%2==0){printf("-");}
            else if(i%6==0){printf("|");}
            else if(k%2==1 && i%6==3){printf("%s",piezas[tablero[(k-1)/2][(i-3)/6]]);}
            //else if(k%2==1 && i%6==3){printf("%c",num[tablero[(k-1)/2][(i-3)/6]]);}
            else{printf(" ");}
            
        }
        if(k%2==1){printf(" %d\n",8-(k-1)/2);}
        else{printf("\n");}
    }
    printf("   a     b     c     d     e     f     g     h\n\n");
}

int comparador_movimientos(mov m1, mov m2){
    if(m1.casilla_origen[0]!=m2.casilla_origen[0]){return 0;}
    if(m1.casilla_origen[1]!=m2.casilla_origen[1]){return 0;}
    if(m1.casilla_destino[0]!=m2.casilla_destino[0]){return 0;}
    if(m1.casilla_destino[1]!=m2.casilla_destino[1]){return 0;}
    /*if(m1.casilla_victima[0]!=m2.casilla_victima[0]){return 0;}
    if(m1.casilla_victima[1]!=m2.casilla_victima[1]){return 0;}*/
    //if(m1.mata!=m1.mata || m1.valor!=m2.valor){return 0;}
    return 1;
}

void posicion_inicial(int tablero[n][n]){
    int p_coronacion[8][8]={//
        0,0,0,0,0,0,0,0,//
        0,0,3,0,1,0,0,0,//
        0,1,0,0,0,0,0,0,//
        0,0,0,0,0,0,0,0,//
        0,0,0,0,0,0,0,0,//
        0,0,0,0,0,0,0,0,//
        0,3,0,3,0,0,0,0,//
        0,0,0,0,0,0,0,0//
    };
    int p_dama[8][8]={//
        0,0,0,0,0,0,0,0,//
        0,0,0,0,0,0,0,0,//
        0,0,0,0,0,0,0,0,//
        0,0,2,0,0,0,2,0,//
        0,0,0,0,0,0,0,0,//
        0,0,0,0,4,0,3,0,//
        0,0,0,2,0,0,0,0,//
        0,0,0,0,0,0,0,0//
    };
    int p_dama_come[8][8]={//
        0,0,0,0,0,0,0,0,//
        0,0,0,0,0,0,3,0,//
        0,3,0,0,0,0,0,0,//
        0,0,0,0,0,0,0,0,//
        0,0,0,2,0,0,0,0,//
        0,0,0,0,3,0,3,0,//
        0,0,0,0,0,0,0,0,//
        0,0,0,0,0,0,0,0//
    };
    int p_dos_damas[8][8]={//
        0,0,0,0,0,0,0,0,//
        0,0,0,0,0,0,0,0,//
        0,0,0,0,0,2,0,0,//
        0,0,0,0,0,0,0,0,//
        0,0,0,2,0,0,0,0,//
        0,0,0,0,0,0,0,0,//
        0,0,0,0,0,0,0,0,//
        0,0,0,0,0,0,3,0//
    };
    int p_matar_doble[8][8]={//
        0,0,0,0,0,0,0,0,//
        0,0,0,0,0,0,0,0,//
        0,0,0,0,0,0,0,0,//
        0,0,0,0,0,0,0,0,//
        0,3,0,3,0,0,0,0,//
        0,0,0,0,1,0,0,0,//
        0,3,0,0,0,0,0,0,//
        1,0,0,0,0,0,0,0//
    };
    int p_come_come[8][8]={//
        0,0,0,0,0,0,0,0,//
        0,0,0,0,0,0,0,0,//
        0,0,0,3,0,0,0,0,//
        0,0,3,0,0,0,0,0,//
        0,0,0,3,0,0,0,0,//
        1,0,0,0,1,0,0,0,//
        0,0,0,0,0,1,0,0,//
        0,0,0,0,0,0,1,0//
    };
    int p_come_no_corona[8][8]={//
        0,0,0,0,0,0,0,0,//
        0,3,0,3,0,3,0,0,//
        1,0,1,0,0,0,0,0,//
        0,0,0,0,0,0,0,0,//
        0,0,0,0,0,0,0,0,//
        0,0,0,0,0,0,0,0,//
        0,0,0,3,0,0,0,0,//
        0,0,0,0,0,0,0,0//
    };
    int p_comer_cadena[8][8]={//
        0,0,0,0,0,0,0,0,//
        0,0,0,0,0,0,3,0,//
        0,0,0,0,0,0,0,0,//
        0,0,3,0,3,0,0,0,//
        0,0,0,1,0,0,0,0,//
        0,0,0,0,0,0,0,0,//
        0,3,0,0,0,0,0,0,//
        1,0,0,0,0,0,0,0//
    };
    int p_debug_cadena[8][8]={//
        0,2,0,0,0,0,0,0,//
        3,0,0,0,0,0,0,0,//
        0,0,0,3,0,0,0,0,//
        0,0,0,0,0,0,3,0,//
        0,0,0,0,0,0,0,0,//
        0,0,0,0,3,0,0,0,//
        0,0,0,0,0,0,0,0,//
        0,0,0,0,0,0,0,0//
    };
    for(int k=0;k<8;k++){
        for(int i=0;i<8;i++){
            tablero[k][i]=p_debug_cadena[k][i];
        }
    }
}

void posicion_inicial_engine(int tablero[n][n]){
    int p_d2_1[8][8]={//
        0,0,0,0,0,0,0,0,//
        0,0,3,0,0,0,0,0,//
        0,0,0,0,0,0,0,0,//
        0,0,3,0,0,0,3,0,//
        0,0,0,1,0,0,0,0,//
        0,0,0,0,0,0,0,0,//
        0,0,0,0,0,0,0,0,//
        0,0,0,0,0,0,0,0//
    };
    
    int p_d2_2[8][8]={//
        0,0,0,0,0,0,0,0,//
        0,0,0,0,0,0,0,0,//
        0,0,0,0,0,0,0,3,//
        0,0,0,0,0,0,0,0,//
        0,0,0,0,0,1,0,0,//
        0,0,0,0,0,0,0,0,//
        0,0,0,0,0,0,0,0,//
        0,0,0,0,0,0,0,0//
    };
    
    int p_d2_3[8][8]={//
        0,0,0,0,0,0,0,0,//
        0,0,0,0,0,0,0,0,//
        0,0,0,0,0,3,0,0,//
        0,0,0,0,0,0,0,0,//
        0,0,0,0,0,1,0,0,//
        0,0,0,0,0,0,0,0,//
        0,0,0,0,0,0,0,0,//
        0,0,0,0,0,0,0,0//
    };
    
    int p_d2_4[8][8]={//
        0,0,0,0,0,0,0,0,//
        0,0,0,0,3,0,0,0,//
        0,0,0,0,0,0,0,0,//
        0,0,0,0,3,0,0,0,//
        0,0,0,0,0,1,0,0,//
        0,0,0,0,0,0,0,0,//
        0,0,0,0,0,0,0,0,//
        0,0,0,0,0,0,0,0//
    };
    
    int p_d3_1[8][8]={//
        0,0,0,0,0,0,0,0,//
        0,0,0,0,0,0,0,0,//
        0,0,0,0,0,0,0,3,//
        0,0,0,0,0,0,0,0,//
        0,0,0,0,0,0,0,0,//
        0,0,0,0,1,0,0,0,//
        0,0,0,0,0,0,0,0,//
        0,0,0,0,0,0,0,0//
    };
    
    // https://www.checkercruncher.com/problems/3
    int p_d3_2[8][8]={//
        0,0,0,0,0,3,0,0,//
        0,0,3,0,0,0,1,0,//
        0,3,0,0,0,0,0,1,//
        0,0,3,0,0,0,0,0,//
        0,0,0,0,0,0,0,3,//
        0,0,3,0,0,0,0,0,//
        0,0,0,1,0,3,0,3,//
        0,0,0,0,1,0,1,0//
    };
    
    int p_d3_3[8][8]={//
        0,0,0,0,0,3,0,0,//
        0,0,3,0,0,0,1,0,//
        0,3,0,0,0,0,0,3,//
        3,0,3,0,0,0,0,0,//
        0,0,0,3,0,0,0,3,//
        1,0,0,0,1,0,0,0,//
        0,0,0,0,0,0,0,3,//
        0,0,1,0,1,0,1,0//
    };
    // en realidad seria d=2
    int p_d3_4[8][8]={//
        0,2,0,0,0,0,0,0,//
        0,0,3,0,0,0,0,0,//
        0,0,0,0,0,0,0,3,//
        0,0,0,0,0,0,0,0,//
        0,0,0,0,0,0,0,0,//
        0,0,0,0,0,0,0,0,//
        0,0,0,3,0,0,0,0,//
        0,0,0,0,0,0,0,0//
    };
    
    // tiene que jugar b2-c3 para protejer la ficha de e5
    int p_d4_1[8][8]={//
        0,0,0,0,0,0,0,0,//
        0,0,3,0,3,0,3,0,//
        0,0,0,0,0,0,0,0,//
        0,0,0,0,1,0,0,0,//
        0,0,0,0,0,0,0,0,//
        0,0,0,0,0,0,1,0,//
        0,1,0,0,0,0,0,0,//
        0,0,0,0,0,0,0,0//
    };
    
    int p_d4_2[8][8]={//
        0,0,0,0,0,3,0,0,//
        0,0,0,0,0,0,3,0,//
        0,3,0,3,0,0,0,0,//
        3,0,0,0,0,0,3,0,//
        0,0,0,0,0,0,0,1,//
        0,0,0,0,0,0,1,0,//
        0,1,0,0,0,0,0,0,//
        1,0,1,0,1,0,1,0//
    };
    
    // version con mas profundidad del p_d2_3. deberia ver que esta perdido
    int p_d4_3[8][8]={//
        0,0,0,0,0,0,0,0,//
        0,0,0,0,3,0,0,0,//
        0,0,0,0,0,0,0,0,//
        0,0,0,0,0,0,0,0,//
        0,0,0,0,0,0,0,0,//
        0,0,0,0,1,0,0,0,//
        0,0,0,0,0,0,0,0,//
        0,0,0,0,0,0,0,0//
    };
    // si las blancas juegan e5-d6 entonces las negras contestan con f6-e5 y tras el come com eforzado las negras han ganado una pieza (no estoy seguro si hay una alternativa mejor)
    // si las blancas juegan d4-c5 las negras se comen 4 de golpe
    //1. a3-b4 b6-a5 2. e3-d4 c7-b6 3. d4-c5 b6xd4 4. c3xe5xc7 d8xb6 5. g3-f4 a5xc3 6. b2xd4 e7-d6 7. d2-c3 b6-a5 8. f2-e3 a7-b6 9. h2-g3 d6-c5 10. f4-e5 c5-b4 11. e3-f4 b4xd2 12. c1xe3 a5-b4 13. e5-d6 f6-e5 14. d4xf6 g7xe5xc7 15. f4-e5 b4-a3 16. e5-f6 b6-a5 17. e3-d4 a5-b4 18. d4-e5 b4-c3 19. g3-f4 a3-b2 *
    int p_d5_1[8][8]={//
        0,3,0,0,0,3,0,3,//
        0,0,0,0,0,0,3,0,//
        0,3,0,0,0,3,0,3,//
        0,0,0,0,1,0,0,0,//
        0,3,0,1,0,1,0,0,//
        0,0,0,0,1,0,1,0,//
        0,0,0,0,0,0,0,0,//
        1,0,0,0,1,0,1,0//
    };
    
    // version con mas profundidad del p_d4_3. pero aqui no esta perdido
    int p_d6_1[8][8]={//
        0,0,0,3,0,0,0,0,//
        0,0,0,0,0,0,0,0,//
        0,0,0,0,0,0,0,0,//
        0,0,0,0,0,0,0,0,//
        0,0,0,0,0,0,0,0,//
        0,0,0,0,0,0,0,0,//
        0,0,0,1,0,0,0,0,//
        0,0,0,0,0,0,0,0//
    };
    
    // con pronfundidad 5 lo que hace es que como "no ve mas alla" entonces si sacrifica una pieza ve que al final por otro lado acabara comiendo una pieza pero al poner profundidad 6 ve que despues dse la recomeran y la evalacion sería de -1. con profundidad 5 la maquina juega a5-b6
    int p_d6_2[8][8]={//
        0,3,0,3,0,3,0,3,//
        3,0,0,0,3,0,3,0,//
        0,0,0,0,0,0,0,0,//
        1,0,0,0,3,0,0,0,//
        0,0,0,0,0,3,0,0,//
        0,0,0,0,0,0,0,0,//
        0,1,0,1,0,1,0,1,//
        1,0,1,0,1,0,1,0//
    };
    
    // pasa lo mismo que en el caso anterior
    int p_d6_3[8][8]={//
        0,3,0,3,0,3,0,3,//
        0,0,0,0,0,0,3,0,//
        0,0,0,3,0,0,0,0,//
        0,0,3,0,3,0,0,0,//
        0,0,0,0,0,3,0,0,//
        1,0,0,0,0,0,0,0,//
        0,0,0,1,0,1,0,1,//
        1,0,1,0,1,0,1,0//
    };
    
    // casilla del atacante deberia ser dama 
    int p_debug1[8][8]={//
        0,0,0,0,0,3,0,0,//
        0,0,0,0,0,0,0,0,//
        0,1,0,0,0,3,0,0,//
        0,0,0,0,0,0,2,0,//
        0,0,0,3,0,0,0,0,//
        0,0,0,0,0,0,0,0,//
        0,0,0,1,0,0,0,0,//
        1,0,1,0,0,0,0,0//
    };
    
    // casilla del atacante deberia ser dama 
    int p_debug2[8][8]={//
        0,0,0,0,0,0,0,0,//
        0,0,0,0,0,0,0,0,//
        0,0,0,0,0,0,0,3,//
        0,0,0,0,0,0,0,0,//
        0,0,0,0,0,4,0,0,//
        0,0,0,0,3,0,0,0,//
        0,0,0,1,0,0,0,0,//
        1,0,0,0,0,0,0,0//
    };
    
    int p_juego[8][8]={//
        0,0,0,0,0,0,0,0,//
        0,0,0,0,3,0,0,0,//
        0,0,0,3,0,0,0,3,//
        3,0,0,0,0,0,0,0,//
        0,0,0,0,0,0,0,1,//
        0,0,0,0,0,0,0,0,//
        0,0,0,0,0,0,0,0,//
        1,0,0,0,0,0,1,0//
    };
  
    // ESTA Y LAS 4 SIGUIENTES SON PARTE DEL MISMO DEBUG. SI SE PONE MUCHA PROFUNDIDAD PETA
    int debug3[8][8]={//
        0,3,0,0,0,0,0,2,//
        1,0,3,0,0,0,0,0,//
        0,0,0,0,0,0,0,0,//
        0,0,0,0,4,0,0,0,//
        0,1,0,0,0,0,0,0,//
        0,0,0,0,2,0,0,0,//
        0,0,0,0,0,0,0,0,//
        0,0,0,0,0,0,0,0//
    };
  
    int debug4[8][8]={//
        0,3,0,0,0,0,0,0,//
        1,0,0,0,0,0,0,0,//
        0,0,0,3,0,0,0,0,//
        0,0,0,0,0,0,0,0,//
        0,1,0,0,0,0,0,0,//
        0,0,0,0,2,0,0,0,//
        0,2,0,0,0,0,0,0,//
        0,0,0,0,0,0,0,0//
    };
  
    int debug5[8][8]={//
        0,0,0,0,0,0,0,0,//
        1,0,3,0,0,0,0,0,//
        0,0,0,3,0,2,0,0,//
        0,0,0,0,0,0,0,0,//
        0,1,0,0,0,0,0,0,//
        0,0,0,0,2,0,0,0,//
        0,0,0,0,0,0,0,0,//
        0,0,0,0,0,0,0,0//
    };
    //POR CIERTO ESTA POSICION VA BIEN PARA PROBAR AQUELLO DE QUE ESCOJA EL MOVIMIENTO DE COMER MAS
    int debug6[8][8]={//
        0,2,0,0,0,0,0,0,//
        0,0,3,0,0,0,0,0,//
        0,0,0,0,0,2,0,0,//
        0,0,0,0,3,0,0,0,//
        0,1,0,0,0,0,0,0,//
        0,0,0,0,2,0,0,0,//
        0,0,0,0,0,0,0,0,//
        0,0,0,0,0,0,0,0//
    };
  
    int debug7[8][8]={//
        0,0,0,0,0,0,0,0,//
        0,0,0,0,0,0,0,0,//
        0,0,0,2,0,2,0,0,//
        0,0,0,0,3,0,0,0,//
        0,1,0,0,0,0,0,0,//
        0,0,0,0,2,0,0,0,//
        0,0,0,0,0,0,0,0,//
        0,0,0,0,0,0,0,0//
    };
    
    // con profundidad 8
    int debug8[8][8]={//
          0,0,0,3,0,0,0,3,//
          0,0,3,0,0,0,0,0,//
          0,0,0,3,0,3,0,3,//
          3,0,3,0,0,0,0,0,//
          0,3,0,0,0,1,0,1,//
          1,0,1,0,1,0,0,0,//
          0,0,0,1,0,1,0,0,//
          1,0,0,0,0,0,1,0//
      };
    //https://www.youtube.com/watch?v=ibmZRazelwI
    int damas_3_vs_1[8][8]={//
        0,0,0,4,0,0,0,0,//
        0,0,0,0,0,0,0,0,//
        0,0,0,2,0,0,0,0,//
        0,0,0,0,0,0,0,0,//
        0,0,0,0,0,0,0,0,//
        0,0,2,0,2,0,0,0,//
        0,0,0,0,0,0,0,0,//
        0,0,0,0,0,0,0,0//
    };
    // las blancas movieron e3-d4 y fue un fatal error. el mejor mov para responder es f6-g5
    int p_d7_1[8][8]={//
        0,3,0,3,0,0,0,0,//
        0,0,3,0,3,0,3,0,//
        0,0,0,0,0,3,0,0,//
        1,0,0,0,0,0,0,0,//
        0,0,0,0,0,1,0,0,//
        0,0,0,0,1,0,0,0,//
        0,1,0,0,0,1,0,0,//
        0,0,0,0,0,0,1,0//
    };
    int p_d7_2[8][8]={//
        0,3,0,3,0,0,0,0,//
        0,0,3,0,3,0,3,0,//
        0,0,0,0,0,0,0,0,//
        1,0,0,0,0,0,3,0,//
        0,0,0,1,0,1,0,0,//
        0,0,0,0,0,0,0,0,//
        0,1,0,0,0,1,0,0,//
        0,0,0,0,0,0,1,0//
    };
    int k,i;
    for(k=0;k<n;k++){
        for(i=0;i<n;i++){
            tablero[k][i]=damas_3_vs_1[k][i];
        }
    }
}

void inicio_de_partida(int tablero[n][n]){
    for(int i=0; i<n; i++){    // Vaciar el tablero
        for(int j=0; j<n; j++){
            tablero[i][j]=0;
        }
    }
    for(int i=0; i<n/2 -1; i++){    // Configuracion inicial de las damas rojas
        for(int j=0; j<n; j++){    // Rojas o negras (3,4)
            if(j%2!=(i%2)){
                tablero[i][j]=3;
            }
        }
    }
    for(int i=n-1; i>=n/2 +1; i--){    // Configuracion inicial de las damas blancas
        for(int j=0; j<n; j++){    // Blancas (1,2)
            if(j%2!=(i%2)){
                tablero[i][j]=1;
            }
        }
    }
}

int puede_matar(int tablero[n][n], int turno, int posicion_pieza_atacante[2], int print){
    int k,i,j,no_turno;
    if(turno==1){no_turno=3;}
    else{no_turno=1;}
    
    // Si el objetivo de la funcion es ver si existe una pieza que puede matar
    if(posicion_pieza_atacante[0]==0 && posicion_pieza_atacante[1]==0){
        //printf("puede_matar: el objetivo de la funcion es ver si existe una pieza que puede matar\n");
        for(k=0;k<n;k++){
            for(i=0;i<n;i++){
                // Si es una ficha normal del bando que mueve
                if(tablero[k][i]==turno){
                    //printf("puede_matar: en la casilla (%d,%d) hay una ficha normal del turno %d\n",k,i,turno);
                    // Direccion +,+
                    if(tablero[k+1][i+1]==no_turno || tablero[k+1][i+1]==no_turno+1){
                        // Despues de matar estaria dentro del tablero
                        if(k+2<n && i+2<n){
                            if(tablero[k+2][i+2]==0 || tablero[k+2][i+2]==0){
                                if(print==1){printf("puede_matar: (%d,%d) puede matar a (%d,%d)\n",k,i,k+1,i+1);}
                                return 1;
                            }
                        }
                    }
                    // Direccion -,+
                    if(k>0){
                        //printf("puede_matar: direccion -,+\n");
                        if(tablero[k-1][i+1]==no_turno || tablero[k-1][i+1]==no_turno+1){
                            //printf("puede_matar: en la casilla (%d,%d) hay un %d\n",k-1,i+1,tablero[k-1][i+1]);
                           // Despues de matar estaria dentro del tablero
                           if(k-2>=0 && i+2<n){
                                if(tablero[k-2][i+2]==0 || tablero[k-2][i+2]==0){
                                    if(print==1){printf("puede_matar: (%d,%d) puede matar a (%d,%d)\n",k,i,k-1,i+1);}
                                    return 1;
                                }
                            }
                        }
                    }
                    // Direccion +,-
                    if(i>0){
                        //printf("puede_matar: direccion +,-: en la casilla (%d,%d) hay un %d\n",k+1,i-1,tablero[k+1][i-1]);
                        if(tablero[k+1][i-1]==no_turno || tablero[k+1][i-1]==no_turno+1){
                            // Despues de matar estaria dentro del tablero
                            if(k+2<n && i-2>=0){
                                //printf("puede_matar: aun no se acaba el tablero\n");
                                if(tablero[k+2][i-2]==0 || tablero[k+2][i-2]==0){
                                    if(print==1){printf("puede_matar: (%d,%d) puede matar a (%d,%d)\n",k,i,k+1,i-1);}
                                    return 1;
                                }
                            }
                            //printf("puede_matar: se saldria del tablero porque %d+2 no es menor a %d y %d-2 no son mayores a 0\n",k,n,i);
                        }
                    }
                    // Direccion -,-
                    if(k>0 && i>0){
                        //printf("puede_matar: direccion -,-: en la casilla (%d,%d) hay un %d\n",k-1,i-1,tablero[k-1][i-1]);
                        if(tablero[k-1][i-1]==no_turno || tablero[k-1][i-1]==no_turno+1){
                            // Despues de matar estaria dentro del tablero
                            if(k-2>=0 && i-2>=0){
                                //printf("puede_matar: aun no se acaba el tablero\n");
                                if(tablero[k-2][i-2]==0 || tablero[k-2][i-2]==0){
                                    if(print==1){printf("puede_matar: (%d,%d) puede matar a (%d,%d)\n",k,i,k-1,i-1);}
                                    return 1;
                                }
                            }
                            //printf("puede_matar: se saldria del tablero porque %d-2 y %d-2 no son mayores a 0\n",k,i);
                        }
                    }
                }
                // Si es una dama del bando que mueve
                else if(tablero[k][i]==turno+1){
                    int direccion[4][2]={{1,1},{-1,1},{1,-1},{-1,-1}};
                    int longitud=1;
                    // Para cada una de las direcciones
                    for(j=0;j<4;j++){
                        longitud=1;
                        // Mientras no te salgas del tablero encuentra una casilla no vacia
                        //printf("longitud=%d, -1<%d<n  -1<%d<n hay un %d\n",longitud,k+longitud*direccion[j][0],i+longitud*direccion[j][1],tablero[k+longitud*direccion[j][0]][i+longitud*direccion[j][1]]);
                        while(tablero[k+longitud*direccion[j][0]][i+longitud*direccion[j][1]]==0 && k+longitud*direccion[j][0]>-1 && k+longitud*direccion[j][0]<n  && i+longitud*direccion[j][1]>-1 && i+longitud*direccion[j][1]<n){
                            //printf("puede_matar: direccion (%d,%d) la casilla (%d,%d) esta vacia (%d)\n",direccion[j][0],direccion[j][1],k+longitud*direccion[j][0],i+longitud*direccion[j][1],tablero[k+longitud*direccion[j][0]][i+longitud*direccion[j][1]]);
                            longitud++;
                        }
                        // Si la pieza es aliada mira otra direccion
                        //printf("puede_matar: para la direccion (%d,%d) en la casilla (%d,%d) he encontrado un %d y le toca a turno=%d\n",direccion[j][0],direccion[j][1],k+longitud*direccion[j][0],i+longitud*direccion[j][1],tablero[k+longitud*direccion[j][0]][i+longitud*direccion[j][1]],turno);
                        if(tablero[k+longitud*direccion[j][0]][i+longitud*direccion[j][1]]==turno || tablero[k+longitud*direccion[j][0]][i+longitud*direccion[j][1]]==turno+1){continue;}
                        // Si la pieza es enemiga
                        else if(tablero[k+longitud*direccion[j][0]][i+longitud*direccion[j][1]]==no_turno || tablero[k+longitud*direccion[j][0]][i+longitud*direccion[j][1]]==no_turno+1){
                            longitud++;
                            // Y la siguiente esta vacia y dentro del tablero
                            if(k+longitud*direccion[j][0]<n && k+longitud*direccion[j][0]>=0 && i+longitud*direccion[j][1]<n && i+longitud*direccion[j][1]>=0){
                                if(tablero[k+longitud*direccion[j][0]][i+longitud*direccion[j][1]]==0){
                                    longitud--;
                                    if(print==1){printf("(%d,%d) puede matar con dama a (%d,%d)\n",k,i,k+longitud*direccion[j][0],i+longitud*direccion[j][1]);}
                                    return 1;
                                }
                            }
                        }
                        longitud=1;
                    }
                }
            }
        }
    }
    // Si el objetivo de la funcion es ver si una pieza en concreto puede matar
    else{
        //printf("puede_matar: el objetivo de la funcion es ver si la pieza de (%d,%d) puede matar\n",posicion_pieza_atacante[0],posicion_pieza_atacante[1]);
        if(tablero[posicion_pieza_atacante[0]][posicion_pieza_atacante[1]]==turno){
            //printf("en la casilla (%d,%d) hay una ficha del turno %d\n",posicion_pieza_atacante[0],i,turno);
            // Direccion +,+
            if(tablero[posicion_pieza_atacante[0]+1][posicion_pieza_atacante[1]+1]==no_turno || tablero[posicion_pieza_atacante[0]+1][posicion_pieza_atacante[1]+1]==no_turno+1){
                // Despues de matar estaria dentro del tablero
                if(posicion_pieza_atacante[0]+2<n && posicion_pieza_atacante[1]+2<n){
                    if(tablero[posicion_pieza_atacante[0]+2][posicion_pieza_atacante[1]+2]==0 || tablero[posicion_pieza_atacante[0]+2][posicion_pieza_atacante[1]+2]==0){
                        if(print==1){printf("puede_matar: (%d,%d) puede matar a (%d,%d)\n",posicion_pieza_atacante[0],posicion_pieza_atacante[1],posicion_pieza_atacante[0]+1,posicion_pieza_atacante[1]+1);}
                        return 1;
                    }
                }
            }
            // Direccion -,+
            if(posicion_pieza_atacante[0]>0){
                if(tablero[posicion_pieza_atacante[0]-1][posicion_pieza_atacante[1]+1]==no_turno || tablero[posicion_pieza_atacante[0]-1][posicion_pieza_atacante[1]+1]==no_turno+1){
                    //printf("puede_matar: en la direccion -,+ en la siguiente casilla (%d,%d) hay un %d\n",posicion_pieza_atacante[0]-1,posicion_pieza_atacante[1]+1,tablero[posicion_pieza_atacante[0]-1][posicion_pieza_atacante[1]+1]);
                   // Despues de matar estaria dentro del tablero
                   if(posicion_pieza_atacante[0]-2>=0 && posicion_pieza_atacante[1]+2<n){
                        if(tablero[posicion_pieza_atacante[0]-2][posicion_pieza_atacante[1]+2]==0 || tablero[posicion_pieza_atacante[0]-2][posicion_pieza_atacante[1]+2]==0){
                            if(print==1){printf("puede_matar: (%d,%d) puede matar a (%d,%d)\n",posicion_pieza_atacante[0],posicion_pieza_atacante[1],posicion_pieza_atacante[0]-1,posicion_pieza_atacante[1]+1);}
                            return 1;
                        }
                    }
                }
            }
            // Direccion +,-
            if(posicion_pieza_atacante[1]>0){
                if(tablero[posicion_pieza_atacante[0]+1][posicion_pieza_atacante[1]-1]==no_turno || tablero[posicion_pieza_atacante[0]+1][posicion_pieza_atacante[1]-1]==no_turno+1){
                    // Despues de matar estaria dentro del tablero
                    if(posicion_pieza_atacante[0]+2<n && posicion_pieza_atacante[1]-2>=0){
                        if(tablero[posicion_pieza_atacante[0]+2][posicion_pieza_atacante[1]-2]==0 || tablero[posicion_pieza_atacante[0]+2][posicion_pieza_atacante[1]-2]==0){
                            if(print==1){printf("puede_matar: (%d,%d) puede matar a (%d,%d)\n",posicion_pieza_atacante[0],posicion_pieza_atacante[1],posicion_pieza_atacante[0]+1,posicion_pieza_atacante[1]-1);}
                            return 1;
                        }
                    }
                }
            }
            // Direccion -,-
            // si esta dentro del tablero
            if(posicion_pieza_atacante[0]>0 && posicion_pieza_atacante[1]>0){
                if(tablero[posicion_pieza_atacante[0]-1][posicion_pieza_atacante[1]-1]==no_turno || tablero[posicion_pieza_atacante[0]-1][posicion_pieza_atacante[1]-1]==no_turno+1){
                    // Despues de matar estaria dentro del tablero
                    if(posicion_pieza_atacante[0]-2>=0 && posicion_pieza_atacante[1]-2>=0){
                        if(tablero[posicion_pieza_atacante[0]-2][posicion_pieza_atacante[1]-2]==0 || tablero[posicion_pieza_atacante[0]-2][posicion_pieza_atacante[1]-2]==0){
                            if(print==1){printf("puede_matar: (%d,%d) puede matar a (%d,%d)\n",posicion_pieza_atacante[0],posicion_pieza_atacante[1],posicion_pieza_atacante[0]-1,posicion_pieza_atacante[1]-1);}
                            return 1;
                        }
                    }
                }
            }
        }
        // Si es una dama del bando que mueve
        else if(tablero[posicion_pieza_atacante[0]][posicion_pieza_atacante[1]]==turno+1){
            int direccion[4][2]={{1,1},{-1,1},{1,-1},{-1,-1}};
            int longitud=1;
            // Para cada una de las direcciones
            for(j=0;j<4;j++){
                longitud=1;
                // Mientras no te salgas del tablero encuentra una casilla no vacia
                //printf("puede_matar: longitud=%d, -1<%d<n  -1<%d<n hay un %d\n",longitud,posicion_pieza_atacante[0]+longitud*direccion[j][0],posicion_pieza_atacante[1]+longitud*direccion[j][1],tablero[posicion_pieza_atacante[0]+longitud*direccion[j][0]][posicion_pieza_atacante[1]+longitud*direccion[j][1]]);
                while(tablero[posicion_pieza_atacante[0]+longitud*direccion[j][0]][posicion_pieza_atacante[1]+longitud*direccion[j][1]]==0 && posicion_pieza_atacante[0]+longitud*direccion[j][0]>-1 && posicion_pieza_atacante[0]+longitud*direccion[j][0]<n  && posicion_pieza_atacante[1]+longitud*direccion[j][1]>-1 && posicion_pieza_atacante[1]+longitud*direccion[j][1]<n){
                    //printf("puede_matar: direccion (%d,%d) la casilla (%d,%d) esta vacia (%d)\n",direccion[j][0],direccion[j][1],posicion_pieza_atacante[0]+longitud*direccion[j][0],posicion_pieza_atacante[1]+longitud*direccion[j][1],tablero[posicion_pieza_atacante[0]+longitud*direccion[j][0]][posicion_pieza_atacante[1]+longitud*direccion[j][1]]);
                    longitud++;
                }
                //printf("puede_matar: con una longitud de %d, la casilla (%d,%d) no esta vacia, hay un %d\n",longitud,posicion_pieza_atacante[0]+longitud*direccion[j][0],posicion_pieza_atacante[1]+longitud*direccion[j][1],tablero[posicion_pieza_atacante[0]+longitud*direccion[j][0]][posicion_pieza_atacante[1]+longitud*direccion[j][1]]);
                // Si la pieza es aliada mira otra direccion
                //printf("puede_matar: para la direccion (%d,%d) en la casilla (%d,%d) he encontrado un %d y le toca a turno=%d asique miro otra direccion\n",direccion[j][0],direccion[j][1],posicion_pieza_atacante[0]+longitud*direccion[j][0],posicion_pieza_atacante[1]+longitud*direccion[j][1],tablero[posicion_pieza_atacante[0]+longitud*direccion[j][0]][i+longitud*direccion[j][1]],turno);
                if(tablero[posicion_pieza_atacante[0]+longitud*direccion[j][0]][posicion_pieza_atacante[1]+longitud*direccion[j][1]]==turno || tablero[posicion_pieza_atacante[0]+longitud*direccion[j][0]][posicion_pieza_atacante[1]+longitud*direccion[j][1]]==turno+1){continue;}
                // Si la pieza es enemiga
                else if(tablero[posicion_pieza_atacante[0]+longitud*direccion[j][0]][posicion_pieza_atacante[1]+longitud*direccion[j][1]]==no_turno || tablero[posicion_pieza_atacante[0]+longitud*direccion[j][0]][posicion_pieza_atacante[1]+longitud*direccion[j][1]]==no_turno+1){
                    //printf("puede_matar: para la direccion (%d,%d) en la casilla (%d,%d) he encontrado un %d asique puede que pueda matar\n",direccion[j][0],direccion[j][1],posicion_pieza_atacante[0]+longitud*direccion[j][0],posicion_pieza_atacante[1]+longitud*direccion[j][1],tablero[posicion_pieza_atacante[0]+longitud*direccion[j][0]][i+longitud*direccion[j][1]]);
                    longitud++;
                    // Y la siguiente esta vacia y sigue sin salirse del tablero
                    if(tablero[posicion_pieza_atacante[0]+longitud*direccion[j][0]][posicion_pieza_atacante[1]+longitud*direccion[j][1]]==0 && posicion_pieza_atacante[0]+longitud*direccion[j][0]>-1 && posicion_pieza_atacante[0]+longitud*direccion[j][0]<n  && posicion_pieza_atacante[1]+longitud*direccion[j][1]>-1 && posicion_pieza_atacante[1]+longitud*direccion[j][1]<n){
                        longitud--;
                        if(print==1){printf("(%d,%d) puede matar con dama a (%d,%d)\n",posicion_pieza_atacante[0],posicion_pieza_atacante[1],posicion_pieza_atacante[0]+longitud*direccion[j][0],posicion_pieza_atacante[1]+longitud*direccion[j][1]);}
                        return 1;
                    }
                }
                longitud=1;
            }
        }
    }
    return 0;
}

// Esta funcion devuelve 1 si corona y 0 si no.
int corona(int posicion_pieza[2], int tablero[n][n], int turno, int acaba_de_matar){
    if(tablero[posicion_pieza[0]][posicion_pieza[1]]==2 || tablero[posicion_pieza[0]][posicion_pieza[1]]==4){return 0;}
    if(acaba_de_matar==1){
        if(puede_matar(tablero,turno,posicion_pieza,0)==1){
            return 0;
        }
    }
    // blancas
    if(turno==1 && posicion_pieza[0]==0){return 1;}
    // negras
    if(turno==3 && posicion_pieza[0]==n-1){return 1;}
    return 0;
}

int moivmiento_valido(int tablero[n][n], mov datos_movimiento, int turno, int casilla_del_atacante[2], int *dir_acaba_de_coronar){
    int diferencia_y[4]={0,1,0,-1};// Esto es para controlar que si turno=1 entonces van hacia arriba y si turno=3 van hacia abajo
    int turno_enemigo;
    int k;
    int cualquiera[2]={0,0};
    int acaba_de_matar=0;// Esta variable sirve para escoger de que manera se llama a la funcion puede_matar
    if(turno==3){turno_enemigo=1;}
    else{turno_enemigo=3;}
    
    if(casilla_del_atacante[0]!=99){
        if(casilla_del_atacante[0]==datos_movimiento.casilla_origen[0] && casilla_del_atacante[1]==datos_movimiento.casilla_origen[1] && datos_movimiento.mata==1){
            
        }
        else{printf("movimiento_valido: Estas obligado a comer de nuevo con la misma pieza que acabas de comer!\n");return 0;}
    }
    
    //=================================//
    //            pieza normal         //
    //=================================//
    
    if(tablero[datos_movimiento.casilla_origen[0]][datos_movimiento.casilla_origen[1]]==turno){
    //printf("moivmiento_valido: Quieres mover una ficha normal\n");
    // Si no mata
    if(datos_movimiento.mata==0 && puede_matar(tablero,turno,cualquiera,0)==0){
        // Si va en la direccion correcta
        if(datos_movimiento.casilla_origen[0]-datos_movimiento.casilla_destino[0]==diferencia_y[turno] && abs(datos_movimiento.casilla_origen[1]-datos_movimiento.casilla_destino[1])==1){
            // Si la casilla destino esta vacia
            if(tablero[datos_movimiento.casilla_destino[0]][datos_movimiento.casilla_destino[1]]==0){
                tablero[datos_movimiento.casilla_destino[0]][datos_movimiento.casilla_destino[1]]=turno+corona(datos_movimiento.casilla_destino,tablero,turno,acaba_de_matar);
                tablero[datos_movimiento.casilla_origen[0]][datos_movimiento.casilla_origen[1]]=0;
                return 1;
            }
            else{return 0;}
        }
        else{return 0;}
    }
    // Si si que mata
    else if(datos_movimiento.mata==1 && puede_matar(tablero,turno,cualquiera,0)==1){
        //printf("movimiento_valido: si que matas porque puedes\n");
        // Si va en la direccion correcta
        if(abs(datos_movimiento.casilla_origen[0]-datos_movimiento.casilla_destino[0])==2 && abs(datos_movimiento.casilla_origen[1]-datos_movimiento.casilla_destino[1])==2){
            //printf("puede_matar: el ataque va en una direccion valida\n");
            // Si la casilla destino esta vacia y en la del medio una enemiga
            if(tablero[datos_movimiento.casilla_destino[0]][datos_movimiento.casilla_destino[1]]==0 && ( tablero[(datos_movimiento.casilla_destino[0]+datos_movimiento.casilla_origen[0])/2][(datos_movimiento.casilla_destino[1]+datos_movimiento.casilla_origen[1])/2]==turno_enemigo || tablero[(datos_movimiento.casilla_destino[0]+datos_movimiento.casilla_origen[0])/2][(datos_movimiento.casilla_destino[1]+datos_movimiento.casilla_origen[1])/2]==turno_enemigo+1 ) ){
                //printf("puede_matar: ataque valido\n");
                tablero[datos_movimiento.casilla_destino[0]][datos_movimiento.casilla_destino[1]]=turno;
                tablero[(datos_movimiento.casilla_destino[0]+datos_movimiento.casilla_origen[0])/2][(datos_movimiento.casilla_destino[1]+datos_movimiento.casilla_origen[1])/2]=0;
                tablero[datos_movimiento.casilla_origen[0]][datos_movimiento.casilla_origen[1]]=0;
                // Una vez ha movido mira si en la posicion final corona
                acaba_de_matar=1;
                *dir_acaba_de_coronar=corona(datos_movimiento.casilla_destino,tablero,turno,acaba_de_matar);
                tablero[datos_movimiento.casilla_destino[0]][datos_movimiento.casilla_destino[1]]=tablero[datos_movimiento.casilla_destino[0]][datos_movimiento.casilla_destino[1]]+(*dir_acaba_de_coronar);
                return 1;
            }
            else{
                printf("movimiento_valido: El movimineto no es correcto porque o bien despues de comer no hay una casilla vacia (%d) o bien porque no es una pieza enemiga la que quieres comer (%d)\n",tablero[datos_movimiento.casilla_destino[0]][datos_movimiento.casilla_destino[1]],tablero[(datos_movimiento.casilla_destino[0]+datos_movimiento.casilla_origen[0])/2][(datos_movimiento.casilla_destino[1]+datos_movimiento.casilla_origen[1])/2]);
            }
        }
        else{printf("moivmiento_valido: direccion incorrecta\n");return 0;}
    }
    else{printf("moivmiento_valido: no puedes matar y quieres o viceversa\n");return 0;}
    //printf("puedes matar y no lo has hecho\n");return 0;
    }
    
    //=================================//
    //               DAMA              //
    //=================================//
    
    else if(tablero[datos_movimiento.casilla_origen[0]][datos_movimiento.casilla_origen[1]]==turno+1){
        // Si no mata
        //printf("moivmiento_valido: Quieres mover una dama\n");
        if(datos_movimiento.mata==0 && puede_matar(tablero,turno,cualquiera,0)==0){
            //printf("moivmiento_valido: no matas porque no puedes\n");
            // Direccion correcta
            int desplazamiento_vertical=datos_movimiento.casilla_destino[0]-datos_movimiento.casilla_origen[0];
            int desplazamiento_horizontal=datos_movimiento.casilla_destino[1]-datos_movimiento.casilla_origen[1];
            int sentido_horizontal=desplazamiento_horizontal/abs(desplazamiento_horizontal);
            int sentido_vertical=desplazamiento_vertical/abs(desplazamiento_vertical);
            
            // Si el desplazamiento horizontal es igual al vertical quiere decir que es un movmiento en diagonal y por lo tanto de momento vamos bien
            if(abs(desplazamiento_horizontal)==abs(desplazamiento_vertical)){
                //printf("moivmiento_valido: destino-inicio: (%d,%d)-(%d,%d)=(%d,%d)\n",datos_movimiento.casilla_destino[0],datos_movimiento.casilla_destino[1],datos_movimiento.casilla_origen[0],datos_movimiento.casilla_origen[1],desplazamiento_horizontal,desplazamiento_vertical);
                //printf("moivmiento_valido: movimineto en diagonal correcto, sentido horizontal: %d, sentido vertical: %d\n",sentido_horizontal,sentido_vertical);
                // Si las casillas del medio y la final estan vacias
                for(k=1;k<=abs(desplazamiento_vertical);k++){
                    //printf("moivmiento_valido: miro que la casilla (%d,%d) este vacia\n",datos_movimiento.casilla_origen[0]+sentido_vertical*k,datos_movimiento.casilla_origen[1]+sentido_horizontal*k);
                    if(tablero[datos_movimiento.casilla_origen[0]+sentido_vertical*k][datos_movimiento.casilla_origen[1]+sentido_horizontal*k]!=0){printf("moivmiento_valido: movimiento no correcto: la casilla (%d,%d) no esta vacia porque hay un %d\n",datos_movimiento.casilla_origen[0]+sentido_vertical*k,datos_movimiento.casilla_origen[1]+sentido_horizontal*k,tablero[datos_movimiento.casilla_origen[0]+sentido_vertical*k][datos_movimiento.casilla_origen[1]+sentido_horizontal*k]);imprimir_tablero(tablero);return 0;}
                }
                tablero[datos_movimiento.casilla_destino[0]][datos_movimiento.casilla_destino[1]]=turno+1;
                tablero[datos_movimiento.casilla_origen[0]][datos_movimiento.casilla_origen[1]]=0;
                return 1;
            }
            else{printf("moivmiento_valido: el movimiento no es diagonal porque v=%d=!%d=h\n",desplazamiento_vertical,desplazamiento_horizontal);}
        }
        // Si si que mata
        else if(datos_movimiento.mata==1 && puede_matar(tablero,turno,cualquiera,0)==1){
            // Direccion correcta
            int desplazamiento_vertical=datos_movimiento.casilla_destino[0]-datos_movimiento.casilla_origen[0];
            int desplazamiento_horizontal=datos_movimiento.casilla_destino[1]-datos_movimiento.casilla_origen[1];
            int sentido_horizontal=desplazamiento_horizontal/abs(desplazamiento_horizontal);
            int sentido_vertical=desplazamiento_vertical/abs(desplazamiento_vertical);
            
            // Si el desplazamiento horizontal es igual al vertical quiere decir que es un movmiento en diagonal y por lo tanto de momento vamos bien
            if(abs(desplazamiento_horizontal)==abs(desplazamiento_vertical)){
                //printf("moivmiento_valido: destino-inicio: (%d,%d)-(%d,%d)=(%d,%d)\n",datos_movimiento.casilla_destino[0],datos_movimiento.casilla_destino[1],datos_movimiento.casilla_origen[0],datos_movimiento.casilla_origen[1],desplazamiento_horizontal,desplazamiento_vertical);
                //printf("moivmiento_valido: movimineto en diagonal correcto, sentido horizontal: %d, sentido vertical: %d\n",sentido_horizontal,sentido_vertical);
                // Si las casillas estan vacias excepto la casilla comn la pieza victima
                int hay_victima=0;// Esta variable controla que haya una una pieza enemiga en la trayectoria
                int casilla_victima[2]={99,99};// Aqui se guarda la casilla de la pieza victima
                for(k=1;k<abs(desplazamiento_vertical);k++){
                    //printf("moivmiento_valido: miro que la casilla (%d,%d) este vacia (o este la pieza victima)\n",datos_movimiento.casilla_origen[0]+sentido_vertical*k,datos_movimiento.casilla_origen[1]+sentido_horizontal*k);
                    switch(tablero[datos_movimiento.casilla_origen[0]+sentido_vertical*k][datos_movimiento.casilla_origen[1]+sentido_horizontal*k]){
                        case 0:
                            break;
                        case 1:
                        case 2:
                            if(turno==1){
                                printf("moivmiento_valido: movimiento incorrecto: la casilla (%d,%d) no esta vacia porque hay un %d\n",datos_movimiento.casilla_origen[0]+sentido_vertical*k,datos_movimiento.casilla_origen[1]+sentido_horizontal*k,tablero[datos_movimiento.casilla_origen[0]+sentido_vertical*k][datos_movimiento.casilla_origen[1]+sentido_horizontal*k]);
                                return 0;
                            }
                            if(turno==3){
                                if(hay_victima==0){
                                    hay_victima=1;
                                    casilla_victima[0]=datos_movimiento.casilla_origen[0]+sentido_vertical*k;
                                    casilla_victima[1]=datos_movimiento.casilla_origen[1]+sentido_horizontal*k;
                                    //printf("movimineto_valido: Acabo de detectar la victima en (%d,%d)\n",casilla_victima[0],casilla_victima[1]);
                                }
                                else{printf("movimiento_valido: Hay dos piezas enemigas a lo largo del movimiento\n");return 0;}
                            }
                            break;
                        case 3:
                        case 4:
                            if(turno==1){
                                if(hay_victima==0){
                                    hay_victima=1;
                                    casilla_victima[0]=datos_movimiento.casilla_origen[0]+sentido_vertical*k;
                                    casilla_victima[1]=datos_movimiento.casilla_origen[1]+sentido_horizontal*k;
                                    //printf("movimineto_valido: Acabo de detectar la victima en (%d,%d)\n",casilla_victima[0],casilla_victima[1]);
                                }
                                else{printf("movimiento_valido: Hay dos piezas enemigas a lo largo del movimiento\n");return 0;}
                            }
                            if(turno==3){
                                printf("moivmiento_valido: movimiento incorrecto: la casilla (%d,%d) no esta vacia porque hay un %d\n",datos_movimiento.casilla_origen[0]+sentido_vertical*k,datos_movimiento.casilla_origen[1]+sentido_horizontal*k,tablero[datos_movimiento.casilla_origen[0]+sentido_vertical*k][datos_movimiento.casilla_origen[1]+sentido_horizontal*k]);
                                return 0;
                            }
                            
                            break;
                        }
                }
                if(hay_victima==1){
                    tablero[datos_movimiento.casilla_destino[0]][datos_movimiento.casilla_destino[1]]=turno+1;
                    tablero[casilla_victima[0]][casilla_victima[1]]=0;
                    tablero[datos_movimiento.casilla_origen[0]][datos_movimiento.casilla_origen[1]]=0;
                    return 1;
                }
                else{printf("movimiento_valido: ERROR, en la casilla (%d,%d) hay un %d\n",datos_movimiento.casilla_origen[0]+sentido_vertical*(abs(desplazamiento_vertical)-1),datos_movimiento.casilla_origen[1]+sentido_horizontal*(abs(desplazamiento_vertical)-1),tablero[datos_movimiento.casilla_origen[0]+sentido_vertical*(abs(desplazamiento_vertical)-1)][datos_movimiento.casilla_origen[1]+sentido_horizontal*(abs(desplazamiento_vertical)-1)]);}
            }
            else{printf("movimineto_valido: movimineto no diagonal!\n");}
        }
        else{printf("movimiento_valido: movimiento.mata=%d pero puede_matar dice que %d\n",datos_movimiento.mata,puede_matar(tablero,turno,cualquiera,0));}
    }
    else{printf("movimiento_valido: En la casilla origen (%d,%d) no hay una pieza tuya, hay un %d y turno=%d\n",datos_movimiento.casilla_origen[0],datos_movimiento.casilla_origen[1],tablero[datos_movimiento.casilla_origen[0]][datos_movimiento.casilla_origen[1]],turno);}
    return 0;
}

// Funcion para luego fusionarlo con el python selenium
int pedir_movimiento_selenium(mov *datos_movimiento, int turno, int ply, char movimientos_PDN[250][20], char siguientes_movimientos[3][6], int *cantidad_de_siguientes_movimientos, char movimiento[20]){
    // Esta funcion escanea movimientos. si la longitud del movimiento es 5 y tiene una sintaxis correcta coom por ejemplo a3xc5 entonces es un movimiento valido
    // Si la longitud del movimieto es la de forma 5+3k con sintaxis correcta como por ejemplo a3xc5xe7 entonces lo divide en k+1 movimientos diferentes
    // a3xc5xe7 -> a3xc5 + c5xe7
    // Estos movimientos se ponen a la cola y la proxima vez no hara falta escanear ningun movimiento
    
    int k,i;
    int longitud_del_movimiento;
    
    // Si no hay movimientos en la cola
    if((*cantidad_de_siguientes_movimientos)==0){
        // Si la longitud del movimiento es 8,11,14... entonces tendre que partirlo (a3xc5xe7 -> a3xc5 + c5xe7)
        

        //PDN y otros 
        if((movimiento[0]=='P' && movimiento[1]=='D' && movimiento[2]=='N') || movimiento[0]=='-'){
            for(int k=0;k<20;k++){movimiento[k]=movimientos_PDN[ply][k];}
            printf("pedir_movimiento: %s\n",movimiento);
        }
        printf("%s: No habian movimientos en la cola y el movimiento del contrario ha sido: %s\n",__FUNCTION__,movimiento);
        longitud_del_movimiento=strlen(movimiento);
        switch(longitud_del_movimiento){
            case 5:
                break;
            case 8:
            case 11:
            case 14:
                printf("pedir_movimiento_selenium:\n\tmovimiento (%s) de longitud %d.\n",movimiento,longitud_del_movimiento);
                (*cantidad_de_siguientes_movimientos)=(longitud_del_movimiento-5)/3;
                printf("\tPongo a la cola %d movimientos:\n",*cantidad_de_siguientes_movimientos);
                for(k=0;k<(*cantidad_de_siguientes_movimientos);k++){
                    for(i=0;i<5;i++){
                        siguientes_movimientos[k][i]=movimiento[3+3*k+i];
                    }
                    printf("\t%s\n",siguientes_movimientos[k]);
                }
                break;

            default:
                printf("pedir_movimiento: ERROR: longitud inesperada del movimiento (%d)\n",longitud_del_movimiento);
                return 0;
        }
    }
    // Si hay movimientos en la cola
    else{
        printf("pedir_movimiento: Hay %d movimientos en la cola que son:\n",(*cantidad_de_siguientes_movimientos));
        // El movimiento sera el siguiente en la cola
        for(k=0;k<5;k++){movimiento[k]=siguientes_movimientos[0][k];}
        // Ahora lo quitamos de la cola y movemos al resto 
        for(k=0;k<(*cantidad_de_siguientes_movimientos)-1;k++){
            for(i=0;i<5;i++){siguientes_movimientos[k][i]=siguientes_movimientos[k+1][i];}
        }
        (*cantidad_de_siguientes_movimientos)=(*cantidad_de_siguientes_movimientos)-1;
    }
    
    // Compruebo que el movimiento escaneado sea de la forma letranumero-letranumero. 
    // No se como hacer esto de manera mas general si cambiamos la n. habria que añadir mas digitos o algo o dale otros nombres a las casilas
    // Aqui como mucho espero que n=10
    if(n>8){printf("\npedir_movimiento: ERROR, no va leer bien movimiento porque n=%d>10\n",n);}
    char letras_permitidas[20]="abcdefghij";
    char numeros_permitidos[20]="12345678";
    int bool1=0,bool2=0,bool3=0,bool4=0,bool5=0;// Estas variables se encargaran de controlar que la sintaxis del movimiento escaneado sea correcta
    int mata=0; // Esta variable guardara la informacion de si el movimineto mata o no
    
    for(k=0;k<n;k++){
        if(movimiento[0]==letras_permitidas[k]){bool1=1;(*datos_movimiento).casilla_origen[1]=k;}
        if(movimiento[3]==letras_permitidas[k]){bool4=1;(*datos_movimiento).casilla_destino[1]=k;}
    }
    for(k=0;k<n;k++){
        if(movimiento[1]==numeros_permitidos[k]){bool2=1;(*datos_movimiento).casilla_origen[0]=n-k-1;}
        if(movimiento[4]==numeros_permitidos[k]){bool5=1;(*datos_movimiento).casilla_destino[0]=n-k-1;}
    }
    if(movimiento[2]=='-'){bool3=1;(*datos_movimiento).mata=0;}
    if(movimiento[2]=='x'){bool3=1;(*datos_movimiento).mata=1;}
    
    // Movmiento valido sintacticamente
    if(bool1*bool2*bool3*bool4*bool5==1){
        print_movimiento("pedir_movimiento",*datos_movimiento,0);
        return 1;
    }
    else{printf("pedir_movimiento: algo ha fallado\n");return 0;}
}

int pedir_movimiento(mov *datos_movimiento, int turno, int ply, char movimientos_PDN[250][20], char siguientes_movimientos[3][6], int *cantidad_de_siguientes_movimientos){
    // Esta funcion escanea movimientos. si la longitud del movimiento es 5 y tiene una sintaxis correcta coom por ejemplo a3xc5 entonces es un movimiento valido
    // Si la longitud del movimieto es la de forma 5+3k con sintaxis correcta como por ejemplo a3xc5xe7 entonces lo divide en k+1 movimientos diferentes
    // a3xc5xe7 -> a3xc5 + c5xe7
    // Estos movimientos se ponen a la cola y la proxima vez no hara falta escanear ningun movimiento
    
    int k,i;
    int longitud_del_movimiento;
    char movimiento[20]; // Los movimientos del estilo d4xf6xd8 se escanean uno a uno. primero d4xf6 y luego f6xd8
    
    // Si no hay movimientos en la cola
    if((*cantidad_de_siguientes_movimientos)==0){
        printf("\nIntroduce un movimiento. Turno %d. Juegan las %s:\n",ply,BN[turno]);
        // Si la longitud del movimiento es 8,11,14... entonces tendre que partirlo (a3xc5xe7 -> a3xc5 + c5xe7)
        if(!scanf("%s",movimiento)){}


        //PDN y otros 
        if((movimiento[0]=='P' && movimiento[1]=='D' && movimiento[2]=='N') || movimiento[0]=='-'){
            for(int k=0;k<20;k++){movimiento[k]=movimientos_PDN[ply][k];}
            printf("pedir_movimiento: %s\n",movimiento);
        }
        longitud_del_movimiento=strlen(movimiento);
        switch(longitud_del_movimiento){
            case 5:
                break;
            case 8:
            case 11:
            case 14:
                printf("pedir_movimiento: movimiento (%s) de longitud %d.\n",movimiento,longitud_del_movimiento);
                (*cantidad_de_siguientes_movimientos)=(longitud_del_movimiento-5)/3;
                printf("Lo parto en %d movimientos:\n",*cantidad_de_siguientes_movimientos);
                for(k=0;k<(*cantidad_de_siguientes_movimientos);k++){
                    for(i=0;i<5;i++){
                        siguientes_movimientos[k][i]=movimiento[3+3*k+i];
                    }
                    printf("%s\n",siguientes_movimientos[k]);
                }
                break;

            default:
                printf("pedir_movimiento: ERROR: longitud inesperada del movimiento (%d)\n",longitud_del_movimiento);
                return 0;
        }
    }
    // Si hay movimientos en la cola
    else{
        printf("pedir_movimiento: Hay %d movimientos en la cola que son:\n",(*cantidad_de_siguientes_movimientos));
        // El movimiento sera el siguiente en la cola
        for(k=0;k<5;k++){movimiento[k]=siguientes_movimientos[0][k];}
        // Ahora lo quitamos de la cola y movemos al resto 
        for(k=0;k<(*cantidad_de_siguientes_movimientos)-1;k++){
            for(i=0;i<5;i++){siguientes_movimientos[k][i]=siguientes_movimientos[k+1][i];}
        }
        (*cantidad_de_siguientes_movimientos)=(*cantidad_de_siguientes_movimientos)-1;
    }
    
    // Compruebo que el movimiento escaneado sea de la forma letranumero-letranumero. 
    // No se como hacer esto de manera mas general si cambiamos la n. habria que añadir mas digitos o algo o dale otros nombres a las casilas
    // Aqui como mucho espero que n=10
    if(n>8){printf("\npedir_movimiento: ERROR, no va leer bien movimiento porque n=%d>10\n",n);}
    char letras_permitidas[20]="abcdefghij";
    char numeros_permitidos[20]="12345678";
    int bool1=0,bool2=0,bool3=0,bool4=0,bool5=0;// Estas variables se encargaran de controlar que la sintaxis del movimiento escaneado sea correcta
    int mata=0; // Esta variable guardara la informacion de si el movimineto mata o no
    
    for(k=0;k<n;k++){
        if(movimiento[0]==letras_permitidas[k]){bool1=1;(*datos_movimiento).casilla_origen[1]=k;}
        if(movimiento[3]==letras_permitidas[k]){bool4=1;(*datos_movimiento).casilla_destino[1]=k;}
    }
    for(k=0;k<n;k++){
        if(movimiento[1]==numeros_permitidos[k]){bool2=1;(*datos_movimiento).casilla_origen[0]=n-k-1;}
        if(movimiento[4]==numeros_permitidos[k]){bool5=1;(*datos_movimiento).casilla_destino[0]=n-k-1;}
    }
    if(movimiento[2]=='-'){bool3=1;(*datos_movimiento).mata=0;}
    if(movimiento[2]=='x'){bool3=1;(*datos_movimiento).mata=1;}
    
    // Movmiento valido sintacticamente
    if(bool1*bool2*bool3*bool4*bool5==1){
        print_movimiento("pedir_movimiento",*datos_movimiento,0);
        return 1;
    }
    else{printf("pedir_movimiento: algo ha fallado\n");return 0;}
}

// Esta funcion devuelve 1 si el bando "turno" esta sin fichas y 0 en caso contrario.
int sin_fichas(int tablero[n][n], int turno){
    int k,i;
    for(k=0;k<n;k++){
        for(i=0;i<n;i++){
            if(tablero[k][i]==turno || tablero[k][i]==turno+1){return 0;}
        }
    }
    return 1;
}

void guardar_partida(int tablero[n][n], int copia[n][n]){
    int k,i;
    for(k=0;k<n;k++){
        for(i=0;i<n;i++){
            copia[k][i]=tablero[k][i];
        }
    }
}

int leer_PDN(char movimientos_guardados[250][20]/* ply - Movmiento*/){
    //char texto[1000]="1. g3-h4 d6-c5 2. f2-g3 c7-d6 3. c3-b4 d8-c7 4. b4-a5 f6-e5 5. e3-f4 e7-f6 6. e1-f2 f8-e7 7. d2-e3 f6-g5 8. h4xf6xd8 e5-d4 9. c1-d2 c5-b4 10. a3xc5xe7 g7-f6 e3xc5 b6xd4 12. d8xb6xe3 f6xd8 13. f4-g5 h6xf4 14. e3xh6 d8-e7 15. h6-g5 b8-c7 16. g5xd8xb6 a7xc5 17. d2-e3 h8-g7 18. e3-d4 c5xe3 19. f2xd4 g7-f6 20. g3-h4 f6-e5 21. d4xf6 ";
    //char texto[1000]="1. a3-b4 b6-a5 2. b2-a3 a7-b6 3. c1-b2 b6-c5 4. g3-f4 c7-b6 5. f4-e5 d6xf4 6. b4xd6 e7xc5 7. e3xg5xe7 d8xf6 8. h2-g3 f8-e7 9. c3-b4 a5xc3 10. d2xb4xd6xf8 f6-e5 11. g3-f4 e5xg3 12. f2xh4 g7-f6 13. e1-f2 f6-e5 14. b2-c3 e5-f4 15. g1-h2 h8-g7 16. f2-g3 f4-e3 17. g3-f4 e3xg5 18. h4xf6xh8 b8-c7 19. a1-b2 h6-g5 20. f8-h6 g5-f4 21. h6xe3xa7 c7-d6 22. c3-d4 d6-e5 23. d4xf6";
    char texto[1000]="1. a3-b4 b6-a5 2. b4-c5 d6xb4 3. b2-a3 c7-b6 4. a3xc5 b6xd4xb2 5. a1xc3 f6-g5 6. c3-b4 a5xc3 7. d2xb4 g5-h4 8. b4-a5 e7-d6 9. e3-d4 f8-e7 10. a5-b6 a7xc5xe3 11. f2xd4 h4xf2 12. e1xg3 b8-a7 13. d4-e5 d6xf4 14. g3xe5 d8-c7 15. h2-g3 h6-g5 16. g3-h4 g7-f6 17. e5xg7 h8xf6 18. c1-b2 f6-e5";
    printf("PDN: %s\n\n",texto);
    
    int contador=0;// Indice para el vector TEXTO
    int contador_de_turnos=1;// Cuenta por el turno que va la partida
    int contador_longitud_movimiento=0;// Cuenta cuantos caracteres tiene un movimiento a3xc5 son 5 a3xc5xe7 son 8 etc
    
    while(contador_de_turnos<19){
        // Numero del turno
        if(contador_de_turnos<10){
            if(texto[contador]-'0'!=contador_de_turnos){printf("leer_PND: en la posicion %d me esperaba el inicio del turno %d pero hay un (%c)\n",contador,contador_de_turnos,texto[contador]);return 0;}contador++;
        }
        else{
            if(((texto[contador]-'0')*10+texto[contador+1]-'0')!=contador_de_turnos){printf("leer_PND: en la posicion %d me esperaba el inicio del turno %d pero hay un (%c)\n",contador,contador_de_turnos,texto[contador]);return 0;}contador++;contador++;
                
        }
        
        // Punto tras turno
        if(texto[contador]!='.'){printf("leer_PDN: en la posicion %d me esperaba un punto pero hay un (%c)\n",contador,texto[contador]);return 0;}contador++;
        // Espacio tras punto
        if(texto[contador]!=' '){printf("leer_PDN: en la posicion %d me esperaba un espacio pero hay un (%c)\n",contador,texto[contador]);/*return 0;*/}contador++;
        // Escaneo del movmiento blanco
        while(texto[contador]!=' ' && contador_longitud_movimiento<20){
            movimientos_guardados[2*(contador_de_turnos-1)][contador_longitud_movimiento]=texto[contador];
            contador_longitud_movimiento++;
            contador++;
        }
        //printf("long %d ",contador_longitud_movimiento);
        contador_longitud_movimiento=0;
        contador++;
        //printf("leer_PDN: he escaneado el movimiento blanco: %s\n",movimientos_guardados[2*(contador_de_turnos-1)]);
        // Escaneo del movmiento negro
        while(texto[contador]!=' ' && contador_longitud_movimiento<20){
            movimientos_guardados[2*(contador_de_turnos-1)+1][contador_longitud_movimiento]=texto[contador];
            contador_longitud_movimiento++;
            contador++;
        }
        //printf("leer_PDN: he escaneado el movimiento negro: %s\n",movimientos_guardados[2*(contador_de_turnos-1)+1]);
        //printf("long %d ",contador_longitud_movimiento);
        contador_longitud_movimiento=0;
        contador++;// Porque se espera que haya un espacio
        contador_de_turnos++;
    }
    
    // Print 
    int k; 
    printf("\nPND parseado:\n");
    for(k=0;k<20;k++){printf("%d:\t%s\t%s\n",k+1,movimientos_guardados[2*k],movimientos_guardados[2*k+1]);}
    //printf("(%c)\n",texto[77]);
    
    
    
    return 1;
}

// declaracion de la funcion cantidad_en_cadena() para la funcion generar_movimientos
int cantidad_en_cadena(int tablero[n][n], int turno, mov movimiento, int print);

int generar_movimientos(int tablero[n][n], int turno, mov posibles_movimientos[MAX_MOV], int casilla_del_atacante[2], int print){
    // print=1 significa que imprima los movimientos y 0 que no
    // casilla del atacante tiene que ser por defecto {99,99} si no se quiere usar
    int k,i,j;
    int diferencia_y[4]={0,1,0,-1};// Esto es para controlar que si turno=1 entonces van hacia arriba y si turno=3 van hacia abajo
    int contador_de_posibles_movimientos=0;
    int movimiento_que_mata=0;
    
    // VARIABLES PARA GENERAR MOVIMIENTO DE LAS DAMAS
    int l;
    int longitud=1;// para aumentar la longitud de cada direccion
    int hay_pieza=0;// 1 si hay una pieza enemiga en el camino y 0 si no
    int dir[4][2]={{1,1},{1,-1},{-1,1},{-1,-1}};
    int casilla_victima[2]={99,99};
    
    // genero movimientos para el bando blanco
    if(turno==1){
        for(k=0;k<n;k++){
            for(i=0;i<n/2;i++){
                j=2*i+(k+1)%2;// para no buscar a traves de casillas que no se juega
                switch(tablero[k][j]){
                    case 0:
                    case 3:
                    case 4:
                        break;// vacio o pieza enemiga no dicen nada para generar
                    case 1: // pieza normal
                        // Si puede seguir avanzando
                        if(k-1>=0){
                            // Hacia la izquierda
                            if(j-1>=0){
                                switch(tablero[k-1][j-1]){
                                    // Si esta vacio es un movimiento
                                    case 0:
                                        posibles_movimientos[contador_de_posibles_movimientos].casilla_origen[0]=k;
                                        posibles_movimientos[contador_de_posibles_movimientos].casilla_origen[1]=j;
                                        posibles_movimientos[contador_de_posibles_movimientos].casilla_destino[0]=k-1;
                                        posibles_movimientos[contador_de_posibles_movimientos].casilla_destino[1]=j-1;
                                        posibles_movimientos[contador_de_posibles_movimientos].mata=0;
                                        //printf("generar_movimientos: movimiento posible nuermo %d:\n",contador_de_posibles_movimientos);
                                        //printf("(%d,%d)->(%d,%d)\n",k,j,k-1,j-1);
                                        //print_movimiento(posibles_movimientos[contador_de_posibles_movimientos]);
                                        contador_de_posibles_movimientos++;
                                        break;
                                    case 1:
                                    case 2:
                                        break;// si hay una pieza aliada no puede pasar
                                    case 3:
                                    case 4: // si hay una pieza enemiga hay que ver si se la puede comer 
                                        if(k-2>=0 && j-2>=0 && tablero[k-2][j-2]==0){
                                            posibles_movimientos[contador_de_posibles_movimientos].casilla_origen[0]=k;
                                            posibles_movimientos[contador_de_posibles_movimientos].casilla_origen[1]=j;
                                            posibles_movimientos[contador_de_posibles_movimientos].casilla_destino[0]=k-2;
                                            posibles_movimientos[contador_de_posibles_movimientos].casilla_destino[1]=j-2;
                                            posibles_movimientos[contador_de_posibles_movimientos].mata=1;
                                            posibles_movimientos[contador_de_posibles_movimientos].casilla_victima[0]=k-1;
                                            posibles_movimientos[contador_de_posibles_movimientos].casilla_victima[1]=j-1;
                                            movimiento_que_mata=1;
                                            //print_movimiento(posibles_movimientos[contador_de_posibles_movimientos]);
                                            //printf("generar_movimientos: movimiento posible nuermo %d:\n",contador_de_posibles_movimientos);
                                            //printf("(%d,%d)->(%d,%d)\n",k,j,k-2,j-2);
                                            //print_movimiento(posibles_movimientos[contador_de_posibles_movimientos]);
                                            contador_de_posibles_movimientos++;
                                            break;
                                        }

                                }
                            }
                            // Hacia la derecha
                            if(j+1<8){
                                switch(tablero[k-1][j+1]){
                                    // Si esta vacio es un movimiento
                                    case 0:
                                        posibles_movimientos[contador_de_posibles_movimientos].casilla_origen[0]=k;
                                        posibles_movimientos[contador_de_posibles_movimientos].casilla_origen[1]=j;
                                        posibles_movimientos[contador_de_posibles_movimientos].casilla_destino[0]=k-1;
                                        posibles_movimientos[contador_de_posibles_movimientos].casilla_destino[1]=j+1;
                                        posibles_movimientos[contador_de_posibles_movimientos].mata=0;
                                        //printf("generar_movimientos: movimiento posible nuermo %d:\n",contador_de_posibles_movimientos);
                                        //printf("(%d,%d)->(%d,%d)\n",k,j,k-1,j+1);
                                        //print_movimiento(posibles_movimientos[contador_de_posibles_movimientos]);
                                        contador_de_posibles_movimientos++;
                                        break;
                                    case 1:
                                    case 2:
                                        break;// si hay una pieza aliada no puede pasar
                                    case 3:
                                    case 4: // si hay una pieza enemiga hay que ver si se la puede comer 
                                        if(k-2>=0 && j+2<8 && tablero[k-2][j+2]==0){
                                            posibles_movimientos[contador_de_posibles_movimientos].casilla_origen[0]=k;
                                            posibles_movimientos[contador_de_posibles_movimientos].casilla_origen[1]=j;
                                            posibles_movimientos[contador_de_posibles_movimientos].casilla_destino[0]=k-2;
                                            posibles_movimientos[contador_de_posibles_movimientos].casilla_destino[1]=j+2;
                                            posibles_movimientos[contador_de_posibles_movimientos].mata=1;
                                            movimiento_que_mata=1;
                                            posibles_movimientos[contador_de_posibles_movimientos].casilla_victima[0]=k-1;
                                            posibles_movimientos[contador_de_posibles_movimientos].casilla_victima[1]=j+1;
                                            //printf("generar_movimientos: movimiento posible nuermo %d:\n",contador_de_posibles_movimientos);
                                            //printf("(%d,%d)->(%d,%d)\n",k,j,k-2,j+2);
                                            //print_movimiento(posibles_movimientos[contador_de_posibles_movimientos]);
                                            contador_de_posibles_movimientos++;
                                            break;
                                        }

                                }
                            }
                        }
                        // Si puede comer hacia atras
                        if(k+2<8){
                            // A la izquierda
                            if(j-2>=0 && (tablero[k+1][j-1]==3 || tablero[k+1][j-1]==4) && tablero[k+2][j-2]==0){
                                posibles_movimientos[contador_de_posibles_movimientos].casilla_origen[0]=k;
                                posibles_movimientos[contador_de_posibles_movimientos].casilla_origen[1]=j;
                                posibles_movimientos[contador_de_posibles_movimientos].casilla_destino[0]=k+2;
                                posibles_movimientos[contador_de_posibles_movimientos].casilla_destino[1]=j-2;
                                posibles_movimientos[contador_de_posibles_movimientos].mata=1;
                                movimiento_que_mata=1;
                                posibles_movimientos[contador_de_posibles_movimientos].casilla_victima[0]=k+1;
                                posibles_movimientos[contador_de_posibles_movimientos].casilla_victima[1]=j-1;
                                //printf("generar_movimientos: movimiento posible nuermo %d:\n",contador_de_posibles_movimientos);
                                //printf("(%d,%d)->(%d,%d)\n",k,j,k+2,j-2);
                                //print_movimiento(posibles_movimientos[contador_de_posibles_movimientos]);
                                contador_de_posibles_movimientos++;
                            }
                            // A la derecha 
                            if(j+2<8 && (tablero[k+1][j+1]==3 || tablero[k+1][j+1]==4) && tablero[k+2][j+2]==0){
                                posibles_movimientos[contador_de_posibles_movimientos].casilla_origen[0]=k;
                                posibles_movimientos[contador_de_posibles_movimientos].casilla_origen[1]=j;
                                posibles_movimientos[contador_de_posibles_movimientos].casilla_destino[0]=k+2;
                                posibles_movimientos[contador_de_posibles_movimientos].casilla_destino[1]=j+2;
                                posibles_movimientos[contador_de_posibles_movimientos].mata=1;
                                movimiento_que_mata=1;
                                posibles_movimientos[contador_de_posibles_movimientos].casilla_victima[0]=k+1;
                                posibles_movimientos[contador_de_posibles_movimientos].casilla_victima[1]=j+1;
                                //printf("generar_movimientos: movimiento posible nuermo %d:\n",contador_de_posibles_movimientos);
                                //printf("(%d,%d)->(%d,%d)\n",k,j,k+2,j+2);
                                //print_movimiento(posibles_movimientos[contador_de_posibles_movimientos]);
                                contador_de_posibles_movimientos++;
                            }
                        }
                        break;
                    case 2:
                        // VARIABLES DECLARADAS AL INICIO DE LA FUNCION
                        // por cada una de las direcciones
                        for(l=0;l<4;l++){
                            // mientras no se salga del tablero y no haya una a su lado
                            while(k+longitud*dir[l][0]>=0 && k+longitud*dir[l][0]<n  && j+longitud*dir[l][1]>=0 && j+longitud*dir[l][1]<n && hay_pieza==0){
                                // si hay una pieza que salga del bucle y se acuerde de la casilla
                                if(tablero[k+longitud*dir[l][0]][j+longitud*dir[l][1]]==3 || tablero[k+longitud*dir[l][0]][j+longitud*dir[l][1]]==4){
                                    //printf("generar_movimiento: he encontrado una pieza enemiga en (%d,%d)\n",k+longitud*dir[l][0],j+longitud*dir[l][1]);
                                    casilla_victima[0]=k+longitud*dir[l][0];
                                    casilla_victima[1]=j+longitud*dir[l][1];
                                    hay_pieza=1;
                                    break;
                                }
                                // si hay una pieza aliada que mire otra direccion
                                if(tablero[k+longitud*dir[l][0]][j+longitud*dir[l][1]]==1 || tablero[k+longitud*dir[l][0]][j+longitud*dir[l][1]]==2){break;}
                                // añadimos movimineto que no mata porque la casilla esta vacia
                                posibles_movimientos[contador_de_posibles_movimientos].casilla_origen[0]=k;
                                posibles_movimientos[contador_de_posibles_movimientos].casilla_origen[1]=j;
                                posibles_movimientos[contador_de_posibles_movimientos].casilla_destino[0]=k+longitud*dir[l][0];
                                posibles_movimientos[contador_de_posibles_movimientos].casilla_destino[1]=j+longitud*dir[l][1];
                                posibles_movimientos[contador_de_posibles_movimientos].mata=0;
                                contador_de_posibles_movimientos++;
                                longitud++;
                            }
                            // al salir del bucle significa que o bien hay una pieza que no deja seguir o que se salio del tablero
                            // si hay una pieza enemiga (hay_pieza=1) empezamos los movimientos de matar
                            if(hay_pieza==1){
                                longitud++;// porque en la casilla actual hay una pieza
                                // mientras no se salga del tablero
                                while(k+longitud*dir[l][0]>=0 && k+longitud*dir[l][0]<n  && j+longitud*dir[l][1]>=0 && j+longitud*dir[l][1]<n && tablero[k+longitud*dir[l][0]][j+longitud*dir[l][1]]==0 ){
                                    //printf("generar_movimiento: despues de encontrarme una pieza veo que la casilla (%d,%d) esta vacia\n",k+longitud*dir[l][0],j+longitud*dir[l][1]);
                                    // añadimos movimineto que no mata porque la casilla esta vacia
                                    posibles_movimientos[contador_de_posibles_movimientos].casilla_origen[0]=k;
                                    posibles_movimientos[contador_de_posibles_movimientos].casilla_origen[1]=j;
                                    posibles_movimientos[contador_de_posibles_movimientos].casilla_destino[0]=k+longitud*dir[l][0];
                                    posibles_movimientos[contador_de_posibles_movimientos].casilla_destino[1]=j+longitud*dir[l][1];
                                    posibles_movimientos[contador_de_posibles_movimientos].mata=1;
                                    movimiento_que_mata=1;
                                    posibles_movimientos[contador_de_posibles_movimientos].casilla_victima[0]=casilla_victima[0];
                                    posibles_movimientos[contador_de_posibles_movimientos].casilla_victima[1]=casilla_victima[1];
                                    contador_de_posibles_movimientos++;
                                    
                                    longitud++;
                                }
                            }
                            // preparamos para la nueva direccion
                            longitud=1;
                            hay_pieza=0;
                            casilla_victima[0]=99;
                            casilla_victima[1]=99;
                        }
                        break;
                    default:
                        printf("generar_movimientos: En la casilla (%d,%d) he encontrado un %d\n",k,j,tablero[k][j]);
                        if(printeado_error==0){imprimir_tablero(tablero);printeado_error=1;}
                        return -1;
                }
            }
        }
    }
    // genero movimientos para el bando negro
    else if(turno==3){
        for(k=0;k<n;k++){
            for(i=0;i<n/2;i++){
                j=2*i+(k+1)%2;// para no buscar a traves de casillas que no se juega
                switch(tablero[k][j]){
                    case 0:
                    case 1:
                    case 2:
                        break;// vacio o pieza enemiga no dicen nada para generar
                    case 3: // pieza normal
                        // Si puede seguir avanzando
                        if(k+1<8){
                            // Hacia la izquierda
                            if(j-1>=0){
                                switch(tablero[k+1][j-1]){
                                    // Si esta vacio es un movimiento
                                    case 0:
                                        posibles_movimientos[contador_de_posibles_movimientos].casilla_origen[0]=k;
                                        posibles_movimientos[contador_de_posibles_movimientos].casilla_origen[1]=j;
                                        posibles_movimientos[contador_de_posibles_movimientos].casilla_destino[0]=k+1;
                                        posibles_movimientos[contador_de_posibles_movimientos].casilla_destino[1]=j-1;
                                        posibles_movimientos[contador_de_posibles_movimientos].mata=0;
                                        //printf("generar_movimientos: movimiento posible nuermo %d:\n",contador_de_posibles_movimientos);
                                        //printf("(%d,%d)->(%d,%d)\n",k,j,k+1,j-1);
                                        //print_movimiento(posibles_movimientos[contador_de_posibles_movimientos]);
                                        contador_de_posibles_movimientos++;
                                        break;
                                    case 3:
                                    case 4:
                                        break;// si hay una pieza aliada no puede pasar
                                    case 1:
                                    case 2: // si hay una pieza enemiga hay que ver si se la puede comer 
                                        if(k+2<8 && j-2>=0 && tablero[k+2][j-2]==0){
                                            posibles_movimientos[contador_de_posibles_movimientos].casilla_origen[0]=k;
                                            posibles_movimientos[contador_de_posibles_movimientos].casilla_origen[1]=j;
                                            posibles_movimientos[contador_de_posibles_movimientos].casilla_destino[0]=k+2;
                                            posibles_movimientos[contador_de_posibles_movimientos].casilla_destino[1]=j-2;
                                            posibles_movimientos[contador_de_posibles_movimientos].mata=1;
                                            movimiento_que_mata=1;
                                            posibles_movimientos[contador_de_posibles_movimientos].casilla_victima[0]=k+1;
                                            posibles_movimientos[contador_de_posibles_movimientos].casilla_victima[1]=j-1;
                                            //print_movimiento(posibles_movimientos[contador_de_posibles_movimientos]);
                                            //printf("generar_movimientos: movimiento posible nuermo %d:\n",contador_de_posibles_movimientos);
                                            //printf("(%d,%d)->(%d,%d)\n",k,j,k+2,j-2);
                                            //print_movimiento(posibles_movimientos[contador_de_posibles_movimientos]);
                                            contador_de_posibles_movimientos++;
                                            break;
                                        }

                                }
                            }
                            // Hacia la derecha
                            if(j+1<8){
                                switch(tablero[k+1][j+1]){
                                    // Si esta vacio es un movimiento
                                    case 0:
                                        posibles_movimientos[contador_de_posibles_movimientos].casilla_origen[0]=k;
                                        posibles_movimientos[contador_de_posibles_movimientos].casilla_origen[1]=j;
                                        posibles_movimientos[contador_de_posibles_movimientos].casilla_destino[0]=k+1;
                                        posibles_movimientos[contador_de_posibles_movimientos].casilla_destino[1]=j+1;
                                        posibles_movimientos[contador_de_posibles_movimientos].mata=0;
                                        //printf("generar_movimientos: movimiento posible nuermo %d:\n",contador_de_posibles_movimientos);
                                        //printf("(%d,%d)->(%d,%d)\n",k,j,k+1,j+1);
                                        //print_movimiento(posibles_movimientos[contador_de_posibles_movimientos]);
                                        contador_de_posibles_movimientos++;
                                        break;
                                    case 3:
                                    case 4:
                                        break;// si hay una pieza aliada no puede pasar
                                    case 1:
                                    case 2: // si hay una pieza enemiga hay que ver si se la puede comer 
                                        if(k+2<8 && j+2<8 && tablero[k+2][j+2]==0){
                                            posibles_movimientos[contador_de_posibles_movimientos].casilla_origen[0]=k;
                                            posibles_movimientos[contador_de_posibles_movimientos].casilla_origen[1]=j;
                                            posibles_movimientos[contador_de_posibles_movimientos].casilla_destino[0]=k+2;
                                            posibles_movimientos[contador_de_posibles_movimientos].casilla_destino[1]=j+2;
                                            posibles_movimientos[contador_de_posibles_movimientos].mata=1;
                                            posibles_movimientos[contador_de_posibles_movimientos].casilla_victima[0]=k+1;
                                            posibles_movimientos[contador_de_posibles_movimientos].casilla_victima[1]=j+1;
                                            movimiento_que_mata=1;
                                            //printf("generar_movimientos: movimiento posible nuermo %d:\n",contador_de_posibles_movimientos);
                                            //printf("(%d,%d)->(%d,%d)\n",k,j,k+2,j+2);
                                            //print_movimiento(posibles_movimientos[contador_de_posibles_movimientos]);
                                            contador_de_posibles_movimientos++;
                                            break;
                                        }

                                }
                            }
                        }
                        // Si puede comer hacia atras
                        if(k-2>=0){
                            // A la izquierda
                            if(j-2>=0 && (tablero[k-1][j-1]==1 || tablero[k-1][j-1]==2) && tablero[k-2][j-2]==0){
                                posibles_movimientos[contador_de_posibles_movimientos].casilla_origen[0]=k;
                                posibles_movimientos[contador_de_posibles_movimientos].casilla_origen[1]=j;
                                posibles_movimientos[contador_de_posibles_movimientos].casilla_destino[0]=k-2;
                                posibles_movimientos[contador_de_posibles_movimientos].casilla_destino[1]=j-2;
                                posibles_movimientos[contador_de_posibles_movimientos].mata=1;
                                movimiento_que_mata=1;
                                posibles_movimientos[contador_de_posibles_movimientos].casilla_victima[0]=k-1;
                                posibles_movimientos[contador_de_posibles_movimientos].casilla_victima[1]=j-1;
                                //printf("generar_movimientos: movimiento posible nuermo %d:\n",contador_de_posibles_movimientos);
                                //printf("(%d,%d)->(%d,%d)\n",k,j,k-2,j-2);
                                //print_movimiento(posibles_movimientos[contador_de_posibles_movimientos]);
                                contador_de_posibles_movimientos++;
                            }
                            // A la derecha 
                            if(j+2<8 && (tablero[k-1][j+1]==1 || tablero[k-1][j+1]==2) && tablero[k-2][j+2]==0){
                                posibles_movimientos[contador_de_posibles_movimientos].casilla_origen[0]=k;
                                posibles_movimientos[contador_de_posibles_movimientos].casilla_origen[1]=j;
                                posibles_movimientos[contador_de_posibles_movimientos].casilla_destino[0]=k-2;
                                posibles_movimientos[contador_de_posibles_movimientos].casilla_destino[1]=j+2;
                                posibles_movimientos[contador_de_posibles_movimientos].mata=1;
                                movimiento_que_mata=1;
                                posibles_movimientos[contador_de_posibles_movimientos].casilla_victima[0]=k-1;
                                posibles_movimientos[contador_de_posibles_movimientos].casilla_victima[1]=j+1;
                                //printf("generar_movimientos: movimiento posible nuermo %d:\n",contador_de_posibles_movimientos);
                                //printf("(%d,%d)->(%d,%d)\n",k,j,k-2,j+2);
                                //print_movimiento(posibles_movimientos[contador_de_posibles_movimientos]);
                                contador_de_posibles_movimientos++;
                            }
                        }
                        break;
                    case 4:
                        // VARIABLES DECLARADAS AL INICIO DE LA FUNCION
                        // por cada una de las direcciones
                        for(l=0;l<4;l++){
                            // mientras no se salga del tablero y no haya una a su lado
                            while(k+longitud*dir[l][0]>=0 && k+longitud*dir[l][0]<n  && j+longitud*dir[l][1]>=0 && j+longitud*dir[l][1]<n && hay_pieza==0){
                                // si hay una pieza enemiga que salga del bucle y se acuerde de la casilla
                                if(tablero[k+longitud*dir[l][0]][j+longitud*dir[l][1]]==1 || tablero[k+longitud*dir[l][0]][j+longitud*dir[l][1]]==2){
                                    //printf("generar_movimiento: he encontrado una pieza enemiga en (%d,%d)\n",k+longitud*dir[l][0],j+longitud*dir[l][1]);
                                    casilla_victima[0]=k+longitud*dir[l][0];
                                    casilla_victima[1]=j+longitud*dir[l][1];
                                    hay_pieza=1;
                                    break;
                                }
                                // si hay una pieza aliada que mire otra direccion
                                if(tablero[k+longitud*dir[l][0]][j+longitud*dir[l][1]]==3 || tablero[k+longitud*dir[l][0]][j+longitud*dir[l][1]]==4){break;}
                                // añadimos movimineto que no mata porque la casilla esta vacia
                                posibles_movimientos[contador_de_posibles_movimientos].casilla_origen[0]=k;
                                posibles_movimientos[contador_de_posibles_movimientos].casilla_origen[1]=j;
                                posibles_movimientos[contador_de_posibles_movimientos].casilla_destino[0]=k+longitud*dir[l][0];
                                posibles_movimientos[contador_de_posibles_movimientos].casilla_destino[1]=j+longitud*dir[l][1];
                                posibles_movimientos[contador_de_posibles_movimientos].mata=0;
                                contador_de_posibles_movimientos++;
                                longitud++;
                            }
                            // al salir del bucle significa que o bien hay una pieza que no deja seguir o que se salio del tablero
                            // si hay una pieza enemiga (hay_pieza=1) empezamos los movimientos de matar
                            if(hay_pieza==1){
                                longitud++;// porque en la casilla actual hay una pieza
                                // mientras no se salga del tablero
                                while(k+longitud*dir[l][0]>=0 && k+longitud*dir[l][0]<n  && j+longitud*dir[l][1]>=0 && j+longitud*dir[l][1]<n && tablero[k+longitud*dir[l][0]][j+longitud*dir[l][1]]==0 ){
                                    //printf("generar_movimiento: despues de encontrarme una pieza veo que la casilla (%d,%d) esta vacia\n",k+longitud*dir[l][0],j+longitud*dir[l][1]);
                                    // añadimos movimineto que no mata porque la casilla esta vacia
                                    posibles_movimientos[contador_de_posibles_movimientos].casilla_origen[0]=k;
                                    posibles_movimientos[contador_de_posibles_movimientos].casilla_origen[1]=j;
                                    posibles_movimientos[contador_de_posibles_movimientos].casilla_destino[0]=k+longitud*dir[l][0];
                                    posibles_movimientos[contador_de_posibles_movimientos].casilla_destino[1]=j+longitud*dir[l][1];
                                    posibles_movimientos[contador_de_posibles_movimientos].mata=1;
                                    movimiento_que_mata=1;
                                    posibles_movimientos[contador_de_posibles_movimientos].casilla_victima[0]=casilla_victima[0];
                                    posibles_movimientos[contador_de_posibles_movimientos].casilla_victima[1]=casilla_victima[1];
                                    contador_de_posibles_movimientos++;
                                    
                                    longitud++;
                                }
                            }
                            // preparamos para la nueva direccion
                            longitud=1;
                            hay_pieza=0;
                            casilla_victima[0]=99;
                            casilla_victima[1]=99;
                        }
                        break;
                    default:
                        printf("generar_movimientos: En la casilla (%d,%d) he encontrado un %d\n",k,j,tablero[k][j]);
                        if(printeado_error==0){imprimir_tablero(tablero);printeado_error=1;}
                        return -1;
                }
            }
        }
    }
    else{printf("generar_movimientos: ERROR: turno=%d\n",turno); return -1;}
    if(contador_de_posibles_movimientos==0){return 0;}
    // Si hay un movimiento que mata entonces el resto no son legales
    int cantidad_de_movimientos_que_matan=0;
    if(movimiento_que_mata>0){
        if(movimiento_que_mata>1){printf("generar_movimientos: Es raro porque movimiento_que_mata=%d y deberia valer 1 o 0\n",movimiento_que_mata);}
        //printf("generar_movimientos: hay %d movimientos que matan\n",movimiento_que_mata);
        for(k=0;k<contador_de_posibles_movimientos;k++){
            if(posibles_movimientos[k].mata==1){
                posibles_movimientos[cantidad_de_movimientos_que_matan]=posibles_movimientos[k];
                cantidad_de_movimientos_que_matan++;
            }
        }
        contador_de_posibles_movimientos=cantidad_de_movimientos_que_matan;
        if(cantidad_de_movimientos_que_matan==0){
            printf("generar_movimientos: movimiento_que_mata>0 pero cantidad_de_movimientos_que_matan=0\n");
        }
    }
    int cantidad_normales=k+1;
    
    // Si en el anterior movimiento una ficha mato y ahora puede seguir matando tiene que hacerlo
    int matan_de_nuevo=0;
    if(casilla_del_atacante[0]!=99){
        if(tablero[casilla_del_atacante[0]][casilla_del_atacante[1]]!=turno && tablero[casilla_del_atacante[0]][casilla_del_atacante[1]]!=turno+1){
        printf("generar_movimientos: turno = %d y casilla_del_atacante=(%d,%d) en la cual hay un (%d)\n",turno,casilla_del_atacante[0],casilla_del_atacante[1],tablero[casilla_del_atacante[0]][casilla_del_atacante[1]]);
        }
        for(k=0;k<contador_de_posibles_movimientos;k++){
            if(posibles_movimientos[k].casilla_origen[0]==casilla_del_atacante[0] &&  posibles_movimientos[k].casilla_origen[1]==casilla_del_atacante[1]){
                posibles_movimientos[matan_de_nuevo]=posibles_movimientos[k];
                matan_de_nuevo++;
            }
        }
        contador_de_posibles_movimientos=matan_de_nuevo;
    }
  
  
    // de todos los movimientos legales que matan ( si hay) hay que quedarse solo con los que mas puedan matar en cadena
    int vector_cantidad_cadena[contador_de_posibles_movimientos];// para cada movimiento (legal hasta ahora) se guarda el numero de fichas en cadena que puede comer para despues dejar unicamente las que tengan el mayor numero
    // si hay mas de un movimiento que mata entonces hay que ver si con uno puede matar mas que con otro posteriomente
    int m=0;//almacena el numero maximo de en cadena que se puede comer algun movimiento (osea el maximo del vector de arriba)
    if(cantidad_de_movimientos_que_matan>1 && contador_de_posibles_movimientos>1 /*&& print==1*/){// estas dos comprobaciones son necesarias para asegurarse de que los movimientos que hay matan y que ademas todos son de la misma ficha (si fuera necesario)
      for(k=0;k<contador_de_posibles_movimientos;k++){
        if(posibles_movimientos[k].mata==0){printf("generar_movimientos(): ERROR: le estoy pasando un movimiento que no mata!\n");}
        vector_cantidad_cadena[k]=cantidad_en_cadena(tablero,turno,posibles_movimientos[k],0);
        if(vector_cantidad_cadena[k]>m){m=vector_cantidad_cadena[k];}
      }
      int matan_mucho=0;// esta variable guardara la cantidad de movimimientos legales ifnales
      for(k=0;k<contador_de_posibles_movimientos;k++){
        if(vector_cantidad_cadena[k]==m){
          posibles_movimientos[matan_mucho]=posibles_movimientos[k];
          matan_mucho++;
        }
      }
      contador_de_posibles_movimientos=matan_mucho;
    }
  
  
  
  
    if(print==1){
        printf("generar_movimientos: El jugador (%d) puede hacer (%d) movimientos en esta posicion:\n",turno,contador_de_posibles_movimientos);
        for(k=0;k<contador_de_posibles_movimientos;k++){
            printf("mov %d: (%d,%d) -> (%d,%d). %s mata\n",k+1,posibles_movimientos[k].casilla_origen[0],posibles_movimientos[k].casilla_origen[1],posibles_movimientos[k].casilla_destino[0],posibles_movimientos[k].casilla_destino[1],Pns[posibles_movimientos[k].mata]);
        }
    }
    
    // prints para facilitar el debug
    for(k=0;k<contador_de_posibles_movimientos;k++){
        if(posibles_movimientos[k].mata==1 && tablero[posibles_movimientos[k].casilla_victima[0]][posibles_movimientos[k].casilla_victima[1]]==0){
            print_movimiento("generar_movimientos:(movimiento que mata pero victima=0)",posibles_movimientos[k],0);
        }
        if(posibles_movimientos[k].mata==1 && (posibles_movimientos[k].casilla_victima[0]>7 || posibles_movimientos[k].casilla_victima[1]>7)){
            print_movimiento("generar_movimientos:(casilla de la victima fuera del tablero)",posibles_movimientos[k],0);
        }
        if(posibles_movimientos[k].mata==0 && casilla_del_atacante[0]!=99){
            printf("generar_movimientos(): ERROR: se paso un atacante en la casilla (%d,%d) pero el movimiento generado no mata\n",casilla_del_atacante[0],casilla_del_atacante[1]);
            print_movimiento("este no mata:",posibles_movimientos[k],0);
        }
    }
    /*if(contador_de_posibles_movimientos==0 && (cantidad_de_movimientos_que_matan>0 || movimiento_que_mata>0)){
        printf("generar_movimientos: En la siguiente posicion:\n");
        imprimir_tablero(tablero);
        printf("generar_movimientos: hubieron %d movimientos totales de los cuales %d mataban y %d de los cuales los puede hacer la pieza atacante situada en (%d,%d)\n",cantidad_normales,cantidad_de_movimientos_que_matan,matan_de_nuevo,casilla_del_atacante[0],casilla_del_atacante[1]);
        for(k=0;k<cantidad_normales;k++){
            printf("mov %d: (%d,%d) -> (%d,%d). %s mata\n",k+1,posibles_movimientos[k].casilla_origen[0],posibles_movimientos[k].casilla_origen[1],posibles_movimientos[k].casilla_destino[0],posibles_movimientos[k].casilla_destino[1],Pns[posibles_movimientos[k].mata]);
        }
        printf("generar_movimientos: la funcion puede_matar en esta posicion dice:\n");
        puede_matar(tablero,turno,casilla_del_atacante,0);
    }*/
  
    //if(print==1){printf("generar_movimientos(): salgo de la funcion\n");}
    return contador_de_posibles_movimientos;
}

int cantidad_en_cadena(int tablero[n][n], int turno, mov movimiento, int print){
  if(print==1){
    print_movimiento("cantidad_en_cadena(): ",movimiento,0);
  }
  //asumo que ya se que le movimiento mata
  // para debug
  if(movimiento.mata==0){printf("cantidad_en_cadena(): ERROR!! movimiento no mata\n");exit(0);}
  // si al matar corona entonces ya no sigue
  if(tablero[movimiento.casilla_origen[0]][movimiento.casilla_origen[1]]==turno || tablero[movimiento.casilla_origen[0]][movimiento.casilla_origen[1]]==turno+1){}
  else{
    printf("cantidad_en_cadena(): ERROR!! turno=%d y en la casilla origen=(%d,%d) hay un %d\n",turno,movimiento.casilla_origen[0],movimiento.casilla_origen[1],tablero[movimiento.casilla_origen[0]][movimiento.casilla_origen[1]]);
    imprimir_tablero(tablero);
    exit(0);
  }
  //bajo las siguiente dos condiciones le movimiento corona y no puede seguir matando en cadena
  if(turno==1 && movimiento.casilla_destino[0]==0){return 0;}
  if(turno==3 && movimiento.casilla_destino[0]==n-1){return 0;}
  
  
  //int comidas_en_cadena=0;
  int victima=tablero[movimiento.casilla_victima[0]][movimiento.casilla_victima[1]];//guardo la victima para posteriormente deshacer el movimiento
  tablero[movimiento.casilla_destino[0]][movimiento.casilla_destino[1]]=tablero[movimiento.casilla_origen[0]][movimiento.casilla_origen[1]];
  tablero[movimiento.casilla_origen[0]][movimiento.casilla_origen[1]]=0;
  tablero[movimiento.casilla_victima[0]][movimiento.casilla_victima[1]]=0;
  int atacante[2];// gurda la casilla destino del mov para pasarsela a la funcion generar_movimientos() para que sepa quien ha sido el atacante
  atacante[0]=movimiento.casilla_destino[0];
  atacante[1]=movimiento.casilla_destino[1];
  mov movimientos_generados[MAX_MOV];// para llamar recursivamente a la funcion generar_movimiento
  int p=puede_matar(tablero,turno,atacante,0);
  int cantidad_de_movimientos=0;// cantridad d emovimientos generados para el proximo movimiento
  int m=0;//acumula el maximo de las que puede comer en cadena en funcion de las distintas vias. sera el valor que devuelva la funcion
  if(p==1){
    cantidad_de_movimientos=generar_movimientos(tablero,turno,movimientos_generados,atacante,0);
    if(cantidad_de_movimientos>0){
      for(int k=0;k<cantidad_de_movimientos;k++){
        if(movimientos_generados[k].mata==0){printf("cantidad_en_cadena(): ERROR: se ha generado un movimiento que no mata\n");imprimir_tablero(tablero);print_movimiento("movimiento malo:",movimientos_generados[k],0);print_movimiento("esta posicion viene precedida por el movimiento:",movimiento,0);exit(0);}
        if(movimientos_generados[k].casilla_origen[0]!=movimiento.casilla_destino[0] || movimientos_generados[k].casilla_origen[1]!=movimiento.casilla_destino[1]){printf("cantidad_en_cadena(): ERROR: se ha generado un movimiento de otra ficha!\n");exit(0);}
        int c=cantidad_en_cadena(tablero,turno,movimientos_generados[k],0);
        if(c>m){m=c;}
      }
    }
    else{printf("cantidad_en_cadena(): ERROR: puede matar pero hay 0 movimiento generados\n");}
  }

  
  tablero[movimiento.casilla_origen[0]][movimiento.casilla_origen[1]]=tablero[movimiento.casilla_destino[0]][movimiento.casilla_destino[1]];
  tablero[movimiento.casilla_destino[0]][movimiento.casilla_destino[1]]=0;
  tablero[movimiento.casilla_victima[0]][movimiento.casilla_victima[1]]=victima;
  if(cantidad_de_movimientos==0 || p==0){if(print==1){printf("Tiene %d comidas en cadena\n",0);}return 0;}
  if(print==1){printf("Tiene %d comidas en cadena\n",m+1);}
  return m+1;
}

// hay que vigilar con esta funcion cuando hagamos el juegue con negras por el tema de devolver 100 cuando no haya fichas
double valorar_posicion(int tablero[n][n], int turno){
    
    if(sin_fichas(tablero,(turno+2)%4)==1){return -100*(turno-2);}
    int k,i,j;
    double material_blanco=0;
    double material_negro=0;
    for(k=0;k<n;k++){
        for(i=0;i<n/2;i++){
            j=2*i+(k+1)%2;// para no buscar a traves de casillas que no se juega
            //printf("miro la casilla (%d,%d). ",k,j);
            switch(tablero[k][j]){
                case 0:
                    //printf("esta vacia");
                    break;
                case 1:
                    //printf("hay una blanca");
                    material_blanco++;
                    break;
                case 2:
                    //printf("hay una dama blanca");
                    material_blanco=material_blanco+3;
                    break;
                case 3:
                    //printf("hay una negra");
                    material_negro++;
                    break;
                case 4:
                    //printf("hay una dama negra");
                    material_negro=material_negro+3;
                    break;
                default:
                    printf("valorar_posicion: En la casilla (%d,%d) he encontrado un %d\n",k,j,tablero[k][j]);
                    if(printeado_error==0){imprimir_tablero(tablero);printeado_error=1;}
                    
            }
            //printf("\n");
        }
    }
    //printf("valorar_posicion:\tmaterial blanco: %lf\tmaterial negro: %lf\n",material_blanco,material_negro);
    contador++;
    //printf("%'lu\r",contador);
    //fflush(stdout);
    return material_blanco-material_negro;
}

// hay que vigilar con esta funcion cuando hagamos el juegue con negras por el tema de devolver 100 cuando no haya fichas
double valorar_posicion2(int tablero[n][n], int turno){
    
    if(sin_fichas(tablero,(turno+2)%4)==1){return -100*(turno-2);}
    int k,i,j;
    float ponderaciones_blancas[8][8]={
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.3, 0.0, 0.3, 0.0, 0.3, 0.0, 0.3, 0.0,
        0.0, 0.2, 0.0, 0.2, 0.0, 0.2, 0.0, 0.0,
        0.1, 0.0, 0.2, 0.0, 0.2, 0.0, 0.1, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
    };
    double material_blanco=0;
    double material_negro=0;
    for(k=0;k<n;k++){
        for(i=0;i<n/2;i++){
            j=2*i+(k+1)%2;// para no buscar a traves de casillas que no se juega
            //printf("miro la casilla (%d,%d). ",k,j);
            switch(tablero[k][j]){
                case 0:
                    //printf("esta vacia");
                    break;
                case 1:
                    //printf("hay una blanca");
                    material_blanco=material_blanco+1+ponderaciones_blancas[k][j];
                    break;
                case 2:
                    //printf("hay una dama blanca");
                    material_blanco=material_blanco+3;
                    break;
                case 3:
                    //printf("hay una negra");
                    // Lo de [7-k][7-j] es para invertir la posicion y poder usar la misma tabla que las blancas
                    material_negro=material_negro+1+ponderaciones_blancas[7-k][7-j];
                    break;
                case 4:
                    //printf("hay una dama negra");
                    material_negro=material_negro+3;
                    break;
                default:
                    printf("valorar_posicion: En la casilla (%d,%d) he encontrado un %d\n",k,j,tablero[k][j]);
					imprimir_tablero(tablero);exit(0);
                    if(printeado_error==0){imprimir_tablero(tablero);printeado_error=1;}
                    
            }
            //printf("\n");
        }
    }
    //printf("valorar_posicion:\tmaterial blanco: %lf\tmaterial negro: %lf\n",material_blanco,material_negro);
    contador++;
    //printf("%'lu\r",contador);
    //fflush(stdout);
    return material_blanco-material_negro;
}

double valorar_posicion3(int tablero[n][n], int turno){
    
    if(sin_fichas(tablero,(turno+2)%4)==1){return -100*(turno-2);}
    int k,i,j;
    float ponderaciones_blancas[8][8]={
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.3, 0.0, 0.3, 0.0, 0.3, 0.0, 0.3, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.3,
        0.3, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.1,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
    };
    double material_blanco=0;
    double material_negro=0;
    for(k=0;k<n;k++){
        for(i=0;i<n/2;i++){
            j=2*i+(k+1)%2;// para no buscar a traves de casillas que no se juega
            //printf("miro la casilla (%d,%d). ",k,j);
            switch(tablero[k][j]){
                case 0:
                    //printf("esta vacia");
                    break;
                case 1:
                    //printf("hay una blanca");
                    material_blanco=material_blanco+1+ponderaciones_blancas[k][j];
                    break;
                case 2:
                    //printf("hay una dama blanca");
                    material_blanco=material_blanco+3;
                    break;
                case 3:
                    //printf("hay una negra");
                    // Lo de [7-k][7-j] es para invertir la posicion y poder usar la misma tabla que las blancas
                    material_negro=material_negro+1+ponderaciones_blancas[7-k][7-j];
                    break;
                case 4:
                    //printf("hay una dama negra");
                    material_negro=material_negro+3;
                    break;
                default:
                    printf("valorar_posicion: En la casilla (%d,%d) he encontrado un %d\n",k,j,tablero[k][j]);
                    if(printeado_error==0){imprimir_tablero(tablero);printeado_error=1;}
                    
            }
            //printf("\n");
        }
    }
    //printf("valorar_posicion:\tmaterial blanco: %lf\tmaterial negro: %lf\n",material_blanco,material_negro);
    contador++;
    //printf("%'lu\r",contador);
    //fflush(stdout);
    return material_blanco-material_negro;
}

// Esta funcion se usa para cambiar entre las funciones de valorar posicion para compararlas
double comparador_valorar_posicion(int tablero[n][n], int turno, int jugador){
  return valorar_posicion2(tablero,turno);
  if(jugador==1){
    return valorar_posicion2(tablero,turno)+0.01*(rand()%9);
  }
  return valorar_posicion2(tablero,turno)+0.01*(rand()%9);
}

// declaracion de la funcion max para que la funcion min tambien la pueda llamar
// las funciones sin poda tienen un bug ocn el tema de la coronacion. en el resto esta corregido
mov max_sinpoda(int tablero[n][n], int turno, int casilla_del_atacante[2], int print, int profundidad, int jugador);

mov min_sinpoda(int tablero[n][n], int turno, int casilla_del_atacante[2], int print, int profundidad, int jugador){
    mov movimientos[MAX_MOV];
    int k;
    int cantidad_movimientos;
    int mueve,victima=99;// para luego deshacer el movimiento
    double valor_min=101;// pongo este numero para que siempre haya un movimiento con una mejor valoracion que -101
    int indice_min=0;
    mov mejor_respuesta={.casilla_origen={99,99},.casilla_destino={99,99},.mata=2,.casilla_victima={99,99},.valor=-101};// para el caso profundidad>1
    mov mejor_movimiento={.casilla_origen={99,99},.casilla_destino={99,99},.mata=2,.casilla_victima={99,99},.valor=-101};// para el return 
    int l;//indice para el print
    int acaba_de_coronar=0;// esta variable se usa para saber si se acaba de coronar porque sino tras el movimiento y la coronacion la funcion puede_matar() sira que si que puede volver a matar pero en este caso tiene que cambiar de turno
    int movimientos_similares[MAX_MOV];// Este vector almacenara los indices de los movimientos con un valor igual al valor del mejor movimiento. Se usara para posteriormente escoger uno de manera aleatoria entre ellos.
    int cantidad_de_movimientos_similares=0;// indice para el vector anterior.
    if((depth-profundidad)>max_depth_showing){print=0;}
    cantidad_movimientos=generar_movimientos(tablero,turno,movimientos,casilla_del_atacante,print);
    if(cantidad_movimientos==0){
        if(casilla_del_atacante[0]!=99 && printeado_error==0){printf("min(%d): En la siguiente posicion la funcion puede_matar dice que (%d,%d) puede volver a matar pero la funcion generar_movimientos no encuentra ningun movimiento de matar para esa ficha\n",profundidad,casilla_del_atacante[0],casilla_del_atacante[1]);imprimir_tablero(tablero);printeado_error=1;}
        // ESTO HABRA QUE MODIFICARLO EN FUNCION DEL ORDEN EN EL QUE LLAME A MIN MAX CUANDO JUEGEN CON NEGRAS
        mejor_movimiento.valor=101;
        return mejor_movimiento;
    }
    
    // por cada uno de los moviminetos posibles de la posicion
    for(k=0;k<cantidad_movimientos;k++){
        if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
        printf("min(%d): miro el movimiento (%d,%d) -> (%d,%d), %s mata\n",profundidad,movimientos[k].casilla_origen[0],movimientos[k].casilla_origen[1],movimientos[k].casilla_destino[0],movimientos[k].casilla_destino[1],Pns[movimientos[k].mata]);}
        // hacer el movimiento
        mueve=tablero[movimientos[k].casilla_origen[0]][movimientos[k].casilla_origen[1]];
        acaba_de_coronar=0;
        if(mueve==3){acaba_de_coronar=corona(movimientos[k].casilla_destino,tablero,turno,movimientos[k].mata);}
        tablero[movimientos[k].casilla_origen[0]][movimientos[k].casilla_origen[1]]=0;
        tablero[movimientos[k].casilla_destino[0]][movimientos[k].casilla_destino[1]]=mueve+acaba_de_coronar;
        if(mueve+acaba_de_coronar>4){printf("min(%d): mueve=%d, acaba_de_coronar=%d\n",profundidad,mueve,acaba_de_coronar);}
        //printf("min: he movido\n");
        if(movimientos[k].mata==1){
            victima=tablero[movimientos[k].casilla_victima[0]][movimientos[k].casilla_victima[1]];
            tablero[movimientos[k].casilla_victima[0]][movimientos[k].casilla_victima[1]]=0;
        }
        // evaluarlo en funcion de la profundidad
        if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
        printf("min(%d): evaluo\n",profundidad);}
        if(profundidad==1){
            // PARA ESCOGER QUE FUNCION SE QUIERE USAR PARA VALORAR. CHAPUZA MOMENTANEA
            if(jugador==1){movimientos[k].valor=valorar_posicion2(tablero,turno);}
            else{movimientos[k].valor=valorar_posicion2(tablero,turno);}
            if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
            printf("min(%d): el movimiento numero %d tiene una valoracion de %lf\n",profundidad,k,movimientos[k].valor);}
        }
        if(profundidad>1){
            //si hay mas profundidad pero ya no le quedan piezas pues ya esta. habria que tener en cuenta tambien si no puede mover
            if(valorar_posicion(tablero,turno)==-100){
                // deshacer el movimiento
                tablero[movimientos[k].casilla_origen[0]][movimientos[k].casilla_origen[1]]=mueve;
                tablero[movimientos[k].casilla_destino[0]][movimientos[k].casilla_destino[1]]=0;
                if(movimientos[k].mata==1){
                    tablero[movimientos[k].casilla_victima[0]][movimientos[k].casilla_victima[1]]=victima;
                }
                mejor_movimiento=movimientos[k];
                mejor_movimiento.valor=-100;
                if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
                printf("min(%d): el movimiento numero %d tiene una valoracion de %lf\n",profundidad,k,mejor_movimiento.valor);}
                return mejor_movimiento;
            }
            // depende si puede volver a matar ( y si ha matado si lo puede hacer con la misma pieza)
            if(puede_matar(tablero,turno,movimientos[k].casilla_destino,0)==1 && acaba_de_coronar==0 && movimientos[k].mata==1){
                if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
                printf("min(%d): vuelvo a llamar a la min() porque puedo volver a comer\n",profundidad);}
                casilla_del_atacante[0]=movimientos[k].casilla_destino[0];
                casilla_del_atacante[1]=movimientos[k].casilla_destino[1];
                mejor_respuesta=min_sinpoda(tablero,turno,casilla_del_atacante,print,profundidad-1,jugador);
                if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
                printf("min(%d): he vuelto de la funcion min()\n",profundidad);}
            }
            else{
                if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
                printf("min(%d): llamo a la funcion max()\n",profundidad);}
                //printf("min(%d): llamo a la funcion max()\n",profundidad);
                // cambio de turno
                turno=(turno+2)%4;
                casilla_del_atacante[0]=99;
                casilla_del_atacante[1]=99;
                mejor_respuesta=max_sinpoda(tablero,turno,casilla_del_atacante,print,profundidad-1,jugador);
                turno=(turno+2)%4;
                if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
                printf("min(%d): vuelvo de la funcion max()\n",profundidad);}
            }
            
            movimientos[k].valor=mejor_respuesta.valor;
        }
        
        if(movimientos[k].valor<valor_min){
            if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
            printf("min(%d): como %lf<%lf entonces el %d-essimo movimiento generado es el mejor por ahora\n",profundidad,movimientos[k].valor,valor_min,k);}
            valor_min=movimientos[k].valor;
            indice_min=k;
            //Limpio los antiguos movimientos similares y meto el mejor
            movimientos_similares[0]=k;
            cantidad_de_movimientos_similares=1;
        }
        else{
            if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
            printf("min(%d): como %lf>=%lf entonces descarto el %d-essimo movimiento\n",profundidad,movimientos[k].valor,valor_min,k);}
            //Si es un movimiento con puntuacion igual lo guardo
            if(movimientos[k].valor==valor_min){
                movimientos_similares[cantidad_de_movimientos_similares]=k;
                cantidad_de_movimientos_similares++;
            }
        }
        // deshacer el movimiento
        tablero[movimientos[k].casilla_origen[0]][movimientos[k].casilla_origen[1]]=mueve;
        tablero[movimientos[k].casilla_destino[0]][movimientos[k].casilla_destino[1]]=0;
        if(movimientos[k].mata==1){
            tablero[movimientos[k].casilla_victima[0]][movimientos[k].casilla_victima[1]]=victima;
        }
        if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
        printf("min(%d): he deshecho el movimiento\n",profundidad);}
    }
    if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
    printf("min(%d): el mejor movimiento para el bando %d es el %d-essimo movimiento\n",profundidad,turno,indice_min);}
    // si la profundidad es 1 solo necesito el valor
  
    //Escojo uno aleatorio
    int aleatorio=0;
    if(cantidad_de_movimientos_similares>0){
      //aleatorio=(rand()%cantidad_de_movimientos_similares);
      //aleatorio=NUM%cantidad_de_movimientos_similares;
    }
    else{
        printf("min(%d): ERROR: cantidad_de_movimientos_similares=%d=0\n",profundidad,cantidad_de_movimientos_similares);
    }
    
    return movimientos[movimientos_similares[aleatorio]];
    
}

mov max_sinpoda(int tablero[n][n], int turno, int casilla_del_atacante[2], int print, int profundidad, int jugador){
    mov movimientos[MAX_MOV];// aqui se guardaran los posibles movimientos de la posicion
    int k;
    int cantidad_movimientos;// cantidad de movimientos generados de la posicion
    int mueve,victima=99;// para luego deshacer el movimiento
    double valor_max=-101;// pongo este numero para que siempre haya un movimiento con una mejor valoracion de -101 y por lo tanto escogera cualquier movimiento en lugar de este
    int indice_max=0;
    mov mejor_respuesta={.casilla_origen={99,99},.casilla_destino={99,99},.mata=2,.casilla_victima={99,99},.valor=-101};// para el caso profundidad>1
    mov mejor_movimiento={.casilla_origen={99,99},.casilla_destino={99,99},.mata=2,.casilla_victima={99,99},.valor=-101};// para el return 
    int l;//indice para el print
    int acaba_de_coronar=0;// esta variable se usa para saber si se acaba de coronar porque sino tras el movimiento y la coronacion la funcion puede_matar() dira que si que puede volver a matar pero en este caso tiene que cambiar de turno
    // for(l=0;0<depth-profundidad;l++){printf("\t");}
    // if(print==1){printf("max(%d): ",profundidad);}
    int movimientos_similares[MAX_MOV];// Este vector almacenara los indices de los movimientos con un valor igual al valor del mejor movimiento. Se usara para posteriormente escoger uno de manera aleatoria entre ellos.
    int cantidad_de_movimientos_similares=0;// indice para el vector anterior.
    if((depth-profundidad)>max_depth_showing){print=0;}
    cantidad_movimientos=generar_movimientos(tablero,turno,movimientos,casilla_del_atacante,print);
    if(cantidad_movimientos==0){
        if(casilla_del_atacante[0]!=99 && printeado_error==0){printf("max(%d): En la siguiente posicion la funcion puede_matar dice que (%d,%d) puede volver a matar pero la funcion generar_movimientos no encuentra ningun movimiento de matar para esa ficha\n",profundidad,casilla_del_atacante[0],casilla_del_atacante[1]);imprimir_tablero(tablero);printeado_error=1;}

        // ESTO HABRA QUE MODIFICARLO EN FUNCION DEL ORDEN EN EL QUE LLAME A MIN MAX CUANDO JUEGEN CON NEGRAS
        mejor_movimiento.valor=-101;
        return mejor_movimiento;
    }
    // por cada uno de los moviminetos posibles de la posicion
    for(k=0;k<cantidad_movimientos;k++){
        if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
        printf("max(%d): miro el movimiento (%d,%d) -> (%d,%d), %s mata\n",profundidad,movimientos[k].casilla_origen[0],movimientos[k].casilla_origen[1],movimientos[k].casilla_destino[0],movimientos[k].casilla_destino[1],Pns[movimientos[k].mata]);}
        // hacer el movimiento
        mueve=tablero[movimientos[k].casilla_origen[0]][movimientos[k].casilla_origen[1]];
        acaba_de_coronar=0;
        if(mueve==1){acaba_de_coronar=corona(movimientos[k].casilla_destino,tablero,turno,movimientos[k].mata);}
        tablero[movimientos[k].casilla_origen[0]][movimientos[k].casilla_origen[1]]=0;
        tablero[movimientos[k].casilla_destino[0]][movimientos[k].casilla_destino[1]]=mueve+acaba_de_coronar;
        if(mueve+acaba_de_coronar>4){printf("max(%d): mueve=%d, acaba_de_coronar=%d\n",profundidad,mueve,acaba_de_coronar);}
        if(movimientos[k].mata==1){
            victima=tablero[movimientos[k].casilla_victima[0]][movimientos[k].casilla_victima[1]];
            //if(victima!=1 && victima!=2 && victima!=3 && victima!=4){printf("max(%d): ERROR: victima=(%d)\n",profundidad,victima);print_movimiento(" ",movimientos[k],0);}
            tablero[movimientos[k].casilla_victima[0]][movimientos[k].casilla_victima[1]]=0;
        }
        // evaluarlo en funcion de la profundidad
        if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
        printf("max(%d): evaluo\n",profundidad);}
        if(profundidad==1){
            // PARA ESCOGER QUE FUNCION SE QUIERE USAR PARA VALORAR. CHAPUZA MOMENTANEA
            if(jugador==1){movimientos[k].valor=valorar_posicion2(tablero,turno);}
            else{movimientos[k].valor=valorar_posicion2(tablero,turno);}
            //imprimir_tablero(tablero);
            if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
            printf("max(%d): el movimiento numero %d tiene una valoracion de %lf\n",profundidad,k,movimientos[k].valor);}
        }
        if(profundidad>1){
            //si hay mas profundidad pero ya no le quedan piezas pues ya esta. habria que tener en cuenta tambien si no puede mover
            if(valorar_posicion(tablero,turno)==100){
                // deshacer el movimiento
                tablero[movimientos[k].casilla_origen[0]][movimientos[k].casilla_origen[1]]=mueve;
                tablero[movimientos[k].casilla_destino[0]][movimientos[k].casilla_destino[1]]=0;
                if(movimientos[k].mata==1){
                    tablero[movimientos[k].casilla_victima[0]][movimientos[k].casilla_victima[1]]=victima;
                }
                mejor_movimiento=movimientos[k];
                mejor_movimiento.valor=100;
                if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
                printf("max(%d): el movimiento numero %d tiene una valoracion de %lf\n",profundidad,k,mejor_movimiento.valor);}
                return mejor_movimiento;
            }
            // depende si puede volver a matar ( y si ha matado si lo puede hacer con la misma pieza)
            if(puede_matar(tablero,turno,movimientos[k].casilla_destino,0)==1 && acaba_de_coronar==0 && movimientos[k].mata==1){
                if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
                printf("max(%d): vuelvo a llamar a la max() porque puedo volver a comer\n",profundidad);}
                casilla_del_atacante[0]=movimientos[k].casilla_destino[0];
                casilla_del_atacante[1]=movimientos[k].casilla_destino[1];
                mejor_respuesta=max_sinpoda(tablero,turno,casilla_del_atacante,print,profundidad-1,jugador);
                //printf("HOLAAAAAAAAAAAA2\n");
                if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
                printf("max(%d): he vuelto de la funcion max()\n",profundidad);}
            }
            else{
                if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
                printf("max(%d): llamo a la funcion min()\n",profundidad);}
                // cambio de turno
                turno=(turno+2)%4;
                casilla_del_atacante[0]=99;
                casilla_del_atacante[1]=99;
                mejor_respuesta=min_sinpoda(tablero,turno,casilla_del_atacante,print,profundidad-1,jugador);
                turno=(turno+2)%4;
                if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
                printf("max(%d): vuelvo de la funcion min()\n",profundidad);}
            }
            
            movimientos[k].valor=mejor_respuesta.valor;
        }
        
        if(movimientos[k].valor>valor_max){
            if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
            printf("max(%d): como %lf>%lf entonces el %d-essimo movimiento generado es el mejor por ahora\n",profundidad,movimientos[k].valor,valor_max,k);}
            valor_max=movimientos[k].valor;
            indice_max=k;
            //Limpio los antiguos movimientos similares y meto el mejor
            movimientos_similares[0]=k;
            cantidad_de_movimientos_similares=1;
        }
        else{
            if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
            printf("max(%d): como %lf<=%lf entonces descarto el %d-essimo movimiento\n",profundidad,movimientos[k].valor,valor_max,k);}
            //Si es un movimiento con puntuacion igual lo guardo
            if(movimientos[k].valor==valor_max){
                movimientos_similares[cantidad_de_movimientos_similares]=k;
                cantidad_de_movimientos_similares++;
            }
        }
        // deshacer el movimiento
        tablero[movimientos[k].casilla_origen[0]][movimientos[k].casilla_origen[1]]=mueve;
        tablero[movimientos[k].casilla_destino[0]][movimientos[k].casilla_destino[1]]=0;
        if(movimientos[k].mata==1){
            tablero[movimientos[k].casilla_victima[0]][movimientos[k].casilla_victima[1]]=victima;
        }
        if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
        printf("max(%d): he deshecho el movimiento\n",profundidad);}
    }
    if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
    printf("max(%d): el mejor movimiento para el bando %d es el %d-essimo movimiento\n",profundidad,turno,indice_max);}
    
    //Escojo uno aleatorio
    int aleatorio=0;
    if(cantidad_de_movimientos_similares>0){
      //aleatorio=(rand()%cantidad_de_movimientos_similares);
      //aleatorio=NUM%cantidad_de_movimientos_similares;
    }
    else{
        printf("max(%d): ERROR: cantidad_de_movimientos_similares=%d=0\n",profundidad,cantidad_de_movimientos_similares);
    }
    /*if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
    printf("max(%d): cantidad_de_movimientos_similares=%d\t aleatorio=%d\n",profundidad,cantidad_de_movimientos_similares,aleatorio);}
    if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
    print_movimiento("Movmiento escogido:",movimientos[movimientos_similares[aleatorio]],1);}*/
  
    //SOLO PARA DEBUGEAR
    /*printf("Hay %d movimientos con el mismo valor\n",cantidad_de_movimientos_similares);
    for(k=0;k<cantidad_de_movimientos_similares;k++){
        print_movimiento("movimiento numero ",movimientos[movimientos_similares[k]],1);
    }
    printf("El numero aleatorio ha sido el %d\n",aleatorio);*/
    /*printf("HOLAAAAAAAAAAAA1 movimientos_similares[aleatorio]=%d\n",movimientos_similares[aleatorio]);
    if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
    print_movimiento("Movmiento0:",movimientos[0],1);}
    if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
    print_movimiento("Movmiento1:",movimientos[1],1);}
    if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
    print_movimiento("Movmiento2:",movimientos[2],1);}
    mov movimiento_final=movimientos[movimientos_similares[aleatorio]];*/
    return movimientos[movimientos_similares[aleatorio]];
}

mov computadora_sinpoda(int tablero[n][n], int turno, int casilla_del_atacante[2], int profundidad, int print){
    
    
    //Algoritmo:
    //llama a la funcion max(). esta funcion llamara a la funcion generar_movimientos() y por cada movimiento lo movera evaluara y lo deshara para probar con el siguiente movimiento
    //la funcion max(), a su vez, llamara a la funcion min() (dependera de la variable depth) y "hara lo mismo"
    
    
    
    clock_t comienzo=clock();
    // DE AQUI PARA ABAJO TIENE QUE IR TODO EL CODIGO
    int casilla_guardada[2];// para guardar la casilla del atcante
    casilla_guardada[0]=casilla_del_atacante[0];
    casilla_guardada[1]=casilla_del_atacante[1];
    mov mejor_movimiento={.casilla_origen={99,99},.casilla_destino={99,99},.mata=2,.casilla_victima={99,99},.valor=-101};// para el return 
    if(print==1){printf("turno=%d, computadora(%d): empiezo, llamo a max()\n",turno,profundidad);}
    if(turno==1){mejor_movimiento=max_sinpoda(tablero,turno,casilla_del_atacante,print,profundidad,1);}
    if(turno==3){mejor_movimiento=min_sinpoda(tablero,turno,casilla_del_atacante,print,profundidad,2);}
    /*if(show_best_move==1){
      char cabecero[80];
      strcpy(cabecero,"computadora(");
      //char prof_char[1]=profundidad+'0';
      char prof_char[2];
      sprintf(prof_char, "%d", profundidad);
      strcat(cabecero,prof_char);
      strcat(cabecero,"): mejor mov");
      print_movimiento(cabecero,mejor_movimiento,1);
    }*/
    
    // dejo intacta la casilla del atacante
    casilla_del_atacante[0]=casilla_guardada[0];
    casilla_del_atacante[1]=casilla_guardada[1];
    // DE AQUI PARA ARRIBA TIENE QUE IR TODO EL CODIGO
    clock_t final=clock();
    double tiempo_invertido=(double)(final-comienzo)/CLOCKS_PER_SEC;
    if(engine_plays_as!=5 || show_best_move==1){
        printf("computadora(%d): ",profundidad);print_movimiento("mejor mov",mejor_movimiento,1);
        printf("\ttiempo invertido %.2lf segundos\n",tiempo_invertido);
        printf("\tposiciones calculadas totales: %'lu\n", contador);
        printf("\tposiciones calculadas por segundo: %'d\n", (int)(contador/tiempo_invertido));
    }
    contador = 0;
    return mejor_movimiento;
}

// declaracion de la funcion max para que la funcion min tambien la pueda llamar
mov max_conpoda(int tablero[n][n], int turno, int casilla_del_atacante[2], int print, int profundidad, int jugador, double alfa, double beta);

mov min_conpoda(int tablero[n][n], int turno, int casilla_del_atacante[2], int print, int profundidad, int jugador, double alfa, double beta){
    mov movimientos[MAX_MOV];
    int k;
    int cantidad_movimientos;
    int mueve,victima=99;// para luego deshacer el movimiento
    double valor_min=200;// pongo este numero para que siempre haya un movimiento con una mejor valoracion que 200
    int indice_min=0;
    mov mejor_respuesta={.casilla_origen={99,99},.casilla_destino={99,99},.mata=2,.casilla_victima={99,99},.valor=-101};// para el caso profundidad>1
    mov mejor_movimiento={.casilla_origen={99,99},.casilla_destino={99,99},.mata=2,.casilla_victima={99,99},.valor=-101};// para el return 
    int l;//indice para el print
    int acaba_de_coronar=0;// esta variable se usa para saber si se acaba de coronar porque sino tras el movimiento y la coronacion la funcion puede_matar() sira que si que puede volver a matar pero en este caso tiene que cambiar de turno
    int movimientos_similares[MAX_MOV];// Este vector almacenara los indices de los movimientos con un valor igual al valor del mejor movimiento. Se usara para posteriormente escoger uno de manera aleatoria entre ellos.
    int cantidad_de_movimientos_similares=0;// indice para el vector anterior.
    if((depth-profundidad)>max_depth_showing){print=0;}
    cantidad_movimientos=generar_movimientos(tablero,turno,movimientos,casilla_del_atacante,print);
    if(cantidad_movimientos==0){
        if(casilla_del_atacante[0]!=99 && printeado_error==0){printf("min(%d): En la siguiente posicion la funcion puede_matar dice que (%d,%d) puede volver a matar pero la funcion generar_movimientos no encuentra ningun movimiento de matar para esa ficha\n",profundidad,casilla_del_atacante[0],casilla_del_atacante[1]);imprimir_tablero(tablero);printeado_error=1;}
        // ESTO HABRA QUE MODIFICARLO EN FUNCION DEL ORDEN EN EL QUE LLAME A MIN MAX CUANDO JUEGEN CON NEGRAS
        mejor_movimiento.valor=101+profundidad;
        return mejor_movimiento;
    }
    
    // por cada uno de los moviminetos posibles de la posicion
    for(k=0;k<cantidad_movimientos;k++){
        if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
        printf("min(%d): miro el movimiento (%d,%d) -> (%d,%d), %s mata\n",profundidad,movimientos[k].casilla_origen[0],movimientos[k].casilla_origen[1],movimientos[k].casilla_destino[0],movimientos[k].casilla_destino[1],Pns[movimientos[k].mata]);}
        // hacer el movimiento
        mueve=tablero[movimientos[k].casilla_origen[0]][movimientos[k].casilla_origen[1]];
        tablero[movimientos[k].casilla_origen[0]][movimientos[k].casilla_origen[1]]=0;
        tablero[movimientos[k].casilla_destino[0]][movimientos[k].casilla_destino[1]]=mueve;
        if(movimientos[k].mata==1){
            victima=tablero[movimientos[k].casilla_victima[0]][movimientos[k].casilla_victima[1]];
            tablero[movimientos[k].casilla_victima[0]][movimientos[k].casilla_victima[1]]=0;
        }
        acaba_de_coronar=0;
        if(mueve==3){acaba_de_coronar=corona(movimientos[k].casilla_destino,tablero,turno,movimientos[k].mata);}
        if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
        printf("min(%d): el movimiento %s corona.\n",profundidad,Pns[acaba_de_coronar]);}
        if(acaba_de_coronar==1){tablero[movimientos[k].casilla_destino[0]][movimientos[k].casilla_destino[1]]=mueve+1;}
        if(mueve+acaba_de_coronar>4){printf("min(%d): mueve=%d, acaba_de_coronar=%d\n",profundidad,mueve,acaba_de_coronar);}
        //printf("min: he movido\n");
        // evaluarlo en funcion de la profundidad
        if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
        printf("min(%d): evaluo\n",profundidad);}
        if(profundidad==1){
            // PARA ESCOGER QUE FUNCION SE QUIERE USAR PARA VALORAR. CHAPUZA MOMENTANEA
            /*if(jugador==1){movimientos[k].valor=valorar_posicion2(tablero,turno);}
            else{movimientos[k].valor=valorar_posicion2(tablero,turno);}*/
            movimientos[k].valor=comparador_valorar_posicion(tablero,turno,jugador);
            if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
            printf("min(%d): el movimiento numero %d tiene una valoracion de %lf\n",profundidad,k,movimientos[k].valor);}
        }
        if(profundidad>1){
            //si hay mas profundidad pero ya no le quedan piezas pues ya esta. habria que tener en cuenta tambien si no puede mover
            if(valorar_posicion(tablero,turno)==-100){
                // deshacer el movimiento
                tablero[movimientos[k].casilla_origen[0]][movimientos[k].casilla_origen[1]]=mueve;
                tablero[movimientos[k].casilla_destino[0]][movimientos[k].casilla_destino[1]]=0;
                if(movimientos[k].mata==1){
                    tablero[movimientos[k].casilla_victima[0]][movimientos[k].casilla_victima[1]]=victima;
                }
                mejor_movimiento=movimientos[k];
                mejor_movimiento.valor=-100-profundidad;
                if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
                printf("min(%d): el movimiento numero %d tiene una valoracion de %lf\n",profundidad,k,mejor_movimiento.valor);}
                return mejor_movimiento;
            }
            // depende si puede volver a matar ( y si ha matado si lo puede hacer con la misma pieza)
            if(puede_matar(tablero,turno,movimientos[k].casilla_destino,0)==1 && acaba_de_coronar==0 && movimientos[k].mata==1){
                if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
                printf("min(%d): vuelvo a llamar a la min() porque puedo volver a comer\n",profundidad);}
                casilla_del_atacante[0]=movimientos[k].casilla_destino[0];
                casilla_del_atacante[1]=movimientos[k].casilla_destino[1];
                mejor_respuesta=min_conpoda(tablero,turno,casilla_del_atacante,print,profundidad-1,jugador,-200,200);
                if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
                printf("min(%d): he vuelto de la funcion min()\n",profundidad);}
            }
            else{
                if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
                printf("min(%d): Como no puede matar, coronar=%d y mata=%d, llamo a la funcion max()\n",profundidad,acaba_de_coronar,movimientos[k].mata);}
                //printf("min(%d): llamo a la funcion max()\n",profundidad);
                // cambio de turno
                turno=(turno+2)%4;
                casilla_del_atacante[0]=99;
                casilla_del_atacante[1]=99;
                mejor_respuesta=max_conpoda(tablero,turno,casilla_del_atacante,print,profundidad-1,jugador,alfa,beta);
                turno=(turno+2)%4;
                if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
                printf("min(%d): vuelvo de la funcion max()\n",profundidad);}
            }
            
            movimientos[k].valor=mejor_respuesta.valor;
        }
        if(movimientos[k].valor<beta){
            beta=movimientos[k].valor;
        }
        if(beta<=alfa){
            // PODA
            //mejor_movimiento.valor=movimientos[k].valor;
            // deshacer el movimiento
            tablero[movimientos[k].casilla_origen[0]][movimientos[k].casilla_origen[1]]=mueve;
            tablero[movimientos[k].casilla_destino[0]][movimientos[k].casilla_destino[1]]=0;
            if(movimientos[k].mata==1){
                tablero[movimientos[k].casilla_victima[0]][movimientos[k].casilla_victima[1]]=victima;
            }
            //movimientos[k].valor=-99;
            return movimientos[k];
        }
        if(movimientos[k].valor<valor_min){
            if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
            printf("min(%d): como %lf<%lf entonces el %d-essimo movimiento generado es el mejor por ahora\n",profundidad,movimientos[k].valor,valor_min,k);}
            valor_min=movimientos[k].valor;
            indice_min=k;
            //Limpio los antiguos movimientos similares y meto el mejor
            movimientos_similares[0]=k;
            cantidad_de_movimientos_similares=1;
        }
        else{
            if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
            printf("min(%d): como %lf>=%lf entonces descarto el %d-essimo movimiento\n",profundidad,movimientos[k].valor,valor_min,k);}
            //Si es un movimiento con puntuacion igual lo guardo
            if(movimientos[k].valor==valor_min){
                movimientos_similares[cantidad_de_movimientos_similares]=k;
                cantidad_de_movimientos_similares++;
            }
        }
        // deshacer el movimiento
        tablero[movimientos[k].casilla_origen[0]][movimientos[k].casilla_origen[1]]=mueve;
        tablero[movimientos[k].casilla_destino[0]][movimientos[k].casilla_destino[1]]=0;
        if(movimientos[k].mata==1){
            tablero[movimientos[k].casilla_victima[0]][movimientos[k].casilla_victima[1]]=victima;
        }
        if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
        printf("min(%d): he deshecho el movimiento\n",profundidad);}
    }
    if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
    printf("min(%d): el mejor movimiento para el bando %d es el %d-essimo movimiento\n",profundidad,turno,indice_min);}
    // si la profundidad es 1 solo necesito el valor
  
    //Escojo uno aleatorio
    int aleatorio=0;
    if(cantidad_de_movimientos_similares>0){
      //aleatorio=(rand()%cantidad_de_movimientos_similares);
      //aleatorio=NUM%cantidad_de_movimientos_similares;
    }
    else{
        printf("min(%d): ERROR: cantidad_de_movimientos_similares=%d=0\n",profundidad,cantidad_de_movimientos_similares);
    }
    
    return movimientos[movimientos_similares[aleatorio]];
    
}

mov max_conpoda(int tablero[n][n], int turno, int casilla_del_atacante[2], int print, int profundidad, int jugador, double alfa, double beta){
    mov movimientos[MAX_MOV];// aqui se guardaran los posibles movimientos de la posicion
    int k;
    int cantidad_movimientos;// cantidad de movimientos generados de la posicion
    int mueve,victima=99;// para luego deshacer el movimiento
    double valor_max=-200;// pongo este numero para que siempre haya un movimiento con una mejor valoracion de -101 y por lo tanto escogera cualquier movimiento en lugar de este
    int indice_max=0;
    mov mejor_respuesta={.casilla_origen={99,99},.casilla_destino={99,99},.mata=2,.casilla_victima={99,99},.valor=-101};// para el caso profundidad>1
    mov mejor_movimiento={.casilla_origen={99,99},.casilla_destino={99,99},.mata=2,.casilla_victima={99,99},.valor=-101};// para el return 
    int l;//indice para el print
    int acaba_de_coronar=0;// esta variable se usa para saber si se acaba de coronar porque sino tras el movimiento y la coronacion la funcion puede_matar() dira que si que puede volver a matar pero en este caso tiene que cambiar de turno
    // for(l=0;0<depth-profundidad;l++){printf("\t");}
    // if(print==1){printf("max(%d): ",profundidad);}
    int movimientos_similares[MAX_MOV];// Este vector almacenara los indices de los movimientos con un valor igual al valor del mejor movimiento. Se usara para posteriormente escoger uno de manera aleatoria entre ellos.
    int cantidad_de_movimientos_similares=0;// indice para el vector anterior.
    if((depth-profundidad)>max_depth_showing){print=0;}
    cantidad_movimientos=generar_movimientos(tablero,turno,movimientos,casilla_del_atacante,print);
    if(cantidad_movimientos==0){
        if(casilla_del_atacante[0]!=99 && printeado_error==0){printf("max(%d): En la siguiente posicion la funcion puede_matar dice que (%d,%d) puede volver a matar pero la funcion generar_movimientos no encuentra ningun movimiento de matar para esa ficha\n",profundidad,casilla_del_atacante[0],casilla_del_atacante[1]);imprimir_tablero(tablero);printeado_error=1;}

        // ESTO HABRA QUE MODIFICARLO EN FUNCION DEL ORDEN EN EL QUE LLAME A MIN MAX CUANDO JUEGEN CON NEGRAS
        mejor_movimiento.valor=-101-profundidad;
        return mejor_movimiento;
    }
    // por cada uno de los moviminetos posibles de la posicion
    for(k=0;k<cantidad_movimientos;k++){
        if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
        printf("max(%d): miro el movimiento (%d,%d) -> (%d,%d), %s mata\n",profundidad,movimientos[k].casilla_origen[0],movimientos[k].casilla_origen[1],movimientos[k].casilla_destino[0],movimientos[k].casilla_destino[1],Pns[movimientos[k].mata]);}
        // hacer el movimiento
        mueve=tablero[movimientos[k].casilla_origen[0]][movimientos[k].casilla_origen[1]];
        tablero[movimientos[k].casilla_origen[0]][movimientos[k].casilla_origen[1]]=0;
        tablero[movimientos[k].casilla_destino[0]][movimientos[k].casilla_destino[1]]=mueve;
        if(movimientos[k].mata==1){
            victima=tablero[movimientos[k].casilla_victima[0]][movimientos[k].casilla_victima[1]];
            //if(victima!=1 && victima!=2 && victima!=3 && victima!=4){printf("max(%d): ERROR: victima=(%d)\n",profundidad,victima);print_movimiento(" ",movimientos[k],0);}
            tablero[movimientos[k].casilla_victima[0]][movimientos[k].casilla_victima[1]]=0;
        }
        acaba_de_coronar=0;
        if(mueve==1){acaba_de_coronar=corona(movimientos[k].casilla_destino,tablero,turno,movimientos[k].mata);}
        if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
        printf("max(%d): el movimiento %s corona.\n",profundidad,Pns[acaba_de_coronar]);}
        if(acaba_de_coronar==1){tablero[movimientos[k].casilla_destino[0]][movimientos[k].casilla_destino[1]]=mueve+1;}
        if(mueve+acaba_de_coronar>4){printf("max(%d): mueve=%d, acaba_de_coronar=%d\n",profundidad,mueve,acaba_de_coronar);}
        // evaluarlo en funcion de la profundidad
        if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
        printf("max(%d): evaluo\n",profundidad);}
        if(profundidad==1){
            // PARA ESCOGER QUE FUNCION SE QUIERE USAR PARA VALORAR. CHAPUZA MOMENTANEA
            /*if(jugador==1){movimientos[k].valor=valorar_posicion2(tablero,turno);}
            else{movimientos[k].valor=valorar_posicion2(tablero,turno);}*/
            movimientos[k].valor=comparador_valorar_posicion(tablero,turno,jugador);
            //imprimir_tablero(tablero);
            if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
            printf("max(%d): el movimiento numero %d tiene una valoracion de %lf\n",profundidad,k,movimientos[k].valor);}
        }
        if(profundidad>1){
            //si hay mas profundidad pero ya no le quedan piezas pues ya esta. habria que tener en cuenta tambien si no puede mover
            if(valorar_posicion(tablero,turno)==100){
                // deshacer el movimiento
                tablero[movimientos[k].casilla_origen[0]][movimientos[k].casilla_origen[1]]=mueve;
                tablero[movimientos[k].casilla_destino[0]][movimientos[k].casilla_destino[1]]=0;
                if(movimientos[k].mata==1){
                    tablero[movimientos[k].casilla_victima[0]][movimientos[k].casilla_victima[1]]=victima;
                }
                mejor_movimiento=movimientos[k];
                mejor_movimiento.valor=100+profundidad;
                if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
                printf("max(%d): el movimiento numero %d tiene una valoracion de %lf\n",profundidad,k,mejor_movimiento.valor);}
                return mejor_movimiento;
            }
            // depende si puede volver a matar ( y si ha matado si lo puede hacer con la misma pieza)
            if(puede_matar(tablero,turno,movimientos[k].casilla_destino,0)==1 && acaba_de_coronar==0 && movimientos[k].mata==1){
                if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
                printf("max(%d): vuelvo a llamar a la max() porque puedo volver a comer\n",profundidad);}
                casilla_del_atacante[0]=movimientos[k].casilla_destino[0];
                casilla_del_atacante[1]=movimientos[k].casilla_destino[1];
                mejor_respuesta=max_conpoda(tablero,turno,casilla_del_atacante,print,profundidad-1,jugador,-200,200);
                //printf("HOLAAAAAAAAAAAA2\n");
                if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
                printf("max(%d): he vuelto de la funcion max()\n",profundidad);}
            }
            else{
                if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
                printf("max(%d): Como no puede matar, coronar=%d y mata=%d, llamo a la funcion min()\n",profundidad,acaba_de_coronar,movimientos[k].mata);}
                // cambio de turno
                turno=(turno+2)%4;
                casilla_del_atacante[0]=99;
                casilla_del_atacante[1]=99;
                mejor_respuesta=min_conpoda(tablero,turno,casilla_del_atacante,print,profundidad-1,jugador,alfa,beta);
                turno=(turno+2)%4;
                if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
                printf("max(%d): vuelvo de la funcion min()\n",profundidad);}
            }
            
            movimientos[k].valor=mejor_respuesta.valor;
        }
        if(movimientos[k].valor>alfa){
            alfa=movimientos[k].valor;
        }
        if(beta<=alfa){
            // PODA
            //mejor_movimiento->valor=movimientos[k].valor;
            // deshacer el movimiento
            tablero[movimientos[k].casilla_origen[0]][movimientos[k].casilla_origen[1]]=mueve;
            tablero[movimientos[k].casilla_destino[0]][movimientos[k].casilla_destino[1]]=0;
            if(movimientos[k].mata==1){
                tablero[movimientos[k].casilla_victima[0]][movimientos[k].casilla_victima[1]]=victima;
            }
            //movimientos[k].valor=99;
            return movimientos[k];
        }
        if(movimientos[k].valor>valor_max){
            if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
            printf("max(%d): como %lf>%lf entonces el %d-essimo movimiento generado es el mejor por ahora\n",profundidad,movimientos[k].valor,valor_max,k);}
            valor_max=movimientos[k].valor;
            indice_max=k;
            //Limpio los antiguos movimientos similares y meto el mejor
            movimientos_similares[0]=k;
            cantidad_de_movimientos_similares=1;
        }
        else{
            if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
            printf("max(%d): como %lf<=%lf entonces descarto el %d-essimo movimiento\n",profundidad,movimientos[k].valor,valor_max,k);}
            //Si es un movimiento con puntuacion igual lo guardo
            if(movimientos[k].valor==valor_max){
                movimientos_similares[cantidad_de_movimientos_similares]=k;
                cantidad_de_movimientos_similares++;
            }
        }
        // deshacer el movimiento
        tablero[movimientos[k].casilla_origen[0]][movimientos[k].casilla_origen[1]]=mueve;
        tablero[movimientos[k].casilla_destino[0]][movimientos[k].casilla_destino[1]]=0;
        if(movimientos[k].mata==1){
            tablero[movimientos[k].casilla_victima[0]][movimientos[k].casilla_victima[1]]=victima;
        }
        if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
        printf("max(%d): he deshecho el movimiento\n",profundidad);}
    }
    if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
    printf("max(%d): el mejor movimiento para el bando %d es el %d-essimo movimiento\n",profundidad,turno,indice_max);}
    
    //Escojo uno aleatorio
    int aleatorio=0;
    if(cantidad_de_movimientos_similares>0){
      //aleatorio=(rand()%cantidad_de_movimientos_similares);
      //aleatorio=NUM%cantidad_de_movimientos_similares;
    }
    else{
        printf("max(%d): ERROR: cantidad_de_movimientos_similares=%d=0\n",profundidad,cantidad_de_movimientos_similares);
    }
    /*if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
    printf("max(%d): cantidad_de_movimientos_similares=%d\t aleatorio=%d\n",profundidad,cantidad_de_movimientos_similares,aleatorio);}
    if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
    print_movimiento("Movmiento escogido:",movimientos[movimientos_similares[aleatorio]],1);}*/
  
    //SOLO PARA DEBUGEAR
    /*printf("Hay %d movimientos con el mismo valor\n",cantidad_de_movimientos_similares);
    for(k=0;k<cantidad_de_movimientos_similares;k++){
        print_movimiento("movimiento numero ",movimientos[movimientos_similares[k]],1);
    }
    printf("El numero aleatorio ha sido el %d\n",aleatorio);*/
    /*printf("HOLAAAAAAAAAAAA1 movimientos_similares[aleatorio]=%d\n",movimientos_similares[aleatorio]);
    if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
    print_movimiento("Movmiento0:",movimientos[0],1);}
    if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
    print_movimiento("Movmiento1:",movimientos[1],1);}
    if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
    print_movimiento("Movmiento2:",movimientos[2],1);}
    mov movimiento_final=movimientos[movimientos_similares[aleatorio]];*/
    return movimientos[movimientos_similares[aleatorio]];
}

mov computadora_conpoda(int tablero[n][n], int turno, int casilla_del_atacante[2], int profundidad, int print){
    
    
    //Algoritmo:
    //llama a la funcion max(). esta funcion llamara a la funcion generar_movimientos() y por cada movimiento lo movera evaluara y lo deshara para probar con el siguiente movimiento
    //la funcion max(), a su vez, llamara a la funcion min() (dependera de la variable depth) y "hara lo mismo"
    
    
    
    clock_t comienzo=clock();
    // DE AQUI PARA ABAJO TIENE QUE IR TODO EL CODIGO
    int casilla_guardada[2];// para guardar la casilla del atcante
    casilla_guardada[0]=casilla_del_atacante[0];
    casilla_guardada[1]=casilla_del_atacante[1];
    mov mejor_movimiento={.casilla_origen={99,99},.casilla_destino={99,99},.mata=2,.casilla_victima={99,99},.valor=-101};// para el return 
    mov movimientos[MAX_MOV];
    if(generar_movimientos(tablero,turno,movimientos,casilla_del_atacante,0)==1){
      if(show_best_move==1){
        printf("computadora(%d): devuelvo el unico movimiento disponible\n",profundidad);
        print_movimiento("",movimientos[0],0);
      }
      return movimientos[0];
    }
    if(print==1){printf("turno=%d, computadora(%d): empiezo, llamo a max()\n",turno,profundidad);}
    if(turno==1){mejor_movimiento=max_conpoda(tablero,turno,casilla_del_atacante,print,profundidad,1,-200.0,200.0);}
    if(turno==3){mejor_movimiento=min_conpoda(tablero,turno,casilla_del_atacante,print,profundidad,2,-200.0,200.0);}
    if(show_best_move==1 && 1==0){
      char cabecero[80];
      strcpy(cabecero,"computadora(");
      //char prof_char[1]=profundidad+'0';
      char prof_char[2];
      sprintf(prof_char, "%d", profundidad);
      strcat(cabecero,prof_char);
      strcat(cabecero,"): mejor mov");
      print_movimiento(cabecero,mejor_movimiento,1);
    }
    
    // dejo intacta la casilla del atacante
    casilla_del_atacante[0]=casilla_guardada[0];
    casilla_del_atacante[1]=casilla_guardada[1];
    // DE AQUI PARA ARRIBA TIENE QUE IR TODO EL CODIGO
    clock_t final=clock();
    double tiempo_invertido=(double)(final-comienzo)/CLOCKS_PER_SEC;
    if(engine_plays_as!=5 || show_best_move==1){
        printf("computadora(%d): ",profundidad);print_movimiento("mejor mov",mejor_movimiento,1);
        printf("\ttiempo invertido %.2lf segundos\n",tiempo_invertido);
        printf("\tposiciones calculadas totales: %'lu\n", contador);
        printf("\tposiciones calculadas por segundo: %'d\n", (int)(contador/tiempo_invertido));
    }
    contador = 0;
    return mejor_movimiento;
}

// FUNCIONES DE TVP
unsigned long long random_llu(){
  //printf("RAND_MAX=%d\n",RAND_MAX);//4 bytes=32 bits
  unsigned long long r=random();
  return r|(random()<<32);
}

void inicializar_hash(unsigned long long xor_hash[8*4][4]){
  //8*4=32 casillas en las que puede haber piezas * 4 tipos de piezas
  for(int k=0;k<8*4;k++){
    for(int i=0;i<4;i++){
      xor_hash[k][i]=random_llu();
    }
  }
}

unsigned long long t_a_hash(int tablero[n][n], int turno, unsigned long long xor_hash[8*4][4]){
  // falta que el hash tenga en cuenta el turno y seguramente la casilla del atacante
  unsigned long long hash=0;
  int j;
  for(int k=0;k<8;k++){
    for(int i=0;i<4;i++){
      j=2*i+(k+1)%2;
      if(tablero[k][j]==0){continue;}
      hash=hash^xor_hash[4*k+i][tablero[k][j]-1];
    }
  }
  hash=hash^(unsigned long long)turno;
  return hash;
}

void limpiar_TVP(TVP *tabla){
  mov nulo={.casilla_origen={99,99},.casilla_destino={99,99},.mata=2,.casilla_victima={99,99},.valor=-101};
  for(int k=0;k<tabla->num_entradas;k++){
    tabla->tabla[k].hash_key=(unsigned long long)(0);
    tabla->tabla[k].movimiento=nulo;
  }
  return ;
}

void inicializar_TVP(TVP *tabla){
  int tamano_TVP=0x1000000*20;// 2MB
  tabla->num_guardadas=0;
  tabla->num_entradas=(tamano_TVP/sizeof(TVP_pos))-2;
  free(tabla->tabla);
  tabla->tabla=(TVP_pos*)malloc(tabla->num_entradas*sizeof(TVP_pos));
  limpiar_TVP(tabla);
  //printf("inicializar_tabla(): La tabla tiene %d/%lu = %d entradas\n",tamano_TVP,sizeof(TVP_pos),tabla->num_entradas);
}

void guardar_en_TVP(TVP *tabla, unsigned long long hash_key, mov movimiento){
  int indice=hash_key%(tabla->num_entradas);
  tabla->tabla[indice].hash_key=hash_key;
  tabla->tabla[indice].movimiento=movimiento;
  tabla->num_guardadas++;
  //printf("guardar_en_TVP(): Se ha guardado en el indice %d el hash: %llX y el movimiento:\n",indice,hash_key);
  //print_movimiento("",movimiento,1);
}

mov buscar_en_TVP(TVP *tabla, unsigned long long hash_key){
  //return -1;
  // en profundidad 4 hasta el 28 no difiere
  
  //10
  //{3,4,3,3,3,3,3,4,4,4,5,5,2,4,4,2,1,0,3,3,4,4,2,2,2,0,2,0,0,6,2,2,5,5,5,5} 2.3 seg
  
  //11
  //{3,4,3,3,4,3,3,3,2,2,4,4,1,0,2,4,4,2,5,5,3,3,4,4,2,2,2,2,1,1,1,1,6,6,7,0,0,7,6,6,7,5} 2.7 seg
  //{3,4,3,3,4,3,3,3,2,2,4,4,1,0,2,4,4,2,5,5,3,3,4,4,2,2,2,2,1,1,1,1,6,6,7,0,0,7,6,6,7,5} 20.8 seg
  
  //12
  //{3,4,3,4,3,3,4,3,3,4,2,4,4,5,1,0,5,3,3,6,7,1,4,4,1,1,1,1,1,1,0,7,7,7,7,7,0,0,7,7,0,0,0,0,6,6,6,6,6,6,6,2} 14.6 seg
  //{3,4,3,4,3,3,4,3,3,4,2,4,4,5,1,0,5,3,3,6,7,1,4,4,1,1,1,1,1,1,0,7,7,7,0,0,6,6,6,6,6,6,6,0,0,0,0,7,7,7,7,2} 66.5 seg
  //return -1;
  int indice=hash_key%(tabla->num_entradas);
  if(tabla->tabla[indice].hash_key==hash_key){
    //printf("buscar_en_TVP(): He encontrado la columna %d para el hash: %llX\n",tabla->tabla[indice].columna,hash_key);
    return tabla->tabla[indice].movimiento;
  }
  mov nulo={.casilla_origen={99,99},.casilla_destino={99,99},.mata=2,.casilla_victima={99,99},.valor=-101};
  return nulo;
}

int ordenar_movimientos(int cantidad_movimientos, mov movimientos[MAX_MOV], mov m){
  int indice=-1;
  for(int k=0;k<cantidad_movimientos;k++){
    if(comparador_movimientos(movimientos[k],m)==1){
      if(k==0){return 1;}
      indice=k;
      break;
    }
  }
  if(indice==-1){
    /*printf("ordenar_movimientos(): No se ha encontrado el movimiento para ordenar\n");
    print_movimiento("ordenar_movimientos(): intercalar:",m,1);
    printf("ordenar_movimientos(): Lista de movimientos:\n");
    for(int k=0;k<cantidad_movimientos;k++){
      print_movimiento("",movimientos[k],1);
    }*/
    //exit(0);
    return -1;
  }
  // los intercambio con ayuda de un auxiliar
  mov aux;
  aux.casilla_origen[0]=movimientos[0].casilla_origen[0];
  aux.casilla_origen[1]=movimientos[0].casilla_origen[1];
  aux.casilla_destino[0]=movimientos[0].casilla_destino[0];
  aux.casilla_destino[1]=movimientos[0].casilla_destino[1];
  aux.mata=movimientos[0].mata;
  aux.casilla_victima[0]=movimientos[0].casilla_victima[0];
  aux.casilla_victima[1]=movimientos[0].casilla_victima[1];
  aux.valor=movimientos[0].valor;
  
  movimientos[0].casilla_origen[0]=m.casilla_origen[0];
  movimientos[0].casilla_origen[1]=m.casilla_origen[1];
  movimientos[0].casilla_destino[0]=m.casilla_destino[0];
  movimientos[0].casilla_destino[1]=m.casilla_destino[1];
  movimientos[0].mata=m.mata;
  movimientos[0].casilla_victima[0]=m.casilla_victima[0];
  movimientos[0].casilla_victima[1]=m.casilla_victima[1];
  movimientos[0].valor=m.valor;
  
  movimientos[indice].casilla_origen[0]=aux.casilla_origen[0];
  movimientos[indice].casilla_origen[1]=aux.casilla_origen[1];
  movimientos[indice].casilla_destino[0]=aux.casilla_destino[0];
  movimientos[indice].casilla_destino[1]=aux.casilla_destino[1];
  movimientos[indice].mata=aux.mata;
  movimientos[indice].casilla_victima[0]=aux.casilla_victima[0];
  movimientos[indice].casilla_victima[1]=aux.casilla_victima[1];
  movimientos[indice].valor=aux.valor;
  
  return -1;
}

// declaracion de la funcion max para que la funcion min tambien la pueda llamar
mov max(int tablero[n][n], int turno, int casilla_del_atacante[2], int print, int profundidad, int jugador, double alfa, double beta, int bool_TVP, TVP *tabla, unsigned long long xor_hash[8*4][4], int timeset, int* timeout, clock_t comienzo);

mov min(int tablero[n][n], int turno, int casilla_del_atacante[2], int print, int profundidad, int jugador, double alfa, double beta, int bool_TVP, TVP *tabla, unsigned long long xor_hash[8*4][4], int timeset, int* timeout, clock_t comienzo){
    mov movimientos[MAX_MOV];
    int k;
    int cantidad_movimientos;
    int mueve,victima=99;// para luego deshacer el movimiento
    double valor_min=200;// pongo este numero para que siempre haya un movimiento con una mejor valoracion que -101
    int indice_min=0;
    mov mejor_respuesta={.casilla_origen={99,99},.casilla_destino={99,99},.mata=2,.casilla_victima={99,99},.valor=-101};// para el caso profundidad>1
    mov mejor_movimiento={.casilla_origen={99,99},.casilla_destino={99,99},.mata=2,.casilla_victima={99,99},.valor=-101};// para el return 
    int l;//indice para el print
    int acaba_de_coronar=0;// esta variable se usa para saber si se acaba de coronar porque sino tras el movimiento y la coronacion la funcion puede_matar() sira que si que puede volver a matar pero en este caso tiene que cambiar de turno
    int movimientos_similares[MAX_MOV];// Este vector almacenara los indices de los movimientos con un valor igual al valor del mejor movimiento. Se usara para posteriormente escoger uno de manera aleatoria entre ellos.
    int cantidad_de_movimientos_similares=0;// indice para el vector anterior.
    if(*timeout==1){return mejor_movimiento;}
    if((depth-profundidad)>max_depth_showing){print=0;}
    cantidad_movimientos=generar_movimientos(tablero,turno,movimientos,casilla_del_atacante,print);
    if(cantidad_movimientos==0){
        if(casilla_del_atacante[0]!=99 && printeado_error==0){printf("min(%d): En la siguiente posicion la funcion puede_matar dice que (%d,%d) puede volver a matar pero la funcion generar_movimientos no encuentra ningun movimiento de matar para esa ficha\n",profundidad,casilla_del_atacante[0],casilla_del_atacante[1]);imprimir_tablero(tablero);printeado_error=1;return mejor_movimiento;}
        // ESTO HABRA QUE MODIFICARLO EN FUNCION DEL ORDEN EN EL QUE LLAME A MIN MAX CUANDO JUEGEN CON NEGRAS
        mejor_movimiento.valor=101+profundidad;
        return mejor_movimiento;
    }
    //TVP
    if(bool_TVP==1){
      mov mov_TVP=buscar_en_TVP(tabla,t_a_hash(tablero,turno,xor_hash));
      if(mov_TVP.casilla_origen[0]!=99){
        int a=ordenar_movimientos(cantidad_movimientos,movimientos,mov_TVP);
        veces_encontrado++;
        //if(a==-1){imprimir_tablero(tablero);exit(0);}
      }
    }
    
    // por cada uno de los moviminetos posibles de la posicion
    for(k=0;k<cantidad_movimientos;k++){
        if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
        printf("min(%d): miro el movimiento (%d,%d) -> (%d,%d), %s mata\n",profundidad,movimientos[k].casilla_origen[0],movimientos[k].casilla_origen[1],movimientos[k].casilla_destino[0],movimientos[k].casilla_destino[1],Pns[movimientos[k].mata]);}
        // hacer el movimiento
        mueve=tablero[movimientos[k].casilla_origen[0]][movimientos[k].casilla_origen[1]];
        tablero[movimientos[k].casilla_origen[0]][movimientos[k].casilla_origen[1]]=0;
        tablero[movimientos[k].casilla_destino[0]][movimientos[k].casilla_destino[1]]=mueve+acaba_de_coronar;
        if(movimientos[k].mata==1){
            victima=tablero[movimientos[k].casilla_victima[0]][movimientos[k].casilla_victima[1]];
            tablero[movimientos[k].casilla_victima[0]][movimientos[k].casilla_victima[1]]=0;
        }
        acaba_de_coronar=0;
        if(mueve==3){acaba_de_coronar=corona(movimientos[k].casilla_destino,tablero,turno,movimientos[k].mata);}
        if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
        printf("min(%d): el movimiento %s corona.\n",profundidad,Pns[acaba_de_coronar]);}
        if(acaba_de_coronar==1){tablero[movimientos[k].casilla_destino[0]][movimientos[k].casilla_destino[1]]=mueve+1;}
        if(mueve+acaba_de_coronar>4){printf("min(%d): mueve=%d, acaba_de_coronar=%d\n",profundidad,mueve,acaba_de_coronar);}
        //printf("min: he movido\n");
        // evaluarlo en funcion de la profundidad
        if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
        printf("min(%d): evaluo\n",profundidad);}
        if(profundidad==1){
            // PARA ESCOGER QUE FUNCION SE QUIERE USAR PARA VALORAR. CHAPUZA MOMENTANEA
            /*if(jugador==1){movimientos[k].valor=valorar_posicion2(tablero,turno);}
            else{movimientos[k].valor=valorar_posicion2(tablero,turno);}*/
            movimientos[k].valor=comparador_valorar_posicion(tablero,turno,jugador);
            if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
            printf("min(%d): el movimiento numero %d tiene una valoracion de %lf\n",profundidad,k,movimientos[k].valor);}
            if(contador%2000==0){if(clock()-comienzo>timeset){*timeout=1;/*printf("check tiempo, contador=%lu, transcurrido: %lf\n",contador,(double)(clock()-comienzo)/CLOCKS_PER_SEC);*/}}
        }
        if(profundidad>1){
            //si hay mas profundidad pero ya no le quedan piezas pues ya esta. habria que tener en cuenta tambien si no puede mover
            if(valorar_posicion(tablero,turno)==-100){
                // deshacer el movimiento
                tablero[movimientos[k].casilla_origen[0]][movimientos[k].casilla_origen[1]]=mueve;
                tablero[movimientos[k].casilla_destino[0]][movimientos[k].casilla_destino[1]]=0;
                if(movimientos[k].mata==1){
                    tablero[movimientos[k].casilla_victima[0]][movimientos[k].casilla_victima[1]]=victima;
                }
                mejor_movimiento=movimientos[k];
                mejor_movimiento.valor=-100-profundidad;
                if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
                printf("min(%d): el movimiento numero %d tiene una valoracion de %lf\n",profundidad,k,mejor_movimiento.valor);}
                if(bool_TVP==1){guardar_en_TVP(tabla,t_a_hash(tablero,turno,xor_hash),mejor_movimiento);}
                return mejor_movimiento;
            }
            // depende si puede volver a matar ( y si ha matado si lo puede hacer con la misma pieza)
            if(puede_matar(tablero,turno,movimientos[k].casilla_destino,0)==1 && acaba_de_coronar==0 && movimientos[k].mata==1){
                if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
                printf("min(%d): vuelvo a llamar a la min() porque puedo volver a comer\n",profundidad);}
                casilla_del_atacante[0]=movimientos[k].casilla_destino[0];
                casilla_del_atacante[1]=movimientos[k].casilla_destino[1];
                mejor_respuesta=min(tablero,turno,casilla_del_atacante,print,profundidad-1,jugador,-200,200,bool_TVP,tabla,xor_hash,timeset,timeout,comienzo);
                //if(printeado_error==1){imprimir_tablero(tablero);puede_matar(tablero,turno,movimientos[k].casilla_destino,1);exit(0);}
                if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
                printf("min(%d): he vuelto de la funcion min()\n",profundidad);}
            }
            else{
                if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
                printf("min(%d): Como no puede matar, coronar=%d y mata=%d, llamo a la funcion max()\n",profundidad,acaba_de_coronar,movimientos[k].mata);}
                //printf("min(%d): llamo a la funcion max()\n",profundidad);
                // cambio de turno
                turno=(turno+2)%4;
                casilla_del_atacante[0]=99;
                casilla_del_atacante[1]=99;
                mejor_respuesta=max(tablero,turno,casilla_del_atacante,print,profundidad-1,jugador,alfa,beta,bool_TVP,tabla,xor_hash,timeset,timeout,comienzo);
                turno=(turno+2)%4;
                if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
                printf("min(%d): vuelvo de la funcion max()\n",profundidad);}
            }
            
            movimientos[k].valor=mejor_respuesta.valor;
        }
        if(movimientos[k].valor<beta){
            beta=movimientos[k].valor;
        }
        if(beta<=alfa){
            // PODA
            //mejor_movimiento.valor=movimientos[k].valor;
            // deshacer el movimiento
            tablero[movimientos[k].casilla_origen[0]][movimientos[k].casilla_origen[1]]=mueve;
            tablero[movimientos[k].casilla_destino[0]][movimientos[k].casilla_destino[1]]=0;
            if(movimientos[k].mata==1){
                tablero[movimientos[k].casilla_victima[0]][movimientos[k].casilla_victima[1]]=victima;
            }
            //movimientos[k].valor=-99;
            if(bool_TVP==1){guardar_en_TVP(tabla,t_a_hash(tablero,turno,xor_hash),movimientos[k]);}
            if(k==0){veces_podado_primera++;}
            return movimientos[k];
        }
        if(movimientos[k].valor<valor_min){
            if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
            printf("min(%d): como %lf<%lf entonces el %d-essimo movimiento generado es el mejor por ahora\n",profundidad,movimientos[k].valor,valor_min,k);}
            valor_min=movimientos[k].valor;
            indice_min=k;
            //Limpio los antiguos movimientos similares y meto el mejor
            movimientos_similares[0]=k;
            cantidad_de_movimientos_similares=1;
        }
        else{
            if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
            printf("min(%d): como %lf>=%lf entonces descarto el %d-essimo movimiento\n",profundidad,movimientos[k].valor,valor_min,k);}
            //Si es un movimiento con puntuacion igual lo guardo
            if(movimientos[k].valor==valor_min){
                movimientos_similares[cantidad_de_movimientos_similares]=k;
                cantidad_de_movimientos_similares++;
            }
        }
        // deshacer el movimiento
        tablero[movimientos[k].casilla_origen[0]][movimientos[k].casilla_origen[1]]=mueve;
        tablero[movimientos[k].casilla_destino[0]][movimientos[k].casilla_destino[1]]=0;
        if(movimientos[k].mata==1){
            tablero[movimientos[k].casilla_victima[0]][movimientos[k].casilla_victima[1]]=victima;
        }
        if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
        printf("min(%d): he deshecho el movimiento\n",profundidad);}
    }
    if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
    printf("min(%d): el mejor movimiento para el bando %d es el %d-essimo movimiento\n",profundidad,turno,indice_min);}
    // si la profundidad es 1 solo necesito el valor
  
    //Escojo uno aleatorio
    int aleatorio=0;
    if(cantidad_de_movimientos_similares>0){
      //aleatorio=(rand()%cantidad_de_movimientos_similares);
      //aleatorio=NUM%cantidad_de_movimientos_similares;
    }
    else{
        printf("min(%d): ERROR: cantidad_de_movimientos_similares=%d=0\n",profundidad,cantidad_de_movimientos_similares);
    }
    
    return movimientos[movimientos_similares[aleatorio]];
    
}

mov max(int tablero[n][n], int turno, int casilla_del_atacante[2], int print, int profundidad, int jugador, double alfa, double beta, int bool_TVP, TVP *tabla, unsigned long long xor_hash[8*4][4], int timeset, int* timeout, clock_t comienzo){
    mov movimientos[MAX_MOV];// aqui se guardaran los posibles movimientos de la posicion
    int k;
    int cantidad_movimientos;// cantidad de movimientos generados de la posicion
    int mueve,victima=99;// para luego deshacer el movimiento
    double valor_max=-200;// pongo este numero para que siempre haya un movimiento con una mejor valoracion de -101 y por lo tanto escogera cualquier movimiento en lugar de este
    int indice_max=0;
    mov mejor_respuesta={.casilla_origen={99,99},.casilla_destino={99,99},.mata=2,.casilla_victima={99,99},.valor=-101};// para el caso profundidad>1
    mov mejor_movimiento={.casilla_origen={99,99},.casilla_destino={99,99},.mata=2,.casilla_victima={99,99},.valor=-101};// para el return 
    int l;//indice para el print
    int acaba_de_coronar=0;// esta variable se usa para saber si se acaba de coronar porque sino tras el movimiento y la coronacion la funcion puede_matar() dira que si que puede volver a matar pero en este caso tiene que cambiar de turno
    // for(l=0;0<depth-profundidad;l++){printf("\t");}
    // if(print==1){printf("max(%d): ",profundidad);}
    int movimientos_similares[MAX_MOV];// Este vector almacenara los indices de los movimientos con un valor igual al valor del mejor movimiento. Se usara para posteriormente escoger uno de manera aleatoria entre ellos.
    int cantidad_de_movimientos_similares=0;// indice para el vector anterior.
    if(*timeout==1){return mejor_movimiento;}
    if((depth-profundidad)>max_depth_showing){print=0;}
    cantidad_movimientos=generar_movimientos(tablero,turno,movimientos,casilla_del_atacante,print);
    if(cantidad_movimientos==0){
        if(casilla_del_atacante[0]!=99 && printeado_error==0){printf("max(%d): En la siguiente posicion la funcion puede_matar dice que (%d,%d) puede volver a matar pero la funcion generar_movimientos no encuentra ningun movimiento de matar para esa ficha\n",profundidad,casilla_del_atacante[0],casilla_del_atacante[1]);imprimir_tablero(tablero);printeado_error=1;}

        // ESTO HABRA QUE MODIFICARLO EN FUNCION DEL ORDEN EN EL QUE LLAME A MIN MAX CUANDO JUEGEN CON NEGRAS
        mejor_movimiento.valor=-101-profundidad;
        return mejor_movimiento;
    }
    //TVP
    if(bool_TVP==1){
      mov mov_TVP=buscar_en_TVP(tabla,t_a_hash(tablero,turno,xor_hash));
      if(mov_TVP.casilla_origen[0]!=99){
        int a=ordenar_movimientos(cantidad_movimientos,movimientos,mov_TVP);
        //if(a==-1){imprimir_tablero(tablero);exit(0);}
        veces_encontrado++;
      }
    }
    
    // por cada uno de los moviminetos posibles de la posicion
    for(k=0;k<cantidad_movimientos;k++){
        if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
        printf("max(%d): miro el movimiento (%d,%d) -> (%d,%d), %s mata\n",profundidad,movimientos[k].casilla_origen[0],movimientos[k].casilla_origen[1],movimientos[k].casilla_destino[0],movimientos[k].casilla_destino[1],Pns[movimientos[k].mata]);}
        // hacer el movimiento
        mueve=tablero[movimientos[k].casilla_origen[0]][movimientos[k].casilla_origen[1]];
        tablero[movimientos[k].casilla_origen[0]][movimientos[k].casilla_origen[1]]=0;
        tablero[movimientos[k].casilla_destino[0]][movimientos[k].casilla_destino[1]]=mueve+acaba_de_coronar;
        if(movimientos[k].mata==1){
            victima=tablero[movimientos[k].casilla_victima[0]][movimientos[k].casilla_victima[1]];
            //if(victima!=1 && victima!=2 && victima!=3 && victima!=4){printf("max(%d): ERROR: victima=(%d)\n",profundidad,victima);print_movimiento(" ",movimientos[k],0);}
            tablero[movimientos[k].casilla_victima[0]][movimientos[k].casilla_victima[1]]=0;
        }
        acaba_de_coronar=0;
        if(mueve==1){acaba_de_coronar=corona(movimientos[k].casilla_destino,tablero,turno,movimientos[k].mata);}
        if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
        printf("max(%d): el movimiento %s corona.\n",profundidad,Pns[acaba_de_coronar]);}
        if(acaba_de_coronar==1){tablero[movimientos[k].casilla_destino[0]][movimientos[k].casilla_destino[1]]=mueve+1;}
        if(mueve+acaba_de_coronar>4){printf("max(%d): mueve=%d, acaba_de_coronar=%d\n",profundidad,mueve,acaba_de_coronar);}
        // evaluarlo en funcion de la profundidad
        if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
        printf("max(%d): evaluo\n",profundidad);}
        if(profundidad==1){
            // PARA ESCOGER QUE FUNCION SE QUIERE USAR PARA VALORAR. CHAPUZA MOMENTANEA
            /*if(jugador==1){movimientos[k].valor=valorar_posicion2(tablero,turno);}
            else{movimientos[k].valor=valorar_posicion2(tablero,turno);}*/
            movimientos[k].valor=comparador_valorar_posicion(tablero,turno,jugador);
            //imprimir_tablero(tablero);
            if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
            printf("max(%d): el movimiento numero %d tiene una valoracion de %lf\n",profundidad,k,movimientos[k].valor);}
             if(contador%2000==0){if(clock()-comienzo>timeset){*timeout=1;/*printf("check tiempo, contador=%lu, transcurrido: %lf\n",contador,(double)(clock()-comienzo)/CLOCKS_PER_SEC);*/}}
        }
        if(profundidad>1){
            //si hay mas profundidad pero ya no le quedan piezas pues ya esta. habria que tener en cuenta tambien si no puede mover
            if(valorar_posicion(tablero,turno)==100){
                // deshacer el movimiento
                tablero[movimientos[k].casilla_origen[0]][movimientos[k].casilla_origen[1]]=mueve;
                tablero[movimientos[k].casilla_destino[0]][movimientos[k].casilla_destino[1]]=0;
                if(movimientos[k].mata==1){
                    tablero[movimientos[k].casilla_victima[0]][movimientos[k].casilla_victima[1]]=victima;
                }
                mejor_movimiento=movimientos[k];
                mejor_movimiento.valor=100+profundidad;
                if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
                printf("max(%d): el movimiento numero %d tiene una valoracion de %lf\n",profundidad,k,mejor_movimiento.valor);}
                if(bool_TVP==1){guardar_en_TVP(tabla,t_a_hash(tablero,turno,xor_hash),mejor_movimiento);}
                return mejor_movimiento;
            }
            // depende si puede volver a matar ( y si ha matado si lo puede hacer con la misma pieza)
            if(puede_matar(tablero,turno,movimientos[k].casilla_destino,0)==1 && acaba_de_coronar==0 && movimientos[k].mata==1){
                if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
                printf("max(%d): vuelvo a llamar a la max() porque puedo volver a comer\n",profundidad);}
                casilla_del_atacante[0]=movimientos[k].casilla_destino[0];
                casilla_del_atacante[1]=movimientos[k].casilla_destino[1];
                mejor_respuesta=max(tablero,turno,casilla_del_atacante,print,profundidad-1,jugador,-200,200,bool_TVP,tabla,xor_hash,timeset,timeout,comienzo);
                //if(printeado_error==1){imprimir_tablero(tablero);puede_matar(tablero,turno,movimientos[k].casilla_destino,1);exit(0);}
                //printf("HOLAAAAAAAAAAAA2\n");
                if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
                printf("max(%d): he vuelto de la funcion max()\n",profundidad);}
            }
            else{
                if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
                printf("max(%d): Como no puede matar, coronar=%d y mata=%d, llamo a la funcion min()\n",profundidad,acaba_de_coronar,movimientos[k].mata);}
                // cambio de turno
                turno=(turno+2)%4;
                casilla_del_atacante[0]=99;
                casilla_del_atacante[1]=99;
                mejor_respuesta=min(tablero,turno,casilla_del_atacante,print,profundidad-1,jugador,alfa,beta,bool_TVP,tabla,xor_hash,timeset,timeout,comienzo);
                turno=(turno+2)%4;
                if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
                printf("max(%d): vuelvo de la funcion min()\n",profundidad);}
            }
            
            movimientos[k].valor=mejor_respuesta.valor;
        }
        if(movimientos[k].valor>alfa){
            alfa=movimientos[k].valor;
        }
        if(beta<=alfa){
            // PODA
            //mejor_movimiento->valor=movimientos[k].valor;
            // deshacer el movimiento
            tablero[movimientos[k].casilla_origen[0]][movimientos[k].casilla_origen[1]]=mueve;
            tablero[movimientos[k].casilla_destino[0]][movimientos[k].casilla_destino[1]]=0;
            if(movimientos[k].mata==1){
                tablero[movimientos[k].casilla_victima[0]][movimientos[k].casilla_victima[1]]=victima;
            }
            //movimientos[k].valor=99;
            if(bool_TVP==1){guardar_en_TVP(tabla,t_a_hash(tablero,turno,xor_hash),movimientos[k]);}
            if(k==0){veces_podado_primera++;}
            return movimientos[k];
        }
        if(movimientos[k].valor>valor_max){
            if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
            printf("max(%d): como %lf>%lf entonces el %d-essimo movimiento generado es el mejor por ahora\n",profundidad,movimientos[k].valor,valor_max,k);}
            valor_max=movimientos[k].valor;
            indice_max=k;
            //Limpio los antiguos movimientos similares y meto el mejor
            movimientos_similares[0]=k;
            cantidad_de_movimientos_similares=1;
        }
        else{
            if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
            printf("max(%d): como %lf<=%lf entonces descarto el %d-essimo movimiento\n",profundidad,movimientos[k].valor,valor_max,k);}
            //Si es un movimiento con puntuacion igual lo guardo
            if(movimientos[k].valor==valor_max){
                movimientos_similares[cantidad_de_movimientos_similares]=k;
                cantidad_de_movimientos_similares++;
            }
        }
        // deshacer el movimiento
        tablero[movimientos[k].casilla_origen[0]][movimientos[k].casilla_origen[1]]=mueve;
        tablero[movimientos[k].casilla_destino[0]][movimientos[k].casilla_destino[1]]=0;
        if(movimientos[k].mata==1){
            tablero[movimientos[k].casilla_victima[0]][movimientos[k].casilla_victima[1]]=victima;
        }
        if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
        printf("max(%d): he deshecho el movimiento\n",profundidad);}
    }
    if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
    printf("max(%d): el mejor movimiento para el bando %d es el %d-essimo movimiento\n",profundidad,turno,indice_max);}
    
    //Escojo uno aleatorio
    int aleatorio=0;
    if(cantidad_de_movimientos_similares>0){
      //aleatorio=(rand()%cantidad_de_movimientos_similares);
      //aleatorio=NUM%cantidad_de_movimientos_similares;
    }
    else{
        printf("max(%d): ERROR: cantidad_de_movimientos_similares=%d=0\n",profundidad,cantidad_de_movimientos_similares);
    }
    /*if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
    printf("max(%d): cantidad_de_movimientos_similares=%d\t aleatorio=%d\n",profundidad,cantidad_de_movimientos_similares,aleatorio);}
    if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
    print_movimiento("Movmiento escogido:",movimientos[movimientos_similares[aleatorio]],1);}*/
  
    //SOLO PARA DEBUGEAR
    /*printf("Hay %d movimientos con el mismo valor\n",cantidad_de_movimientos_similares);
    for(k=0;k<cantidad_de_movimientos_similares;k++){
        print_movimiento("movimiento numero ",movimientos[movimientos_similares[k]],1);
    }
    printf("El numero aleatorio ha sido el %d\n",aleatorio);*/
    /*printf("HOLAAAAAAAAAAAA1 movimientos_similares[aleatorio]=%d\n",movimientos_similares[aleatorio]);
    if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
    print_movimiento("Movmiento0:",movimientos[0],1);}
    if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
    print_movimiento("Movmiento1:",movimientos[1],1);}
    if(print==1){for(l=0;l<depth-profundidad;l++){printf("\t");}
    print_movimiento("Movmiento2:",movimientos[2],1);}
    mov movimiento_final=movimientos[movimientos_similares[aleatorio]];*/
    return movimientos[movimientos_similares[aleatorio]];
}

mov computadora(int tablero[n][n], int turno, int casilla_del_atacante[2], int profundidad, int print, int bool_TVP, int timeset){
    
    
    //Algoritmo:
    //llama a la funcion max(). esta funcion llamara a la funcion generar_movimientos() y por cada movimiento lo movera evaluara y lo deshara para probar con el siguiente movimiento
    //la funcion max(), a su vez, llamara a la funcion min() (dependera de la variable depth) y "hara lo mismo"
    
    
    
    clock_t comienzo=clock();
    int timeout=0;
    //timeset=timeset*1000000; // paso de seg a micro seg. la variable CLOCKS_PER_SEC tiene un valor de 10**6. mejor lo hare al llamar las funciones min y max
    // DE AQUI PARA ABAJO TIENE QUE IR TODO EL CODIGO
    int casilla_guardada[2];// para guardar la casilla del atcante
    casilla_guardada[0]=casilla_del_atacante[0];
    casilla_guardada[1]=casilla_del_atacante[1];
    mov mejor_movimiento={.casilla_origen={99,99},.casilla_destino={99,99},.mata=2,.casilla_victima={99,99},.valor=-101};// para el return 
    mov mejor_movimiento_temp={.casilla_origen={99,99},.casilla_destino={99,99},.mata=2,.casilla_victima={99,99},.valor=-101};// para el return 
    mov movimientos[MAX_MOV];
    // TVP
    unsigned long long xor_hash[8*4][4];
    inicializar_hash(xor_hash);
    TVP* tabla=(TVP*)malloc(sizeof(TVP));;
    inicializar_TVP(tabla);
  
    if(generar_movimientos(tablero,turno,movimientos,casilla_del_atacante,0)==1){
      if(show_best_move==1){
        printf("computadora(%d): devuelvo el unico movimiento disponible\n",profundidad);
        print_movimiento("",movimientos[0],0);
      }
      free(tabla->tabla);
      free(tabla);
      return movimientos[0];
    }
    if(print==1){printf("turno=%d, computadora(%d): empiezo, llamo a max()\n",turno,profundidad);}
    if(turno==1){
      for(int k=1;k<=profundidad;k++){
        mejor_movimiento_temp=max(tablero,turno,casilla_del_atacante,print,k,1,-200.0,200.0,bool_TVP,tabla,xor_hash,timeset*1000000,&timeout,comienzo);
        // dejo intacta la casilla del atacante
        casilla_del_atacante[0]=casilla_guardada[0];
        casilla_del_atacante[1]=casilla_guardada[1];
        if(timeout==1){/*printf("computadora(): profundidad limitada por el tiempo a %d\n",k-1);*/profundidad=k-1;break;}
        mejor_movimiento=mejor_movimiento_temp;
        if(mejor_movimiento.valor==100 || mejor_movimiento.valor==-100){limpiar_TVP(tabla);profundidad=k-1;break;}
      }
    }
    if(turno==3){
      for(int k=1;k<=profundidad;k++){
        mejor_movimiento_temp=min(tablero,turno,casilla_del_atacante,print,k,2,-200.0,200.0,bool_TVP,tabla,xor_hash,timeset*1000000,&timeout,comienzo);
        // dejo intacta la casilla del atacante
        casilla_del_atacante[0]=casilla_guardada[0];
        casilla_del_atacante[1]=casilla_guardada[1];
        if(timeout==1){/*printf("computadora(): profundidad limitada por el tiempo a %d\n",k-1);*/profundidad=k-1;break;}
        mejor_movimiento=mejor_movimiento_temp;
        if(mejor_movimiento.valor==100 || mejor_movimiento.valor==-100){limpiar_TVP(tabla);profundidad=k-1;break;}
      }
    }
    //limpiar_TVP(tabla);
    if(show_best_move==1 && 1==0){
      char cabecero[80];
      strcpy(cabecero,"computadora(");
      //char prof_char[1]=profundidad+'0';
      char prof_char[2];
      sprintf(prof_char, "%d", profundidad);
      strcat(cabecero,prof_char);
      strcat(cabecero,"): mejor mov");
      print_movimiento(cabecero,mejor_movimiento,1);
    }
    
    // dejo intacta la casilla del atacante
    casilla_del_atacante[0]=casilla_guardada[0];
    casilla_del_atacante[1]=casilla_guardada[1];
    // DE AQUI PARA ARRIBA TIENE QUE IR TODO EL CODIGO
    clock_t final=clock();
    double tiempo_invertido=(double)(final-comienzo)/CLOCKS_PER_SEC;
    if(engine_plays_as!=5 || show_best_move==1){
        printf("computadora(%d): ",profundidad);print_movimiento("mejor mov",mejor_movimiento,1);
        printf("\ttiempo invertido %.2lf/%d segundos, timeout=%d\n",tiempo_invertido,timeset,timeout);
        printf("\tposiciones calculadas totales: %'lu\n", contador);
        printf("\tposiciones calculadas por segundo: %'d\n", (int)(contador/tiempo_invertido));
        printf("\tTVP: %d/%d (%.1f %%)\n",tabla->num_guardadas,tabla->num_entradas,(float)tabla->num_guardadas*100/tabla->num_entradas);
    }
    contador = 0;
    free(tabla->tabla);
    free(tabla);
    return mejor_movimiento;
}

float DAMAS(void){    //AQUI IRA TODO EL CODIGO
    int tablero[n][n];
    int cualquiera[2]={0,0};
    int turno=1;// 1= mueven blancas 3= mueven negras
    inicio_de_partida(tablero);
    //posicion_inicial(tablero);
    //posicion_inicial_engine(tablero);
    //if(!system("clear"));
    mov movimiento_pedido;
    int MEMORIA_PARTIDA[200][n][n];// Aqui se guardaran todas las posiciones de la parida para poder mover hacia atras. como mucho 200 moviminetos
    int ply=0; // Contador para lo de arriba
    char siguientes_movimientos[3][6];// para la funcion pedir_movimiento
    int cantidad_de_siguientes_movimientos=0;// para la funcion pedir_movimiento
    int casilla_del_atacante[2]={99,99};// para que las funciones movimiento_valido y generar_movimiento sepan que pieza comio en el movimiento anteior. tiene un valor de 99 si no importa
    mov  posibles_movimientos[MAX_MOV];// para la funcion generar movimiento
    char movimientos_PDN[250][20];
    //leer_PDN(movimientos_PDN);
    if(engine_plays_as!=5){imprimir_tablero(tablero);}
    mov mejor_movimiento;// para la computadora
    mov mejor_movimiento_sinpoda;
    mov mejor_movimiento_conpoda;
    int acaba_de_coronar=0;// esta variable se usa para saber si se acaba de coronar porque sino tras el movimiento y la coronacion la funcion puede_matar() sira que si que puede volver a matar pero en este caso tiene que cambiar de turno

    /*clock_t comienzo=clock();
    unsigned long long xor_hash[8*4][4];
    inicializar_hash(xor_hash);
    printf("La inicializacion de la TVP ha tardado %lf seg\n",(double)(clock()-comienzo)/1000000);
    TVP* tabla=(TVP*)malloc(sizeof(TVP));;
    inicializar_TVP(tabla);
    printf("La inicializacion de la TVP ha tardado %lf seg\n",(double)(clock()-comienzo)/1000000);*/
  
    // Bucle del juego
    int valido=0;
    int limite=0;
    int max_plys=100;// una chapuza momentanea para envitar las tablas infinitas
    while(sin_fichas(tablero,turno)==0 && generar_movimientos(tablero, turno, posibles_movimientos, casilla_del_atacante,0)>0 && ply<max_plys){// falta poner aqui generar_movimientos>0 pero no lo pongo para no liarla hasta que no este acabada
        //generar_movimientos(tablero, turno, posibles_movimientos, casilla_del_atacante,1);
        /*if(turno==1){mejor_movimiento=computadora(tablero,turno,casilla_del_atacante,depth,show_engine_process);}
        else{mejor_movimiento=computadora_sinpoda(tablero,turno,casilla_del_atacante,depth,show_engine_process);}*/
        //mejor_movimiento=computadora(tablero,turno,casilla_del_atacante,depth,show_engine_process,1,30);
        //mejor_movimiento_sinpoda=computadora_sinpoda(tablero,turno,casilla_del_atacante,depth,show_engine_process);
        //mejor_movimiento_conpoda=computadora_conpoda(tablero,turno,casilla_del_atacante,depth,show_engine_process);
		mejor_movimiento=computadora_conpoda(tablero,turno,casilla_del_atacante,depth,show_engine_process);
        /*mov mejor_movimiento_sinpoda=computadora_sinpoda(tablero,turno,casilla_del_atacante,depth,show_engine_process);
        //if(comparador_movimientos(mejor_movimiento,mejor_movimiento_sinpoda)==0){
        if(mejor_movimiento.valor!=mejor_movimiento_sinpoda.valor){
            imprimir_tablero(tablero);
            print_movimiento("con poda:",mejor_movimiento,1);
            print_movimiento("sin poda:",mejor_movimiento_sinpoda,1);
            inicializar_TVP(tabla);
            mejor_movimiento=computadora(tablero,turno,casilla_del_atacante,depth,1,1,tabla,xor_hash);
            print_movimiento("con poda:",mejor_movimiento,1);
            computadora_sinpoda(tablero,turno,casilla_del_atacante,depth,1);
            exit(0);
        }*/
        if(turno==engine_plays_as || engine_plays_as==5){
            //if(engine_plays_as==5){printf("DAMAS: ply:%d turno:%d ev:%lf\r",ply,turno,mejor_movimiento.valor);}
            movimiento_pedido=mejor_movimiento;
            acaba_de_coronar=0;
            valido=moivmiento_valido(tablero,mejor_movimiento,turno,casilla_del_atacante,&acaba_de_coronar);
        }
        else{
            // Pido movimineto valido y muevo
            while(valido==0 && limite<5){
                //pedir_movimiento(&movimiento_pedido,turno,ply,movimientos_PDN,siguientes_movimientos,&cantidad_de_siguientes_movimientos);
		/*char pal_selenium[20];
		if(cantidad_de_siguientes_movimientos==0){if(!scanf("%s",pal_selenium)){}}		
		pedir_movimiento_selenium(&movimiento_pedido,turno,ply,movimientos_PDN,siguientes_movimientos,&cantidad_de_siguientes_movimientos,pal_selenium);*/
                pedir_movimiento(&movimiento_pedido,turno,ply,movimientos_PDN,siguientes_movimientos,&cantidad_de_siguientes_movimientos);
                printf("DAMAS: acaba de acabar la funcion pedir movimiento\n");
                printf("DAMAS: casilla del atacante = (%d,%d)\n",casilla_del_atacante[0],casilla_del_atacante[1]);
                acaba_de_coronar=0;
                valido=moivmiento_valido(tablero,movimiento_pedido,turno,casilla_del_atacante,&acaba_de_coronar);
                limite++;
            }
            if(limite==5){printf("DAMAS: ERROR: algo raro ha sucedido al pedir un movimiento valido\n");}
        }
        
        
        // Imprimo tablero //engine_plays_as!=5 &&
        if(mostrar_tablero==1){
            //if(!system("clear"));
            imprimir_tablero(tablero);
        }
        valido=0;
        limite=0;
        // Guardo partida
        guardar_partida(tablero,MEMORIA_PARTIDA[ply]);
        // Cambio de turno adecuado
        if(puede_matar(tablero,turno,movimiento_pedido.casilla_destino,0)==1 && acaba_de_coronar==0 && movimiento_pedido.mata==1){
            if(engine_plays_as!=5){printf("DAMAS: No cambio el turno porque puedes volver a matar con la misma ficha con la que acabas de matar\n");}
            casilla_del_atacante[0]=movimiento_pedido.casilla_destino[0];
            casilla_del_atacante[1]=movimiento_pedido.casilla_destino[1];
            continue;}
        else{
            casilla_del_atacante[0]=99;
            casilla_del_atacante[1]=99;
            if(turno==1){turno=3;}
            else{turno=1;}
            ply++;
        }
    }
    //printf("DAMAS(): ply:%d turno:%d ev:%lf\n",ply,turno,mejor_movimiento.valor);
    /*clock_t comienzo2=clock();
    printf("DAMAS(): tabla: %d/%d\n",tabla->num_guardadas,tabla->num_entradas);
    free(tabla->tabla);
    free(tabla);
    printf("La inicializacion de la TVP ha tardado %lf seg\n",(double)(clock()-comienzo2)/1000000);*/
    //Empate
    if(ply==max_plys){
        if(mostrar_tablero==2){printf("\nDAMAS: van por los %d plys, seguramente hayan sido tablas\n",max_plys-1);}
        return 0;
    }
    // Cambio de turno para saber el ganador
    if(turno==1){turno=3;}
    else{turno=1;}
    //if(system("clear"));
    //imprimir_tablero(tablero);
    if(mostrar_tablero==1){printf("\nHAN GANADO LAS %s!!!\n",BN[turno]);}
    //printf("\n");// para el print de la funcion DAMAS cuando engine_plays_as = 5 que tiene un \r al final
    //imprimir_tablero(tablero);
    if(turno==0){printf("DAMAS(): ERROR: turno=%d=0\n",turno);}
    return turno;
    
}

// Me gusta tener el main vacio
int main(void){
    //setlocale(LC_NUMERIC, "");
    srand(time(NULL)); // Esto cambia la semilla de los numeros aleatorios
    //printf("%d\n",rand()%1);
    DAMAS();
    /*int k,partidas=1,empates=0;
    float contador=0.0,resultado;
    clock_t comienzo=clock();
    for(k=0;k<partidas;k++){
        //NUM=rand();
        printf("main(): Partida numero %d:  ",k);
        resultado=DAMAS();
        printf("main(): resultado=%f, empates=%d. tiempo: %.1lf. p/e: %d/%d\n",resultado,empates,(double)(clock()-comienzo)/CLOCKS_PER_SEC,veces_podado_primera,veces_encontrado);
        veces_encontrado=0;
        veces_podado_primera=0;
        if(resultado==1.0){contador=contador+1.0;}
        else if(resultado==0){contador=contador+0.5;empates++;}
    }
    clock_t final=clock();
    double tiempo_invertido=(double)(final-comienzo)/CLOCKS_PER_SEC;
    printf("Resultado: blancas %f, negras %f. Han habido %d empates. \n",contador,partidas-contador,empates);
    printf("Tiempo: %lf segundos\n",tiempo_invertido);*/
    
    /*int tablero2[8][8]={//
        0,3,0,3,0,1,0,0,//
        0,0,3,0,3,0,0,0,//
        0,0,0,0,0,0,0,0,//
        1,0,0,0,0,0,0,0,//
        0,0,0,1,0,0,0,0,//
        0,0,0,0,0,0,0,0,//
        0,1,0,0,0,1,0,0,//
        0,0,0,0,0,0,1,0//
    };
    int tablero[8][8]={//
        0,3,0,3,0,0,0,0,//
        0,0,0,0,0,0,0,0,//
        0,0,0,0,0,0,0,0,//
        1,0,0,0,0,0,0,0,//
        0,0,0,0,0,0,0,0,//
        0,0,0,0,0,0,0,0,//
        0,0,0,0,0,1,0,0,//
        3,0,0,0,0,0,1,0//
    };
    imprimir_tablero(tablero);
    int casilla[2]={7,0};
    int c=corona(casilla,tablero,3,1);
    printf("%s corona\n",Pns[c]);*/
    

  
    //NOTAS:
    // NORMA QUE NO SE SI HE TENIDO EN CUENTA O ES QUE ES UNA SITUACION IMPOSIBLE: si una dama esta entre dos piezas enemigas, tras comerse una puede cambiar se dentido para comerse la otra ? si es que  si que puede guay, si no puede hay que cambiarlo 
    //EL CONTADOR DE POSICIONES CALCULADAS ESTA MAL 
    //SI QUEDAN EN TABLAS LA FUNCION DAMAS DEBERIA DEVOLVER 0               (HECHO TEMPORALMENTE)
    //PONER VARIABLE PARA QUE NO PRINTEE EL TABLERO SI LA FRASE: HAN GANADO LAS %s!!!
    //HAY ALGO CON ESTO DE LOS NUMEROS ALEATORIOS QUE VA MAL PORQUE A VECES PETA. IMAGINO QUE SERA CUANDO HAY UN UNICO MOVIMIENTO POSIBLE O ALGO
  
    //printf("%f",0.01*(rand()%9));
    /*printf("%ld\n",CLOCKS_PER_SEC);
    printf("%lf\n",(double)CLOCKS_PER_SEC);
    printf("%d\n",(int)CLOCKS_PER_SEC);*/
    /*float f=100.0;
    printf("%f\n",f+5);*/
    return 0;
}

/*1. e3-d4 h6-g5 2. d4-e5 f6xd4 3. c3xe5 d6xf4 4. g3xe5 e7-d6 5. f2-g3 d6xf4 6. g3xe5 f8-e7 7. e1-f2 e7-d6 8. f2-g3 d6xf4 9. g3xe5 b6-c5 10. g1-f2 c7-b6 11. d2-c3 c5-d4 12. e5-d6 d8-c7 13. c3xe5 g7-f6 14. e5xg7 h8xf6 15. a3-b4 c7xe5 16. f2-g3 g5-f4 17. c1-d2 f6-g5 18. g3-h4 b6-c5 19. h4xf6xd4xb6 a7xc5xa3xc1xe3*/


