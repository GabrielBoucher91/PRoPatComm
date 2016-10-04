//============================================================================
// Name        : Comunication_Arduino.cpp
// Author      : Gabriel Boucher
// Version     :
// Copyright   : --
// Description : Hello World in C++, Ansi-style
//============================================================================


#include <stdio.h>
#include <windows.h>
#include <string.h>

#define OK 0
#define NO_INPUT 1
#define TOO_LONG 2

void program_init();
static int getLine(char *prmpt,char *buff, size_t sz);


int main() {
	Sleep(1000);
	char bufferRead[12];
	char bufferSend[13];
	DWORD dwBytesRead=0;
	DWORD dwBytesSend=0;
	int startupbuffer=0;		//Buffer pour s'assurer qu'on commence à transmettre quand c'est stable
	int rc;
	char promptbuffA[15];
	char promptbuffD[15];

	program_init();

	Sleep(1000);

	rc=getLine("Entrer le COM port du Arduino: ",promptbuffA,sizeof(promptbuffA));
	rc=getLine("Entrer le COM port du DSP: ",promptbuffD,sizeof(promptbuffD));

	//Initialisation des ports séries. Cette section sera à modifier si les ports changent.
	//Il serait intéressant d'ajouter la commande pour sélectionner les ports.

	HANDLE CommArduino = ::CreateFile(promptbuffA,
			GENERIC_READ,
			0,
			0,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			0);

	if(CommArduino==INVALID_HANDLE_VALUE){
		if(GetLastError()==ERROR_FILE_NOT_FOUND){
			printf("Arduino port does not exist");
		}
	}

	DCB dcbSerialParamsA = {0};

	dcbSerialParamsA.DCBlength=sizeof(dcbSerialParamsA);

	if (!GetCommState(CommArduino,&dcbSerialParamsA)){
		printf("Error when creating the DCB for Arduino");
	}

	dcbSerialParamsA.BaudRate=CBR_57600;
	dcbSerialParamsA.ByteSize=8;
	dcbSerialParamsA.StopBits=ONESTOPBIT;
	dcbSerialParamsA.Parity=NOPARITY;

	if(!SetCommState(CommArduino,&dcbSerialParamsA)){
		printf("Error when setting parameters for Arduino");
	}

	COMMTIMEOUTS timeoutsA={0};

	timeoutsA.ReadIntervalTimeout=75;
	timeoutsA.ReadTotalTimeoutConstant=0;
	timeoutsA.ReadTotalTimeoutMultiplier=0;

	if(!SetCommTimeouts(CommArduino,&timeoutsA)){
		printf("Error when setting the timeouts for Arduino");
	}

	//Setup pour le port du DSP. À modifier pour l bluetooth
	HANDLE CommDSP = ::CreateFile(promptbuffD,
				GENERIC_WRITE,
				0,
				0,
				OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL,
				0);

	if(CommDSP==INVALID_HANDLE_VALUE){
		if(GetLastError()==ERROR_FILE_NOT_FOUND){
			printf("DSP port does not exist");
		}
	}
	DCB dcbSerialParamsD = {0};

	dcbSerialParamsD.DCBlength=sizeof(dcbSerialParamsD);

	if (!GetCommState(CommDSP,&dcbSerialParamsD)){
		printf("Error when creating the DCB for DSP");
	}


	dcbSerialParamsD.BaudRate=CBR_115200;
	dcbSerialParamsD.ByteSize=8;
	dcbSerialParamsD.StopBits=ONESTOPBIT;
	dcbSerialParamsD.Parity=NOPARITY;

	if(!SetCommState(CommDSP,&dcbSerialParamsD)){
		printf("Error when setting parameters for DSP");
	}

	COMMTIMEOUTS timeoutsD={0};

	timeoutsD.WriteTotalTimeoutConstant=0;
	timeoutsD.WriteTotalTimeoutMultiplier=50;

	if(!SetCommTimeouts(CommDSP,&timeoutsD)){
		printf("Error when setting the timeouts for DSP");
	}



	printf("Configuration des ports terminée\n");

	//Main loop de transmission
	while(1){
		//Lecture du port Arduino jusqu'au \r
		ReadFile(CommArduino,bufferRead,11,&dwBytesRead,NULL);
		//WriteFile(screen,bufferRead,strlen(bufferRead),NULL,NULL);
		printf("%s\n",bufferRead);
		//On evoi le buffer par le port du DSP.
		if(startupbuffer>30){
			strcat(bufferSend,"M");
			strcat(bufferSend,bufferRead);
			//printf("%s\n",bufferSend);
			int err1=WriteFile(CommDSP,bufferSend,strlen(bufferSend),&dwBytesSend,NULL);
			printf("%ld\n",dwBytesSend);

			if(err1==0){
				printf("La transmission a échouée");
			}
		}
		else{
			startupbuffer++;
		}

		for(int erase=0; erase<13;erase=erase+1){
			bufferRead[erase]='\0';
		}

		for(int erase=0; erase<14;erase=erase+1){
			bufferSend[erase]='\0';
		}
	}

	return 0;
}

void program_init(){
	printf("Communication entre le microcontroleur et le DSP pour le controle du robot patineur\n");
	Sleep(500);
	printf("\nUtilisation du programme et du robot:\n"
			"Etapes pour connecter le robot et le mettre en marche:\n"
			"-Aligner les patins du robot pour qu'il soit a zéro (tout droit)\n"
			"-Brancher l'alimentation du DSP et attendre quelques secondes\n"
			"-Mettre le courant de la batterie a ON\n"
			"-Connecter l'ordinateur au Bluetooth du DSP\n"
			"-Entrer les numéros de ports du Arduino et DSP a l'endroit approprie\n"
			"-Lorsque la connection est effectuee, mettre l'intérupteur du DSP vers le haut\n\n");


}

static int getLine(char *prmpt,char *buff, size_t sz){
	int ch, extra;

	//Get line with buffer overrun protection
	if (prmpt!=NULL){
		printf("%s", prmpt);
		fflush(stdout);
	}
	if(fgets(buff,sz,stdin)==NULL){
		return NO_INPUT;
	}
	if(buff[strlen(buff)-1]!='\n'){
		extra=0;
		while(((ch=getchar())!='\n') && (ch!=EOF))
			extra=1;
		return (extra==1) ? TOO_LONG :OK;
	}

	buff[strlen(buff)-1]='\0';
	return OK;
}
