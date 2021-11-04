#include<stdio.h>
#include<stdlib.h>
#include <sys/time.h>
#include <string.h>
struct timeval st;
struct t{
	int doc;
	int random;
};
int pesquisa(int contLin, int numLin,struct t* vet){
	int i;
	for (i=0; i<numLin; i++) {
    if (vet[i].doc==contLin) {
			return(i);
		}
	}
	return(-1);
}

int cmp(const void *a, const void *b){
	struct t *p,*q;
	p=(struct t*)a;
	q=(struct t*)b;
	if (p->random < q->random ) {
		return -1;
	}
	if (p->random >q->random) {
    return(1);
	}
	return(0);
}
int main(int arc, char** argv){
//execucao: ./gera5FoldXValidation nome_da_colecao nome_do_diretorio num.folds
	FILE *f= fopen(argv[1], "rt");
	if (!f) {
    fprintf(stderr, "Arquivo %s nao pode ser aberto\n", argv[1]);
		exit(1);
	}
	char buff[1000000];
	int numLin=0;
	//Contando o numero de linhas do arquivo com a colecao de documentos
	while (fscanf(f,"%[^\n]", buff)!=EOF) {
    fgetc(f);
		numLin++;
	}
	printf("numLin: %d\n", numLin);
	rewind(f);
	//Gerando uma sequencia aleatoria com a semente obtida pelo tempo
	//atual.
	gettimeofday(&st,NULL);
	srand(st.tv_usec); //usando a quantidade de segundos como semente

	/*Cria um vetor com dois campos: identificador de documento e um 
	numero aleatorio. Esse vetor sera usado para embaralhar os
	identificadores de documentos antes de distribui-los nos folds
	*/

	struct t vet[numLin];
	int i;
	for (i=0; i<numLin; i++) {
    vet[i].doc=i;
		vet[i].random=rand();
	}
	// Ordena o vetor pelo valor aleatorio para embaralhar os identificadores de documenos dentro do vetor
	qsort(vet,numLin,sizeof(struct t),cmp);
	printf("Ordenei\n");

	int posVet, fold, tamParte,contLin=0;
	int numFolds= atoi(argv[3]);
	FILE* arqFolds[numFolds]; //Vetor de ponteiros para arquivos de folds

	//Abrir arquivos que armazenam os folds
	char nomeFold[3000];
	for(i=0;i<numFolds;i++){
		sprintf(nomeFold,"fold%d", i);
		arqFolds[i]=fopen(nomeFold,"wt");
		if (!arqFolds[i]) {
			fprintf(stderr, "Arquivo %s nao pode ser aberto\n", nomeFold);
			exit(1);
		}
	}
	printf("Lendo a colecao e distribuindo em folds ...\n");
	//Lendo novamente a colecao e distribuindo os documentos em folds
	int parte=numLin/numFolds;
	while (fscanf(f,"%[^\n]", buff)!=EOF) {
    fgetc(f);
		posVet=pesquisa(contLin,numLin,vet);
		if (posVet<0) {
			printf("Erro o documento nao foi encontrado no vetor de posicoes\n");
			exit(1);
		}
		fold=posVet/parte;
		fold = fold>numFolds-1?numFolds-1: fold;
		fprintf(arqFolds[fold],"%s\n",buff);
		contLin++;
	}
	printf ("fechando arquivos de folds\n");
	for(i=0;i<numFolds;i++){
		fclose(arqFolds[i]);
	}
	fclose(f);

	sprintf(buff, "mkdir %s", argv[2]); //Criando o diretorio com arquivos de treino e teste correspondentes de cada um dos 5 folds.
	system(buff);

	for (i=0; i<numFolds; i++) {
		int j;
		strcpy(buff, "cat ");
    for (j=0; j<numFolds; j++) {
			if(i!=j){
				sprintf(nomeFold,"fold%d ",j);
				strcat(buff, nomeFold);
			}
		}
		sprintf(buff,"%s > %s/treino%d.txt\n", buff, argv[2],i);
		system(buff);

		//system
		sprintf(buff, "cp fold%d %s/teste%d.txt\n", i,argv[2],i);
		system(buff);
	}
	strcpy(buff, "rm -rf fold*");
	system(buff);
	return(0);
}