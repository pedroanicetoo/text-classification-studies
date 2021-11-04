# textclassification
Classificação de Texto - TCC - PEDRO

Requisitos 
SVM 
./train -C <treino>
./train -c <coef_c> <treino>
./predict <teste> <treino.model> <saida>
download:
https://www.csie.ntu.edu.tw/~cjlin/libsvm/

======================================================================
 
Programas importantes para estratégia de geração de atributos

1º Programa: "parTermos21"
Gera Pares de Termos que poderão ter inclusos na coleção original e computa algumas medidas importantes para o desenpenho do mesmo.
Como por exemplo o tempo de execução do processamento para gerar os seguintes contextos:
 
2º Programa: "expan21"
Expande a coleção de treino e teste com os atributos fornecidos na saída do parTermos21
 
3º Programa:  calibrate_params 
Definição estática (dentro do programa) do limiares de dominância e suporte escolhidos para fazer um grid search entre eles, gerando saídas com as estatísticas 
pertencentes a cada par de <dominância, suporte> do limiar.

Outros programas utilizados :

4º Programa: cross_validation
Faz a validação cruzada de uma coleção gerandos as coleões treino/teste
 
5º Programa: geraMetricas
Gera algumas meidas macro através da entrada do documento de teste e da saída do classificador

Obs: programas com (depreciated) foram tentativas de implementação que resolvemos deixar para futuras consultas.
 
======================================================================

COMANDO PARA CONTAGEM DE CATEGORIAS DO DOCUMENTO :

gawk '{v[$1]++}END{for(k in v)print k,v[k];}' treino0.txt


COMANDO PARA CONTAGEM DE MÉDIA DE TERMOS EM CADA DOCUMENTO :

gawk '{total += NF-1} END {print total / NR}' total0.txt

 









