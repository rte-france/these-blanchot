import random
import sys, os
import networkx as nx
import matplotlib.pyplot as plt
import copy
import numpy as np
import math
import time
import datetime


def liste_degres(n, m) :
	liste = []
	somme = 0
	degre = 0
	sommets_restant = n
	demiArretes_restantes = 2*m
	deg_min = 0
	deg_max = 0
	ctr_cardinal = 0

	min_degre_tires = 0
	Sn_1 = 0
	Pn_1 = 0

	for i in range(n-1) :
		sommets_restant -= 1

		Pn_1 = min_degre_tires - Sn_1
		ctr_cardinal = demiArretes_restantes - (n-1)*Pn_1 - (n-2)*(sommets_restant - Pn_1)

		#deg_min = max(1, demiArretes_restantes - sommets_restant*(n-1), ctr_cardinal, Sn_1)
		deg_min = max(1, demiArretes_restantes - sommets_restant*(n-1))
		deg_max = min(demiArretes_restantes - sommets_restant, n-1)

		degre = random.randint(deg_min, deg_max)
		liste.append(degre)
		somme += degre
		min_degre_tires = min(min_degre_tires, degre)
		if degre == (n-1) :
			Sn_1 += 1

		demiArretes_restantes -= degre

	liste.append(demiArretes_restantes)
	return liste
	

def havel_hakimi(G, n, m) :

	degres = graphic_generation_degrees(n, m)
	degres.sort(reverse=True)

	# Liste de tuples (sommets, degre du sommet)
	sommets = [(i,degres[i]) for i in range(n)]
	stop = False
	cur_deg = sommets[0][1]
	cur_som = sommets[0][0]

	while not stop :
		# On trie les sommets dans l'ordre decroissant des degres residuels
		sommets.sort(key=lambda t: t[1], reverse=True)
		cur_deg = sommets[0][1]
		cur_som = sommets[0][0]

		liste_voisins = [(v,d) for (v,d) in sommets[1:] if d>0]

		if len(liste_voisins) >= cur_deg :
			# On dispose d'assez de voisins pour connecter le sommet
			G.add_edges_from( (cur_som,t) for (t,d) in liste_voisins[:cur_deg] )
			for k in range(cur_deg) :
				# On reduit le degre de 1
				sommets[k+1] = (sommets[k+1][0],sommets[k+1][1]-1)
			sommets[0] = (cur_som,0)

			#Critere d'arret 
			if sum( [d for (v,d) in sommets] ) == 0 :
				stop = True
		else :
			return 0

	return 1


def graphic_generation_degrees(n, m) :

	degre = 0
	liste_degres = [1,1]
	liste_sommets = [0,1]
	deg_min = 0
	deg_max = 0
	demiArretes_restantes = 2*m
	current_sum = 0

	if n >= 3 :
		for i in range(3, n+1) :

			deg_min = max(1, (demiArretes_restantes - current_sum - 2*(n-i)*(n-1))/2 )
			deg_max = min(i-1, demiArretes_restantes/2 - n + i )
			
			degre = random.randint(deg_min, deg_max)

			# On melange conjointement les sommets et leur degres
			degres_sommets = list(zip(liste_sommets, liste_degres))
			random.shuffle(degres_sommets)
			liste_sommets, liste_degres = zip(*degres_sommets)

			for k in range(degre) :
				liste_degres[k] += 1
			#G.add_edges_from( (i,j) for (j) in liste_sommets[:degre] )
			# Puis on ajoute le nouveau sommet du degre tire
			liste_sommets.append(i)
			liste_degres.append(degre)

			# On met tout le monde a jour
			current_sum += 2*degre
			demiArretes_restantes -= 2*degre

	return liste_degres


def graphic_generation(G, n, m, maximum=-1) :

	if maximum == -1 :
		maximum = n-1
	degre = 0
	liste_degres = [1,1]
	liste_sommets = [0,1]
	G.add_edge(0,1) 
	deg_min = 0
	deg_max = 0
	demiArretes_restantes = 2*(m-1)
	current_sum = 0

	if n >= 3 :
		for i in range(3, n+1) :

			deg_min = max(1, (demiArretes_restantes - current_sum - 2*(n-i)*(n-1))/2 )
			deg_max = min(i-1, demiArretes_restantes/2 - n + i, maximum)
			
			degre = random.randint(deg_min, deg_max)

			# On melange conjointement les sommets et leur degres
			degres_sommets = list(zip(liste_sommets, liste_degres))
			random.shuffle(degres_sommets)
			liste_sommets, liste_degres = zip(*degres_sommets)
			liste_sommets 	= list(liste_sommets)
			liste_degres 	= list(liste_degres)

			for k in range(degre) :
				liste_degres[k] += 1
			G.add_edges_from( (i-1,j) for (j) in liste_sommets[:degre] )
			# Puis on ajoute le nouveau sommet du degre tire
			liste_sommets.append(i-1)
			liste_degres.append(degre)

			# On met tout le monde a jour
			current_sum += 2*degre
			demiArretes_restantes -= 2*degre
	
	return liste_degres


def read_options_graph(args) :
	stop = False
	index = 0
	n = 0
	m = 0

	arglist = ['--n', '--m', '--%']

	while not stop :
		if args[index] in arglist :
			if args[index] == '--n' :
				n = int(args[index+1])
				index += 2
			elif args[index] == '--m' :
				m = int(args[index+1])
				index += 2
			elif args[index] == '--%' :
				m = int( float(args[index+1])*n*(n-1)/200 ) 
				if m < n-1 :
					pMin = 2/n
					print("Le nombre d'arete doit au moins etre egal a n-1 pour pouvoir creer un graphe connexe. Pour n = %i, cela fait un minimum de %.2f pourcents." % (n, 100*pMin) )
					exit()
				index += 2

			if index >= len(args) :
				stop = True

		else :
			print("Argument non reconnu. Lancer la commande [python3 random_graphs.py --help] pour obtenir la liste des arguments.")
			exit()

	return n, m

def is_connex(G, n) :

	sommets_vu 		= set()
	voisins_connus 	= []
	
	# Initialisation
	sommet_courant = 0
	sommets_vu.add(0)
	voisins_connus = list( G.adj[0].keys() )
	while len(voisins_connus) != 0 :

		if voisins_connus[0] not in sommets_vu :
			sommets_vu.add(voisins_connus[0])
			voisins_connus += list( G.adj[voisins_connus[0]].keys() )
		voisins_connus.remove(voisins_connus[0])

	if len(sommets_vu) == n :
		return 1
	else :
		return 0


#Try exchange the arcs (i,j) and (k,l) to (i,l) and (k,j)
def exchange_arc(G, n, i, j, k, l) :

	G.remove_edge(i,j)
	G.remove_edge(k,l)
	G.add_edge(i,l)
	G.add_edge(k,j)
	if is_connex(G, n) :
		return 1
	else :
		G.add_edge(i,j)
		G.add_edge(k,l)
		G.remove_edge(i,l)
		G.remove_edge(k,j)
		return 0


def is_exchange_possible(G, n, i, j, k, l) :
	# On verifie quon ne cree pas de multi arete  ou de boucle
	if i not in [k,l] and j not in [k,l]  and i != j and k != l :
		# On verifie quon ne cree pas une arete deja existante
		if (i,l) not in list(G.edges) and (k,j) not in list(G.edges) and (l,i) not in list(G.edges) and (j,k) not in list(G.edges):
			# on verifie quon ne connecte pas deux sommets de degre 1 (deconnexion)
			if G.degree[i] + G.degree[l] > 2 and G.degree[k] + G.degree[j] > 2 :
				return 1
			else :
				return 0
		else :
			return 0
	else :
		return 0

def random_arc_exchange(G, n, m) :
	# On choisit deux noeuds initiaux
	[(i,j), (k,l)] = random.sample(list(G.edges), 2)
	result = 0
	if is_exchange_possible(G, n, i, j, k, l) :
		result += exchange_arc(G, n, i, j, k, l)
	return result

def randomize(G,n,m,T) :
	reussite = 0
	for t in range(T) :
		reussite += random_arc_exchange(G, n, m)
	reussite *= 100/T
	return reussite

def generate_random_graph(G, n, m, T, maximum=-1) :

	G.add_nodes_from([i for i in range(n)])
	graphic_generation(G, n, m, maximum)
	reussite = randomize(G, n, m, T)
	return reussite

def distrib_cumulee(list_degre) :
	
	connection = [100*i/(n-1) for i in list_degre]
	connection.sort()
	
	liste = [0]*100
	for elt in connection :
		add = [0]*(int(elt)) + [1/n] + [0]*(100-int(elt)-1)
		liste = [ liste[i] + add[i] for i in range(100)]
	
	return liste

def sample_positions(pos,n,distances,ratioN=2, ratioP=3, proportion=0.005) :

	# Sample ranomly x and y in a grid
	# Definition de la grille (si ratio = m/p) : Xmax = p*sqrt(n/(m.p.proportion)), Ymax = m*sqrt(n/(m.p.proportion))
	Xmax = int(ratioP*math.sqrt(n/(ratioN*ratioP*proportion)))
	Ymax = int(ratioN*math.sqrt(n/(ratioN*ratioP*proportion)))
	x = 0
	y = 0
	for i in range(n) :
		distances[i] = {}
		x = random.randint(0,Xmax)
		y = random.randint(0,Ymax)
		while (x,y) in pos.values() :
			x = random.randint(0,Xmax)
			y = random.randint(0,Ymax)
		pos[i] = (x,y)
		for j in range(i-1) :
			distances[i][j] = dist(pos,i,j)


def dist(pos,i,k) :
	return math.sqrt( (pos[i][0] - pos[k][0])**2 + (pos[i][1] - pos[k][1])**2 )

def try_remove_edge(G,T,n,cnt) :
	liste_edges = random.sample(list(G.edges), T)
	for (i,j) in liste_edges :
		G.remove_edge(i,j)
	if not is_connex(G,n) :
		for (i,j) in liste_edges :
			G.add_edge(i,j)
		return cnt + 1
	return 0



def grid_generation(G, n, m, pos, ratioN=2, ratioP=3, proportion=0.005, Nbr_deg_1=0.08, n_remove=50) :

	G.add_nodes_from([i for i in range(n)])
	Dmax = 0
	distances = {}
	sample_positions(pos,n,distances,proportion=proportion)

	while not is_connex(G,n) :
		Dmax += 1
		for i in range(n) :
			nouveaux_voisins = [v for v in range(i-1) if Dmax-1 < distances[i][v] <= Dmax]
			G.add_edges_from([(v,i) for v in nouveaux_voisins])

	couleurs = [10*j for (i,j) in list(G.degree)]
	#plt.figure(1)
	#nx.draw(G, pos=pos, with_labels=False, node_color=couleurs ,font_weight='bold', alpha=0.8, node_size=300)
	
	degre1 = [ v for v in list(G.nodes) if G.degree[v] == 1 ]
	try_remove_counter = 0
	while(len(degre1) <= max(2,Nbr_deg_1*n)) :
		if try_remove_counter == 100 :
			print("Echec de la suppression d aretes")
			print("Nombre de degrs 1 final : %i" % len(degre1))
			break	
		try_remove_counter = try_remove_edge(G,n_remove,n,try_remove_counter)
		degre1 = [ v for v in list(G.nodes) if G.degree[v] == 1 ]

	couleurs = [j*10 for (i,j) in list(G.degree)]
	#nx.draw(G, with_labels=False, pos=pos, node_color=couleurs ,font_weight='bold', alpha=0.8, node_size=300)
	#plt.figure(2)
	#plt.show()

#####################################################################################################################
#####################################################################################################################
#################################                        MAIN  						#################################
#####################################################################################################################
#####################################################################################################################
if __name__ == '__main__' :

	n, m = read_options_graph(sys.argv[1:])

	G = nx.Graph()
	# K = 1
	# if m <= n*(n-1)/2 :
	# 	for k in range(1,K+1) :
	# 		m += 20
	# 		G = nx.Graph()
	# 		G.add_nodes_from([i for i in range(n)])
	# 		list_degres = graphic_generation(G, n, m)
	# 		G.clear()
	# 		result = distrib_cumulee(list_degres)
	# 		x = [i for i in range(100)]
	# 		plt.plot(x, result)
	# 	plt.show()

	pos = {}
	grid_generation(G, n, m, ratioN=2, ratioP=3, proportion=0.005, Nbr_deg_1=0.08)

	exit()

	if m <= n*(n-1)/2 :

		#On cree le graphe G avec les noeuds allant de 0 a n-1
		G = nx.Graph()
		G.add_nodes_from([i for i in range(n)])

		graphic_generation(G, n, m)
		couleurs = [j for (i,j) in list(G.degree)]



		# Taux de reussite d echange d arretes
		reussite = 0
		for t in range(200) :
			reussite += random_arc_exchange(G, n, m)
		reussite /= 2
		print('REUSSITE = ', reussite )

		nx.draw(G, with_labels=False, font_weight='bold', node_color =couleurs, alpha=0.8, node_size=100)
		plt.show()

	else :
		print("Le nombre d'arete doit etre inferieur a n(n-1)/2.")
		exit(0)

