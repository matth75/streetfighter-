/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : freertos.c
 * Description        : Code for freertos applications
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stm32746g_discovery_lcd.h"
#include "stm32746g_discovery_ts.h"
#include "stdio.h"
#include "string.h"
#include "lwip.h"
#include "gpio.h"
#include "lwip/netif.h"
#include "lwip/netbuf.h"
#include "lwip/api.h"

#include "adc.h"
#include "dac.h"

#include "hadouken.h"
#include "normal.h"
#include "jaby.h"
#include "Flamme.h"

#include "hadouken2.h"
#include "normal2.h"
#include "jaby2.h"
#include "Flamme2.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define TCP_SERVER_PORT 8080
#define Dim_v 268
#define Dim_h 478
#define SPRITE_WIDTH 40
#define PORTEE_COUP 80  // distance max entre joueurs pour qu’un coup touche
#define DEGATS_COUP 10  // points de vie en moins
#define VIE_MAX 100
#define ENVOI 1
#define RECEPTION 2
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
char text[30] = {};
char other[50] = {};
char self[] = "Client to server";
extern struct netif gnetif;

char buf[30];


uint8_t jeu_termine = 0;
uint8_t attente_relachement_BP = 0;
uint8_t j1_KO = 0;
uint8_t j2_KO = 0;

uint8_t vie_j1 = VIE_MAX;
int coup_j1_duree = 0;

int vie_j2 = VIE_MAX;
int coup_j2_duree = 0;
uint8_t etat_j2 = 0;  // 0 normal, 1 attaque


static int pos_x_j1= Dim_h/8;
int pos_y_j1;
int pos_x_j2 =Dim_h - Dim_h/8;
int pos_y_j2;
uint16_t vitesse =300;
uint8_t etat_j1 =0; //0 normal , 1 : jab , 2 hadouken


int flamme_x_j1 = -1; // -1 = inactive
int flamme_x_j2 = -1;


int pos_y_j1 = 0;  // 0 = sol, > 0 = en l’air
uint8_t saut_j1_en_cours = 0;
int saut_j1_duree = 0;

int pos_y_j2 = 0;  // 0 = sol, > 0 = en l’air
uint8_t saut_j2_en_cours = 0;
int saut_j2_duree = 0;


typedef struct {
	uint8_t joueur;
	uint8_t jeu_termine;
	uint8_t attente_relachement_BP;
	uint8_t vie_j1;
	int pos_x_j1;
	int pos_y_j1;
	uint8_t etat_j1; //0 normal , 1 : jab , 2 hadouken
}envoi_t;
envoi_t envoi;

typedef struct {
	uint8_t joueur;
	uint8_t jeu_termine;
	uint8_t attente_relachement_BP;
	uint8_t vie_j2;
	int pos_x_j2;
	int pos_y_j2;
	uint8_t etat_j2; //0 normal , 1 : jab , 2 hadouken
}reception_t;

reception_t reception;

/* USER CODE END Variables */
osThreadId defaultTaskHandle;
osThreadId tache_joueur_1Handle;
osThreadId tache_affichageHandle;
osSemaphoreId tcpsemHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void copy_state(int envoi_reception);
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);
void Fonction_tache_joueur_1(void const * argument);
void Fonction_tache_affichage(void const * argument);

extern void MX_LWIP_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
	*ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
	*ppxIdleTaskStackBuffer = &xIdleStack[0];
	*pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
	/* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/**
 * @brief  FreeRTOS initialization
 * @param  None
 * @retval None
 */
void MX_FREERTOS_Init(void) {
	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* USER CODE BEGIN RTOS_MUTEX */
	/* add mutexes, ... */
	/* USER CODE END RTOS_MUTEX */

	/* Create the semaphores(s) */
	/* definition and creation of tcpsem */
	osSemaphoreDef(tcpsem);
	tcpsemHandle = osSemaphoreCreate(osSemaphore(tcpsem), 1);

	/* USER CODE BEGIN RTOS_SEMAPHORES */
	/* add semaphores, ... */
	/* USER CODE END RTOS_SEMAPHORES */

	/* USER CODE BEGIN RTOS_TIMERS */
	/* start timers, add new ones, ... */
	/* USER CODE END RTOS_TIMERS */

	/* USER CODE BEGIN RTOS_QUEUES */
	/* add queues, ... */
	/* USER CODE END RTOS_QUEUES */

	/* Create the thread(s) */
	/* definition and creation of defaultTask */
	osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 256);
	defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

	/* definition and creation of tache_joueur_1 */
	osThreadDef(tache_joueur_1, Fonction_tache_joueur_1, osPriorityNormal, 0, 256);
	tache_joueur_1Handle = osThreadCreate(osThread(tache_joueur_1), NULL);

	/* definition and creation of tache_affichage */
	osThreadDef(tache_affichage, Fonction_tache_affichage, osPriorityNormal, 0, 1024);
	tache_affichageHandle = osThreadCreate(osThread(tache_affichage), NULL);

	/* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */
	/* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
	/* init code for LWIP */
	MX_LWIP_Init();
	/* USER CODE BEGIN StartDefaultTask */
	while (ip4_addr_isany_val(*netif_ip4_addr(&gnetif))) {
		osDelay(200);
	}

	sprintf(text,"READY, IP: %s\n", (char*)ip4addr_ntoa(netif_ip4_addr(&gnetif)));
	BSP_LCD_DisplayStringAtLine(1, (uint8_t *) text);

	struct netconn *conn;
	struct netbuf *buf;
	void *data;
	u16_t len;
	err_t err;
	ip_addr_t server_ip;

	IP4_ADDR(&server_ip, 192,168,0,168);

	conn = netconn_new(NETCONN_TCP);

	if (netconn_connect(conn, &server_ip, 5000) != ERR_OK) {
		netconn_delete(conn);
		vTaskDelete(NULL);
	}

	while (1) {
		// Envoi état client
		copy_state(ENVOI);
		err = netconn_write(conn, &envoi, sizeof(envoi), NETCONN_COPY);
		if (err != ERR_OK) break;

		// Réception état serveur
		if (netconn_recv(conn, &buf) == ERR_OK) {
			netbuf_data(buf, &data, &len);
			if (len > 0) {
				memcpy(&reception, data, sizeof(reception));
				copy_state(RECEPTION);
			}
			netbuf_delete(buf);
		} else {
			break;
		}

		osDelay(50);
	}
	netconn_close(conn);
	netconn_delete(conn);
	vTaskDelete(NULL);
	/* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_Fonction_tache_joueur_1 */
/**
 * @brief Function implementing the tache_joueur_1 thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_Fonction_tache_joueur_1 */
void Fonction_tache_joueur_1(void const * argument)
{
	/* USER CODE BEGIN Fonction_tache_joueur_1 */

	uint32_t joystick_h=2000;
	uint32_t  joystick_v;
	static TS_StateTypeDef  TS_State;
	ADC_ChannelConfTypeDef sConfig = {0};
	sConfig.Rank = ADC_REGULAR_RANK_1;
	sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;


	uint8_t BP1 = HAL_GPIO_ReadPin(BP1_GPIO_Port, BP1_Pin);
	static uint8_t BP1_old = GPIO_PIN_SET;

	uint8_t BP2 = HAL_GPIO_ReadPin(BP2_GPIO_Port, BP2_Pin);
	static uint8_t BP2_old = GPIO_PIN_SET;
	/* Infinite loop */
	for(;;)
	{

		sConfig.Channel = ADC_CHANNEL_6;
		HAL_ADC_ConfigChannel(&hadc3, &sConfig);
		HAL_ADC_Start(&hadc3);
		while(HAL_ADC_PollForConversion(&hadc3, 100)!=HAL_OK);
		//potr = HAL_ADC_GetValue(&hadc3);

		sConfig.Channel = ADC_CHANNEL_7;
		HAL_ADC_ConfigChannel(&hadc3, &sConfig);
		HAL_ADC_Start(&hadc3);
		while(HAL_ADC_PollForConversion(&hadc3, 100)!=HAL_OK);
		//potl = HAL_ADC_GetValue(&hadc3);

		sConfig.Channel = ADC_CHANNEL_8;
		HAL_ADC_ConfigChannel(&hadc3, &sConfig);
		HAL_ADC_Start(&hadc3);
		while(HAL_ADC_PollForConversion(&hadc3, 100)!=HAL_OK);
		joystick_v = HAL_ADC_GetValue(&hadc3);

		HAL_ADC_Start(&hadc1);
		while(HAL_ADC_PollForConversion(&hadc1, 100)!=HAL_OK);
		joystick_h = HAL_ADC_GetValue(&hadc1);


		if (joystick_h > 2200 && pos_x_j1 > 0) {
			pos_x_j1--;
		}
		if (joystick_h < 1900 && pos_x_j1 < Dim_h) {
			if (pos_x_j1 + SPRITE_WIDTH < pos_x_j2) {
				pos_x_j1++;
			}
		}




		// Saut : joystick vers le haut
		if (joystick_v  > 2100 && !saut_j1_en_cours && !jeu_termine) {
			saut_j1_en_cours = 1;
			saut_j1_duree = 40;
		}
		// Gestion du saut
		if (saut_j1_en_cours) {
			if (saut_j1_duree > 10) {
				pos_y_j1 = 50;  // hauteur max (monte)
			} else {
				pos_y_j1 = 0;   // retombe au sol
			}
			saut_j1_duree--;
			if (saut_j1_duree == 0) {
				saut_j1_en_cours = 0;
				pos_y_j1 = 0;
				saut_j1_duree = 0;
			}
		}

		//JAB
		BP1 = HAL_GPIO_ReadPin(BP1_GPIO_Port, BP1_Pin);
		if (BP1 == GPIO_PIN_RESET && BP1_old == GPIO_PIN_SET) {
			etat_j1=1;
			coup_j1_duree = 10;
		}
		BP1_old = BP1;


		//Hadouken
		BP2 = HAL_GPIO_ReadPin(BP2_GPIO_Port, BP2_Pin);
		if (BP2 == GPIO_PIN_RESET && BP2_old == GPIO_PIN_SET) {
			etat_j1 = 2;
			coup_j1_duree = 35;
		}

		BP2_old = BP2;

		//RESET JEU APRES KO

		if (jeu_termine) {
			// Attente relâchement
			if (!attente_relachement_BP) {
				if (BP1 == GPIO_PIN_SET) {
					attente_relachement_BP = 1; // le joueur a relâché
				}
			} else {
				if (BP1 == GPIO_PIN_RESET) {
					// RESET DU JEU
					vie_j1 = VIE_MAX;
					vie_j2 = VIE_MAX;
					j1_KO = 0;
					j2_KO = 0;
					etat_j1 = 0;
					etat_j2 = 0;
					coup_j1_duree = 0;
					coup_j2_duree = 0;
					pos_x_j1 = Dim_h / 8;
					pos_x_j2 = Dim_h - Dim_h / 8;
					jeu_termine = 0;
					attente_relachement_BP = 0;
				}
			}
			// On saute le reste de la logique tant que le jeu est terminé
			vTaskDelay(10);
			continue;
		}

		vTaskDelay(10);
	}
	/* USER CODE END Fonction_tache_joueur_1 */
}

/* USER CODE BEGIN Header_Fonction_tache_affichage */
/**
 * @brief Function implementing the tache_affichage thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_Fonction_tache_affichage */
void Fonction_tache_affichage(void const * argument)
{
	/* USER CODE BEGIN Fonction_tache_affichage */
	/* Infinite loop */
	for(;;)
	{

		if (jeu_termine==1) {
			// Écran rouge
			BSP_LCD_Clear(LCD_COLOR_RED);
			BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
			BSP_LCD_SetBackColor(LCD_COLOR_RED);
			BSP_LCD_SetFont(&Font24);
			if (j1_KO) {
				BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize()/2 - 30, (uint8_t*)"JOUEUR 1 KO !", CENTER_MODE);
			} else if (j2_KO) {
				BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize()/2 - 30, (uint8_t*)"JOUEUR 2 KO !", CENTER_MODE);
			}
			BSP_LCD_SetFont(&Font16);
			BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize()/2 + 20, (uint8_t*)"Appuyez sur un bouton pour rejouer", CENTER_MODE);

			// Attente du bouton (dans tache joueur 1 ou 2, ici on met juste pause)
			vTaskDelay(100);
			continue;
		}

		//Si le jeu est en cours de route
		if (jeu_termine==0){
			BSP_LCD_SelectLayer(1);
			BSP_LCD_Clear(00);

			int barre_largeur = (Dim_h / 3); // Largeur totale de la barre
			int hauteur = 10;
			int x = 10;
			int y = 10;

			int largeur_vie = (vie_j1 * barre_largeur) / VIE_MAX;

			//JOUERU1
			// Fond de la barre (gris clair)
			BSP_LCD_SetTextColor(LCD_COLOR_LIGHTGRAY);
			BSP_LCD_FillRect(x, y, barre_largeur, hauteur);

			// Vie restante (blanc)
			BSP_LCD_SetTextColor(LCD_COLOR_GREEN);
			BSP_LCD_FillRect(x, y, largeur_vie, hauteur);

			//JOUERU2
			int x2 = Dim_h - 10 - barre_largeur;
			int largeur_vie_j2 = (vie_j2 * barre_largeur) / VIE_MAX;

			// Fond (gris)
			BSP_LCD_SetTextColor(LCD_COLOR_LIGHTGRAY);
			BSP_LCD_FillRect(x2, y, barre_largeur, hauteur);

			// Vie restante (rouge)
			BSP_LCD_SetTextColor(LCD_COLOR_RED);
			BSP_LCD_FillRect(x2 + (barre_largeur - largeur_vie_j2), y, largeur_vie_j2, hauteur);

			//AFFICHAGE JOUEUR 1
			switch (etat_j1)
			{
			case 0:
				BSP_LCD_DrawBitmap(pos_x_j1, Dim_v - 10 - 80 - pos_y_j1, (uint8_t*)normal_bmp);
				break;
			case 1:
				BSP_LCD_DrawBitmap(pos_x_j1, Dim_v - 10 - 80- pos_y_j1, (uint8_t*)jaby_bmp);

				break;
			case 2:
				BSP_LCD_DrawBitmap(pos_x_j1, Dim_v - 10 -80- pos_y_j1, (uint8_t*)hadouken_bmp);
				break;

			default:
				BSP_LCD_DrawBitmap(pos_x_j1, Dim_v - 10 - 80- pos_y_j1, (uint8_t*)normal_bmp);
				break;
			}

			if (coup_j1_duree > 0)
			{
				coup_j1_duree--;
				if (coup_j1_duree == 0)
				{
					etat_j1 = 0;
					flamme_x_j1 =-1;
				}
			}

			// AFFICHAGE JOUEUR 2
			switch (etat_j2)
			{
			case 0:
				BSP_LCD_DrawBitmap(pos_x_j2, Dim_v - 10 - 80, (uint8_t*)normal2_bmp);
				break;
			case 1:
				BSP_LCD_DrawBitmap(pos_x_j2, Dim_v - 10 -80, (uint8_t*)jaby2_bmp);
				break;
			case 2:
				BSP_LCD_DrawBitmap(pos_x_j2, Dim_v - 10 -80, (uint8_t*)hadouken2_bmp);
				break;
			default:
				BSP_LCD_DrawBitmap(pos_x_j2, Dim_v - 10 - 80, (uint8_t*)normal2_bmp);
				break;
			}

			if (coup_j2_duree > 0)
			{
				coup_j2_duree--;
				if (coup_j2_duree == 0)
				{
					etat_j2 = 0;
					flamme_x_j2 =-1;
				}
			}

			// COLLISIONS DES COUPS

			// J1 touche J2
			if (etat_j1 == 1 && abs(pos_x_j1 - pos_x_j2) < PORTEE_COUP && !j2_KO) {
				if (vie_j2 > 0) {
					vie_j2 -= DEGATS_COUP;
					if (vie_j2 <= 0) {
						vie_j2 = 0;
						j2_KO = 1;
						jeu_termine = 1;
					}
				}
				etat_j1 = 0;
			}

			// J2 touche J1
			if (etat_j2 == 1 && abs(pos_x_j2 - pos_x_j1) < (PORTEE_COUP-20) && !j1_KO) {
				if (vie_j1 > 0) {
					vie_j1 -= DEGATS_COUP*2;
					if (vie_j1 <= 0) {
						vie_j1 = 0;
						j1_KO = 1;
						jeu_termine = 1;
					}
				}
				etat_j2 = 0;
			}

			//FLAMME :
			// FLAMME J1 (Hadouken)
			if (etat_j1 == 2 && coup_j1_duree > 0) {
				if (flamme_x_j1 == -1) {
					flamme_x_j1 = pos_x_j1 + SPRITE_WIDTH; // Départ du projectile
				} else {
					flamme_x_j1 += 5; // Vitesse de la flamme
					BSP_LCD_DrawBitmap(flamme_x_j1, Dim_v - 80, (uint8_t*)Flamme_bmp);
					if (flamme_x_j1 > Dim_h) {
						flamme_x_j1 = -1;
					}
				}

				// Collision Hadouken J1 touche J2
				if (abs(flamme_x_j1 - pos_x_j2) < (PORTEE_COUP-20) && !j2_KO) {
					vie_j2 -= DEGATS_COUP*2;
					if (vie_j2 <= 0) {
						vie_j2 = 0;
						j2_KO = 1;
						jeu_termine = 1;
					}
					flamme_x_j1 = -1;
					etat_j1 = 0;
					coup_j1_duree = 0;
				}
			}

			// FLAMME J2 (Hadouken)
			if (etat_j2 == 2 && coup_j2_duree > 0) {
				if (flamme_x_j2 == -1) {
					flamme_x_j2 = pos_x_j2 - SPRITE_WIDTH;
				} else {
					flamme_x_j2 -= 5;
					BSP_LCD_DrawBitmap(flamme_x_j2, Dim_v - 80, (uint8_t*)Flamme2_bmp);
					if (flamme_x_j2 < 0) {
						flamme_x_j2 = -1;
					}
				}

				// Collision Hadouken J2 touche J1
				if (abs(flamme_x_j2 - pos_x_j1) < PORTEE_COUP && !j1_KO) {
					vie_j1 -= DEGATS_COUP;
					if (vie_j1 <= 0) {
						vie_j1 = 0;
						j1_KO = 1;
						jeu_termine = 1;
					}
					flamme_x_j2 = -1;
					etat_j2 = 0;
					coup_j2_duree = 0;
				}
			}
			// Affiche "KO" si un joueur est mort
			if (j1_KO) {
				BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
				BSP_LCD_DisplayStringAtLine(6, (uint8_t*)"JOUEUR 1 KO !");
			}

			if (j2_KO) {
				BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
				BSP_LCD_DisplayStringAtLine(7, (uint8_t*)"JOUEUR 2 KO !");
			}
		}
		vTaskDelay(30);
	}
	/* USER CODE END Fonction_tache_affichage */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void copy_state(int envoi_reception){
	if (envoi_reception == ENVOI){
		envoi.joueur = 1;
		envoi.jeu_termine = jeu_termine;
		envoi.attente_relachement_BP = attente_relachement_BP;
		envoi.vie_j1 = 100;
		envoi.etat_j1 = etat_j1;
		envoi.pos_x_j1 = pos_x_j1;
		envoi.pos_y_j1 = pos_y_j1;
	}
	if(envoi_reception == RECEPTION){
		if ((reception.joueur) == 2){
			jeu_termine = reception.jeu_termine;
			attente_relachement_BP = reception.attente_relachement_BP;
			vie_j2 = reception.vie_j2;
			etat_j2 = reception.etat_j2;
			pos_x_j2 = reception.pos_x_j2;
			pos_y_j2 = reception.pos_y_j2;

		}

	}

}


/* USER CODE END Application */

