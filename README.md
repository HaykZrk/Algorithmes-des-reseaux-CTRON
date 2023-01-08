# Algorithmes des réseaux : CTRON

## Description
Projet destiné à l'UE Algorithmes des réseaux à l'UFR Mathématique-Informatique Strasbourg. Langage C -> Utilisation d'interface graphique ncurses

## Compilation

- Solution 1 : Compilation via la commande make

- Solution 2 : Exécuter directement via les exécutables fournis 

## Exécution

- Solution 1 sur le même ordinateur : 
	- Lancer un premier terminal et exécuter la commande ./serveur [PORT_SERVEUR] [REFRESH_RATE]
	- Lancer un deuxième terminal et exécuter la commande ./client 0.0.0.0 [PORT_SERVEUR] 2
	- Commande Joueur 1 : {Z,Q,S,D,SPACE}
	- Commande Joueur 2 : {I,J,K,L,M}

- Solution 2 sur un ordinateur différent : 
	- Lancer un premier terminal sur le PC1 et exécuter la commande ./serveur [PORT_SERVEUR] [REFRESH_RATE]
	- Lancer un deuxième terminal sur le PC1 et exécuter la commande ./client [IP_SERVEUR] [PORT_SERVEUR] 1
	- Lancer un terminal sur le PC2 et exécuter la commande ./client [IP_SERVEUR] [PORT_SERVEUR] 1
	- Commande Joueur : {Z,Q,S,D,SPACE}

## Auteurs 
Hayk ZARIKIAN et Alexandre DUBERT