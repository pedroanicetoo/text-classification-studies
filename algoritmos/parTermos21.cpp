#include <vector>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <list>
#include <map>
#include <limits.h>
#include <sys/time.h>

using namespace std;
//struct para os nos das listas do indice invertido
typedef struct no {
    int cat;
    int cont;
} Tno_par;

map < pair<int,int>,list<Tno_par> >  indTerm;

map<int,int> suporteClasse;// armazena o numero de documentos de cada classe
vector<int> linha;
//FUNÇÕES

void insereLista(map< pair<int,int>, list<Tno_par> >::iterator it, int categoria );
void insereIndice ( int categoria);
void ImprimeParTermos(FILE* f, float minPred, float minSup, int numClass, int numTerm);


//Executar ./program colecao.svm 0.7(predominancia) 0.05(suporte)  nome_ArqParesTermos.txt

int main(int argc, char** argv) {

	struct timeval start_time;
	struct timeval end_time;
	gettimeofday(&start_time, NULL); //marcando o tempo de inicio do programa

	//variaveis
	int idDocumento, categoria, termo, i,freq, c, tamMenorClass=INT_MAX, numTerm=-1;
	FILE *f = fopen(argv[1], "rt");
	if (!f) {
    fprintf(stderr,"Arquivo nao pode ser aberto\n");
		exit(1);
	}
	int numClass=-1;
	idDocumento=0;
	//Leitura da arquivo e insercao no indice invertido

	while(fscanf(f, "%d", &categoria)!=EOF){
		if (categoria>numClass) {
			numClass=categoria;
		}
		map<int,int>::iterator it=suporteClasse.find(categoria);
		if (it==suporteClasse.end()) {
			suporteClasse[categoria]=1;
		}
		else
			suporteClasse[categoria]++;
		do{
			fscanf(f,"%d:%d",&termo, &freq);
			c=fgetc(f);
			if (termo> numTerm)
				numTerm=termo;
			linha.push_back(termo);

		}while(c!='\n'&& c!=EOF);
		idDocumento++;
		insereIndice(categoria);
		linha.clear();
	}

	fprintf (stderr,"Num. termos: %d\n", ++numTerm);
	fprintf (stderr,"Num. class: %d\n",++numClass);
	fprintf (stderr,"Gerando indice de par de termos...\n");
	fclose(f);
	//Gravando Pares de termos....
	FILE* fout2=fopen(argv[4], "wt");
	if (!fout2) {
    fprintf(stderr, "Arquivo para gravacao de pares de termos: %s nao pode ser aberto\n", argv[4]);
		exit(1);
	}
  fprintf(stderr, "Imprimindo pares de termos\n");
	ImprimeParTermos(fout2, atof(argv[2]), atof(argv[3]), numClass, numTerm);
	fprintf(stderr, "fechando arquivo\n");

	fclose(fout2);
	gettimeofday(&end_time, NULL);
	fprintf(stderr, "Tempo: %ld min. %ld seg.\n",(end_time.tv_sec-start_time.tv_sec)/60,(end_time.tv_sec-start_time.tv_sec)%60);
	return 0;
}

void insereIndice  (int categoria){
	int i,j, tamLinha=linha.size();
	for (i=0; i<tamLinha; i++) {
    for (j=i+1; j<tamLinha; j++) {
			map < pair<int,int>,list<Tno_par> >::iterator it=indTerm.find(make_pair(linha[i],linha[j]));
			if (it==indTerm.end()) {
				Tno_par p;
				p.cont=1;p.cat=categoria;
				list<Tno_par> l;
				l.push_back(p);
				indTerm.insert(make_pair(make_pair(linha[i],linha[j]),l));
			}
			else{
				//printf("encontrou o par\n");
				insereLista(it, categoria);
			}
		}
	}
}
void insereLista(map< pair<int,int>, list<Tno_par> >::iterator it, int categoria ){
	Tno_par p; p.cat=categoria; p.cont=1;
	list<Tno_par>::iterator itl;
	for (itl=it->second.begin(); itl!=it->second.end() && categoria >itl->cat; itl++) {
    ;
	}
	if(itl==it->second.end()){
		it->second.push_back(p);
		return;
	}
	if (categoria==itl->cat) {
    itl->cont++;
		return;
	}
	if (categoria<itl->cat) {
		if (itl==it->second.begin()) {
			it->second.push_front(p);
		}
		else{
			it->second.insert (itl,p);
		}
	}
}

void ImprimeParTermos(FILE* f, float minPred, float minSup, int numClass,int numTerm){
	int  TotPar;
	vector<int> contClass(numClass,0);
	float predominancia, suporteCl;
	map < pair<int, int>, list<Tno_par> >::iterator it = indTerm.begin();
	for (; it!=indTerm.end();it++) {
    pair<int,int> dupla= it->first;
		list<Tno_par>::iterator itl=it->second.begin();
		TotPar=0;
		for (; itl!=it->second.end();itl++) {
			contClass[itl->cat]=itl->cont;
			TotPar+=itl->cont;
		}
		for (int i=0; i<numClass; i++) {
			predominancia=contClass[i]/(float)TotPar;
			suporteCl=(float)contClass[i]/(float)suporteClasse[i];
			if (predominancia>=minPred && suporteCl>=minSup  && TotPar>=3) {
				//Imprime os componentes do par, seu Id,a categoria onde ele domina,a
				//dominancia, o suporte absoluto dele na classe onde ele domina e o df do par
				fprintf(f,"(%d,%d):%d:%d:%lf:%d:%d\n", dupla.first,dupla.second,numTerm++,i,predominancia,contClass[i],TotPar);
			}
			contClass[i]=0;
		}
	}
}
