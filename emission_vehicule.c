#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <time.h>

#define LED 4					/*****	SCHEMA DES TRAMES	*******/
#define CAPTEUR 5				/*****(R)	1. CONNEXION REQUEST : '1' '0'	******/
#define OK 1					/*****(E)	2. CONNEXION_OK : '1' '0' '1'	******/
#define NOK 0					/*****(R)	3. MESSAGE = START + MESSAGE + END : '1' '1' '0' '1' '0' '0'	*****/
#define LED_OK 6				/*****(E)	(4. MESSAGE_ACK : '1' '0' '1' '1' ) *****/


/************* EMISSION	*******************/

// ENVOIE 1 0 1 TOUTE LES 26ns (38kHz)
void connexion_ok()
{
	printf("Connexion...\n");
	digitalWrite(LED, HIGH);
	delayMicroseconds(26);
	digitalWrite(LED, LOW);
	delayMicroseconds(26);
	digitalWrite(LED, HIGH);
	delayMicroseconds(26);
}


void message_ack()
{
	digitalWrite(LED, HIGH);
        delayMicroseconds(26);
        digitalWrite(LED, LOW);
        delayMicroseconds(26);
        digitalWrite(LED, HIGH);
        delayMicroseconds(26);
	digitalWrite(LED, HIGH);
	delayMicroseconds(26);
}

/**********	RECEPTION	******************/

// RECEPTION DU MESSAGE DE CONNEXION
//
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

	}

	if(recept_buffer[0]==1 && recept_buffer[1]==0)
	{
		//si le signal emis correspond, on retourne une valeur ok
		printf("Reception message connexion !\n");
		return OK;
	}else{
		//sinon ERROR
		return NOK;
	}
	
}

//RECEPTION DE L'ACQUITTEMENT DU MESSAGE
int reception_message()
{
	//créer un buffer d'écoute
	int recept_buffer[10];
	//ecoute du signal
	//Dès qu on reçoit un '1' on écoute (bit de start)
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
		recept_buffer[1]=digitalRead(CAPTEUR);
                delayMicroseconds(26);

	}

	if(recept_buffer[1]==1 && recept_buffer[2]==0 && recept_buffer[3]==1 && recept_buffer[4]==0)
	{
		//si le signal emis correspond, on retourne une valeur ok
		digitalWrite(LED_OK, HIGH);
		printf("Message recu.\n");
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
	long CPU_usg;
	long CPU_inst;

	int recept_co=NOK;
	int recept_msg=NOK;

	//Tant qu'on a pas reçu le message final...
	while(recept_msg == NOK)
	{
		//Tant qu'on a pas reçu la requete de connexion
		while(recept_co == NOK)
		{
			//On écoute 
			recept_co = reception_connexion();
		}
		//On l'a reçu, on envoi le message retour connexion_ok
		connexion_ok();
		CPU_inst = __clock_t;
		CPU_usg = __clock_t;
		
		//Tant qu'on a pas reçu le message et que le délai est inférieur à 100ms
		while (recept_msg == NOK || ((CPU_usg - CPU_inst)/CLOCKS_PER_SEC <= 0.1)) //On attend 100ms
		{
			//On écoute le message
			recept_msg = reception_message();
			CPU_usg = __clock_t;
		}
		if(recept_msg == OK)
		{
			printf("Message recu.\n")
			message_ack();
			exit(EXIT_SUCCESS);
		}

	}


}
