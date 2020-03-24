#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wiringPi.h>
#include <time.h>
#include <string.h>
#define LED 4	//GPIO23					/*****	SCHEMA DES TRAMES	*******/
#define CAPTEUR 5	//GPIO24				/*****(E)	1. CONNEXION REQUEST 	******/
#define LED_1 6	//GPIO25					/*****(R)	2. CONNEXION_OK 	******/
											/*****(E)	3. MESSAGE	*****/
											/*****(R)	4. MESSAGE_ACK  *****/
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
//fonction non testé,
//EMISSION D'UN MESSAGE DE 8 BIT AVEC SON CHECKSUM(4bit)
void send_byte(int message[8])
{
    int i = 0;
    int checksum = 0;
    int checkArray[4];
	for( i = 0; i < 8; i++)
    {
		checksum += message[i];
    }
    for (i = 0; i < 4; i++)
        checkArray[3-i] = (checksum >> i) & 1;  
    
	start_message();
    //send message
	for( i = 0; i < 8; i++)
    {
        delayMicroseconds(PERIOD_LED);
        if(message[i] == 1)
            digitalWrite(LED, HIGH);
        if(message[i] == 0)
            digitalWrite(LED, LOW);

    }
    //send checksum
	for( i = 0; i < 4; i++)
    {
        delayMicroseconds(PERIOD_LED);
        if(checkArray[i] == 1)
            digitalWrite(LED, HIGH);
        if(checkArray[i] == 0)
            digitalWrite(LED, LOW);
    }
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
	//créer un buffer d'écoute, la taille a été choisi arbitrairement
	int recept_buffer[10];
	
	//Dès qu'on reçoit le bit de start
	recept_buffer[0]= myDigitalRead(CAPTEUR);

	//On écoute à une fréquence de 38kHz
	if(recept_buffer[0]==1)
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
		//si le signal emis correspond, on retourne une valeur 1
		return 1;
	}else{
		//sinon ERROR
		return 0;
	}
	
}

//RECEPTION DE L'ACQUITTEMENT DU MESSAGE
int reception_ack()
{
	//créer un buffer d'écoute
	int recept_buffer[10];
	//ecoute du signal
	//Dès qu'on reçoit le bit de start
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
		//si le signal emis correspond, on retourne une valeur 1
		//digitalWrite(LED_1, HIGH);
		printf("Bien recu.\n");
		return 1;
	}else{
		//sinon ERROR
		return 0;
	}
}
//fonction non testé,
// récupère les 8 premiers bits d'informations puis 4 bits contenant le checksum de validation
int byte_reception(){

	//créer un buffer d'écoute
	int recept_buffer[12];
	int checksum = 0;
	int newCheckSum = 0;
	//Dès qu'on reçoit le bit de start
	recept_buffer[0]= myDigitalRead(CAPTEUR);

	//On écoute à une fréquence de 38kHz
	if(recept_buffer[0]==1)
	{
		//on récupère le message
		delayMicroseconds(PERIOD_IR);
		recept_buffer[0]=myDigitalRead(CAPTEUR);
		delayMicroseconds(PERIOD_IR);
		recept_buffer[1]=myDigitalRead(CAPTEUR);
		delayMicroseconds(PERIOD_IR);
		recept_buffer[2]=myDigitalRead(CAPTEUR);
		delayMicroseconds(PERIOD_IR);
		recept_buffer[3]=myDigitalRead(CAPTEUR);
		delayMicroseconds(PERIOD_IR);
		recept_buffer[4]=myDigitalRead(CAPTEUR);
		delayMicroseconds(PERIOD_IR);
		recept_buffer[5]=myDigitalRead(CAPTEUR);
		delayMicroseconds(PERIOD_IR);
		recept_buffer[6]=myDigitalRead(CAPTEUR);
		delayMicroseconds(PERIOD_IR);
		recept_buffer[7]=myDigitalRead(CAPTEUR);
		//on récupère le checksum
		delayMicroseconds(PERIOD_IR);
		recept_buffer[8]=myDigitalRead(CAPTEUR);
		delayMicroseconds(PERIOD_IR);
		recept_buffer[9]=myDigitalRead(CAPTEUR);
		delayMicroseconds(PERIOD_IR);
		recept_buffer[10]=myDigitalRead(CAPTEUR);
		delayMicroseconds(PERIOD_IR);
		recept_buffer[11]=myDigitalRead(CAPTEUR);
		
		//on recalcule le checksum et on le compare
		for(int i = 0; i < 8; i++)
			newCheckSum += recept_buffer[i];
		//on récupère la valeur du buffer
		char temp[3];
		char temp2[2];
		char temp3[2];
		char temp4[2];
		sprintf(temp, "%d", recept_buffer[8]);
		sprintf(temp2, "%d", recept_buffer[9]);
		sprintf(temp3, "%d", recept_buffer[10]);
		sprintf(temp4, "%d", recept_buffer[11]);
		strcat(temp, temp2);
    	strcat(temp, temp3);
    	strcat(temp, temp4);
		checksum = (int) strtol(temp, NULL, 2);

		//si le checksum est le même que celui envoyé, c'est qu'on a reçu le message sans erreurs
		if(newCheckSum == checksum)
			return 1;
		else
			return 0;
	}
	
}
int main(int argc, char* argv[])
{
	if(argc==3)
	{
		PERIOD_LED = atoi(argv[1]);
		PERIOD_IR = atoi(argv[2]);
	}
	else if(argc > 3|| argc == 2 )
	{
		fprintf(stderr, RED "INCORRECT PARAMETER NUMBER\n" RESET);
		printf("USAGE:\n");
		printf("./program_name or ./program_name Period_led Period_IR\n");
		exit(EXIT_FAILURE);
	}
	
		printf("Period choose for Led(in us): %d\n", PERIOD_LED);
		printf("Period choose for IR led(in us): %d\n", PERIOD_IR);
		wiringPiSetup();
		pinMode(LED, OUTPUT);
		pinMode(CAPTEUR, INPUT);
		pinMode(LED_1, OUTPUT);
		digitalWrite(LED_1, LOW);
		int recept_co=0;
		int recept_ack=0;
		int counter = 0;
		int MESSAGE_LENGHT = 8;
		int scd_counter = 0;
		char *message = (char *) malloc(( MESSAGE_LENGHT + 1 ) );
		message[8]='\0';
		for(int i=0;i<strlen(message);i++)
		{
			if(i%2==0)
				message[i]='1';
			else
				message[i]='0';
		}
		//send_message(message);
		while(recept_ack == 0)
		{
			printf("Requete de co");
			//Tant qu'on a pas reçu la réponse de connexion
			while(recept_co == 0)
			{
				//On envoie la requete de connexion
				connexion_request();
				//On ecoute le message de retour pendant 50ms
				while(counter < 6 && recept_co == 0)
				{
					counter++;
					recept_co=reception_connexion();
				}
				counter = 0;
			}
			//Tant qu'on a pas reçu l'ack
			printf("Connecte !\n");
			while(recept_ack==0 && counter < 6)
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
				digitalWrite(LED_1, HIGH);
			}
			else
			{	

				recept_co = 0;
				printf("ACK pas reçu\n");

			}
		}
		free(message);
}
