/*Practica 1, remota*/

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

int main(){
	FILE *archivo;
	struct sockaddr_in local,remota;
	struct timeval start, end;
	unsigned char Peticion[109],Datos[516],ACK[4],Error[30],msj[512],nombre[100],tipo[9];
	unsigned char PLect[2]={0x00,0x01};
	unsigned char PEscr[2]={0x00,0x02};
	unsigned char PDato[2]={0x00,0x03};
	unsigned char PACKM[2]={0x00,0x04};
	unsigned char PErro[2]={0x00,0x05};
	unsigned char num[2]={0x00,0x00};
	unsigned char modo[]="octet";
	unsigned char c;
	int udp_socket,lbind,tam,tama,res,rec,salida=0,erro=0,tamtrama,con,arc=0; //Declaracion de Variables
	long mtime=0, seconds, useconds;
	udp_socket = socket(AF_INET, SOCK_DGRAM, 0); //Abrir Puerto
	if (udp_socket == -1){ //Si no se puede abrir el puerto
		perror("\nError al abrir el socket");
		exit(1);
	}else{ //Si pudo abrir el puerto
		perror("\nExito al abrir el socket");
		memset(&local,0x00,sizeof(local)); //Borrar Contenido de Estructura
		local.sin_family=AF_INET; /* address family: AF_INET */
        local.sin_port=htons(0);   /* port in network byte order */
        local.sin_addr.s_addr=INADDR_ANY; /*Internet Address Aqui tecnicamente va la direccion de la computadora que envia pero no es necesario*/
        lbind=bind(udp_socket,(struct sockaddr *)&local,sizeof(local));
		if(lbind==-1){
			perror("\nError en el bind");
			exit(1);
		}else{
			perror("\nExito en el bind");
			memset(&remota,0x00,sizeof(remota)); //Borrar Contenido de Estructura
			remota.sin_family=AF_INET; /* address family: AF_INET */
			remota.sin_port=htons(69);   /* port in network byte order */
			remota.sin_addr.s_addr=inet_addr("8.12.0.240"); /*Internet Address de la computadora que va a RECIBIR*/
			while(salida==0){
				memset(tipo,0x00,9);
				memset(Peticion,0x00,109);
				memset(num,0x00,2);
				printf("\nTipo de Servicio (Lectura / Escritura):");
				gets(tipo);
				if(strcmp(tipo,"Lectura")==0 || strcmp(tipo,"lectura")==0){
					memcpy(Peticion+0,PLect,2);
					printf("\nNombre del archivo:\n");
					gets(nombre);
					archivo=fopen(nombre,"wb");
					memcpy(Peticion+2,nombre,strlen(nombre)+1);
					memcpy(Peticion+strlen(nombre)+3,modo,strlen(modo)+1);
					tama=sendto(udp_socket,Peticion,109,0,(struct sockaddr *)&remota,sizeof(remota));
					do{
						memset(Datos,0x00,516);
						gettimeofday(&start, NULL);
						while(mtime<2000){
							tam=sizeof(remota);
							rec=recvfrom(udp_socket,Datos,516,MSG_DONTWAIT,(struct sockaddr *)&remota,&tam); //recibe mensaje
							if(rec!=-1){
								break;
							}
							gettimeofday(&end, NULL);
							seconds  = end.tv_sec  - start.tv_sec;
							useconds = end.tv_usec - start.tv_usec;
							mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;
						}
						if(Datos[0]==PDato[0] && Datos[1]==PDato[1] && mtime<2000){
							memcpy(msj,Datos+4,rec);
							fwrite(msj,1,rec-4,archivo);
							if(num[1]==255){
								num[0]++;
								num[1]=0x00;
							}else if(num[0]==255 && num[1]==255){
								num[0]=0x00;
								num[1]=0x00;
							}else{
								num[1]++;
							}
							memcpy(ACK+0,PACKM,2);
							memcpy(ACK+2,num,2);
							tama=sendto(udp_socket,ACK,4,0,(struct sockaddr *)&remota,sizeof(remota));
							memset(ACK,0x00,4);
						}else{
							perror("Error en tiempo");
							exit(1);
						}
    				}while(rec==516);
					fclose(archivo);
				}else if(strcmp(tipo,"Escritura")==0 || strcmp(tipo,"escritura")==0){
					memcpy(Peticion+0,PEscr,2);
					printf("\nNombre del archivo:\n");
					gets(nombre);
					archivo=fopen(nombre,"rb");
					memcpy(Peticion+2,nombre,strlen(nombre)+1);
					memcpy(Peticion+strlen(nombre)+3,modo,strlen(modo)+1);
					tama=sendto(udp_socket,Peticion,109,0,(struct sockaddr *)&remota,sizeof(remota));
					gettimeofday(&start, NULL);
					while(mtime<2000){
						tam=sizeof(remota);
						rec=recvfrom(udp_socket,ACK,4,MSG_DONTWAIT,(struct sockaddr *)&remota,&tam); //recibe mensaje
						if(rec!=-1){
							break;
						}
						gettimeofday(&end, NULL);
						seconds  = end.tv_sec  - start.tv_sec;
						useconds = end.tv_usec - start.tv_usec;
						mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;
					}
					if(ACK[0]==PACKM[0] && ACK[1]==PACKM[1] && mtime<2000){
						do{
							arc=fread(msj,1,512,archivo);
							memcpy(Datos+0,PDato,2);
							memcpy(Datos+2,num,2);
							memcpy(Datos+4,msj,arc);
							tama=sendto(udp_socket,Datos,arc+4,0,(struct sockaddr *)&remota,sizeof(remota));
							memset(msj,0x00,512);
							memset(Datos,0x00,516);
							gettimeofday(&start, NULL);
							while(mtime<2000){
								tam=sizeof(remota);
								rec=recvfrom(udp_socket,ACK,4,MSG_DONTWAIT,(struct sockaddr *)&remota,&tam); //recibe mensaje
								if(rec!=-1){
									break;
								}
								gettimeofday(&end, NULL);
								seconds  = end.tv_sec  - start.tv_sec;
								useconds = end.tv_usec - start.tv_usec;
								mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;
							}
							if(mtime==2000 && rec==-1){
								printf("Error de tiempo\n");
								exit(1);
							}else{
								if(ACK[0]==PACKM[0] && ACK[1]==PACKM[1]){
									printf("ACK bueno %02x %02x\n",num[0],num[1]);
								}else{
									printf("Esperaba %02x %02x\n",num[0],num[1]);
									printf("Error de ACK: %02x %02x %02x %02x\n",ACK[0],ACK[1],ACK[2],ACK[3]);
									exit(1);
								}
							}
							if(num[1]==255){
								num[0]++;
								num[1]=0x00;
							}else if(num[0]==255 && num[1]==255){
								num[0]=0x00;
								num[1]=0x00;
							}else{
								num[1]++;
							}
						}while(arc==512);
					}else{
						erro=1;
					}
					fclose(archivo);
				}else{
					printf("\nError tipo de solicitud no valida.\n");
				}
				printf("Otra tarea? (S/N)");
				scanf("%c",&c);
				if(c=='S' || c=='s'){
					salida=0;//preguntar salida
				}else{
					salida=1;
				}
			}
		}
	}
	printf("\nComunicacion Terminada.\n");
	close(udp_socket); //Cerrar puerto IMPORTANTE
	return 1;
}
