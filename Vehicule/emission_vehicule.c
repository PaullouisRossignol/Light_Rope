#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <time.h>

#define LED 4	//GFPIO 23		/*****	SCHEMA DES TRAMES	*******/
#define CAPTEUR 6	//GPIO 25	/*****(R)	1. CONNEXION REQUEST : '1' '0'	******/
#define OK 1				/*****(E)	2. CONNEXION_OK : '1' '0' '1'	******/
#define NOK 0				/*****(R)	3. MESSAGE = START + MESSAGE + END : '1' '1' '0' '1' '0' '0'	*****/
#define LED_OK 5	//GPIO 24	/*****(E)	(4. MESSAGE_ACK : '1' '0' '1' '1' ) *****/

#define RED   "\x1B[31m"
#define RESET "\x1B[0m"
int PERIOD_LED = 10000;
int PERIOD_IR = 10000;


int myDigitalRead(int GPIO)
{
	if(digitalRead(GPIO) == 1)
		return 0;
	if(digitalRead(GPIO) == 0)
		return 1;
}

/************* EMISSION	*******************/

//EMISSION DE LA VALIDATION DE CONNECTION
void connexion_ok()
{
	
	digitalWrite(LED, HIGH);
	delayMicroseconds(PERIOD_IR);
	digitalWrite(LED, LOW);
	delayMicroseconds(PERIOD_IR);
	digitalWrite(LED, HIGH);
	delayMicroseconds(PERIOD_IR);
	digitalWrite(LED,LOW);
}

//EMISSION DU MESSAGE D'ACQUITEMENT
void message_ack()
{
	digitalWrite(LED, HIGH);
   	delayMicroseconds(PERIOD_IR);
   	digitalWrite(LED, LOW);
    	delayMicroseconds(PERIOD_IR);
    	digitalWrite(LED, HIGH);
    	delayMicroseconds(PERIOD_IR);
	digitalWrite(LED, HIGH);
	delayMicroseconds(PERIOD_IR);
}

/**********	RECEPTION	******************/

// RECEPTION DU MESSAGE DE CONNEXION
int reception_connexion()
{
	//créer un buffer d'écoute
	int recept_buffer[10];
	
	//Dès qu'on reçoit un '1' on écoute
	recept_buffer[0]= myDigitalRead(CAPTEUR);
	//delayMicroseconds(PERIOD_LED+(PERIOD_LED/2));
	//On écoute à une fréquence de 38kHz
	if(recept_buffer[0]==1)
	{
		delayMicroseconds(PERIOD_LED+(PERIOD_LED/2));
		recept_buffer[1]=myDigitalRead(CAPTEUR);
		delayMicroseconds(PERIOD_LED);
		recept_buffer[2]=myDigitalRead(CAPTEUR);
		delayMicroseconds(PERIOD_LED);
		recept_buffer[3]=myDigitalRead(CAPTEUR);
		delayMicroseconds(PERIOD_LED);
	}

	if(recept_buffer[0]==1 && recept_buffer[1]==0 && recept_buffer[2]==1 && recept_buffer[3]==0)
	{
		//si le signal emis correspond, on retourne une valeur ok
		printf("Reception message connexion !\n");
		/*for(int i=0; i<4; i++)
		{
			printf("%d", recept_buffer[i]);
		}*/
		//printf("\n");
		return OK;
	}else{
		//sinon ERROR
		return NOK;
	}

}

//RECEPTION DU MESSAGE
int reception_message()
{
	//créer un buffer d'écoute
	int recept_buffer[10];
	//ecoute du signal
	//Dès qu on reçoit un '1' on écoute (bit de start)
	recept_buffer[0]= myDigitalRead(CAPTEUR);		//Start bit (1)

	//On écoute à une fréquence de 38kHz
	if(recept_buffer[0]==1)
	{
		delayMicroseconds(PERIOD_LED);
		recept_buffer[1]=myDigitalRead(CAPTEUR);	//1
		delayMicroseconds(PERIOD_LED);
		recept_buffer[2]=myDigitalRead(CAPTEUR);	//0
		delayMicroseconds(PERIOD_LED);
		recept_buffer[3]=myDigitalRead(CAPTEUR);	//1
		delayMicroseconds(PERIOD_LED);
		recept_buffer[4]=myDigitalRead(CAPTEUR);	//0
                delayMicroseconds(PERIOD_LED);
		recept_buffer[5]=myDigitalRead(CAPTEUR);	// End bit (0)

	}

	/*for(int jul=0; jul<6; jul++)
	{
		printf("%d", recept_buffer[jul]);
	}
	printf("\n");*/

	if(recept_buffer[1]==1 && recept_buffer[2]==0 && recept_buffer[3]==1 && recept_buffer[4]==0)
	{
		//si le signal emis correspond, on retourne une valeur ok
		digitalWrite(LED_OK, HIGH);
	//	printf("Message recu.\n");
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
	pinMode(LED, OUTPUT);
	pinMode(CAPTEUR, INPUT);
	pinMode(LED_OK, OUTPUT);
	//clock_t CPU_usg;
	//clock_t CPU_inst;

	int recept_co=NOK;
	int recept_msg=NOK;
	int counter=0;

	//Tant qu'on a pas reçu le message final...
	while(recept_msg == NOK)
	{
		printf("j ecoute %d\n",recept_co);
		//Tant qu'on a pas reçu la requete de connexion
		while(recept_co==NOK)
		{
			//On écoute 
			recept_co = reception_connexion();
			if(recept_co == OK)
			{
				printf("Bien reçu connexion\n");
				//recept_co = NOK;
			}
		}
		//On l'a reçu, on envoi le message retour connexion_ok
		digitalWrite(LED_OK, HIGH);
		connexion_ok();
	//	CPU_inst = clock();
		printf("J envoie connexion_ok\n");

		while (counter<6) //On attend 1s
		{
			connexion_ok();
			counter++;
		//	CPU_usg = clock();
		}
		printf("J attends ton message petit drone\n");
		counter = 0;
		//Tant qu'on a pas reçu le message et que le délai est inférieur à 100ms
		while (recept_msg == NOK && counter < 6) //On attend 100ms
		{
			//On écoute le message
			recept_msg = reception_message();
			//CPU_usg = clock();
			counter ++;

			if(recept_msg == OK)
			{

				printf("Message recu.\n");
				
			//	for(int i=0; i<60; i++)
				while(1)
				{message_ack();}

				digitalWrite(LED_OK, LOW);
				exit(EXIT_SUCCESS);
			}
		}
		printf("J ai depasse le delais !\n");
		printf("\n");
		recept_co = NOK;
	}
}
