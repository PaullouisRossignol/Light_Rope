#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>

#define LED 4					/*****	SCHEMA DES TRAMES	*******/
#define CAPTEUR 5				/*****(E)	1. CONNEXION REQUEST : '1' '0'	******/
#define OK 1					/*****(R)	2. CONNEXION_OK : '1' '0' '1'	******/
#define NOK 0					/*****(E)	3. MESSAGE = START + MESSAGE + END : '1' '1' '0' '1' '0' '0'	*****/
#define LED_OK 6				/*****(R)	(4. MESSAGE_ACK : '1' '0' '1' '1' ) *****/


/************* EMISSION	*******************/

// ENVOIE 1 0 TOUTE LES 26ns (38kHz)
void connexion_request()
{
	printf("Connexion...\n");
	digitalWrite(LED, HIGH);
	delayMicroseconds(26);
	digitalWrite(LED, LOW);
	delayMicroseconds(26);
}

//BIT DE START = 1
void start_message()
{
	digitalWrite(LED, HIGH);
	delayMicroseconds(26);
}

//BIT DE FIN = 0;
void end_message()
{
	digitalWrite(LED, LOW);
	delayMicroseconds(26);
}

// MESSAGE "STOP" : 1010
void sending_stop_message()
{
	digitalWrite(LED, HIGH);
	delayMicroseconds(26);
	digitalWrite(LED, LOW);
	delayMicroseconds(26);
	digitalWrite(LED, HIGH);
	delayMicroseconds(26);
	digitalWrite(LED, LOW);
	delayMicroseconds(26);
}

void send_full_message()
{
	printf("Envoi message\n");
	start_message();
	sending_stop_message();
	end_message();
}

/**********	RECEPTION	******************/

// RECEPTION DU MESSAGE DE CONNEXION
int reception_connexion()
{
	//créer un buffer d'écoute
	int[10] recept_buffer;
	
	//Dès qu'on reçoit un '1' on écoute
	recept_buffer[0]= digitalRead(CAPTEUR);

	//On écoute à une fréquence de 38kHz
	if(recept_buffer[0]==1)
	{
		delayMicroseconds(26);
		recept_buffer[1]=digitalRead(CAPTEUR);
		delayMicroseconds(26);
		recept_buffer[1]=digitalRead(CAPTEUR);
		delayMicroseconds(26);

	}

	if(recept_buffer[0]==1 && recept_buffer[1]==0 && recept_buffer[2]==1)
	{
		//si le signal emis correspond, on retourne une valeur ok
		printf("Connecte !\n");
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
	recept_buffer[0]= digitalRead(CAPTEUR);

	//On écoute à une fréquence de 38kHz
	if(recept_buffer[0]==1)
	{
		delayMicroseconds(26);
		recept_buffer[1]=digitalRead(CAPTEUR);
		delayMicroseconds(26);
		recept_buffer[1]=digitalRead(CAPTEUR);
		delayMicroseconds(26);
		recept_buffer[1]=digitalRead(CAPTEUR);
		delayMicroseconds(26);
	}

	if(recept_buffer[0]==1 && recept_buffer[1]==0 && recept_buffer[2]==1 && recept_buffer[3]==1)
	{
		//si le signal emis correspond, on retourne une valeur ok
		digitalWrite(LED_OK, HIGH);
		printf("Bien recu.\n");
		return OK;
	}else{
		//sinon ERROR
		return NOK;
	}
}

int main(void)
{
	wiringPiSetup();
	pinMode(LED, OUTPUT);
	pinMode(CAPTEUR, INPUT);
	pinMode(LED_OK, OUTPUT);
	int recept_co=NOK;
	int recept_ack=NOK;

	while(recept_co==NOK)
	{
		connexion_request();
		recept_co=reception_connexion();
		delay(100);
	}
	while(recept_ack==NOK)
	{
		sending_full_message();
		recept_ack=reception_ack();
		delay(100);
	}
}
