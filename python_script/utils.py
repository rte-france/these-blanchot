import random
import sys, os
import networkx as nx
import matplotlib.pyplot as plt
import copy
import numpy as np
import math
import time
import datetime
import shutil

def read_options(args, datas) :

	if '--file' in args :
		read_options_file(args, datas)
	if len(args) >= 3 :
		read_options_line(args[2:], datas)

	if datas.seed == -1 :
		datas.seed = random.randint(0,10000)
		random.seed(datas.seed)
		np.random.seed(datas.seed)

def read_options_file(args, datas) :

	index_file = 0
	for k in range(len(args)) :
		if args[k] in ['--file'] :
			index_file = k+1

	file = open(args[index_file], 'r')
	argFinal = []
	for line in file :
		if len(line) > 0 and line[0] != '#' :
			for elt in line.split() :
				argFinal.append(elt)
	read_options_line(argFinal, datas)


def read_options_line(args, datas) :
	stop = False
	index = 0

	arglist = ['--n', '--m', '--%', '--seed', '--S', '--graphType', '--d1', '--masterSize', '--investInit', '--solve', \
				'--gridCompletion', '--shuffle', '--UBx', '--BigM', '--nCluster', '--flowMin', '--propDemandNodes', \
				'--saveGraph','--log', '--solver', '--folder', '--fullwrite', '--instance_folder',\
				'--costFlowMin', '--costFlowMax', '--costProdMin', '--costProdMax','--n_remove',\
				'--costInvestFlowMin', '--costInvestFlowMax', '--costInvestProdMin', '--costInvestProdMax', '--dMin', '--dMax', '--integerVars']


	while not stop :
		if args[index] in arglist :
			if args[index] == '--n' :
				datas.n = int(args[index+1])
				index += 2

			elif args[index] == '--m' :
				datas.m = int(args[index+1])
				index += 2

			elif args[index] == '--%' :
				if datas.graphType=='GRAPH' :
					datas.m = int( float(args[index+1])*datas.n*(datas.n-1)/200 )
					if datas.m < datas.n-1 :
						pMin = 2/datas.n
						print("Le nombre d'arete doit au moins etre egal a n-1 pour pouvoir \
							creer un graphe connexe. Pour n = %i, cela fait un minimum de %.2f\
							 pourcents." % (datas.n, 100*pMin) )
						exit()
				index += 2

			elif args[index] == '--seed' :
				datas.seed = int(args[index+1])
				random.seed(datas.seed)
				np.random.seed(datas.seed)
				index += 2

			elif args[index] == '--S' :
				datas.S = int(args[index+1])
				index += 2

			elif args[index] == '--graphType' :
				datas.graphType = args[index+1]
				index += 2

			elif args[index] == '--d1' :
				datas.degreeOne = float(args[index+1])
				index += 2

			elif args[index] == '--masterSize' :
				datas.masterSize = float(args[index+1])
				index += 2

			elif args[index] == '--investInit' :
				datas.investInit = float(args[index+1])
				index += 2

			elif args[index] == '--flowMin' :
				datas.flowMin = float(args[index+1])
				index += 2

			elif args[index] == '--saveGraph' :
				datas.saveGraph = bool(int(args[index+1]))
				index += 2
			
			elif args[index] == '--solve' :
				datas.solve = bool(int(args[index+1]))
				index += 2

			elif args[index] == '--shuffle' :
				datas.shuffleGraph = int(args[index+1])
				index += 2

			elif args[index] == '--gridCompletion' :
				datas.gridCompletion = float(args[index+1])
				index += 2

			elif args[index] == '--UBx' :
				datas.UBx = float(args[index+1])
				index += 2

			elif args[index] == '--BigM' :
				datas.BigM = float(args[index+1])
				index += 2

			elif args[index] == '--nCluster' :
				datas.nCluster = int(args[index+1])
				index += 2

			elif args[index] == '--propDemandNodes' :
				datas.propDemandNodes = float(args[index+1])
				index += 2

			elif args[index] == '--log' :
				datas.log = int(args[index+1])
				index += 2

			elif args[index] == '--solver' :
				datas.solver = args[index+1]
				index += 2

			elif args[index] == '--folder' :
				datas.folder_name = args[index+1]
				index += 2

			elif args[index] == '--fullwrite' :
				datas.write_full_prb = bool(int(args[index+1]))
				index += 2

			elif args[index] == '--instance_folder' :
				datas.instance_folder = args[index+1]
				index += 2

			elif args[index] == '--costFlowMin' :
				datas.costMin['flow'] = int(args[index+1])
				index += 2

			elif args[index] == '--costFlowMax' :
				datas.costMax['flow'] = int(args[index+1])
				index += 2

			elif args[index] == '--costProdMin' :
				datas.costMin['prod'] = int(args[index+1])
				index += 2

			elif args[index] == '--costProdMax' :
				datas.costMax['prod'] = int(args[index+1])
				index += 2

			elif args[index] == '--costInvestFlowMin' :
				datas.costMin['Investflow'] = int(args[index+1])
				index += 2

			elif args[index] == '--costInvestFlowMax' :
				datas.costMax['Investflow'] = int(args[index+1])
				index += 2

			elif args[index] == '--costInvestProdMin' :
				datas.costMin['Investprod'] = int(args[index+1])
				index += 2

			elif args[index] == '--costInvestProdMax' :
				datas.costMax['Investprod'] = int(args[index+1])
				index += 2

			elif args[index] == '--dMin' :
				datas.dMin = int(args[index+1])
				index += 2

			elif args[index] == '--dMax' :
				datas.dMax = int(args[index+1])
				index += 2

			elif args[index] == '--n_remove' :
				datas.n_remove = int(args[index+1])
				index += 2

			elif args[index] == '--integerVars' :
				datas.integerVars = int(args[index+1])
				index += 2

			if index >= len(args) :
				stop = True

		else :
			print(args[index])
			print("Argument non reconnu. Lancer la commande [python3 random_graphs.py --help] pour obtenir la liste des arguments.")
			exit()

	if datas.graphType=='GRAPH' :
		if datas.m > datas.n*(datas.n-1)/2 :
			print("Le nombre d'arete doit etre inferieur a n(n-1)/2. Pour n = %i, cela fait un maximum de %i\
				aretes." % (datas.n, int(datas.n*(datas.n-1)/2)) )
			exit(0)


def write_option_file(file_name, options) :

	file = open(file_name, 'w')
	for name in options :
		file.write('%50s%50s\n' % ( name,str(options[name]) ))
	file.close()

# Les variables sont ecrites par ordre alphabetique dans les fichiers .mps par PuLP
def write_structure_file(file_name, datas) :

	datas.masterVars.sort()
	problems = ['master']
	problems += ['SP_%i'%(i+1) for i in range(datas.S)]
	file = open(file_name, 'w')
	for prb in problems :
		index = 0
		for var in datas.masterVars :
			file.write('%-15s%-25s%-15i\n' % (prb,var, index))
			index += 1
	file.close()

def go_to_instance_folder(datas) :
	
	folder_name = ''
	if datas.graphType == 'GRID' :
		folder_name = 'GRID-%i-%i-%i-%.2f'%(datas.n,datas.S,datas.seed, datas.masterSize)
	elif datas.graphType == 'GRAPH' :
		folder_name = 'GRAPH-%i-%i-%i-%i'%(datas.n,datas.m,datas.S,datas.seed)
	else :
		print('WRONG GRAPH TYPE')
		exit()
	folder = datas.instance_folder + folder_name
	os.makedirs(folder, exist_ok=True)
	shutil.copy('options_gen.txt', folder)
	os.chdir(folder)
	

def write_datas(datas) :

	file = open('infos.txt', 'w')
	file.write('%-35s%-10s\n' % ('Type', datas.graphType))
	file.write('%-35s%-10i\n' % ('n', datas.n))
	file.write('%-35s%-10i\n' % ('m', datas.m))
	file.write('%-35s%-10.3f\n' % ('Completion', datas.graphCompletion))
	file.write('%-35s%-10i\n' % ('seed', datas.seed))
	file.write('%-35s%-10i\n' % ('S', datas.S))
	file.write('%-35s%-10.2f\n' % ('masterSize', datas.masterSize))
	file.write('%-35s%-10i\n' % ('NMasterVars', datas.NMasterVars))
	file.write('%-35s%-10i\n' % ('NSlaveVars', datas.NSlaveVars))
	file.write('%-35s%-10i\n' % ('NTot',datas.NMasterVars + datas.S * datas.NSlaveVars))
	file.write('%-35s%-10.2f\n' % ('degre1', datas.degreeOne))
	file.write('%-35s%-10i\n' % ('shuffle', datas.shuffleGraph))
	file.write('%-35s%-10i\n' % ('nCluster', datas.nCluster))
	file.write('%-35s%-10.2f\n' % ('investInit', datas.investInit))
	file.write('%-35s%-10.2f\n' % ('flowMin', datas.flowMin))
	file.write('%-35s%-10.2f\n' % ('propDemandNodes', datas.propDemandNodes))

	file.close()