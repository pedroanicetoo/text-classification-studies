#include <vector>
#include <algorithm>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <list>
#include <map>
#include <limits.h>
#include <sys/time.h>
#include <math.h>
using namespace std;
/*
	Execucao do programa:
	./gera-tf-idf arqTreinoEntrada arqTesteEntrada arqTreinoSaida arqTesteSaida
*/
//struct para os nos das listas do indice de termos originais
typedef struct no1 {
	int df; //suporte do termo na colecao de treino
	short int cat; //categoria em que o termo domina (ocorre mais vezes)
	int* vetSuporteClass; //vetor com o valor de suporte do termo em cada classe
} Tno_term;

/*array para armazenar o indice  de termos */
 Tno_term* indTerm = NULL;

/* Map para armazenar o numero de documentos em cada classe*/
map<int,int> suporteClasse;

//Funcoes auxiliares:
//------
void infoColecaoTreino(char *nomeArqTreinoEntrada, int &idUltTerm, int &numClasses, int &numDocs, int &maxTermPorDoc);
void criaIndiceDeTermos(char* nomeArqTreinoEntrada, int idUltTerm,int numClasses,int numDocs);
void geraArq_tf_idf(char* nomeArqEntrada, char* nomeArqSaida,
										int numClasses, int numDocsTreino);
//------

int main(int argc, char** argv) {
	int numDocs, categoria, i, c, c2;
	int idUltTerm, numClasses, maxTermPorDoc;

	//Ler o arquivo de treino obtendo informacoes sobreo mesmo: identificador do ultimo termo, numero de classes, numero de documentos,  numero maximo de termo em um documento
	infoColecaoTreino(argv[1], idUltTerm, numClasses, numDocs,maxTermPorDoc);
	fprintf(stderr,"Numero de termos do treino: %d: \n",idUltTerm);
	for (int i=0; i<numClasses; i++) {
    fprintf(stderr, "classe: %d num docs.: %d\n",i,suporteClasse[i]);
	}
	fprintf(stderr,"\n");
	/*Lê arquivo de treino novamente obtendo um indice de termos com as seguintes informacoes sobre cada termo: df (suporte) do termo no treino,
		valor da dominância do termo na classe em que ele ocorre mais e o suporte do termo em cada classe. Obter tambem um vetor com o numero de
		documentos por classe.
	*/
	criaIndiceDeTermos(argv[1],idUltTerm,numClasses,numDocs);


	//Ler o arquivo de treino  novamente e gerar arquivo de treino de saída com
	//pesos tf-idf
	fprintf(stderr,"Gerando arquivo de treino com pesos tf-idf..\n");
	geraArq_tf_idf(argv[1], argv[3],numClasses, numDocs);
	fprintf(stderr,"Gerando arquivo de teste com pesos tf-idf...\n");
	//Ler arquivo de teste e gerar novo arquivo de teste com os pesos tf-idf
	geraArq_tf_idf(argv[2], argv[4],numClasses, numDocs);
	return 0;
}

void infoColecaoTreino(char *nomeArqTreinoEntrada, int &idUltTerm, int &numClasses, int &numDocs, int &maxTermPorDoc){

	int categoria, termo, freq, c, numTermPorDoc;
	FILE *f = fopen(nomeArqTreinoEntrada, "rt");
	if (!f) {
    printf("Arquivo de treino %s nao pode ser aberto\n", nomeArqTreinoEntrada);
		exit(1);
	}
	numClasses=-1; idUltTerm=-1; maxTermPorDoc=0, numDocs=0;
	//Leitura da arquivo

	while(fscanf(f, "%d", &categoria)!=EOF){
		numDocs++;
		if (categoria>numClasses) {
			numClasses=categoria;
		}
		map<int,int>::iterator it=suporteClasse.find(categoria);
		if (it==suporteClasse.end()) {
			suporteClasse[categoria]=1;
		}
		else
			suporteClasse[categoria]++;
		numTermPorDoc=0;
		do{
			fscanf(f,"%d:%d",&termo, &freq);
			c=fgetc(f);
			numTermPorDoc++;
			if (termo>idUltTerm) {
				idUltTerm=termo;
			}
		}while(c!='\n'&& c!=EOF);
		if (numTermPorDoc>maxTermPorDoc) {
			maxTermPorDoc=numTermPorDoc;
		}
	}
	numClasses++;
	fclose(f);
}

void criaIndiceDeTermos(char* nomeArqTreinoEntrada, int idUltTerm,int numClasses,int numDocs){
	printf("NumClasses: %d\n",numClasses);
	idUltTerm++;
	indTerm = (Tno_term*)malloc(idUltTerm*sizeof(Tno_term));
	if (!indTerm) {
    fprintf(stderr,"Nao ha memoria suficiente para criar indice de termos\n");
		exit(1);
	}
	for (int i=0; i<=idUltTerm; i++) {
    indTerm[i].df=0; indTerm[i].vetSuporteClass=NULL;
		//Vetor para armazenar o suporte do termo (df) em cada classe
		indTerm[i].vetSuporteClass=(int*)malloc(sizeof(int)*numClasses);
		if (indTerm[i].vetSuporteClass) {
			for (int j=0; j<numClasses; j++) {
				indTerm[i].vetSuporteClass[j]=0;
			}
		}
		else{
			fprintf(stderr, "Nao ha espaco para o indice de termos\n.");
			exit(1);
		}
	}
	// Ler arquivo de treino preenchendo o indice de termos
	int categoria, termo, freq, c;
	FILE *f = fopen(nomeArqTreinoEntrada, "rt");
	if (!f) {
    printf("Arquivo de treino %s nao pode ser aberto\n", nomeArqTreinoEntrada);
		exit(1);
	}
	while(fscanf(f, "%d", &categoria)!=EOF){
		do{
			fscanf(f,"%d:%d",&termo, &freq);
			c=fgetc(f);
			indTerm[termo].df++;
			indTerm[termo].cat=categoria;
			indTerm[termo].vetSuporteClass[categoria]++;
		}while(c!='\n'&& c!=EOF);
	}
	//Computa a classe em que cada termo tem maior dominancia e o valor dessa
	// dominância.
	for (int i=0; i<=idUltTerm; i++) {
		float maiorSuporte=-1; short int classMaiorSuporte=-1;
    for (int j=0; j<numClasses; j++) {
			if (indTerm[i].vetSuporteClass[j]>maiorSuporte) {
				maiorSuporte=indTerm[i].vetSuporteClass[j];
				classMaiorSuporte=j;
			}
		}
		indTerm[i].cat=classMaiorSuporte;
	}
	fclose(f);
}

void geraArq_tf_idf(char* nomeArqEntrada, char* nomeArqSaida,
										int numClasses, int numDocsTreino){

	FILE *fin, *fout;
	int numDocs=0;
	fin=fopen(nomeArqEntrada, "rt");
	if (fin==NULL) {
    fprintf(stderr, "Arquivo %s nao pode ser aberto\n", nomeArqEntrada);
		exit(1);
	}
	fout=fopen(nomeArqSaida, "wt");
	if (fout==NULL) {
    fprintf(stderr, "Arquivo %s nao pode ser aberto\n", nomeArqSaida);
		exit(1);
	}
	int cat, term, freq; char c;
	while(fscanf(fin, "%d", &cat)!=EOF){
		fprintf(fout,"%d", cat);
		do{
			fscanf(fin,"%d:%d",&term,&freq);
			c=fgetc(fin);
			Tno_term aux=indTerm[term];
			double peso;
			if(aux.df==0){
				peso=0.0;
			}
			else{
				peso=(double)freq*log((double)numDocsTreino/(double)aux.df);
			}
			fprintf(fout," %d:%.4lf", term,peso);
		}while(c!='\n'&& c!=EOF);
		fprintf(fout, "\n");
	}
	fclose(fin);
	fclose(fout);
}
