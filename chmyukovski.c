#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <complex.h>
#include <string.h>

#define N 200 // Número de puntos de la circunferencia
#define M 31 // Número de puntos de la malla
#define Mi 50 // Número de puntos de la malla compleja

// PI queda definido como M_PI al importar math.h
// #define M_PI 3.14159265358979323846

/* Declaración de algunos prototipos */
int menu(int control, float * dperfil, float * opc, float * opp, float * opf);
int menu_opciones (float * opc, float * opp, float * opf);
// Con esto evitamos warnings por declaraciones implícitas en funciones que se llaman entre sí


/*****************/
/*** FUNCIONES ***/
/*****************/

/* La función devuelve las n divisiones equiespaciadas del intervalo dado */ 
int linspace(float * vector, float x0, float x, int n)
{
	int i;
	float parte;

	vector[0] = x0; // Valor incial
	vector[n] = x; // Valor final

	parte = (x - x0) / (float) (n-1); // Intervalo

	for (i=1; i < n; i++)
	{
		vector[i] = vector[i-1] + parte;
	}

	return 0;
}

/* Toma grados y devuelve radianes */
float deg2rad(float deg)
{
	return(deg*2*M_PI/360);
}

/* Introducción de los valores para calcular perfil */
int datos_perfil(float * dperfil) 
{
	printf("\033[32m \nIntroduzca los valores:");
	printf("\033[0m\n");

	printf("Xc: "); 
	scanf ("%f", &dperfil[0]);
	printf("Yc: ");
	scanf ("%f", &dperfil[1]);
	printf("R: ");
	scanf ("%f", &dperfil[2]);

	// beta
	dperfil[3] = asin(dperfil[1]/dperfil[2]);

	// b
	dperfil[4] = dperfil[0] + dperfil[2] * cos(dperfil[3]);
															 
	// El caso queda definido al pasar el vector por limites
	dperfil[5] = 0; // Por ahora

	//Futuros limites dinámicos del ploteo
	dperfil[6] = dperfil[0] - dperfil[2] - 1; //-X ploteo circunferencia
	dperfil[7] = dperfil[0] + dperfil[2] + 1; //+X ploteo circunferencia
	dperfil[8] = dperfil[1] - dperfil[2] - 1; //-Y ploteo circunferencia
	dperfil[9] = dperfil[1] + dperfil[2] + 1; //+Y ploteo circunferencia

	// Radio
	dperfil[14] = dperfil[2] * sqrt((pow(1 + dperfil[0],2) + pow(dperfil[1],2)));

	return 0;
}

/* Función interpolada para  límites cuando --> x<1 && y<1 */
float f1(float x, float y) 
{
	float alim;
	alim = 1.0002 * (1.43 * (x - 0.0001) * (y - 0.0001) + (1 - x) * (y - 0.0001) + 1.1 * (x - 0.0001) * (1 - y) + 0.0002 * (1 - x) * (1 - y));
	return alim;
}

/* Función interpolada para  límites cuando --> x>1 && y<1 */
float f2(float x, float y) 
{
	float alim;
	alim = 0.25 * (5.2 * (x - 1) * (y - 0.0001) + 1.43 * (5 - x) * (y - 0.0001) + 5.1 * (x - 1) * (1 - y) + 1.1 * (5 - x) * (1 - y));
	return alim;
}

/* Función interpolada para  límites cuando --> x<1 && y>1 */
float f3 (float x, float y) 
{
	float alim;
	alim = 0.25 * (7 * (x - 0.0001) * (y - 1) + 5 * (1 - x) * (y - 1) + 1.43 * (x - 0.0001) * (5 - y) + (1 - x) * (5 - y));
	return alim;
}

/* Función interpolada para  límites cuando --> x>1 && y>1 */
float f4(float x, float y)
{
	float alim;
	alim = (9 * (x - 1) * (y - 1) + 7 * (5 - x) * (y - 1) + 5.2 * (x - 1) * (5 - y) + 1.43 * (5 - x) * (5 - y))/16;
	return alim;
}

/* Limita el valor de a (radio) para que la transformación sea eficaz */
int limites(float * dperfil)
{
	if (dperfil[2] > 15)
	{
		printf("\033[31mRadio muy grande "); 
		printf("\033[0m\n");	
		return 0;
	}

	if (dperfil[0]>6 || dperfil[1]>6 || dperfil[0]<-6 || dperfil[1]<-6) //Limitacion dperfil menos que 6 o mas que -6
	{
		printf("\033[31mValores no válidos (-6>Xc>6 || -6>Yc>6)");
		printf("\033[0m\n");	
		return 0;
	}

	if (dperfil[2] <= 0) // El valor del radio siempre tiene que ser mayor que cero
	{
		printf("\033[31mValores no válidos (R<=0)"); 
		printf("\033[0m\n");	
		return 0; // 
	}

	if (dperfil[2] < dperfil[1]) // El valor del radio siempre tiene que ser mayor que el alejamiento del eje y
	{
		printf("\033[31mValores no válidos (R<Yc)"); 
		printf("\033[0m\n");	
		return 0;
	}

	if (dperfil[0]==0 && dperfil[1]==0) // CASO 1: no planteado 
	{
		printf("\033[31mValores no válidos (Xc==0 && Yc==0)");
		printf("\033[0m\n");	
		return 0;
	}	

	if (dperfil[0]==0) //CASO 2
	{
		if (dperfil[2]==dperfil[1])
		{
			//printf("Valores válidos (a=yc)\n");
			dperfil[5]=2;
			return 1;			
		}
		else //Si a<xc o a>xc
		{
			printf("\033[31mValores no válidos (R!=Yc)"); 
			printf("\033[0m\n");	
			return 0;
		}	
	}
 
	if (dperfil[1]==0) //CASO 3
	{
		if (dperfil[2]>dperfil[0])
		{
			if (dperfil[2]>dperfil[0]+1)
			{
				dperfil[5]=3;
				return 1;
			}
			else
			{
				printf("\033[31mValores no válidos (Xc muy proximo a R)"); 
				printf("\033[0m\n");	
				return 0;	
			}
		}
		else //Si a==xc o a<xc
		{
			printf("\033[31mValores no válidos (R<=Xc)"); 
			printf("\033[0m\n");	
			return 0;
		}
			
	}

	else //CASO 4
	{
		if (dperfil[0]<=1)
		{
			if (dperfil[1]<=1) //xc<1 yc<1 ----> f1
			{
				if (f1(dperfil[0],dperfil[1])<=dperfil[2])
				{
					//printf("Valores válidos (f1(xc,yc)<=a)\n"); 
					dperfil[5]=4;
					return 1;
				}
				else
				{
					printf("\033[31mValores no válidos (f1(Xc,Yc)>R)"); 
					printf("\033[0m\n");	
					return 0;
				}
			}
			else //xc<1 yc>1 ----> f3
			{
				if (f3(dperfil[0],dperfil[1])<=dperfil[2])
				{
					//printf("Valores válidos (f3(xc,yc)<=a)\n"); 
					dperfil[5]=4;
					return 1;
				}
				else
				{
					printf("\033[31mValores no válidos (f3(Xc,Yc)>R)"); 
					printf("\033[0m\n");	
					return 0;
				}
			}
		}
		else
		{
			if (dperfil[1]<=1) //xc>1 yc<1 ----> f2
			{
				if (f2(dperfil[0],dperfil[1])<=dperfil[2])
				{
					//printf("Valores válidos (f2(xc,yc)<=a)\n"); 
					dperfil[5]=4;
					return 1;
				}
				else
				{
					printf("\033[31mValores no válidos (f2(Xc,Yc)>R)"); 
					printf("\033[0m\n");	
					return 0;
				}
			}
			else //xc>1 yc>1 ----> f4
 			{
				if (f4(dperfil[0],dperfil[1])<=dperfil[2] )
				{
					//printf("Valores válidos (f4(xc,yc)<=a)\n");
					dperfil[5]=4;
					return 1;
				}
				else
				{
					printf("\033[31mValores no válidos (f4(Xc,Yc)>R)"); 
					printf("\033[0m\n");	
					return 0;
				}
			}
		}
	}
}

/* Devuelve matriz nx2 con todos los puntos de la circunferencia */
int matriz_circunferencia(float * dperfil, float ** circunferencia)
{
	int i;

	// Valores de ángulo t para las ecuaciones paramétricas
	float * valores_t;
	valores_t = (float *) malloc(N * sizeof(float));

	linspace(valores_t, 0, 2*M_PI, N); // linspace divide uniformemente el intervalo 2*pi en N partes

	// Cálculo de cada punto para cada valor de t
	// Almacenamiento en matriz nx2
	for (i = 0; i < N; ++i)
	{
		circunferencia[i][0] = dperfil[0] + dperfil[2] * cos(valores_t[i]);
		circunferencia[i][1] = dperfil[1] + dperfil[2] * sin(valores_t[i]);
	}

	return 0;
}

/* Copian en un archivo .dat una lista de puntos (matriz nx2) para imprimir en GNU Plot */
int imprimir_circunferencia(float ** circunferencia)
{
	// Apertura del archivo donde se almacenan los puntos de la circunferencia para ser impresos con GNU Plot
	FILE * file_circunferencia; 
	file_circunferencia = fopen("pts_circun.dat", "w+");

	if (file_circunferencia == NULL)
	{
		printf("\033[31mError al abrir el archivo\n");
		printf("\033[0m\n");	
		return 1;
	}

	int i;

	for (i=0; i < (N); i++) // Escribe cada punto (fila de la matriz) en el archivo
	{
		fprintf(file_circunferencia, "%f %f\n", circunferencia[i][0], circunferencia[i][1]);
	}

	fprintf(file_circunferencia, "%f %f\n", circunferencia[0][0], circunferencia[0][1]); // Termina con el primer punto (para cerrar el polígono)
	fclose(file_circunferencia); // Cierre del archivo

	return 0;
}


/* Plotea los puntos de la circunferencia */
int plotc(float * dperfil, float * opc)
{
	//Calculo de rango dinamico y asignacion en el vector dperfil
	dperfil[6] = dperfil[0] - dperfil[2] - 1; 
	dperfil[7] = dperfil[0] + dperfil[2] + 1;
	dperfil[8] = dperfil[1] - dperfil[2] - 1;
	dperfil[9] = dperfil[1] + dperfil[2] + 1;

	//Modificacion del rango para que se vean los ejes de coordenadas con finalidad didactica
	float minx, maxx, miny, maxy;
	minx = dperfil[6];
	maxx = dperfil[7];
	miny = dperfil[8];
	maxy = dperfil[9];

	//Si la circunferencia esta en cualquiera de los cuadrantes sin cortar a los ejes, se fuerza que el rango crezca
	if (minx<0 && maxx<0)
		maxx=5;
	else if (minx>0 && maxx>0)
		minx=-5;

	if (miny<0 && maxy<0)
		maxy=5;
	else if (miny>0 && maxy>0)
		miny=-5;

	// Tubería UNIX para usar GNU Plot desde el programa
	FILE *pipec = popen ("gnuplot -persistent","w");

	fprintf(pipec, "set size square \n set nokey \n set xzeroaxis \n set yzeroaxis \n"); 
	fprintf(pipec, "plot [%f:%f] [%f:%f] \"pts_circun.dat\" w filledcurves x1 fs pattern %.0f lc %.0f, \"pts_circun.dat\" pt %.0f ps %f lt %.0f\n", minx, maxx, miny, maxy,  opc[3], opc[4], opc[0], opc[1], opc[2]);
	fprintf(pipec, "set term pngcairo \n set output \"circulo.png\" \n replot \n exit");

	fflush(pipec);

	pclose (pipec);

	return 0;
}


/* Aplica la transformación de Yukovski a cada punto de la circunferencia */
int transformacion_yukovski(float * dperfil, float ** circunferencia)
{
	int i; 
	float x, y;

	if (dperfil[5]==0 || dperfil[5]==1) // Así eliminamos el caso 1 y si fuera 0 
	{
		printf("\033[31m¿Cómo has llegado hasta aqui?\n");
		printf("\033[0m\n");	
		return 0;
	}

	for (i = 0; i < N ; i++) //Para el resto de casos funcionan estas ecuaciones 
	{
		x = circunferencia[i][0];
		y = circunferencia[i][1];
		circunferencia[i][0] = x * (1+(pow(dperfil[4],2)/(pow(x,2)+pow(y,2))));
		circunferencia[i][1] = y * (1-(pow(dperfil[4],2)/(pow(x,2)+pow(y,2))));
	}

	return 0;
}


/* Copian en un archivo .dat una lista de puntos (matriz nx2) para imprimir en GNU Plot */
int imprimir_perfil(float ** perfil)
{
	// Apertura del archivo donde se almacenan los puntos del perfil para ser impresos con GNU Plot
	FILE * file_perfil;
	file_perfil = fopen("pts_perfil.dat", "w+");

	if (file_perfil == NULL)
	{
		printf("\033[31mError al abrir el archivo\n");
		printf("\033[0m\n");	
		return 1;
	}

	int i;

	for (i=0; i < (N); i++)
	{
		fprintf(file_perfil, "%f %f\n", perfil[i][0], perfil[i][1]); // Escribe cada punto (fila de la matriz) en el archivo
	}

	fprintf(file_perfil, "%f %f\n", perfil[0][0], perfil[0][1]); // Termina con el primer punto (para cerrar el polígono)
	fclose(file_perfil); // Cierre del archivo

	return 0;
}


/* Plotea los puntos transformados para el perfil */
int plotp (float * dperfil, float ** circunferencia, float * opp)
{
	int i;
	float mayorx = 0.0;
	float menorx = 0.0;
	float mayory = 0.0;
	float menory = 0.0;

	for (i=0; i<N; i++) // Mayor elemento eje x
	{
    	if (circunferencia[i][0] > mayorx)
    	{
   			mayorx = circunferencia[i][0];
    	}
    }
    mayorx = mayorx + dperfil[2];

	for (i=0; i<N; i++) // Mayor elemento eje x
	{
    	if (circunferencia[i][0] < menorx)
    	{
   			menorx = circunferencia[i][0];
    	}
    }
    menorx = menorx - dperfil[2];

	
	for (i=0; i<N; i++) // Mayor elemento eje y
	{
    	if (circunferencia[i][1] > mayory)
    	{
   			mayory = circunferencia[i][1];
	   	}
    }
    mayory = mayory + dperfil[2];

	for (i=0; i<N; i++) // Mayor elemento eje y
	{
    	if (circunferencia[i][1] < menory)
    	{
   			menory = circunferencia[i][1];
    	}
    }
    menory = menory - dperfil[2];

    dperfil[10] = menorx;
    dperfil[11] = mayorx;
    dperfil[12] = menory;
    dperfil[13] = mayory;


    // Tubería UNIX para usar GNU Plot desde el programa
	FILE *pipep = popen ("gnuplot -persistent","w"); 

	fprintf(pipep, "set size ratio 0.3 \n set nokey \n set xzeroaxis \n set yzeroaxis \n");
	fprintf(pipep, "plot [%f:%f] [%f:%f] \"pts_perfil.dat\" w filledcurves x1 fs  pattern %.0f lc %.0f, \"pts_perfil.dat\" pt %.0f ps %f lt %.0f\n", menorx, mayorx, menory, mayory, opp[3], opp[4], opp[0], opp[1], opp[2]);
	fprintf(pipep, "set term pngcairo \n set output \"perfil.png\" \n replot \n exit");

	fflush(pipep);

	pclose (pipep);

	return 0;
}


/* Menú para modificar opciones de plot de circunferencia */
int menu_circ (float * opc, float * opp, float * opf)
{
	printf("\033[33m   1. Tipo de punto \n   2. Tamaño de punto \n   3. Color de punto \n   4. Tipo de malla \n   5. Color de malla\n   6. Salir"); // 1: (p)oint(t)ype   2: (p)oint(s)ize   3: (l)ine(t)ype    4:(pattern) malla     5:(l)ine(c)olor malla  
	printf("\033[0m\n");
	int opcion, pt, lt, pattern, lc;
	float ps;

	scanf ("%d", &opcion);
	while (opcion!=1 && opcion!=2 && opcion!=3 && opcion!=4 && opcion!=5 && opcion!=6) //En caso de que el valor introducido sea diferente del esperado, espera otra introduccion
	{
		printf("\033[31m   Valor no valido"); 
		printf("\033[0m\n");
		scanf("%d", &opcion);
	}

	switch(opcion)
	{
		case 1:
			printf("\033[33m  (2) Cruz \n  (3) Asterisco \n  (4) Cuadrado \n  (7) Punto");	
			printf("\033[0m\n");
			scanf ("%d", &pt);

			while (pt!=2 && pt!=3 && pt!=4 && pt!=7) // En caso de que el valor introducido sea diferente del esperado, espera otra introduccion
			{	
				printf("\033[31m  Valor no valido"); 
				printf("\033[0m\n");
				scanf("%d", &pt);
			}

			opc[0]= (float) pt;  // Introducimos el valor obtenido (con su correspondiente casting) en el vector

			menu_circ (opc, opp, opf);
			break;

		case 2:
			printf("\033[33m  Tamaño del punto: ");
			printf("\033[0m");
			scanf ("%f", &ps);

			opc[1]=ps;  // Introducimos el valor obtenido en el vector

			if (ps > 2)
			{
				printf("\033[31m Te van a quedar puntos muy grandes!\n");
				printf("\033[0m\n");
			}

			else
				printf("\033[0m\n");

			menu_circ (opc, opp, opf); // Se vuelve al menu del circulo
			break;

		case 3:
			printf("\033[33m  (1) Rojo \n  (2) Verde \n  (3) Azul \n  (7) Negro \n  (9) Gris");
			printf("\033[0m\n");
			scanf ("%d", &lt);

			while (lt!=1 && lt!=2 && lt!=3 && lt!=7 && lt!=9)  // En caso de que el valor introducido sea diferente del esperado, espera otra introduccion
			{	
				printf("\033[31m  Valor no valido"); 
				printf("\033[0m\n");
				scanf("%d", &lt);
			}

			opc[2]= (float) lt;  // Introducimos el valor obtenido (con su correspondiente casting) en el vector

			menu_circ (opc, opp, opf); // Se vuelve al menu del circulo
			break;

		case 4:
			printf("\033[33m  (1) Malla 1\n  (2) Malla 2 \n  (3) Superficie");
			printf("\033[0m\n");
			scanf ("%d", &pattern);

			while (pattern!=1 && pattern!=2 && pattern!=3)  // En caso de que el valor introducido sea diferente del esperado, espera otra introduccion
			{	
				printf("\033[31m  Valor no valido");
				printf("\033[0m\n");
				scanf("%d", &pattern);
			}

			opc[3]= (float) pattern; // Introducimos el valor obtenido (con su correspondiente casting) en el vector

			menu_circ (opc, opp, opf); // Se vuelve al menu del circulo
			break;

		case 5:
			printf("\033[33m  (1) Rojo \n  (2) Verde \n  (3) Azul \n  (7) Negro \n  (9) Gris");
			printf("\033[0m\n");
			scanf ("%d", &lc);

			while (lc!=1 && lc!=2 && lc!=3 && lc!=7 && lc!=9)
			{	
				printf("\033[31m  Valor no valido"); 
				printf("\033[0m\n");
				scanf("%d", &lc);
			}

			opc[4]= (float) lc;

			menu_circ (opc, opp, opf); // Se vuelve al menu del circulo
			break;

		case 6:
			menu_opciones(opc, opp, opf); // Se vuelve al menu de opciones
			break;
	}

	return 0;
}


/* Menú para modificar opciones de plot del perfil */
int menu_perfil (float * opc, float * opp, float * opf)
{
	printf("\033[33m  1. Tipo de punto \n  2. Tamaño de punto \n  3. Color de punto \n  4. Tipo de malla \n  5. Color de malla\n  6. Salir"); // 1: (p)oint(t)ype   2: (p)oint(s)ize   3: (l)ine(t)ype    4:(pattern) malla     5:(l)ine(c)olor malla  
	printf("\033[0m\n");
	int opcion, pt, lt, pattern, lc;
	float ps;

	scanf ("%d", &opcion);
	while (opcion!=1 && opcion!=2 && opcion!=3 && opcion!=4 && opcion!=5 && opcion!=6) //En caso de que el valor introducido sea diferente del esperado, espera otra introduccion
	{
		printf("\033[31m  Valor no valido");
		printf("\033[0m\n");	
		scanf("%d", &opcion);
	}

	switch(opcion)
	{
		case 1:
			printf("\033[33m  (2) Cruz \n  (3) Asterisco \n  (4) Cuadrado \n  (7) Punto");
	        printf("\033[0m\n");
			scanf ("%d", &pt);

			while (pt!=2 && pt!=3 && pt!=4 && pt!=7) // En caso de que el valor introducido sea diferente del esperado, espera otra introduccion
			{	
				printf("\033[31m  Valor no valido"); 	
				printf("\033[0m\n");
				scanf("%d", &pt);
			}

			opp[0]= (float) pt;  // Introducimos el valor obtenido (con su correspondiente casting) en el vector

			menu_perfil (opc, opp, opf); // Se vuelve al menu del perfil
			break;

		case 2:
			printf("\033[33m  Tamaño del punto: ");
	        printf("\033[0m");
			scanf ("%f", &ps);

			opp[1]=ps;  // Introducimos el valor obtenido en el vector

			if (ps > 2)
			{
				printf("\033[31m Te van a quedar puntos muy grandes!\n");
				printf("\033[0m\n");
			}

			else
				printf("\033[0m\n");

			menu_perfil (opc, opp, opf); // Se vuelve al menu del perfil
			break;

		case 3:
			printf("\033[33m  (1) Rojo \n  (2) Verde \n  (3) Azul \n  (7) Negro \n  (9) Gris");
	        printf("\033[0m\n");
			scanf ("%d", &lt);

			while (lt!=1 && lt!=2 && lt!=3 && lt!=7 && lt!=9)  // En caso de que el valor introducido sea diferente del esperado, espera otra introduccion
			{	
				printf("\033[31m  Valor no valido"); 
				printf("\033[0m\n");
				scanf("%d", &lt);
			}

			opp[2]= (float) lt;  // Introducimos el valor obtenido (con su correspondiente casting) en el vector

			menu_perfil (opc, opp, opf); // Se vuelve al menu del perfil
			break;

		case 4:
			printf("\033[33m  (1) Malla 1\n  (2) Malla 2 \n  (3) Superficie");
	        printf("\033[0m\n");
			scanf ("%d", &pattern);

			while (pattern!=1 && pattern!=2 && pattern!=3)  // En caso de que el valor introducido sea diferente del esperado, espera otra introduccion
			{	
				printf("\033[31m  Valor no valido"); 
				printf("\033[0m\n");
				scanf("%d", &pattern);
			}

			opp[3]= (float) pattern; // Introducimos el valor obtenido (con su correspondiente casting) en el vector

			menu_perfil (opc, opp, opf); // Se vuelve al menu del perfil
			break;

		case 5:
			printf("\033[33m  (1) Rojo \n  (2) Verde \n  (3) Azul \n  (7) Negro \n  (9) Gris");
	        printf("\033[0m\n");
			scanf ("%d", &lc);

			while (lc!=1 && lc!=2 && lc!=3 && lc!=7 && lc!=9)
			{	
				printf("\033[31m  Valor no valido"); 
				printf("\033[0m\n");
				scanf("%d", &lc);
			}

			opp[4]= (float) lc;

			menu_perfil (opc, opp, opf); // Se vuelve al menu del perfil
			break;

		case 6:
			menu_opciones(opc, opp, opf); // Se vuelve al menu de opciones
			break;
	}

	return 0;
}


/* Menú para modificar opciones de plot del flujo del cilindro */
int menu_flujo (float * opc, float * opp, float * opf)
{
	printf("\033[33m   1. Color de flujo \n   2. Tamaño de la linea de flujo\n   3. Color del cilindro \n   4. Salir"); // 1: (l)ine(c)olor flujo   2: (l)ine(w)idth   3:(l)ine(c)olor cilindro  
	printf("\033[0m\n");
	int opcion, lcf, lcc;
	float lw;

	scanf ("%d", &opcion);
	while (opcion!=1 && opcion!=2 && opcion!=3 && opcion!=4) //En caso de que el valor introducido sea diferente del esperado, espera otra introduccion
	{
		printf("\033[31mValor no valido"); 
		printf("\033[0m\n");
		scanf("%d", &opcion);
	}

	switch(opcion)
	{
		case 1:
			printf("\033[33m  (1) Rojo \n  (2) Verde \n  (3) Azul \n  (7) Negro \n  (9) Gris");
	        printf("\033[0m\n");
			scanf ("%d", &lcc);

			while (lcc!=1 && lcc!=2 && lcc!=3 && lcc!=7 && lcc!=9)
			{	
				printf("\033[31m  Valor no valido"); 
				printf("\033[0m\n");
				scanf("%d", &lcc);
			}

			opf[0]= (float) lcc;

			menu_flujo (opc, opp, opf); // Se vuelve al menu del flujo
			break;

		case 2:
			printf("\033[33m  Tamaño de la linea de flujo: ");
			printf("\033[0m ");
			scanf ("%f", &lw);

			opf[2]=lw;  // Introducimos el valor obtenido en el vector

			if (lw > 2)
			{
				printf("\033[31m ¡Te van a quedar lineas muy gruesas!\n");
				printf("\033[0m\n");			
			}

			else
				printf("\033[0m\n");

			menu_flujo (opc, opp, opf); // Se vuelve al menu del flujo
			break;

		case 3:
			printf("\033[33m  (1) Rojo \n  (2) Verde \n  (3) Azul \n  (7) Negro \n  (9) Gris");
	        printf("\033[0m\n");
			scanf ("%d", &lcf);

			while (lcf!=1 && lcf!=2 && lcf!=3 && lcf!=7 && lcf!=9)
			{	
				printf("\033[31m  Valor no valido"); 
				printf("\033[0m\n");
				scanf("%d", &lcf);
			}

			opf[2]= (float) lcf;

			menu_flujo (opc, opp, opf); // Se vuelve al menu del perfil
			break;


		case 4: 
			menu_opciones(opc, opp, opf); // Se vuelve al menu de opciones
			break;
	}

	return 0;
}


/* Menu para modificar opciones de plot */ 
int menu_opciones (float * opc, float * opp, float * opf)
{
	int opcion;
	printf("\033[33m 1. Opciones del plot de la circunferencia \n 2. Opciones del plot del perfil \n 3. Opciones del plot del flujo \n 4. Salir");
	printf("\033[0m\n");
	scanf ("%d", &opcion);

	while (opcion!=1 && opcion!=2 && opcion!=3 && opcion!=4) //En caso de que el valor introducido sea diferente del esperado, espera otra introduccion
	{
		scanf("%d", &opcion);
	}

	switch(opcion)
	{
		case 1:
			menu_circ(opc, opp, opf); // Menú opciones círculo
			break;

		case 2:
			menu_perfil(opc, opp, opf); // Menú opciones perfil
			break;

		case 3:
			menu_flujo(opc, opp, opf); // Menú opciones flujo
			break;

		case 4:
			break; // Retorno al menú principal
	}
	
	return 0;
}


/* Construye e imprime el perfil alar con los datos especificados */
/* Primero circunferencia, tras la transformación de Yukovski el perfil */
int perfil(float * dperfil, float * opc, float * opp, float * opf)
{
	int i;

	// Matriz Nx2 que almacenará los puntos de la circunferencia
	float ** circunferencia;
	circunferencia = (float **) malloc(N * sizeof(float *)); // Reserva de memoria para cada vector

	for (i=0; i < N; i++)
	{
		circunferencia[i] = (float *) malloc(2 * sizeof(float)); // Reserva de memoria para las dos coordenadas del vector
	}

	// Toma de datos para el perfil
	do
	{
		datos_perfil (dperfil);
	} while (limites(dperfil) == 0);

	// Calcula los puntos de la circunferencia
	matriz_circunferencia(dperfil, circunferencia);

	// Guarda los puntos de la circunferencia en el archivo pts_circunferencia.dat
	if (imprimir_circunferencia(circunferencia)) // Si la apertura falla vuelve al menú
		menu(0, dperfil, opc, opp, opf);

	// Imprime la circunferencia con GNU Plot
	plotc(dperfil, opc);

	// Aplica la transformación de Yukovski a los puntos de la circunferencia para obener los del perfil
	transformacion_yukovski(dperfil, circunferencia);

	// Guarda los puntos del perfil en el archivo pts_perfil.dat
	if (imprimir_perfil(circunferencia)) // Si la apertura falla vuelve al menú
		menu(0, dperfil, opc, opp, opf);

	// Imprime el perfil con GNU Plot
	plotp(dperfil, circunferencia, opp);

	
	return 0;
}


/* Introducción de los datos para el cálculo del flujo */
int datos_flujo (float *dperfil, float *dflujo)
{
	printf("\033[32m \nIntroduzca los valores:");
	printf("\033[0m\n");

	float U, alpha;

	// Velocidad del aire incidente
	printf("U: ");
	scanf ("%f", &U);
	while (U>=3 || U<=0.9)
	{
		printf("\033[31mValor no valido:");
		printf("\033[0m ");
		scanf ("%f", &U);
	}

	dflujo[0] = U;

	// Ángulo de ataque
	printf("Alpha: ");
	scanf ("%f", &alpha);
	
	alpha = deg2rad(alpha);

	dflujo[1]= alpha;

	// Densidad del aire
	printf("Densidad del aire: ");
	scanf ("%f", &dflujo[2]);
	
	if(dflujo[2] > 3)
	{
		printf("\033[31mEstás en un atmosfera muy densa!");
		printf("\033[0m\n \n");
	}

	// Corriente T
	dflujo[3] = 4 * M_PI * dperfil[4] * U * (dperfil[1] + (1+dperfil[0]) * dflujo[1]);

	//Coeficiente de sustentación CL
	dflujo[4] = 2*M_PI*sin(dflujo[1]+dperfil[3]);


	return 0;
}


/* Crea dos matrices que almacenan las coordenadas x e y de la malla respectivamente */
int meshgrid(float ** xx, float ** yy, float *dperfil)
{
	int i, j;

	float * rango;
	rango = (float *) malloc(M * sizeof(float));

	linspace(rango, dperfil[6]-dperfil[2], dperfil[7]+dperfil[2], M); // Toma las dimensiones de la malla en función de la dimensión del cilindro

	for (i = 0; i < M; i++)
	{
		xx[i] = rango;
	}

	for (i = 0; i < M; i++)
		for (j = 0; j < M; j++)
		{
			yy[i][j] = rango[i];
		}

	return 0;
}	


/* Calcula el flujo en cada punto de la malla y lo devuelve en psi */
int calculo_flujo(float * dperfil, float * dflujo, float ** xx, float ** yy, float ** psi)
{
	int i,j;
	float rho, theta, T;

	for (i = 0; i < M; ++i)
		for (j = 0; j < M; ++j)
		{
			// Transformación desde la forma compleja
			rho = sqrt(pow(xx[i][j] - dperfil[0],2) + pow(yy[i][j] - dperfil[1],2)); 
			theta = atan((yy[i][j] - dperfil[1])/(xx[i][j] - dperfil[0]));

			// Flujo del remolino para que se cumple la condición de Kutta-Yukovski
			T = 4 * M_PI * dperfil[4] * dflujo[0] * sin(dflujo[1]);

			// Flujo complejo en un punto
			if (xx[i][j] > dperfil[0])
				psi[i][j] = dflujo[0] * (rho * sin(theta - dflujo[1]) - (pow(dperfil[2],2)/rho) * sin(theta - dflujo[1])) + (T/(2*M_PI)) * log(fabs(rho));
			else
				psi[i][j] = -dflujo[0] * (rho * sin(theta - dflujo[1]) - (pow(dperfil[2],2)/rho) * sin(theta - dflujo[1])) + (T/(2*M_PI)) * log(fabs(rho));
		}


	return 0;
}


/* Exporta la malla y el flujo en sus puntos para imprimir con GNU Plot */
/* Guarda en "pts_flujo_cilindro.dat" (X Y psi) por columnas */
int imprimir_flujo(float ** xx, float ** yy, float ** psi)
{
	FILE * file_flujo_cilindro;
	file_flujo_cilindro = fopen("pts_flujo_cilindro.dat", "w+");

	if (file_flujo_cilindro == NULL)
	{
		printf("\033[31m Error al abrir el archivo\n");
		printf("\033[0m\n");
		return(0);
	}

	int i, j;

	for (i = 0; i < M; ++i)
		for (j = 0; j < M; ++j)
		{
			fprintf(file_flujo_cilindro, "%f %f %f\n", xx[i][j], yy[i][j], psi[i][j]); // Escribe cada punto (fila de la matriz) en el archivo
		}

	fclose(file_flujo_cilindro); // Cierre del archivo

	return 0;
}


/* Plotea el flujo del cilindro */
int plotfc (float *opf, float ** psi)
{
	int i, j;

	float parte = 0;
	float psimax = 0;
	float psimin = 0;

	for (i = 0; i < M; ++i)
		for (j = 0; j < M; ++j)
		{
			if (psi[i][j] > psimax)
				psimax = psi[i][j];

			if (psi[i][j] < psimin)
				psimin = psi [i][j];
		}

	parte = ((psimax - psimin)/200); // Intervalo

	if (psimin+100*parte < parte)
	{
		parte = parte + (parte / (float) (10));
	}

	if (psimin+100*parte > parte)
	{
		parte = parte + (parte / (float) (10));
	}

	// Tubería UNIX para usar GNU Plot desde el programa
	FILE *pipefc = popen ("gnuplot -persistent","w"); 

	fprintf(pipefc, "set terminal push\n set terminal unknown\n set table 'temp.dat'\n set dgrid3d 31,31\n set view map\n unset surface\n set contour\n set cntrparam bspline\n set cntrparam levels incr %f,%f,%f\n splot 'pts_flujo_cilindro.dat' using 1:2:3 with lines\n unset table\n unset dgrid3d\n unset key\n set terminal pop\n", psimin, parte, psimax);
	fprintf(pipefc, "set size ratio 1\n plot 'temp.dat' with lines lc %.0f lw %f, 'pts_circun.dat' with filledcurves x1 fs pattern 3 lc %.0f\n !rm temp.dat\n", opf[0], opf[1], opf[2]);

	fflush(pipefc);

	pclose (pipefc);


	return 0;
}

/* Transformación de Yukovski en término de números complejos. Toma matriz NxN */
int transformacion_yukovski_imaginario(float * dperfil, complex double * circunferencia_compleja)
{
	int i;

	for (i = 0; i < N; ++i)
	{
		circunferencia_compleja[i] = circunferencia_compleja[i] + pow(dperfil[2],2)/circunferencia_compleja[i];
	}

	return 0;
}

/* Copian en un archivo .dat una lista de puntos (matriz nx2) para imprimir en GNU Plot */
int imprimir_perfil_imaginario(double ** circunferenciapr)
{
	// Apertura del archivo donde se almacenan los puntos de la circunferenciapr para ser impresos con GNU Plot
	FILE * file_perfil_imaginario; 
	file_perfil_imaginario = fopen("pts_perfil_imaginario.dat", "w+");

	if (file_perfil_imaginario == NULL)
	{
		printf("\033[31mError al abrir el archivo\n");
		printf("\033[0m\n");	
		return 1;
	}

	int i;

	for (i=0; i < (N); i++) // Escribe cada punto (fila de la matriz) en el archivo
	{
		fprintf(file_perfil_imaginario, "%f %f\n", circunferenciapr[i][0], circunferenciapr[i][1]);
	}

	fprintf(file_perfil_imaginario, "%f %f\n", circunferenciapr[0][0], circunferenciapr[0][1]); // Termina con el primer punto (para cerrar el polígono)

	fclose(file_perfil_imaginario); // Cierre del archivo


	return 0;
}

/* Crea e imprime el perfil alar usando números complejos */
int perfil_imaginario(float * dperfil)
{
	int i;

	double ** circunferenciapr;
	circunferenciapr = (double **) malloc(N * sizeof(double *));
	for (i=0; i < N; i++)
		circunferenciapr[i] = (double *) malloc(N * sizeof(double));

	complex double t0; // Centro de la circunferenciapr
	t0 =  dperfil[2] * (-dperfil[0] + dperfil[1] * I);

	// Valores de ángulo t para las ecuaciones paramétricas
	float * valores_t;
	valores_t = (float *) malloc(N * sizeof(float));

	linspace(valores_t, 0, 2*M_PI, N); // linspace divide uniformemente el intervalo 2*pi en N partes

	// Puntos de la circunferenciapr
	for (i = 0; i < N; ++i)
	{
		circunferenciapr[i][0] = creal(t0) + (double) dperfil[14] * cos(valores_t[i]);
		circunferenciapr[i][1] = cimag(t0) + (double) dperfil[14] * sin(valores_t[i]);
	}


	complex double * circunferencia_compleja;
	circunferencia_compleja = (complex double *) malloc(N * sizeof(complex double));

	//Circunferenciapr en números complejos
	for (i = 0; i < N; ++i)
		circunferencia_compleja[i] = circunferenciapr[i][0] + circunferenciapr[i][1] * I;

	// Transformación
	transformacion_yukovski_imaginario(dperfil, circunferencia_compleja);

	// Puntos con el perfil
	for (i = 0; i < N; ++i)
	{
		circunferenciapr[i][0] = creal(circunferencia_compleja[i]);
		circunferenciapr[i][1] = cimag(circunferencia_compleja[i]);
	}

	imprimir_perfil_imaginario(circunferenciapr);

	//Obtencion maximos de todos los ejes para meshgrid_imaginario
	float ejexmax = 0;
	float ejexmin = 0;
	float ejeymax = 0;
	float ejeymin = 0;

	for (i = 0; i < N; ++i)
	{
		if (circunferenciapr[i][0] > ejexmax)
			ejexmax = circunferenciapr[i][0];

		if (circunferenciapr[i][0] < ejexmin)
			ejexmin = circunferenciapr[i][0];

		if (circunferenciapr[i][1] > ejeymax)
			ejeymax = circunferenciapr[i][1];

		if (circunferenciapr[i][1] < ejeymin)
			ejeymin = circunferenciapr[i][1];
	}

	dperfil[15]=ejexmin;
	dperfil[16]=ejexmax;	
	dperfil[17]=ejeymin;
	dperfil[18]=ejeymax;

	return 0;
}

/* Crea las matrices que almacenan las coordenadas de la malla compleja. */
/* Almacena en matrices de la misma dimensión que la malla MixMi */
int meshgrid_imaginario(float ** xxpr, float ** yypr, complex double ** tt, float * dperfil)
{
	int i, j;

	int xmax = -dperfil[15]+dperfil[16]+1.25*dperfil[14];  //Rango dinamico dependiendo de las dimensiones del perfil elegido
	int ymax = -dperfil[17]+dperfil[18]+1.15*dperfil[14];

	float * rangox;
	rangox = (float *) malloc(Mi * sizeof(float));

	float * rangoy;
	rangoy = (float *) malloc(Mi * sizeof(float));

	linspace(rangox, -xmax, xmax, Mi);
	linspace(rangoy, -ymax, ymax, Mi);

	for (i = 0; i < Mi; i++) //Matriz XXpr
	{
		xxpr[i] = rangox;
	}

	for (i = 0; i < Mi; i++) //Matriz YYpr
		for (j = 0; j < Mi; j++)
		{
			yypr[i][j] = rangoy[i];
		}

	for (i = 0; i < Mi; i++) //Matriz tt
		for (j = 0; j < Mi; j++)
		{
			tt[i][j] = xxpr[i][j]+yypr[i][j]*I;
		}

	return(0);
}	

/* Calcula el flujo complejo del fluido en el cilindro */
int calculo_flujo_imaginario(complex double ** tt, double ** psipr, float * dperfil)
{
	int i,j;

	complex double t0; // Centro de la circunferenciapr
	t0 =  dperfil[2] * (-dperfil[0] + dperfil[1] * I);

	double U = 1;
	double alpha = 0;
	double T = 4 * M_PI * dperfil[2] * U * (dperfil[1] + (1+dperfil[0]) * alpha);
	for (i = 0; i < Mi; ++i)
		for (j = 0; j < Mi; ++j)
		{
			psipr[i][j] = cimag(U * ((tt[i][j]-t0)*cexp(-alpha*I)+pow(dperfil[14],2)/(tt[i][j]-t0)*cexp(alpha*I)) + I * T/(2*M_PI)*clog(tt[i][j]-t0)); //Formula del flujo complejo sin turbulencias
		}

	return 0;
}

/* Elimina de la malla los valores que distorsionan los resultados del flujo */ 
int arregla_malla(complex double **tt, float * dperfil)
{
	complex double t0 = 0.2 + 0.3 * I;

	int i, j;
	for (i = 0; i < Mi; ++i)
		for (j = 0; j < Mi; ++j)
		{
			if (cabs(tt[i][j]-t0) < 0.95 * dperfil[2]) //Los valores se disponen a 0.95*radio del centro del perfil
				tt[i][j]=0;
		}

	return 0;
}

/* Pasa de la malla compleja a una malla cartesiana */
int nueva_malla (float **xxtau, float **yytau, complex double ** tt, float *dperfil)
{
	complex double tau;

	int i, j;

	for (i = 0; i < Mi; ++i)
		for (j = 0; j < Mi; ++j)
		{
			tau = tt[i][j] + pow(dperfil[2],2)/tt[i][j]; //Nueva malla compleja
			xxtau[i][j] = creal(tau); //Parte real
			yytau[i][j] = cimag(tau); //Parte imaginaria
		}

	return 0;
}

/* Transfiere a un archivo "pts_flujo_perfil.dat" los valores del flujo para cada parte de la malla */
int imprimir_flujo_perfil(float **xxtau, float **yytau, double **psipr)
{
	FILE * matriz_archivo;
	matriz_archivo = fopen("pts_flujo_perfil.dat", "w+");

	int i, j;

	for (j = 0; j < Mi; j++)
	{
		for (i = 0; i < Mi; i++)
		{
			fprintf(matriz_archivo, "%f ", xxtau[i][j]);
			fprintf(matriz_archivo, "%f ", yytau[i][j]);
			fprintf(matriz_archivo, "%lf \n", psipr[i][j]);
		}

		fprintf(matriz_archivo, "\n");
	}
	
	return 0;
}

/* Plotea el flujo en el prefil a través de GNU Plot */
int plotfp (double **psipr, float *opf)
{
	int i, j;

	float parte = 0;
	float psiprmax = 0;
	float psiprmin = 0;

	for (i = 0; i < Mi; ++i)
		for (j = 0; j < Mi; ++j)
		{
			if (psipr[i][j] > psiprmax)
				psiprmax = psipr[i][j];

			if (psipr[i][j] < psiprmin)
				psiprmin = psipr [i][j];
		}

	parte = ((psiprmax - psiprmin)/200); // Intervalo

	if ((psiprmin+100*parte) - parte < 0.1)
		parte += 0.05;

	if ((psiprmin+100*parte) - parte > 0.1)
		parte += 0.05;

	// Tubería UNIX para usar GNU Plot desde el programa
	FILE *pipefp = popen ("gnuplot -persistent","w"); 

	fprintf(pipefp, "set terminal push \n set terminal unknown \n set table 'temp.dat' \n set view map \n set contour \n set cntrparam levels incr %f,%f,%f \n unset surface \n unset clabel \n splot 'pts_flujo_perfil.dat' with lines lc 3 \n unset table \n set terminal pop \n unset key \n set size ratio 0.3 \n plot 'temp.dat' with lines lc %0.f lw %f, 'pts_perfil_imaginario.dat' with filledcurves x1 fs pattern 3 lc %0.f\n !rm temp.dat\n", psiprmin, parte, psiprmax, opf[0], opf[1], opf[2]);

	return 1;
}

/* Límites para el cálculo del flujo complejo en el perfil */
int limites_imaginario(float *dperfil)
{
	if (limites(dperfil)==0)
		return 0;

	if (dperfil[0]<=0)
	{
		printf("\033[31mValores no válidos (Xc<=0)"); 
		printf("\033[0m\n");	
		return 0;
	}

	if (dperfil[0]<0)
	{
		printf("\033[31mValores no válidos (Yc<0)"); 
		printf("\033[0m\n");	
		return 0;
	}

	return 1;
}

/* Función que encapsula los cálculos e impresiones del flujo complejo en cilindro y perfil */
int flujo(float * dperfil, float * opc, float * opp, float * opf)
{
	int i;
	char opcionf, opcion;
	int mct;

	// Vector con los datos del flujo
	float * dflujo;
	dflujo = (float *) malloc(5 * sizeof(float)); // Reserva de memoria para el vector

	// Matriz Nx2 que almacenará los puntos de la circunferencia
	float ** circunferencia;
	circunferencia = (float **) malloc(N * sizeof(float *)); // Reserva de memoria para cada vector
	for (i=0; i < N; i++)
	{
		circunferencia[i] = (float *) malloc(2 * sizeof(float)); // Reserva de memoria para las dos coordenadas del vector
	}

	printf("\033[32m \n1. Obtención del flujo a traves del cilindro (Th. Kutta-Yukovski)\n2. Obtención del flujo a traves del perfil alar");
	printf("\033[31m (abandona el programa)");
	printf("\033[0m \n");

	do
	{
		scanf ("%d", &mct);
	} while((mct != 1) && (mct != 2));

	// Datos del cilindro
	if (dperfil[2]==0) // Si no se han dado los datos del cilindro el radio sera 0
	{
		do
		{
			datos_perfil(dperfil);
		} while (limites_imaginario(dperfil) == 0);
	}
	else
	{
		printf("\033[32m \n¿Desea dar nuevos valores al cilindro? (s/n)");
		printf("\033[0m \n");
		scanf ("%c", &opcionf);

		do
		{
			scanf ("%c", &opcion);
		} while (opcion != 's' && opcion !=  'n');

		if (opcion == 's')
		{
			do
			{
				datos_perfil(dperfil);
			} while (limites_imaginario(dperfil) == 0);
		}
	}

	if (mct == 1)
	{
		// Datos para el flujo
		datos_flujo(dperfil, dflujo);

		// Calcula los puntos de la circunferencia
		matriz_circunferencia(dperfil, circunferencia);

		// Guarda los puntos de la circunferencia en el archivo pts_circunferencia.dat
		if (imprimir_circunferencia(circunferencia)) // Si la apertura falla vuelve al menú
					menu(0, dperfil, opc, opp, opf);

		// Array para las coordenadas x de la malla
		float ** xx;
		xx = (float **) malloc(M * sizeof(float *));
		for (i=0; i < M; i++)
			xx[i] = (float *) malloc(M * sizeof(float));

		// Array para las coordendas y de la malla
		float ** yy;
		yy = (float **) malloc(M * sizeof(float *));
		for (i=0; i < M; i++)
			yy[i] = (float *) malloc(M * sizeof(float));

		// Array para el flujo en cada punto de la malla
		float ** psi;
		psi = (float **) malloc(M * sizeof(float *));
		for (i=0; i < M; i++)
			psi[i] = (float *) malloc(M * sizeof(float));

		// Crea la malla en la que calcularemos el flujo
		meshgrid(xx, yy, dperfil);

		// Calcula el flujo en cada punto de la malla para el cilindro
		calculo_flujo(dperfil, dflujo, xx, yy, psi);

		// Exporta puntos para imprimir el flujo en el cilindro con GNU Plot
		imprimir_flujo(xx, yy, psi);

		if (dflujo[3] != 0)
		{
			printf("\n\033[32mFlujo: ");
			printf("\033[0m %f\n", fabsf(dflujo[3]));
		}

		printf("\033[32mCoeficiente de sustentación:"); 
		printf("\033[0m %f\n", fabsf(dflujo[4]));

		plotfc(opf, psi);
	}

	/* CAMBIAN */

	if (mct == 2)
	{
		int i;

		float ** xxpr;
		xxpr = (float **) malloc(Mi * sizeof(float *));
		for (i=0; i < Mi; i++)
			xxpr[i] = (float *) malloc(Mi * sizeof(float));

		float ** yypr;
		yypr = (float **) malloc(Mi * sizeof(float *));
		for (i=0; i < Mi; i++)
			yypr[i] = (float *) malloc(Mi * sizeof(float));

		complex double ** tt;
		tt = (complex double **) malloc(Mi * sizeof(complex double *));
		for (i=0; i < Mi; i++)
			tt[i] = (complex double *) malloc(Mi * sizeof(complex double));

		double ** psipr;
		psipr = (double **) malloc(Mi * sizeof(double *));
		for (i=0; i < Mi; i++)
			psipr[i] = (double *) malloc(Mi * sizeof(double));

		float ** xxtau;
		xxtau = (float **) malloc(Mi * sizeof(float *));
		for (i=0; i < Mi; i++)
			xxtau[i] = (float *) malloc(Mi * sizeof(float));

		float ** yytau;
		yytau = (float **) malloc(Mi * sizeof(float *));
		for (i=0; i < Mi; i++)
			yytau[i] = (float *) malloc(Mi * sizeof(float));

		perfil_imaginario(dperfil);

		dflujo[0] = 1;
		dflujo[1] = 0.1;
		dflujo[2] = 1.00;
		dflujo[3] = 4 * M_PI * dperfil[4] * dflujo[0] * (dperfil[1] + (1+dperfil[0]) * dflujo[1]);
		dflujo[4] = 2*M_PI*sin(dflujo[1]+dperfil[3]);

		meshgrid_imaginario(xxpr, yypr, tt, dperfil);

		calculo_flujo_imaginario(tt, psipr, dperfil);

		arregla_malla(tt, dperfil);

		nueva_malla(xxtau, yytau, tt, dperfil);

		imprimir_flujo_perfil(xxtau, yytau, psipr);

		if (dflujo[3] != 0)
		{
			printf("\n\033[32mFlujo: ");
			printf("\033[0m %f\n", fabsf(dflujo[3]));
		}

		printf("\033[32mCoeficiente de sustentación:"); 
		printf("\033[0m %f\n", fabsf(dflujo[4]));

		printf("\033[32m************************************************************************\n");
		printf("\033[0m\n");

		if (plotfp(psipr, opf)==1)
			return 1;
	}

	return 0;
}


/* Contiene el menú principal del programa  llama al resto de las funciones */
int menu(int control, float * dperfil, float * opc, float * opp, float * opf)
{
	int opcion;

	if (control==0)
	{
		printf("\033[32m************************************************************************\n");
		printf("CHMYukovski\n\nBienvenido al programa.\nElija la opción que desea ejecutar:\n\n1. Construcción del perfil del ala\n2. Calculo del flujo\n3. Opciones\n4. Salir\n");
		printf("\033[32m************************************************************************");
		printf("\033[0m\n");	
	}

	scanf("%d",&opcion);

	switch(opcion)
	{
		case 1: // Cálculo del perfil alar
		    perfil(dperfil, opc, opp, opf);   
			menu(0, dperfil, opc, opp, opf); // Llama al menú como si el programa volviese a empezar
		    break;
		            
		case 2: // Cálculo del flujo en el perfil
		    if (flujo(dperfil, opc, opp, opf)==1)
		    {
		    	printf("\033[34mProyecto final en C de la asignatura de Informática\nGrado en Ingeniería Aeroespacial\n\n              Adrián Delgado Marcos\n              Javier Lobato Pérez\n              Pablo Rodríguez Robles");
	       		printf("\033[0m\n\n");
		    	return 0;
		    }
		    else
			    menu(0, dperfil, opc, opp, opf);
		    break;

		case 3: // Opciones
		    menu_opciones(opc, opp, opf);
		    menu(0, dperfil, opc, opp, opf);
		    break;

        case 4: // Salir
	        printf("\033[34mProyecto final en C de la asignatura de Informática\nGrado en Ingeniería Aeroespacial\n\n              Adrián Delgado Marcos\n              Javier Lobato Pérez\n              Pablo Rodríguez Robles");
	        printf("\033[0m\n\n");
     	    break; 
     	
     	default:
            menu(1, dperfil, opc, opp, opf); // Si no se selecciona ninguna opción correcta llama al menú sin imprimir las opciones hasta que se elija una que lo sea
            break;        
	}	

	return 0;
}


/*************************/
/*** FUNCIÓN PRINCIPAL ***/
/*************************/


int main(int argc, char const *argv[])
{
	FILE *readme;
	readme = fopen("readme.txt", "w+");
	fprintf(readme, "CHMYukovski\n \n");
	fprintf(readme, "Este programa está diseñado para realizar la transformación de Yukovski sobre una circunferencia y conseguir un perfil alar (obteniendo puntos e imágenes).\n");
	fprintf(readme, "Después, aplicando el teorema de Kutta-Yukovski, se analiza el flujo de aire sobre un cilindro.\n");
	fprintf(readme, "Por último, se calcula el flujo sobre el perfil alar (SIN tener en cuenta las turbulencias).\n");
	fprintf(readme, "\n\nES NECESARIO TENER INSTALADO GNUPLOT\n \n");
	fprintf(readme, "Proyecto final en C de la asignatura de Informática\nGrado en Ingeniería Aeroespacial\n              Adrián Delgado Marcos\n              Javier Lobato Pérez\n              Pablo Rodríguez Robles\n\n");
	fclose (readme);

	if (popen ("gnuplot","w"))
	{
		// Vector con los datos del perfil {Xc, Yc, a, beta, b, caso}
		float * dperfil;
		dperfil = (float *) malloc(19 * sizeof(float)); // Reserva de memoria para el vector
		dperfil[0] = 0;
		dperfil[1] = 0;
		dperfil[2] = 0;
		dperfil[3] = 0;

	 	// Opciones para la impresión de la circunferencia con GNU Plot
		float * opc; 
		opc = (float *) malloc(5 * sizeof(float)); //Reserva de memoria para el vector

		opc[0] = 7;  // 1: (p)oint(t)ype   2: (p)oint(s)ize   3: (l)ine(t)ype    4:(pattern) malla     5:(l)ine(c)olor malla
		opc[1] = 0.75;
		opc[2] = 1;
		opc[3] = 2;
		opc[4] = 7;			

		// Opciones para la impresión del perfil con GNU Plot
		float * opp;	
		opp = (float *) malloc(6 * sizeof(float)); 
		opp[0] = 7;  // 1: (p)oint(t)ype   2: (p)oint(s)ize   3: (l)ine(t)ype    4:(pattern) malla     5:(l)ine(c)olor malla       6:numero de ploteo
		opp[1] = 0.5;	
		opp[2] = 1;	
		opp[3] = 2;
		opp[4] = 9; 
		opp[5] = 1;
		
		// Opciones para la impresión del flujo
		float * opf; 
		opf = (float *) malloc(4 * sizeof(float));
		opf[0] = 3; //1: color del flujo        2: tamaño de la linea de flujo       3: color del cilindro      4:numero ploteo cilindro       5:numero ploteo perfil
		opf[1] = 1;
		opf[2] = 9;
		opc[3] = 1;
		opc[4] = 1;

		// Primera llamada al menú
	 	menu(0 , dperfil, opc, opp, opf);

	  	return 0;
	} 

	else
	{
		printf("\033[31mPor favor instale GNUPlot");
		printf("\033[0m\n");
		return 1;
	}
}