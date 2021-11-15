//Programa entrada
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

int canal, i;
char cadena[16];

int main(){
	/***** Obrim named pipe *****/
	canal = open("./canal", O_WRONLY);
	if (canal<0){
		perror("Error obrint la pipe amb nom");
		exit(1);
	}
	
	/***** Procés d'entrada per teclat *****/
	write(1,"Entra matricula, telefon o nom\n",31);
	i=read(0, cadena, 16);
	while(i != 3 || cadena[1] != 'X' || cadena[0] != 'X'){		
		write(canal,cadena, i);
		i=read(1, cadena, 16);
	}
}