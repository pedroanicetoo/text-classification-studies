#include <vector>
#include <algorithm>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <list>
#include <map>
#include <set>
#include <limits.h>
#include <sys/time.h>
#include <math.h>
using namespace std;
/*
	Execucao do programa:
	./expand21  arqTreinoEntrada arqParesDominantes arqTesteEntrada arqTreinoSaida arqTesteSaida domMin supMin rMinDesbal
*/

/*
  Definir o set que amazena as classes cujos pares de termos que predominam nessas classes serao 
  utilizados na expansão. 
  Um par pode ser utilizado na expansão de um documento somente se ele predominar em uma das classes desse set. As classes que participam
  * desse set são definidas pelo parâmetro razão minima de desbalanceamento rMimDesbal. Uma
  *  classe  c pertence a esse set somente se suporte(maiorClass)/suporte(c)>=rMinDesbal, onde
  * suporte(*) e o numero de documentos do treino que sao da classe *. maiorClass e classe com 
  * maior numero de documentos no treino.   
*/

map<int,int> suporteClasse;

/*
 struct para armazenar informacoes sobre um par de termos
*/
typedef struct {  
		int cat; //categoria em que o par tem maior doninancia
    float dom; // valor da dominancia
    int sup; //valor do suporte do par na categoria
		float df;	//df do par na colecao
		int parId; //id do par: maior que maior id de termo na colecao
} ParInfo;

/*
 map cuja chave de pesquisa é um par: (t_i, t_j), em que t_i<t_j. Esse map armazenara os pares tais que as
 classes em que eles predominam são classes cuja razao de desbalanceamento for maior ou igual a
 * rMinDesbal, como esplicado anteriormente.
*/
map < pair < int,int >, ParInfo>  mapPar;



//Funcoes auxiliares:
//------

//bool comparaTermId (const TTermoPeso &a, const TTermoPeso &b) { return (a.term<b.term); }

bool infoColecaoTreino(char *nomeArqTreinoEntrada, int &idUltTerm, int &numClasses, int &numDocs, 
int &maxTermPorDoc, float rMinDesbal);
void lePares(char* nomeArq, double supMin, double domMin);
void expandeDocs(char* nomeArqEntrada, char* nomeArqSaida, int tamMaiorLinha, int numClasses, int numDocs, double domMin, double supMin);
//------

int main(int argc, char** argv) {
	struct timeval start_time, end_time;// para marcar o tempo de
	gettimeofday(&start_time, NULL); //    execucao do  programa
	//variaveis
	int numDocs,idUltTerm, numClasses, maxTermPorDoc;
	float domMin = atof(argv[6]);
	float supMin = atof(argv[7]);
	float rMinDesbal =atof(argv[8]);

	//Ler o arquivo de treino obtendo informacoes sobre o mesmo: identificador do ultimo termo, numero de classes, numero de documentos,  numero maximo de termo em um documento
	if(!infoColecaoTreino(argv[1], idUltTerm, numClasses, numDocs,maxTermPorDoc, rMinDesbal)){
		fprintf(stderr, "A expansao não sera feita, ");
		fprintf(stderr, "pois a nao ha categoria c cuja razao suporte(maiorCat)/suporte(c) >= rMinDesbal\n");
		exit(1);
	}
	fprintf(stderr,"Numero de termos do treino: %d: \n",idUltTerm);
	fprintf (stderr,"Numero de classes do treino: %d: \n",numClasses);
	fprintf (stderr,"Numero de documentos do treino: %d: \n",numDocs);
	map < int, int>::iterator it;
	for (it=suporteClasse.begin();it!=suporteClasse.end();it++ ) {
    fprintf(stderr, "classe: %d num docs.: %d\n",it->first,it->second);
	}
	fprintf(stderr,"\n");
    fprintf(stderr,"Lendo o arquivo de pares\n");
	//Lê os pares_termos do disco e salva na memória
	lePares(argv[2], supMin, domMin);

	//Ler o arquivo de treino  novamente e gerar novo arquivo de treino expandido com os pares
	fprintf(stderr,"Gerando arquivo de treino expandido com pares...\n");
	expandeDocs(argv[1], argv[4], maxTermPorDoc, numClasses, numDocs, domMin, supMin );
	fprintf(stderr,"Gerando arquivo de teste expandido com pares...\n");
	//Ler arquivo de teste e gerar novo arquivo de teste expandido com os pares
	expandeDocs(argv[3], argv[5], maxTermPorDoc,numClasses, numDocs, domMin, supMin);
	gettimeofday(&end_time, NULL);
	fprintf(stderr, "Tempo: %ld min. %ld seg.\n",(end_time.tv_sec-start_time.tv_sec)/60,(end_time.tv_sec-start_time.tv_sec)%60);
	return 0;
}

bool infoColecaoTreino(char *nomeArqTreinoEntrada, int &idUltTerm, int &numClasses, 
					   int &numDocs, int &maxTermPorDoc, float rMinDesbal){

	int categoria, termo, freq, c, numTermPorDoc, supMaiorCategoria=0;
	FILE *f = fopen(nomeArqTreinoEntrada, "rt");
	if (!f) {
    printf("Arquivo de treino %s nao pode ser aberto\n", nomeArqTreinoEntrada);
		exit(1);
	}
	numClasses=-1; idUltTerm=-1; maxTermPorDoc=0, numDocs=0;
	//Leitura da arquivo
	map<int,int>::iterator it;
	while(fscanf(f, "%d", &categoria)!=EOF){
		numDocs++;
		if (categoria>numClasses) {
			numClasses=categoria;
		}
		it = suporteClasse.find(categoria);
		if (it==suporteClasse.end()) {
			suporteClasse[categoria]=1; 
		}
		else{
			suporteClasse[categoria]++;
			if(suporteClasse[categoria]>supMaiorCategoria)
				supMaiorCategoria=suporteClasse[categoria];
		}
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
	for(it=suporteClasse.begin();it!=suporteClasse.end();){		
		if((double)supMaiorCategoria/(double)it->second < rMinDesbal){
		  map<int,int>::iterator ita=it++;
		  suporteClasse.erase(ita);
		}
		else
			it++;
	}
	if(suporteClasse.empty())
		return(false);
	fclose(f);
	return(true);
}


void lePares(char* nomeArq, double supMin, double domMin){
	/* Le pares de termos na map indTermPar, se a dominancia dom do par <p1,p2> for tal que dom>=domMin e o suporte de <p1,p2> >= supMin,
		entao incluir no map, para a chave p1, o Tno formado por p2, cat,dom, suporte e parId. O valor de parI para o par <p1p2> 
		corresponde ao idMaior atual +1.
	
	*/
	int t1,t2,parId,classe,suporteCl,df;
	double dom;
	FILE *f2 = fopen(nomeArq, "rt");
	if (!f2) {
		fprintf(stderr,"Arquivo TermosParesTreino nao pode ser aberto\n");
		exit(1);
	}
	map < pair<int,int>,ParInfo>::iterator it;
	int car;
	while(fscanf(f2, "(%d,%d):%d:%d:%lf:%d:%d", &t1, &t2,&parId, &classe, &dom, &suporteCl, &df)!=EOF){
			car=fgetc(f2);
			map<int,int>::iterator itc = suporteClasse.find(classe);
			if(itc!=suporteClasse.end()){
			   if((double)suporteCl/(double)suporteClasse[classe] >=supMin && dom  >= domMin){		
					ParInfo p;
					p.cat=classe;
					p.dom  = dom;
					p.sup = suporteCl;
					p.df=df;
					p.parId= parId;
					it=mapPar.find(make_pair(t1,t2));
					if (it==mapPar.end()) {
						mapPar.insert(make_pair(make_pair(t1,t2),p));
					}
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
	map <int,double> mapTermPeso;
	int  numDocsExp=0;
	while(fscanf(fin, "%d", &cat)!=EOF){
		fprintf(fout,"%d", cat);
		c=fgetc(fin);
		mapTermPeso.clear();
		while(c!='\n'&& c!=EOF){
			fscanf(fin,"%d:%d",&term,&freq);
			fprintf(fout," %d:%d", term, freq);
			mapTermPeso[term]=freq;
			c=fgetc(fin);
		}
		bool jaHouveUmParInserido=false;
		numDocs++;
		map< pair<int,int>, ParInfo > :: iterator itPar;
		for(itPar=mapPar.begin();itPar!=mapPar.end();itPar++){
			pair <int, int > parTermo=itPar->first;
			map <int, double> ::iterator itTerm1, itTerm2;
			//Verificando se os termos que compoem o par participam da linha correspondente ao 
			//documento lido.
			itTerm1=mapTermPeso.find(parTermo.first);
			itTerm2=mapTermPeso.find(parTermo.second);
			if(itTerm1!=mapTermPeso.end() && itTerm2!=mapTermPeso.end()){ //Se participam...
				//Considerar o peso do par como a soma dos pesos
				//de seus termos constituintes no documento
				double pesoPar= (itTerm1->second+itTerm2->second);
				//Expandir a linha com o par e seu peso
				fprintf(fout, " %d:%.2f", itPar->second.parId, pesoPar);
				if(!jaHouveUmParInserido){
					numDocsExp++;
					jaHouveUmParInserido=true;
				}
			}
		}
		fprintf(fout, "\n");
	}	
	fprintf(stderr,"Numero de documentos expandidos: %d. Percentagem de expandidos: %.2f\n",
	        numDocsExp,(double)numDocsExp/(double)numDocs);
	fprintf(stderr,"Numero de documentos no arquivo: %d\n",numDocs);
	fclose(fin);
	fclose(fout);
}
