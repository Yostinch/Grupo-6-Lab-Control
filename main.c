/* ========================================
 *
 * Copyright ITCR Laboratorio de Control Automatico, 2019
 * All Rights Reserved
 * PUBLISHED, NON-LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF ITCR.
 *
 * ========================================
*/

#include "project.h"
#include "stdbool.h"
#include "stdio.h"
#include "usbuartio.h"    
#include "math.h"



//Funcion del control
#define V_MAX 6
#define PWM_MAX 12499
#define N_MAX  12500 


/**********Entradas del Psoc para los finales de carrera**********/
int Pin1; // Entrada digital 1 
int Pin2; // Entrada digital 2
int Pin3; // Entrada digital 3
int Pin4; // Entrada digital 4

/**************************************************************/

/*************************Variables****************************/


//Salidas del Sistema
float posx0, posx1,  posx2 =0 ; // Muestras de la poscion x(n), x(n-1), x(n-2)
float posy0 , posy1 , posy2 =0; // Muestra de la poscion  y(n), y(n-1), y(n-2)

//Variables 
float pwm;

float eX, eY;    //ex(n), ey(n)
float eX1, eY1;  //ex(n-1), ey(n-1)
float eX2, eY2;  //ex(n-2), ey(n-2)

float sc, sc1, sc2 =0 ; //salida de control sc(n), sc(n-1), sc(n-2) 

int deltaY, deltaX =0;
int dir=0;
int move=0; 

float dir1=7854;
float psoc=0;
float vel=0;
float vel1=0;
float vel2=0;


/****Constantes Control*****/
float c1= 0.00635;
float c2= 0.006054;
float c3= 1236.5;
float c4= 2000;


////////////////////////////

/****Funciones****/
int fcarreraX();
int fcarreraY();

void controlY();
void controlX();
void velocidad();
void movimiento();
//////////////////

char displayStrH[20]= {'\0'};//Display LCD
char displayStrV[20]= {'\0'};//Display LCD
char displayStrm[20]= {'\0'};//Display LCD




int timeold=0, time; //Variables para conteo de 1s por ms
int ms_count=0;
int ms_count2=0;
int ms_count_old=0;
int ms_count_old2=0;
int enable=0;
int enable2=0;





//Interrupcion del timer para conteo de 1 segundo, calculo de velocidades cada 10ms
CY_ISR(isr_Timer_1){
    ms_count_old=ms_count;
    ms_count++;
    ms_count_old2=ms_count2;
    ms_count2++;
    if(ms_count == 5) { // 0,1 second
        ms_count = 0; // reset ms counter
        timeold=time;
        time=time+1;
        enable=1;        
    }
    if(ms_count2 == 10000) { // 10 second
        ms_count2 = 0; // reset ms counter
        enable2=1;        
    }
    
}


int main(void)
{
    CyGlobalIntEnable; /* Enable global interrupts. */
    
    /*Startup de los bloques del sistema*/
    Timer_Start();//Inicialización del timer para 5ms, para darle el tiempo de lectura a los Quad
    LCD_Start();       
    QuadDecH_Start(); //Sensor cuadratura para movimiento horizontal
    QuadDecV_Start(); //Sensor cuadratura para movimiento vertical
    Millis_Start();
    isr_Timer_1_StartEx(isr_Timer_1); //Inicializacion de la interrupcion para contar segundos con ms
    PWMv_Start();
    PWMh_Start();
  
       
    //-------------------------BUCLE PRINCIPAL----------------------------
    for(;;)
    {
    
        dir=QuadDecH_GetCounter();// Transformacion a Centimetros aprox.
        posx0=dir*0.001273239; // Transformacion a Centimetros aprox.
        
        velocidad();
        movimiento();
        
        
      
        if (move==0)
        {
            
            controlY();
            pwm= (int16) ((fabs(sc)/V_MAX)*N_MAX + 0.5);
        
            if (pwm > PWM_MAX)
            {
                pwm=PWM_MAX;
            }
        
            PWMh_WriteCompare2(pwm);
            PWMh_WriteCompare1(0);
            
        
        }
        
        else if (move==1)
        {
            controlY();
            pwm= (int16) ((fabs(sc)/V_MAX)*N_MAX + 0.5);
        
            if (pwm > PWM_MAX)
            {
                pwm=PWM_MAX;
            }
        
            PWMh_WriteCompare1(pwm);
            PWMh_WriteCompare2(0);
    
        }

        
       
        
        LCD_Position(0,0);
        sprintf(displayStrH ,"VerH: %.3f", posx0);
        LCD_PrintString(displayStrH);
    
        LCD_Position(1,0);
        sprintf(displayStrV ,"VelV: %.3f", vel);
        LCD_PrintString(displayStrV);        
    
    
}




}













void controlY()// En esta funcion se encuentra la funcion de transferencia del control
{

    eX= 10 - vel; // referencia de velocidad  
    
    sc= c1*eX - c2*eX1 + sc1;
    
    eX1=eX; //x(n)-> x(n-1)
    eX2=eX1; //x(n-1)-> x(n-2)
    
    vel1=vel;//
    vel2=vel1;//
    
    sc1=sc;
    
     
    
}

void controlX()// En esta funcion se encuentra la funcion de transferencia del control
{

 //   eX= REFF - posx0;
    
    sc= c1*eX - c2*eX1 - c3*eX2 + c4*posx2;
    
    eX1=eX; //x(n)-> x(n-1)
    eX2=eX1; //x(n-1)-> x(n-2)
    
    posx1=posx0;//
    posx2=posx1;//
    
    sc1=sc;
    sc2=sc;

}


void velocidad()
{
    if (enable==1)
        {
            vel=(fabs(posx0)-fabs(posx1)*100);
            posx1=posx0;
            enable=0;
        }
    
}

void movimiento()
{
    deltaX=dir1-dir;
    if (deltaX==0&&move==0)
    {
        move=1;
    }
    
    
}


/* [] END OF FILE */
