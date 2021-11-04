#include <vector>
#include <algorithm>
#include <stdio.h>

std::vector<int> labelsAtrib, labelsTeste;
typedef struct
{
	double precision;
	double recall;
	double F1;
} TmetricasPorClasse;
std::vector<TmetricasPorClasse> vecMetricasPorClasse; //armazena os totais de acertos e de erros do classificador

// por classe
int main(int argc, char **argv)
{
	/* Le o arquivo de saida do classificador liblinear e le o arquivo de teste para obter os rotulos
 * dos documentos de teste com os rotulos gerados pelo classificador e gerar métricas de efetividade
 * 
 */
	/*if (argc!=2){
		fprintf (stderr, "Execucao correta do progrma: ./geraMetricas arqSaidaSVMliblinear, arquivo de teste\n");
	}*/
	FILE *f1 = fopen(argv[1], "rt");
	if (!f1)
	{
		fprintf(stderr, "Arquivos %s nao pode ser aberto. Encerrando o programa.\n", argv[1]);
		return (0);
	}
	FILE *f2 = fopen(argv[2], "rt");
	if (!f1)
	{
		fprintf(stderr, "Arquivos %s nao pode ser aberto. Encerrando o programa.\n", argv[2]);
		return (0);
	}
	int l1, l2;
	char c;
	while (fscanf(f1, "%d", &l1) != EOF)
	{
		labelsAtrib.push_back(l1);
	}
	fclose(f1);
	int numRotulos = -1;
	while (fscanf(f2, "%d", &l2) != EOF)
	{
		labelsTeste.push_back(l2);
		if (l2 > numRotulos)
			numRotulos = l2;
		do
			c = fgetc(f2);
		while (c != '\n');
	}
	numRotulos++;
	fclose(f2);
	if (labelsAtrib.size() != labelsTeste.size())
	{
		fprintf(stderr, "Erro o numero de elementos rotulados pelo Liblinear é diferente do numero de elmentos no teste\n");
		return (0);
	}
	// printf("Tamanho do vetor lido %lu: \n", labelsTeste.size());

	/* Definir e iniciar com zeros a matriz de confusao entre as classes*/
	unsigned long int matConfusao[numRotulos][numRotulos];
	for (int k = 0; k < numRotulos; k++)
		for (int l = 0; l < numRotulos; l++)
			matConfusao[k][l] = 0;

	for (unsigned int i = 0; i < labelsTeste.size(); i++)
		matConfusao[labelsAtrib[i]][labelsTeste[i]]++;

	//Imprimindo a matriz de confusao
	int k, l;
	for (k = 0; k < numRotulos; k++)
	 	printf("\t%d:", k);
	 printf("\n");
	 for (int k = 0; k < numRotulos; k++)
	 {
	 	printf("%d:\t", k);
	 	for (l = 0; l < numRotulos; l++)
	 		printf("%lu \t", matConfusao[k][l]);
	 	printf("\n");
	 }
	vecMetricasPorClasse.resize(numRotulos);
	//Computando precisao para cada classe. Para cada classe k, somamos os valores na linha k
	//da matriz de confusao, para depois dividir o valor de matConfusao[k][k] por essa soma
	for (k = 0; k < numRotulos; k++)
	{
		vecMetricasPorClasse[k].precision = 0.0;
		for (l = 0; l < numRotulos; l++)
			vecMetricasPorClasse[k].precision += matConfusao[k][l];
		if (vecMetricasPorClasse[k].precision == 0.0)
		{
			vecMetricasPorClasse[k].precision = 0.0;
		}
		else
		{
			vecMetricasPorClasse[k].precision = (double)matConfusao[k][k] / vecMetricasPorClasse[k].precision;
		}
	}
	for (k = 0; k < numRotulos; k++)
	{
		vecMetricasPorClasse[k].recall = 0.0;
		for (l = 0; l < numRotulos; l++)
			vecMetricasPorClasse[k].recall += matConfusao[l][k];
		if (vecMetricasPorClasse[k].recall == 0.0)
		{
			vecMetricasPorClasse[k].recall = 0.0;
		}
		else
		{
			vecMetricasPorClasse[k].recall = (double)matConfusao[k][k] / vecMetricasPorClasse[k].recall;
		}
	}
	double macroF1 = 0.0, totAcertos = 0.0, totErros = 0.0;
	double macroPrecision = 0.0, macroRecall = 0.0;
	for (k = 0; k < numRotulos; k++)
	{
		if (vecMetricasPorClasse[k].precision + vecMetricasPorClasse[k].recall == 0.0)
		{
			vecMetricasPorClasse[k].F1 = 0.0;
		}
		else
		{
			vecMetricasPorClasse[k].F1 = (2.0 * vecMetricasPorClasse[k].precision * vecMetricasPorClasse[k].recall) /
										 (vecMetricasPorClasse[k].precision + vecMetricasPorClasse[k].recall);
		}
		macroF1 += vecMetricasPorClasse[k].F1;
		macroPrecision += vecMetricasPorClasse[k].precision;
		macroRecall += vecMetricasPorClasse[k].recall;
		// printf("Classe %d:\t", k);
		// printf("Precision: %.4lf\t", vecMetricasPorClasse[k].precision);
		// printf("Recall: %.4lf\t", vecMetricasPorClasse[k].recall);
		// printf("F1: %.4lf\t", vecMetricasPorClasse[k].F1);
		// printf("\n");
	}
	for (k = 0; k < numRotulos; k++)
		for (l = 0; l < numRotulos; l++)
		{
			if (l == k)
				totAcertos += matConfusao[k][k];
			else
				totErros += matConfusao[k][l];
		}
	printf("============================================================\n");
	printf("| Macro Precision: %.4lf      |    Macro Recall:  %.4lf   |\n", macroPrecision / (double)numRotulos, macroRecall / (double)numRotulos);
	printf("| Micro F1 (Accuracy): %.4lf  |    Macro F1: %.4lf        |\n", totAcertos / (totAcertos + totErros), macroF1 / (double)numRotulos);
	printf("============================================================");
	return (0);
}
