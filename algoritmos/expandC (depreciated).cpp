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
	./expandC arqTreinoEntrada arqParesDominantes arqTesteEntrada  arqTreinoSaida arqTesteSaida domMin supMin tipoExpansao
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

/*struct para os nos do indice invertido de pares. Cada termo t no ínidce contem uma lista dos termos com id superior a t que formam
	formam par com t, tal que esse par tem suporte >= supMin e dominancia>= domMin
*/
typedef struct no2 {
    int term2; //id do termo que forma par com t, tal que term2> t
		int cat; //categoria em que o par tem maior doninancia
    float dom; // valor da dominancia
    int sup; //valor do suporte do par na categoria
		int df;	//df do par na colecao
		int parId; //id do par: maior que maior id de termo na colecao
} Tno_par;
map < int,list<Tno_par> >  indTermPar;

typedef struct{
	int parId;
	int cat;
	double peso;
}TParPeso;

typedef struct termoPeso{
	int term;
	short int cat;
	double peso;
}TTermoPeso;
typedef struct{
	short int cat;
	double peso;
}TSomaCat;
//Funcoes auxiliares:
//------
bool comparaPesosPar (const TParPeso &a, const TParPeso &b) { return (a.peso>b.peso); }
bool comparaParId (const TParPeso &a, const TParPeso &b) { return (a.parId<b.parId); }
bool comparaPesosTermos (const TTermoPeso &a, const TTermoPeso &b) { return (a.peso>b.peso); }
bool comparaTermId (const TTermoPeso &a, const TTermoPeso &b) { return (a.term<b.term); }
void infoColecaoTreino(char *nomeArqTreinoEntrada, int &idUltTerm, int &numClasses, int &numDocs, int &maxTermPorDoc);
void criaIndiceDeTermos(char* nomeArqTreinoEntrada, int idUltTerm,int numClasses,int numDocs);
//void insereLista(map< pair<int,int>, list<Tno_term> >::iterator it, int categoria );
//void insereIndiceTermos (int termo, int doc, int categoria, int freq);
//void insereTermo(int doc, int termo, int categoria, int frequencia);


void lePares(char* nomeArq, float supMin, float domMin);
void expandeDocs(char* nomeArqEntrada, char* nomeArqSaida, int tamMaiorLinha, int numClasses, int numDocs, double domMin, double supMin);
//------

int main(int argc, char** argv) {
	struct timeval start_time, end_time;// para marcar o tempo de
	gettimeofday(&start_time, NULL); //    execucao do  programa
	//variaveis
	int numDocs, categoria, i, c, c2;
	int idUltTerm, numClasses, maxTermPorDoc;
	double domMin = atof(argv[6]);
	double supMin = atof(argv[7]);

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

	//Lê os pares_termos do disco e salva na memória
	lePares(argv[2], supMin, domMin);

	//Ler o arquivo de treino  novamente e gerar novo arquivo de treino expandido com os pares
	fprintf(stderr,"Gerando arquivo de treino expandido com pares...\n");
	expandeDocs(argv[1], argv[4], maxTermPorDoc, numClasses, numDocs, domMin, supMin );
	fprintf(stderr,"Gerando arquivo de teste expandido com pares...\n");
	//Ler arquivo de teste e gerar novo arquivo de teste expandido com os pares
	expandeDocs(argv[3], argv[5], maxTermPorDoc,numClasses, numDocs, domMin, supMin);
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
			indTerm[termo].df+=freq;// VOLTAR DEPOIS PARA ++
			indTerm[termo].cat=categoria;
			indTerm[termo].vetSuporteClass[categoria]+=freq;//VOLTAR DEPOIS PARA++
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

void lePares(char* nomeArq, float supMin, float domMin){
	/* Le pares de termos na map indTermPar, se a dominancia dom do par <p1,p2> for tal que dom>=domMin e o suporte de <p1,p2> >= supMin,
		entao incluir no map, para a chave p1, o Tno formado por p2, cat,dom, suporte e parId. O valor de parI para o par <p1p2> 
		corresponde ao idMaior atual +1.
	
	*/
	int p1,p2,parId,classe,suporteCl,df; char c;
	float dom;
	FILE *f2 = fopen(nomeArq, "rt");
	if (!f2) {
		fprintf(stderr,"Arquivo TermosParesTreino nao pode ser aberto\n");
		exit(1);
	}
	while(fscanf(f2, "(%d,%d):%d:%d:%f:%d:%d", &p1, &p2,&parId, &classe, &dom, &suporteCl, &df)!=EOF){
			c=fgetc(f2);
			if (dom>domMin && (float)suporteCl>=supMin) {
				Tno_par p;
				p.term2=p2;
				p.cat=classe;
				p.dom  = dom;
				p.sup = suporteCl;
				p.df=df;
				p.parId= parId;
				//*****DEBUG***********
				//Esse printf é apenas para DEBUG, para verificar se a expansao esta
				//sendo feita corretamente. Deve ser removido depois
				//printf("(%d:%d):%d:%d:%lf:%d:%d\n", p1,p2,p.parId,classe,dom,suporteCl,df);
				map < int,list<Tno_par> >::iterator it;
				it=indTermPar.find(p1);
				if (it==indTermPar.end()) {
					list<Tno_par> l;
					l.push_back(p);
					indTermPar.insert(make_pair(p1,l));
				}
				else{
					indTermPar[p1].push_back(p);
				}
			}

	}
	fclose(f2);
}

void expandeDocs(char* nomeArqEntrada, char* nomeArqSaida, int tamMaiorLinha,  int numClasses, int numDocsTreino, double domMin, double supMin){
  fprintf(stderr, "domMin: %lf supMin: %lf\n",domMin, supMin);
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
	map <int,float> mapTermPeso;
	vector<TParPeso> vetParesExpand;
	vector <double> soma(numClasses,0.0);

	vector<TTermoPeso> vetTermos;
	int  numdocsExp=0;
	while(fscanf(fin, "%d", &cat)!=EOF){
		fprintf(fout,"%d", cat);
		mapTermPeso.clear();
		vetParesExpand.clear();
		vetTermos.clear();
		do{
			fscanf(fin,"%d:%d",&term,&freq);
			//fprintf(fout," %d:%d", term, freq);
			//armazenar o termo e seu peso
			//o peso experimental e: tf*dfClDom^2*NumDocs/(df^2*|ClDom|)
			Tno_term aux=indTerm[term];
			double peso1,peso2,peso;
			if(aux.df==0){
				peso=0.0;
			}
			else{
				peso1=(double)freq*(double)aux.vetSuporteClass[aux.cat];
				peso2=(double)aux.df;//*log(suporteClasse[aux.cat]);
				peso=peso1/peso2;//*(log(numDocsTreino)-log(aux.df));
			}
			TTermoPeso aux1; aux1.term=term;aux1.peso=peso;aux1.cat=aux.cat;
			vetTermos.push_back(aux1);
			mapTermPeso[term]=freq;
			c=fgetc(fin);
			//soma[aux1.cat]+=aux1.peso;
			//fprintf(fout," %d:%.4lf", term,peso);
		}while(c!='\n'&& c!=EOF);
		numDocs++;

		//sort(vetTermos.begin(), vetTermos.end(), comparaPesosTermos);
		int primClasses[3];
		int indPrimClasses=0;
		for (int k=0; k<3; k++) {
			primClasses[indPrimClasses++]=vetTermos[k].cat;
		}

		map<int,float>::iterator mapTermPesoIt;
		//para cada termo lido e armazenado, do documento, procurar no map de
		//pares se ha algum par que se inicia com esse termo
		for (mapTermPesoIt=mapTermPeso.begin(); mapTermPesoIt!=mapTermPeso.end(); mapTermPesoIt++) {
			//if (mapTermPesoIt->(double)vetSuporteClass[mapTermPesoIt->cat]/(double)mapTermPesoIt->df<0.5) {
				//continue;
			//}
			map < int,list<Tno_par> >::iterator indTermParIt;
			indTermParIt= indTermPar.find(mapTermPesoIt->first);
			if (indTermParIt!=indTermPar.end()) { //Ha pelo menos um par que se inicia
				//com o termo correspondente a mapTermFreqIt->first
				// Acessar todos os pares que se iniciam com esse termo e verificar
				// se os segundos elementos desses pares também aparecem no documento
				list<Tno_par>::iterator itl;
				for (itl=indTermParIt->second.begin(); itl!=indTermParIt->second.end(); itl++) {
						map<int,float>::iterator mapTermPesoIt2=mapTermPeso.find(itl->term2);
						if (mapTermPesoIt2 != mapTermPeso.end()) {// o par ocorre no documento.
							//if (itl->cat==primClasses[0] || itl->cat==primClasses[1]){
								TParPeso aux; aux.parId=itl->parId; aux.cat=itl->cat;
								double peso1=itl->sup*(mapTermPesoIt->second+mapTermPesoIt2->second)/2.0;
								double peso2=(double)itl->df;//*log(suporteClasse[itl->cat]);
								//aux.peso=peso1/peso2*log(itl->sup*numDocsTreino/suporteClasse[itl->cat]);
								aux.peso=peso1/peso2;
								//*log(itl->sup)*(log(numDocsTreino)-log(itl->df))
								if(aux.peso>=domMin && itl->sup>=supMin){
									vetParesExpand.push_back(aux);
								}

							//}
						}
				}
			}
		}
		if(vetParesExpand.size()>0){
			numdocsExp++;
		}

		int pos=0; double pesoAnt=0;
		for (int i=1; i<vetTermos.size(); i++) {
			if (vetTermos[i].peso >=0.5) {
				if (vetTermos[i].peso!=pesoAnt) {
					pesoAnt=vetTermos[i].peso;
					pos++;
				}
				//soma[vetTermos[i].cat]+=vetTermos[i].peso/log(pos+1);
			}
		}

		sort(vetParesExpand.begin(), vetParesExpand.end(), comparaPesosPar);
		pos=0; pesoAnt=0;
		for (int i=0;i<vetParesExpand.size(); i++) {
			if (vetParesExpand[i].peso!=pesoAnt) {
				pesoAnt=vetParesExpand[i].peso;
				pos++;
			}
			soma[vetParesExpand[i].cat]+=vetParesExpand[i].peso;
		}
		int primClasse=-1, segClasse=-1;
		double primMaiorPeso=0.0, segMaiorPeso=0.0;
		for (int i=0; i<numClasses; i++) {
			if(soma[i]>primMaiorPeso){
				segMaiorPeso=primMaiorPeso;
				segClasse=primClasse;
				primMaiorPeso=soma[i];
				primClasse=i;
			}
			else{
				if (soma[i]>segMaiorPeso) {
					segMaiorPeso=soma[i];
					segClasse=i;
				}
			}
			soma[i]=0.0;
		}
		//sort(vetTermos.begin(), vetTermos.end(), comparaTermId);
		//fprintf(fout, " %d :%.4lf %d :%.4lf # %lu --",primClasse, primMaiorPeso, segClasse, segMaiorPeso,vetParesExpand.size());

		for (int k=0; k<vetTermos.size(); k++) {
			fprintf(fout," %d:%.4lf", vetTermos[k].term, vetTermos[k].peso);
		}


		sort(vetParesExpand.begin(), vetParesExpand.end(), comparaParId);
		for (int i=0; i<vetParesExpand.size(); i++) {
		  //if(vetParesExpand[i].cat==primClasses[0]||vetParesExpand[i].cat==
				 //primClasses[1])// || vetParesExpand[i].cat==primClasses[2]
				fprintf(fout," %d:%.4lf",vetParesExpand[i].parId, vetParesExpand[i].peso);
		}
		fprintf(fout, "\n");
	}
	fprintf(stderr,"Numero de documentos expandidos: %d\n",numdocsExp);
	fprintf(stderr,"Numero de documentos no arquivo: %d\n",numDocs);
	fclose(fin);
	fclose(fout);
}
