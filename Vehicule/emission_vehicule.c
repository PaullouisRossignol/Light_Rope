#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <time.h>

#define LED 4	//GPIO23					/*****	SCHEMA DES TRAMES	*******/
#define CAPTEUR 5	//GPIO24				/*****(E)	1. CONNEXION REQUEST 	******/
#define LED_1 6	//GPIO25				    /*****(R)	2. CONNEXION_ok 	******/
											/*****(E)	3. MESSAGE	*****/
											/*****(R)	4. MESSAGE_ACK  *****/

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
//BIT DE START = 1
void start_message()
{
	digitalWrite(LED, HIGH);
	delayMicroseconds(PERIOD_IR);
}
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
        delayMicroseconds(PERIOD_IR);
        if(message[i] == 1)
            digitalWrite(LED, HIGH);
        if(message[i] == 0)
            digitalWrite(LED, LOW);

    }
    //send checksum
	for( i = 0; i < 4; i++)
    {
        delayMicroseconds(PERIOD_IR);
        if(checkArray[i] == 1)
            digitalWrite(LED, HIGH);
        if(checkArray[i] == 0)
            digitalWrite(LED, LOW);
    }
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
		return 1;
	}else{
		//sinon ERROR
		return 0;
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
		digitalWrite(LED_1, HIGH);
	//	printf("Message recu.\n");
		return 1;
	}else{
		//sinon ERROR
		return 0;
	}
}
//fonction non testé,
// récupère les 8 premiers bits d'informations puis 3 bits contenant le checksum de validation
int byte_reception(){

	//créer un buffer d'écoute
	int recept_buffer[11];
	int checksum = 0;
	int newCheckSum = 0;
	//Dès qu'on reçoit le bit de start
	recept_buffer[0]= myDigitalRead(CAPTEUR);

	//On écoute à une fréquence de 38kHz
	if(recept_buffer[0]==1)
	{
		//on récupère le message
		delayMicroseconds(PERIOD_LED);
		recept_buffer[0]=myDigitalRead(CAPTEUR);
		delayMicroseconds(PERIOD_LED);
		recept_buffer[1]=myDigitalRead(CAPTEUR);
		delayMicroseconds(PERIOD_LED);
		recept_buffer[2]=myDigitalRead(CAPTEUR);
		delayMicroseconds(PERIOD_LED);
		recept_buffer[3]=myDigitalRead(CAPTEUR);
		delayMicroseconds(PERIOD_LED);
		recept_buffer[4]=myDigitalRead(CAPTEUR);
		delayMicroseconds(PERIOD_LED);
		recept_buffer[5]=myDigitalRead(CAPTEUR);
		delayMicroseconds(PERIOD_LED);
		recept_buffer[6]=myDigitalRead(CAPTEUR);
		delayMicroseconds(PERIOD_LED);
		recept_buffer[7]=myDigitalRead(CAPTEUR);
		//on récupère le checksum
		delayMicroseconds(PERIOD_LED);
		recept_buffer[8]=myDigitalRead(CAPTEUR);
		delayMicroseconds(PERIOD_LED);
		recept_buffer[9]=myDigitalRead(CAPTEUR);
		
		//on recalcule le checksum et on le compare
		for(int i = 0; i < 8; i++)
			newCheckSum += recept_buffer[i];
		//on récupère la valeur du buffer
		char temp[3];
		char temp2[2];
		char temp3[2];
		sprintf(temp, "%d", recept_buffer[8]);
		sprintf(temp2, "%d", recept_buffer[9]);
		sprintf(temp3, "%d", recept_buffer[10]);
		strcat(temp, temp2);
    	strcat(temp, temp3);
		checksum = (int) strtol(temp, NULL, 2);

		//si le checksum est le même que celui envoyé, c'est qu'on a reçu le message sans erreurs
		if(newCheckSum - 1 == checksum)
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
	pinMode(LED_1, OUTPUT);
	//clock_t CPU_usg;
	//clock_t CPU_inst;

	int recept_co=0;
	int recept_msg=0;
	int counter=0;

	//Tant qu'on a pas reçu le message final...
	while(recept_msg == 0)
	{
		printf("j ecoute %d\n",recept_co);
		//Tant qu'on a pas reçu la requete de connexion
		while(recept_co==0)
		{
			//On écoute 
			recept_co = reception_connexion();
			if(recept_co == 1)
			{
				printf("Bien reçu connexion\n");
				//recept_co = 0;
			}
		}
		//On l'a reçu, on envoi le message retour connexion_ok
		digitalWrite(LED_1, HIGH);
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
		while (recept_msg == 0 && counter < 6) //On attend 100ms
		{
			//On écoute le message
			recept_msg = reception_message();
			//CPU_usg = clock();
			counter ++;

			if(recept_msg == 1)
			{

				printf("Message recu.\n");
				
			//	for(int i=0; i<60; i++)
				while(1)
				{message_ack();}

				digitalWrite(LED_1, LOW);
				exit(EXIT_SUCCESS);
			}
		}
		printf("J ai depasse le delais !\n");
		printf("\n");
		recept_co = 0;
	}
}
