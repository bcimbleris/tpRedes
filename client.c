#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

typedef struct {
    double latitude;
    double longitude;
}Coordinate;
//Exemplo de como rodar o programa, aparece caso o programa seja usado errado
void usage(int argc, char **argv) {
	printf("usage: %s <server IP> <server port>\n", argv[0]);
	printf("example: %s 127.0.0.1 51511\n", argv[0]);
	exit(EXIT_FAILURE);
}

#define BUFSZ 1024

int main(int argc, char **argv) {
	//criando coordenada para Parque Guanabara
	Coordinate coordClient = {-19.8595, -43.9784};
	//verificando se o programa foi utilizado de maneira correta
	if (argc < 3) {
		usage(argc, argv);
	}
	//pode funcionar como sockaddrin ou sockaddr6
	struct sockaddr_storage storage;
	//retorna 0 quando funciona,recebe endereço do servidor, porto e ponteiro para o sockaddr_storage que vai ser inicializado
	if (0 != addrparse(argv[1], argv[2], &storage)) {
		usage(argc, argv);
	}
	//abre um socket de conexão TCP 
	int s;
	s = socket(storage.ss_family, SOCK_STREAM, 0);
	//verificando se deu erro ao abrir o socket, toda vez que o socket dá erro retorna -1
	if (s == -1) {
		logexit("socket");
	}
	//uma interface que instancia um ponteiro e é feito o cast para o tipo e coloca dentro da variável
	struct sockaddr *addr = (struct sockaddr *)(&storage);
	//chamando a função connect, toda vez que ela deu certo retorna 0, portanto já é feito o tratamento de erro

	//passa o socket s, o endereço do servidor e o tamanho dessa estrutura
	if (0 != connect(s, addr, sizeof(storage))) {
		logexit("connect");
	}
	//endereço do endereço é guardado nessa variável
	char addrstr[BUFSZ];
	//imprime o endereço nessa string, recebe endereço, string e tamanho do buffer
	addrtostr(addr, addrstr, BUFSZ);

	printf("connected to %s\n", addrstr);
	//comunicação do cliente com servidor
	char buf[BUFSZ];
	//inicializa o buffer com 0
	memset(buf, 0, BUFSZ);
	printf("mensagem> ");
	//le do teclado o que a pessoa vai digitar e manda para o servidor
	fgets(buf, BUFSZ-1, stdin);
	//socket, o dado que vai mandar, o número de bytes que vai mandar com +1 para incluir o /0 e por fim o 0 é porque não precisamos utilizar nenhuma função especial
	//fala qual é o número de bytes que foram efetivamente transmitidos na rede e termina a conexão caso tenha erro
	size_t count = send(s, buf, strlen(buf)+1, 0);
	if (count != strlen(buf)+1) {
		logexit("send");
	}

	memset(buf, 0, BUFSZ);
	//total de bytes recebidos até o momento, utilizado para ir colocando o dado à frente no buffer
	unsigned total = 0;
	//fica recebendo dados do servidor até terminar a conexão
	while(1) {
		//recebe a resposta do servidor no socket s, coloca o dado no buff, o tanto de dado que vai receber no bufsz e flag 
		count = recv(s, buf + total, BUFSZ - total, 0);
		//caso em que nada foi recebido
		if (count == 0) {
			// Connection terminated.
			break;
		}
		//acrescenta o tanto de bytes recebido nessa variável
		total += count;
	}
	//fecha o socket após terminar a conexão
	close(s);
	//imprime o tanto de bytes recebidos e a mensagem
	printf("received %u bytes\n", total);
	puts(buf);

	exit(EXIT_SUCCESS);
}