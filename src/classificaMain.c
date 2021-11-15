//Programa classifica 
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <sys/file.h>
#include <signal.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>

int pidTlf, pidMat, pidNom; //Processos fork
int fTlf, fMat, fNom; //Fitxers
int p[2]; //Pipe interna

void guardaNom(){ //Llegeix nom de pipe i el guarda a fitxer
	char cad[16];
	int j = read(p[0], cad, 16);
	write(fNom,cad,j);
}

void guardaMatricula(){ //Llegeix matricula de pipe i el guarda a fitxer
	char cad[16];
	int j = read(p[0], cad, 16);
	write(fMat,cad,j);
}

void guardaTelefon(){ //Llegeix telefon de pipe i el guarda a fitxer
	char cad[16];
	int j = read(p[0], cad, 16);
	write(fTlf,cad,j);
}

void sortir(){ //Acaba el procés que l'invoca
	exit(1);
}

int main(){
	/***** Named pipe && pipe interna *****/
	if(mkfifo("./canal",00660)!=0){ perror("Error creant la pipe amb nom"); exit(1); }
	
	write(1,"Classifica Iniciat: esperant programa entrada...\n",49);
	int canal = open("./canal", O_RDONLY);
	if (canal<0){ perror("Error obrint la pipe amb nom"); exit(1); }
	
	if(pipe(p)<0){ perror("Error pipe interna"); exit(1); }
	
	
	
	/****** Creem processos ******/
	if ((pidTlf=fork()) == 0){ //Procés telefon
		fTlf = open("./Telefons.txt", O_WRONLY | O_CREAT | O_TRUNC, 00600); //Obrim fitxer sortida
		if(fTlf < 0){ perror("Error amb fitxer"); exit(1); }
		signal(SIGUSR1, guardaTelefon);
		signal(SIGUSR2, sortir);
		while(1) pause(); //Esperem fins a rebre senyal
	} 
	else if (((pidMat=fork()) == 0)){ //Procés Matricula
		fMat = open("./Matricules.txt", O_WRONLY | O_CREAT | O_TRUNC, 00600); //Obrim fitxer sortida
		if(fMat < 0){ perror("Error amb fitxer"); exit(1); }
		signal(SIGUSR1, guardaMatricula);
		signal(SIGUSR2, sortir);
		while(1) pause(); //Esperem fins a rebre senyal
	} 
	else if (((pidNom=fork()) == 0)){ //Proces nom
		fNom = open("./Noms.txt", O_WRONLY | O_CREAT | O_TRUNC, 00600); //Obrim fitxer sortida
		if(fNom < 0){ perror("Error amb fitxer"); exit(1); }
		signal(SIGUSR1, guardaNom);
		signal(SIGUSR2, sortir);
		while(1) pause(); //Esperem fins a rebre senyal
	} 
	else if(pidTlf == -1 || pidMat == -1 || pidNom == -1){
		perror("Error al crear processos fills");
		exit(1);
	}
	else { //Procés classifica
		char cadena[16];
		
		//Selecció
		int i=read(canal, cadena, 16);
		while(i > 0){
			write(p[1],cadena,i);
			if(cadena[i-2] <= '9' && cadena[i-2] >= 0) kill(pidTlf, SIGUSR1); //És telefon
			else if(cadena[0] <= '9' && cadena[0] >= 0) kill(pidMat, SIGUSR1); //És Matricula
			else kill(pidNom, SIGUSR1); //És Nom
			i=read(canal,cadena, 16);
		}
		
		//Enviem senyal per acabar processos
		kill(pidTlf, SIGUSR2);
		kill(pidMat, SIGUSR2);
		kill(pidNom, SIGUSR2);
		
		//Esperem a acabar processos
		int estat1, estat2, estat3;
		wait(&estat1); wait(&estat2); wait(&estat3);
		
		//Tanquem i eliminem named pipe
		if(remove("./canal")!=0) write(1,"Canal no eliminat\n",15);
	}
	close(p[0]); close(p[1]);
}
