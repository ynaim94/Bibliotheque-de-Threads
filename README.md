# Auteurs :
*Manon Barbier
*Nour Grati
*Guillaume Lebreton
*Youssef Naïm
*Hugo Vikström


## Liste des commandes du Makefile
-) make
   compile toutes les sources
-) make nom_test
   compile le test de nom <nom_test> et génère un exécutable <nom_test>
-) make clean
   supprime les .o et les exécutables du dossier


#src :
## contenu du répertoire :
* Ce répertoire contient notre implémentation de l'interface thread.h
* le fichier time.sh est le script qui permet de mesurer le temps des programmes
  pour le tracé des courbes

#tst :
## contenu du répertoire :
*Ce répertoire contient l'ensemble des tests qui nous ont été donnés ainsi que les 
tests que nous avons écris nous même

## Comment tracer une courbe
   * Pour tracer une courbe et comparer notre implémentation à celle de la 
   bibliothèque pthread, il faut suivre les étapes suivantes :
   * faire un make <nom_test>
   * lancer ./time.sh <nom_test> <valeur>
   * utiliser gnuplot pour tracer la courbe