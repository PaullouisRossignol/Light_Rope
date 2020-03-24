#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wiringPi.h>
#include <time.h>
#include <string.h>
#define LED 4	//GPIO23					/*****	SCHEMA DES TRAMES	*******/
#define CAPTEUR 5	//GPIO24				/*****(E)	1. CONNEXION REQUEST : '1' '0'	******/
#define OK 1					/*****(R)	2. CONNEXION_OK : '1' '0' '1'	******/
#define NOK 0					/*****(E)	3. MESSAGE = START + MESSAGE + END : '1' '1' '0' '1' '0' '0'	*****/
#define LED_OK 6	//GPIO25				/*****(R)	(4. MESSAGE_ACK : '1' '0' '1' '1' ) *****/

//color to display errors
#define RED   "\x1B[31m"
#define RESET "\x1B[0m"

int PERIOD_LED = 10000;
int PERIOD_IR = 10000;


/************* EMISSION	*******************/

// ENVOIE 1 0 TOUTE LES 26ns (38kHz)

int myDigitalRead(int GPIO)
{
	if(digitalRead(GPIO) == 1)
		return 0;
	else if(digitalRead(GPIO) == 0)
		return 1;
}


void connexion_request()
{
	digitalWrite(LED, HIGH);
	delayMicroseconds(PERIOD_LED);
	digitalWrite(LED, LOW);
	delayMicroseconds(PERIOD_LED);
	digitalWrite(LED, HIGH);
	delayMicroseconds(PERIOD_LED);
	digitalWrite(LED, LOW);
	delayMicroseconds(PERIOD_LED);
}

//BIT DE START = 1
void start_message()
{
	digitalWrite(LED, HIGH);
	delayMicroseconds(PERIOD_LED);
}

//BIT DE FIN = 0;
void end_message()
{
	digitalWrite(LED, LOW);
	delayMicroseconds(PERIOD_LED);
}

// MESSAGE "STOP" : 1010
void sending_stop_message()
{
	digitalWrite(LED, HIGH);
	delayMicroseconds(PERIOD_LED);
	digitalWrite(LED, LOW);
	delayMicroseconds(PERIOD_LED);
	digitalWrite(LED, HIGH);
	delayMicroseconds(PERIOD_LED);
	digitalWrite(LED, LOW);
	delayMicroseconds(PERIOD_LED);
}

void send_message(char* message)
{
	printf("message is: ");
	for(int i = 0; i < strlen(message); i++)
		printf( "bit : %c\n " , message[i]);
}

void send_full_message()
{
	//printf("Envoi message\n");
	start_message();
	sending_stop_message();
	end_message();
}

/**********	RECEPTION	******************/

// RECEPTION DU MESSAGE DE CONNEXION
int reception_connexion()
{
	//créer un buffer d'écoute
	int recept_buffer[10];
	
	//Dès qu'on reçoit un '1' on écoute
	recept_buffer[0]= myDigitalRead(CAPTEUR);
	clock_t inst = clock();
	clock_t del = clock();

	//On écoute à une fréquence de 38kHz
	if(recept_buffer[0]==1)// LE CAPTEUR EST INVERSE SA RACE
	{
		delayMicroseconds(PERIOD_IR);
		recept_buffer[1]=myDigitalRead(CAPTEUR);
		delayMicroseconds(PERIOD_IR);
		recept_buffer[2]=myDigitalRead(CAPTEUR);
		/*for(int i=0; i<3; i++) 
			printf("%d", recept_buffer[i]);
		printf("\n");*/

	}


	if(recept_buffer[0]==1 && recept_buffer[1]==0 && recept_buffer[2]==1)
	{
		//si le signal emis correspond, on retourne une valeur ok
		return OK;
	}else{
		//sinon ERROR
		return NOK;
	}
	
}

//RECEPTION DE L'ACQUITTEMENT DU MESSAGE
int reception_ack()
{
	//créer un buffer d'écoute
	int recept_buffer[10];
	//ecoute du signal
	//Dès qu'on reçoit un '1' on écoute
	recept_buffer[0]=myDigitalRead(CAPTEUR);

	//On écoute à une fréquence de 38kHz
	if(recept_buffer[0]==1)
	{
		delayMicroseconds(PERIOD_IR);
		recept_buffer[1]=myDigitalRead(CAPTEUR);
		delayMicroseconds(PERIOD_IR);
		recept_buffer[2]=myDigitalRead(CAPTEUR);
		delayMicroseconds(PERIOD_IR);
		recept_buffer[3]=myDigitalRead(CAPTEUR);
		for(int i=0; i<4; i++) 
			printf("%d", recept_buffer[i]);
		printf("\n");
	}

	if(recept_buffer[0]==1 && recept_buffer[1]==0 && recept_buffer[2]==1 && recept_buffer[3]==1)
	{
		//si le signal emis correspond, on retourne une valeur ok
		//digitalWrite(LED_OK, HIGH);
		printf("Bien recu.\n");
		return OK;
	}else{
		//sinon ERROR
		return NOK;
	}
}

int main(int argc, char* argv[])
{
	if(argc==3)
	{
		PERIOD_LED = atoi(argv[1]);
		PERIOD_IR = atoi(argv[2]);
	}
	else if(argc > 3||argc == 2 )
	{
		fprintf(stderr, RED "INCORRECT PARAMETER NUMBER\n" RESET);
		printf("USAGE:\n");
		printf("./program_name or ./program_name Period_led Period_IR\n");
		exit(EXIT_FAILURE);
	}
	
		printf("Period choose for Led(in us): %d\n", PERIOD_LED);
		printf("Period choose for IR led(in us): %d\n", PERIOD_IR);
		wiringPiSetup();
		fflush(stdout);
		printf("Curry");
		pinMode(LED, OUTPUT);
		pinMode(CAPTEUR, INPUT);
		pinMode(LED_OK, OUTPUT);
		digitalWrite(LED_OK, LOW);
		int recept_co=NOK;
		int recept_ack=NOK;
		int counter = 0;
		int MESSAGE_LENGHT = 8;
		int scd_counter = 0;
		char *message = (char *) malloc(( MESSAGE_LENGHT + 1 ) );
		message[8]='\0';
		fflush(stdout);
		for(int i=0;i<strlen(message);i++)
		{
			if(i%2==0)
				message[i]='1';
			else
				message[i]='0';
		}
		//send_message(message);
		while(recept_ack == NOK)
		{
			printf("Requete de co");
			//Tant qu'on a pas reçu la réponse de connexion
			while(recept_co == NOK)
			{
				//On envoie la requete de connexion
				connexion_request();
				//On ecoute le message de retour pendant 50ms
				while(counter < 6 && recept_co == NOK)
				{
					counter++;
					recept_co=reception_connexion();
				}
				counter = 0;
			}
			//Tant qu'on a pas reçu l'ack
			printf("Connecte !\n");
			while(recept_ack==NOK && counter < 6)
			{
				counter++;
				//On envoie le message
				send_full_message();
				//On ecoute la reponse pendant 500ms
				while(scd_counter < 6 && !recept_ack)
				{
					scd_counter++;
					recept_ack = reception_ack();
				}
				scd_counter = 0;
				
			}
			if(recept_ack)
			{
				printf("Bien recu ACK\n");
				digitalWrite(LED_OK, HIGH);
			}
			else
			{	

				recept_co = NOK;
				printf("ACK pas reçu\n");

			}
		}
		free(message);
}
