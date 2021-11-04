from sklearn import metrics
import pandas as pd
import os
import subprocess
import sys
import re

print('colecao: '+str(sys.argv[1]))
print('folder: '+str(sys.argv[2]))
print('Dom_min: 0.6')
print('Dom_max: 0.9')
print('Sup_min: 0.005')
print('Sup_max: 0.05')
# python calibrate_params.py colecao folder razão de desbalanceamento > log-acm.txt
#sys.argv[1] = colecao
#sys.argv[2] = folder
# sys.argv[3] = razão de desbalanceamento
target_fold = '../colecoes/'+str(sys.argv[2])+'/'+str(sys.argv[1])+'/'
target_fold_collecion_train = '../colecoes/' + \
    str(sys.argv[2])+'/'+str(sys.argv[1])+'/treino'+str(sys.argv[2])+'.txt '
target_fold_collecion_test = '../colecoes/' + \
    str(sys.argv[2])+'/'+str(sys.argv[1])+'/teste'+str(sys.argv[2])+'.txt '
target_fold_pair = '../colecoes/' + \
    str(sys.argv[2])+'/PairTerms/'+str(sys.argv[1]) + \
    '/terms'+str(sys.argv[2])+'.txt '

for i in str(sys.argv[2]):
    for j in [0.4, 0.5, 0.6, 0.7, 0.8, 0.9]:
        for k in [0.005, 0.01, 0.015, 0.02, 0.025, 0.03, 0.035, 0.04, 0.045, 0.05]:
            avg_precision = 0.0
            avg_recall = 0.0
            avg_acc = 0.0
            avg_macro = 0.0
            print('		 ======================')
            print('		| Dom: '+str(j)+' e '+'Sup: '+str(k)+' |')
            print('		 ======================')
            alg_expand = './expan21 '
            arq_train_in = target_fold_collecion_train
            arq_test_in = target_fold_collecion_test
            arq_pair_in = target_fold_pair
            train_file_expand_out = target_fold + \
                '/train'+str(j)+'-'+str(k)+'.txt '
            test_file_expand_out = target_fold + \
                '/test'+str(j)+'-'+str(k)+'.txt '
            # Expand file
            a = subprocess.check_output(alg_expand+arq_train_in+arq_pair_in+arq_test_in+train_file_expand_out +
                                        test_file_expand_out+str(j)+' '+str(k)+' '+str(sys.argv[3]), shell=True)
            # Doing Cross-validation
            alg_cross = './cross_validation '
            name_folder = target_fold+'/sub-folder'+str(j)+'-'+str(k)
            alg_cross+train_file_expand_out+name_folder+' 3'
            b = subprocess.check_output(
                alg_cross+train_file_expand_out+name_folder+' 3', shell=True)
            # Train Subfolders
            for m in range(3):
                alg_train = './train -C '
                train_file = name_folder+'/treino'+str(m)+'.txt '
                test_file = name_folder+'/teste'+str(m)+'.txt '
                train = subprocess.check_output(
                    alg_train+train_file, shell=True)
                # normalization
                norm = re.sub('b', '', str(train))
                './sub-folder-'+str(m)+'-norm: '+norm.split()[-5]
                alg_train_norm = './train -c '+norm.split()[-5]+' '
                train_norm = subprocess.check_output(
                    alg_train_norm+train_file, shell=True)
                # get_accuracy
                alg_acc = './predict '
                acc = subprocess.check_output(
                    alg_acc+'./'+test_file+'./treino'+str(m)+'.txt.model '+name_folder+'/x.txt', shell=True)
                # get_metrics
                output_metrics = subprocess.check_output(
                    './geraMetricas '+name_folder+'/x.txt '+test_file, shell=True)
                print(str(output_metrics, "utf-8"))
                a = str(output_metrics, "utf-8")
                avg_precision += float(a.split("Macro Precision")
                                       [1].split(':')[1].split('|')[0].strip())
                avg_recall += float(a.split("Macro Recall")
                                    [1].split(':')[1].split('|')[0].strip())
                avg_acc += float(a.split("Micro F1 (Accuracy)")
                                 [1].split(':')[1].split('|')[0].strip())
                avg_macro += float(a.split("Macro F1")
                                   [1].split(':')[1].split('|')[0].strip())
            print("==============================AVG's========================== ")
            print("| Macro Precision: "+str(avg_precision/3)+"      	           ")
            print("| Macro Recall: "+str(avg_recall/3) +
                  "      	                 ")
            print("| Micro F1 (Accuracy): " +
                  str(avg_acc/3)+"      	             ")
            print("| Macro F1: "+str(avg_macro/3) +
                  "      	                       ")
            print("=============================================================")
