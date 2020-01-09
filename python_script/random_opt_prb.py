from random_graph 	import *
from utils 			import *
from pulp 			import *
import time
import datetime
import re

class Datas :
	def __init__(self):
		
		self.seed 		= -1

		self.instance_folder = '../instances/' 

		# Generation du graphe
		self.n 			= 0
		self.m 			= 0
		self.S 			= 1
		self.graphCompletion = 0.0
		self.graphType 	= 'GRID'

		# Uniquement pour les graphes type GRID
		# Entre 0 et 1, proportion du nombre de points de la grille occupes par les n points du graphe
		self.gridCompletion = 0.08
		# Entre 0 et 1, proportion du nombre total de noeuds qui seront de degre 1
		self.degreeOne = 0.05
		self.shuffleGraph = 1000
		
		self.UBx 	= 3000
		self.BigM 	= 100 

		self.nCluster = 1
		# Entre 0 et 1, proportion des variables de production et de flot
		self.masterSize 	= 0.25
		self.invest_edges 	= []
		self.masterVars 	= []
		self.NMasterVars 	= 0
		self.NSlaveVars 	= 0
		
		# Entre 0 et 1, proportion de la demande moyenne satisfaite initialement
		self.investInit = 0.5
		# Le min mis sur un flot pour assurer la connexite du graphe initial
		self.flowMin 	= 1

		# Proportion de noeuds de degre 1 qui seront assignes a des demandes,les autres seront des productions
		self.propDemandNodes 	= 0.8
		self.prod_nodes 		= []
		self.demand_nodes 		= []

		# Resolution pendant la generation de la reformulation determinite avec XPRESS
		self.write_full_prb		= False
		self.solve 				= False
		self.saveGraph 			= False
		self.log 				= 0
		self.solver 			= 'XPRESS'
		self.costMin			= {'flow': 1, 	'prod' : 1, 	'Investflow' : 1, 	'Investprod' : 1}
		self.costMax			= {'flow': 10, 	'prod' : 10, 	'Investflow' : 10, 	'Investprod' : 10}

		self.dMin 				= 1
		self.dMax 				= 10

		self.n_remove			= 50
		self.integerVars 		= 0

class Cost :
	def __init__(self, S):
		self.prod = [{} for i in range(S)]
		self.flow = [{} for i in range(S)]
		self.invest_prod = {}
		self.invest_flow = {}

def generate_graph(G, datas, pos) :
	if datas.graphType == 'GRID' :
		grid_generation(G, datas.n, datas.m, pos, ratioN=2, ratioP=3, proportion=datas.gridCompletion, Nbr_deg_1=datas.degreeOne, n_remove=datas.n_remove)
		datas.m = G.number_of_edges()
		datas.graphCompletion = 2 * datas.m / (datas.n * (datas.n-1))
	elif datas.graphType == 'GRAPH' :
		generate_random_graph(G, datas.n, datas.m, datas.shuffleGraph)

def prod_n_demand_nodes(G, datas) :

	# On cree les noeuds de demande et de production
	degre1 = [ v for v in list(G.nodes) if G.degree[v] == 1 ]
	if len(degre1) <= 1 :
		print('Pas assez de noeuds de degre 1')
		return 0
	sep_ind = int(datas.propDemandNodes * len(degre1) )
	for k in range(len(degre1)) :
		if k < sep_ind :
			datas.demand_nodes.append(degre1[k])
		else :
			datas.prod_nodes.append(degre1[k])
	return 1

def invest_edges(G, datas) :
	liste = list(G.edges)
	random.shuffle(liste)
	datas.invest_edges = liste[:int(datas.masterSize*len(list(G.edges)))]

def save_graph(G, datas, pos) :

	if(datas.saveGraph) :
		couleurs = [0 for i in range(G.number_of_nodes())]
		for i in list(G.nodes) :
			if i in datas.prod_nodes :
				couleurs[i] = '#BD5835'
			elif i in datas.demand_nodes :
				couleurs[i] = '#38A747'
			else :
				couleurs[i] = '#19A8C8'

		edgeColor = ['#808080' for i in range(G.number_of_edges())]
		for k in range(len(list(G.edges))) :
			if list(G.edges)[k] in datas.invest_edges :
				edgeColor[k] = '#FA8072'


		if datas.graphType == 'GRID' :
			nx.draw(G, with_labels=False, font_weight='bold', node_color=couleurs, alpha=0.8, edge_color=edgeColor, node_size=50, pos=pos)
		else :
			nx.draw(G, with_labels=False, font_weight='bold', node_color=couleurs, alpha=0.8, edge_color=edgeColor, node_size=50)
		plt.savefig('graph.pdf')
		plt.close()


def get_LB_from_file(LB_prod, LB_flow, G, demand_nodes, prod_nodes, invest_init='invest_init.txt') :
	
	i = 0
	j = 0
	val = 0
	name = ''

	for v in prod_nodes :
		LB_prod[v] = 0
	for k in list(G.edges) :
		LB_flow[k]  =0
	if os.path.exists(invest_init) :
		init_file = open(invest_init, 'r')
		for line in init_file :
			if len(line.split()) > 0 :
				if line.split()[0] not in ['Valeur','Temps'] :
					name = line.split()[0] 
					val = float(line.split()[-1])
					if name.split('_')[2] == 'prod' :
						LB_prod[name.split('_')[3]] = val
					elif name.split('_')[2] == 'flot' :
						i = int(re.split('(\d)', name)[1])
						j = int(re.split('(\d)', name)[3])
						LB_flow[(i,j)] = val

# definition et ecriture au format .MPS du probleme maitre de Benders
def write_master_prb(G, datas, cost, invest_init='invest_init.txt') :

	master_vars = []

	master_prb = LpProblem('master', LpMinimize)

	# Investissement
	if datas.integerVars >= 1 :
		invest_prod = LpVariable.dicts("_invest_prod", datas.prod_nodes, 	lowBound=0, upBound=datas.UBx, cat=LpInteger)
	else :
		invest_prod = LpVariable.dicts("_invest_prod", datas.prod_nodes, 	lowBound=0, upBound=datas.UBx)
	if datas.integerVars == 2 :
		invest_flow = LpVariable.dicts("_invest_flot", datas.invest_edges, 	lowBound=0, upBound=datas.UBx, cat=LpInteger)
	else :
		invest_flow = LpVariable.dicts("_invest_flot", datas.invest_edges, 	lowBound=0, upBound=datas.UBx)

	master_prb += lpSum([cost.invest_prod[v]*invest_prod[v] 		for v in datas.prod_nodes]) \
				+ lpSum([cost.invest_flow[(i,j)]*invest_flow[(i,j)] for (i,j) in datas.invest_edges])

	master_prb.writeMPS("master.mps")

	datas.masterVars 	= [i.name for i in master_prb.variables() ]
	datas.NMasterVars = len(datas.masterVars)

def write_subproblems(G, datas, demandes, cost) : 

	sub_prb = LpProblem('SP_1', LpMinimize)

	# Variables de prod
	invest_prod = LpVariable.dicts("_invest_prod", 	datas.prod_nodes, lowBound=0)
	vars_prod 	= LpVariable.dicts("prod", 		 	datas.prod_nodes, lowBound=0)
	penal_prod 	= LpVariable.dicts("penal_prod", 	datas.prod_nodes, lowBound=0)
	# Contraintes de prod
	for v in datas.prod_nodes :
		sub_prb += vars_prod[v] <= invest_prod[v] + penal_prod[v] , "ProdMax_%i"%v

	# Variables et contraintes de flot
	links = [(i,j) for (i,j) in list(G.edges)] + [(j,i) for (i,j) in list(G.edges)]

	voisins_entrant = {}
	for v in list(G.nodes) :
		voisins_entrant[v] = [(i,j) for (i,j) in links if j==v]

	constant_edges = [(i,j) for (i,j) in list(G.edges) if (i,j) not in datas.invest_edges]
	other_nodes = [v for v in list(G.nodes) if v not in datas.prod_nodes and v not in datas.demand_nodes]

	vars_flow 	= LpVariable.dicts("flot", 			links, 				lowBound=0)
	invest_flow = LpVariable.dicts("_invest_flot", 	datas.invest_edges, lowBound=0)
	penal_flow 	= LpVariable.dicts("penal_flot", 	links, 				lowBound=0)	
	for (i,j) in datas.invest_edges :
		sub_prb += vars_flow[(j,i)] <= invest_flow[(i,j)] + datas.flowMin + penal_flow[(j,i)] , "FlotMax_(%i,%i)"%(j,i)
		sub_prb += vars_flow[(i,j)] <= invest_flow[(i,j)] + datas.flowMin + penal_flow[(i,j)] , "FlotMax_(%i,%i)"%(i,j)
	for (i,j) in constant_edges :
		sub_prb += vars_flow[(j,i)] <= datas.flowMin + penal_flow[(j,i)] , "FlotMax_(%i,%i)"%(j,i)
		sub_prb += vars_flow[(i,j)] <= datas.flowMin + penal_flow[(i,j)] , "FlotMax_(%i,%i)"%(i,j)

	#Contraintes de flot
	for v in datas.prod_nodes :
		sub_prb += lpSum(vars_flow[i,j] for (i,j) in voisins_entrant[v]) == lpSum(vars_flow[i,j] for (j,i) in voisins_entrant[v]) - vars_prod[v]		, "flot_%i"%v
	for v in datas.demand_nodes :
		sub_prb += lpSum(vars_flow[i,j] for (i,j) in voisins_entrant[v]) == lpSum(vars_flow[i,j] for (j,i) in voisins_entrant[v]) + demandes[0][v] 	, "flot_%i"%v
	for v in other_nodes :
		sub_prb += lpSum(vars_flow[i,j] for (i,j) in voisins_entrant[v]) == lpSum(vars_flow[i,j] for (j,i) in voisins_entrant[v]) 					, "flot_%i"%v
	
	# Objectif
	sub_prb += 	lpSum(cost.prod[0][v]*vars_prod[v] for v in datas.prod_nodes) + lpSum(cost.flow[0][(i,j)]*vars_flow[(i,j)] for (i,j) in links) \
			+ 	lpSum(datas.BigM*penal_prod[v] 	for v in datas.prod_nodes) + lpSum(datas.BigM*penal_flow[(i,j)] for (i,j) in links)

	sub_prb.writeMPS("SP_1.mps")
	sub_prb.writeLP("SP_1.lp")


	for s in range(1,datas.S) :
		sub_prb.name = 'SP_%i'%(s+1)

		for v in datas.demand_nodes :
			sub_prb.constraints["flot_%i"%v] = lpSum(vars_flow[i,j] for (i,j) in voisins_entrant[v]) == lpSum(vars_flow[i,j] for (j,i) in voisins_entrant[v]) + demandes[s][v]

		sub_prb.objective = lpSum(cost.prod[s][v]*vars_prod[v] for v in datas.prod_nodes) + lpSum(cost.flow[s][(i,j)]*vars_flow[(i,j)] for (i,j) in links) \
			+ 	lpSum(datas.BigM*penal_prod[v] 	for v in datas.prod_nodes) + lpSum(datas.BigM*penal_flow[(i,j)] for (i,j) in links)

		sub_prb.writeMPS("SP_%i.mps"%(s+1))

	datas.NSlaveVars = len(sub_prb.variables())

def moyenne_on_scenarios(list_moy, list_init, S, index_list, alpha) :
	for k in index_list :
		list_moy[0][k] = 0
		for s in range(S) :
			list_moy[0][k] += alpha*list_init[s][k]/S

def find_init_invest(G, datas, cost, demandes) :

	links = [(i,j) for (i,j) in list(G.edges)] + [(j,i) for (i,j) in list(G.edges)]
	d_moy = [{}]

	cost_moy = Cost(1)
	moyenne_on_scenarios(d_moy, 		demandes, 	datas.S, datas.demand_nodes, 	datas.investInit)
	moyenne_on_scenarios(cost_moy.prod, cost.prod, 	datas.S, datas.prod_nodes, 		datas.investInit)
	moyenne_on_scenarios(cost_moy.flow,	cost.flow, 	datas.S, links, 				datas.investInit)
	cost_moy.invest_prod = cost.invest_prod
	cost_moy.invest_flow = cost.invest_flow

	write_n_solve_deterministic_global_problem(	G, datas, 1, cost_moy, d_moy, write_sol=True, sol_file='invest_init.txt', log=0)


def write_n_solve_deterministic_global_problem(	G, datas, S, cost, demandes, write_sol=True, sol_file='solution.txt', log=0) :

	start = datetime.datetime.now()
	prb = LpProblem('ReformulationDeterministe', LpMinimize)

	obj = 0

	# Investissement
	# Investissement
	if datas.integerVars >= 1 :
		invest_prod = LpVariable.dicts("_invest_prod", datas.prod_nodes, 	lowBound=0, upBound=datas.UBx, cat=LpInteger)
	else :
		invest_prod = LpVariable.dicts("_invest_prod", datas.prod_nodes, 	lowBound=0, upBound=datas.UBx)
	if datas.integerVars == 2 :
		invest_flow = LpVariable.dicts("_invest_flot", datas.invest_edges, 	lowBound=0, upBound=datas.UBx, cat=LpInteger)
	else :
		invest_flow = LpVariable.dicts("_invest_flot", datas.invest_edges, 	lowBound=0, upBound=datas.UBx)

	links = [(i,j) for (i,j) in list(G.edges)] + [(j,i) for (i,j) in list(G.edges)]

	voisins_entrant = {}
	for v in list(G.nodes) :
		voisins_entrant[v] = [(i,j) for (i,j) in links if j==v]

	constant_edges = [(i,j) for (i,j) in list(G.edges) if (i,j) not in datas.invest_edges]
	other_nodes = [v for v in list(G.nodes) if v not in datas.prod_nodes and v not in datas.demand_nodes]

	# on numerote les sous problemes a partir de 1
	penal_prod 	= {}
	vars_prod 	= {}
	vars_flow 	= {}
	penal_flow 	= {}

	obj_s = []

	for s in range(S) :

		penal_prod[s]	= LpVariable.dicts("penal_prod_%i"%(s+1), 	datas.prod_nodes, 	lowBound=0)
		vars_prod[s] 	= LpVariable.dicts("prod_%i"%(s+1), 		datas.prod_nodes, 	lowBound=0)
		vars_flow[s] 	= LpVariable.dicts("flot_%i"%(s+1), 		links, 				lowBound=0)
		penal_flow[s] 	= LpVariable.dicts("penal_flot_%i"%(s+1), 	links, 				lowBound=0)
		

		for v in datas.prod_nodes :
			prb += vars_prod[s][v] <= invest_prod[v] + penal_prod[s][v] , "ProdMax_S%i_%i"%(s+1,v)

		for (i,j) in datas.invest_edges :
			prb += vars_flow[s][(i,j)] <= invest_flow[(i,j)] + datas.flowMin + penal_flow[s][(i,j)] , "FlotMax_S%i_(%i,%i)"%((s+1),i,j)
			prb += vars_flow[s][(j,i)] <= invest_flow[(i,j)] + datas.flowMin + penal_flow[s][(j,i)] , "FlotMax_S%i_(%i,%i)"%((s+1),j,i)
		for (i,j) in constant_edges :
			prb += vars_flow[s][(i,j)] <= datas.flowMin + penal_flow[s][(i,j)] , "FlotMax_S%i_(%i,%i)"%((s+1),i,j)
			prb += vars_flow[s][(j,i)] <= datas.flowMin + penal_flow[s][(j,i)] , "FlotMax_S%i_(%i,%i)"%((s+1),j,i)

		for v in datas.prod_nodes :
			prb += lpSum(vars_flow[s][i,j] for (i,j) in voisins_entrant[v]) - lpSum(vars_flow[s][i,j] for (j,i) in voisins_entrant[v]) + vars_prod[s][v] == 0	, "flot_S%i_%i"%((s+1),v)
		for v in datas.demand_nodes :
			prb += lpSum(vars_flow[s][i,j] for (i,j) in voisins_entrant[v]) - lpSum(vars_flow[s][i,j] for (j,i) in voisins_entrant[v]) - demandes[s][v]  == 0 , "flot_S%i_%i"%((s+1),v)
		for v in other_nodes :
			prb += lpSum(vars_flow[s][i,j] for (i,j) in voisins_entrant[v]) - lpSum(vars_flow[s][i,j] for (j,i) in voisins_entrant[v]) == 0  					, "flot_S%i_%i"%((s+1),v)

		obj_s.append( (1/S) * ( lpSum(cost.prod[s][v]*vars_prod[s][v] for v in datas.prod_nodes) \
					+ 	lpSum(cost.flow[s][(i,j)]*vars_flow[s][(i,j)] for (i,j) in links) \
					+ 	lpSum(datas.BigM*penal_prod[s][v] for v in datas.prod_nodes) \
					+	lpSum(datas.BigM*penal_flow[s][(i,j)] for (i,j) in links) ) )


	# Fonction objectif
	obj_s.append( lpSum(cost.invest_prod[v]*invest_prod[v] 			for v in datas.prod_nodes) \
		+ 	lpSum(cost.invest_flow[(i,j)]*invest_flow[(i,j)] 	for (i,j) in datas.invest_edges) )

	obj = lpSum( elt for elt in obj_s )
	prb += obj

	prb.writeMPS("ReformulationDeterministe.mps")

	if(datas.solve) :

		stop1 = datetime.datetime.now()
		
		if(datas.solver == 'XPRESS') :
			prb.solve(XPRESS(msg=log))
		else :
			prb.solve()

		stop2 = datetime.datetime.now()


		# Ecriture du fichier solution.txt pour retenir la (une) solution optimale
		if write_sol :
			solution = open(sol_file, 'w')
			solution.write('Valeur = %.2f\n'   % value(prb.objective))
			solution.write('Temps  = %.2f\n\n' % (stop2 - stop1).total_seconds())
			for v in datas.prod_nodes :
				if invest_prod[v].varValue != 0 :
					solution.write('%-35s%-.2f\n' % (invest_prod[v].name,invest_prod[v].varValue))
			for (i,j) in datas.invest_edges :
				if invest_flow[(i,j)].varValue != 0 :
					solution.write('%-35s%-.2f\n' % (invest_flow[(i,j)].name,invest_flow[(i,j)].varValue))

			solution.close()

		return (stop2 - stop1).total_seconds()

	return 0

def operational_cost_generator(G, datas, cost) :
	links = [(i,j) for (i,j) in list(G.edges)] + [(j,i) for (i,j) in list(G.edges)]

	for s in range(datas.S) :
		valeurs = np.random.randint(datas.costMin['flow'], datas.costMax['flow'], len(links))
		cost.flow[s] = dict( zip(links,valeurs) )
		for v in datas.prod_nodes :
			cost.prod[s][v] = random.randint(datas.costMin['prod'], datas.costMax['prod'])

def invest_cost_generator(G, datas, cost) :
	valeurs = np.random.randint(datas.costMin['Investflow'], datas.costMax['Investflow'], len(datas.prod_nodes))
	cost.invest_prod=dict( zip(datas.prod_nodes, valeurs) )

	valeurs = np.random.randint(datas.costMin['Investprod'], datas.costMax['Investprod'], len(datas.invest_edges))
	cost.invest_flow = dict( zip(datas.invest_edges, valeurs) )

def demand_generator(G, datas, demandes) :
	for s in range(datas.S) :
		local_sum = 0
		valeurs = np.random.randint(datas.dMin, datas.dMax, len(datas.demand_nodes))
		demandes[s] = dict( zip(datas.demand_nodes, valeurs) )
	return 0



def one_problem_fulle_gen(datas) : 

	start = datetime.datetime.now()	

	# On se place dans le dossier associe a l instance
	owd = os.getcwd()
	go_to_instance_folder(datas)
	

	start_graph = datetime.datetime.now()
	G = nx.Graph()
	pos = {}
	generate_graph(G, datas, pos)
	stop_graph = datetime.datetime.now()
	print('Temps graphe              : ', stop_graph- start_graph)

	start_coef = datetime.datetime.now()
	prod_n_demand_nodes(G, datas)
	invest_edges(G, datas)
	save_graph(G, datas, pos)
	nx.write_edgelist(G, path="graph.txt", delimiter=";")

	# Generation des demandes et des couts
	cost = Cost(datas.S)
	demandes = {}
	operational_cost_generator(G, datas, cost)
	invest_cost_generator(G, datas, cost)
	demand_generator(G, datas, demandes)
	stop_coef = datetime.datetime.now()
	print('Temps coef                : ', stop_coef - start_coef)

	#Graphe initial
	start_invest_init = datetime.datetime.now()
	find_init_invest(G, datas, cost, demandes)
	stop_invest_init = datetime.datetime.now()
	print('Temps invest init         : ', stop_invest_init - start_invest_init)

	# Ecriture des fichiers .MPS
	start_write = datetime.datetime.now()
	write_master_prb(G, datas, cost)
	write_subproblems(G, datas, demandes, cost)
	stop_write = datetime.datetime.now()
	print('Temps ecriture MPS        : ', stop_write - start_write)

	# Ecriture de la reformulation deterministe du probleme
	# Resolution et ecriture de la reformulation deterministe
	time_resolution = 0.0
	if datas.write_full_prb :
		start_solve = datetime.datetime.now()
		time_resolution = write_n_solve_deterministic_global_problem(G, datas, datas.S, cost, demandes, log=datas.log)
		stop_solve = datetime.datetime.now()
		print('Temps solve               : ', stop_solve - start_solve)


	options = {}
	options_creator(options)
	write_option_file('options.txt', options)
	
	write_structure_file('structure.txt', datas)

	stop = datetime.datetime.now()
	print('Temps total               : ', stop - start)
	

	write_datas(datas)

	os.chdir(owd)

	return time_resolution, datas.NMasterVars, datas.NSlaveVars 


def options_creator(options) :
	options["LOG_LEVEL"] 				= 3 
	options["MAX_ITERATIONS"] 			= -1
	options["GAP"] 						= 1e-4
	options["AGGREGATION"] 				= 0
	options["TRACE"] 					= 0
	options["DELETE_CUT"] 				= 0
	options["LOG_OUTPUT"] 				= "COMMAND"
	options["SLAVE_WEIGHT"] 			= "UNIFORM"
	options["MASTER_NAME"] 				= "master"
	options["SLAVE_NUMBER"] 			= -1
	options["STRUCTURE_FILE"] 			= "structure.txt"
	options["INPUTROOT"] 				= "."
	options["BASIS"] 					= 0
	options["THRESHOLD_AGGREGATION"] 	= 0
	options["RAND_AGGREGATION"] 		= 1
	options["RAND_SEED"] 				= 0
	options["ETA_IN_OUT"] 				= 0.5
	options["TRICK_FISCHETTI"] 			= 0
	options["DYNAMIC_STABILIZATION"] 	= 1
	options["SOLVER"] 					= "CPLEX"
	options["ALGORITHM"] 				= "INOUT"
	options["SAMPLING_STRATEGY"] 		= "ORDERED"


#####################################################################################################################
#####################################################################################################################
#################################                        MAIN  						#################################
#####################################################################################################################
#####################################################################################################################
if __name__ == '__main__' :

	time_resolution = 0.0
	nMasterVars 	= 0
	nSlaveVars 		= 0

	datas = Datas()
	read_options(sys.argv[1:], datas)
	time_resolution, nMasterVars, nSlaveVars = one_problem_fulle_gen(datas)
	exit()

	for n in range(50,150,50) :
		for S in range(200,500,200) :
			for seed in range(2) :
				for masterSize in np.arange(0.1,0.4,0.2) :
					datas = Datas()
					read_options(sys.argv[1:], datas)
					datas.n = n
					datas.S = S
					datas.seed = seed
					datas.masterSize = masterSize

					time_resolution, nMasterVars, nSlaveVars = one_problem_fulle_gen(datas)
					print()
	