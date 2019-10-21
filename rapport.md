# Projet de Compilation Avancée
Gavoille Clément - Skutnik Jean-Baptiste

## Introduction

## Partie 1: Vérification de la séquence d'appel aux fonctions collectives MPI

La passe de compilation travaille sur une représentation du code interne à `GCC`, qui prend la forme d'un graphe orienté, nommé `Control Flow Graph`, qui représente le code et les différentes suites d'instructions pouvant être exécutées lors de l'exécution d'un programme.  
Les liaisons entre les différents noeuds correspondent aux tests logiques effectués lors de l'exécutiondu programme.  
Cette structure de graphe est nommée `Control Flow Graph`, ou `CFG` dans la suite de ce rapport. Les noeuds qui le composent sont eux appelés des `Basic Blocks`.

La lecture de ce graphe est indépendante du langage du code source dont il résulte. Les portions de code sont transformées en déclarations de forme normée `GENERIC`, mais l'analyse est faite sur des simplifications de cette norme: le format `GIMPLE`.

### Détection des appels aux fonctions collectives MPI

La détection des appels de fonction à l'intérieur du code se fait en parcourant la liste des instructions contenues dans les blocs de codes du `CFG`.  
Ces blocs, sous format `GIMPLE` sont parcourus à la recherche d'appels de fonctions : les fonctions appelées sont alors comparées à une liste pré-déterminée des fonctions d'intérêt sur lesquelles notre analyse se base. 

Le fichier `MPI_colectives.def` définit les appels `MPI` supportés. Il contient les déclarations suivantes:
```
DEFMPICOLLECTIVES( MPI_INIT, "MPI_Init" )
DEFMPICOLLECTIVES( MPI_FINALIZE, "MPI_Finalize" )
DEFMPICOLLECTIVES( MPI_REDUCE, "MPI_Reduce" )
DEFMPICOLLECTIVES( MPI_ALL_REDUCE, "MPI_AllReduce" )
DEFMPICOLLECTIVES( MPI_BARRIER, "MPI_Barrier" )
```

Ce format nous permet de créer dynamiquement du code à l'aide de `macros` `C`:
```
#define DEFMPICOLLECTIVES( CODE, NAME ) if(!strcmp(func_name, NAME)){return index;}else{index++;};
int is_mpi(const char* func_name) {
    int index = 0;
#include "MPI_collectives.def"
    return -1;
};
#undef DEFMPICOLLECTIVES
```

Rajouter une collective cible à notre analyse revient donc à ajouter une simple ligne dans le fichier `MPI_collectives.def`. 

Dans le code, le fichier `MPI_collectives.def` est interprêté pour créer un `enum`, dont les codes correspondent à la ligne de la fonction `MPI` dans le fichier.

### Préparation du contexte

Les `basic blocks`, composant les noeuds du graphe, peuvent contenir de multiples appels de fonctions. Pour faciliter l'analyse du code, la première étape de notre passe est une étape de modification du graphe qui consiste à scinder les `basic blocks` pour qu'ils contiennent __au plus__ un appel de collective `MPI`.

La mise en place de cet axiome facilite grandement l'analyse du graphe, et ne change en rien la sémantique du programme. Par la suite, il sera supposé que chaque appel à une collective `MPI` est unique dans le `basic block` où elle se trouve.

La séparation des blocs se fait grâce à la fonction `isolate_mpi` définie dans `mpi_detection.cpp`. Elle appelle la méthode `split_block` de l'`API GCC` pour scinder les noeuds, en donnant l'assertion `GIMPLE` correspondante.

### Étude du graphe de flot de contrôle pour déterminer les divergences

### Détermination des noeuds à risque

Une fois l'ensemble des noeuds à tester déterminé, il s'agit de vérifier que les divergences potentielles existent.

Pour ce faire, un parcours en profondeur du graphe depuis les noeuds sélectionnés va être effectué. 

L'algorithme effectuant le parcours est le suivant, en pseudo-code
```
std::vector<unsigned int> suite = {}
typedef stack_el std::pair<basic_block, unsigned int>
std::vector<stack_el> pile = {}

chemin = parcours_simple(bb)

tant que pile.size() > 0:
	current_bb, index = pile.pop()
	si collective dans current_bb:
		si collective != chemin[index]:
			retourner false
		sinon
			index = index + 1
	
	pour tout successeur de current_bb:
		pile.push((successor, index))

retourner true
```

Tout d'abord, et pour simplifier le code, une suite est déterminée avec un parcours simple du graphe: seul le premier successeur de chaque noeud est pris en compte, et tous les appels aux collectives sont relevés et stockés, dans l'ordre, dans le tableau 'suite'.

Cette suite n'a pas pour vocation d'être exacte, mais sert de référence pour le reste de l'algorithme: si toutes les suites d'appels sont identiques, alors cette suite est égale à tous les autres suites calculées.

L'algorithme va alors effectuer un parcours en profondeur classique du graphe, mais stockant dans la pile l'index de la collective recherchée dans la suite d'appel.  
À chaque appel `MPI` rencontré, le programme vérifie qu'il corresponde à celui de la suite de référence, à l'index indiqué.  
Si il correspond, le parcours continue, en incrémentant de 1 l'index ; sinon, le programme s'arrête en renvoyant `false`, car cela implique que cette suite d'appel ne suit pas la référence.

### Affichage d'un warning à l'utilisateur

L'implémentation des avertissement est basée sur la structure fournie par `GCC` :
```
warning_at (location_t location, int opt, const char *gmsgid, ...)
```

Pour s'intégrer au mieux aux messages du compilateur et profiter de l'archtecture existante autour de ces derniers. Le code traduit le `basic block` et le code de la collective générant l'avertissement en accédant à la dernière assertion du `basic block`, en traduisant le nom de la collective à partir de son code, et en fournissant les deux à `gcc`:

```
warning_at(gimple_location(stmt),	// Assertion posant problème
		   0,						// Options, aucune ici
		   "Calls to %s may be avoided from this location",
		   mpi_collective_name[collective]);
```

Cette gymnastique produit des avertissements conventionnels pour `GCC`:
```
test2.c: In function ‘main’:
test2.c:15:4: warning: Calls to MPI_Barrier may be avoided from this location
   15 |  if(c<10)
      |    ^
```

## Partie 2: Gestion des directives (obligatoire)

### Définition et format

### Gestion des directives
La gestion de cette directive est active des que le plugin GCC de ce projet est utilisé. Les fonctions présentes dans ces directives sont alors analysées. Si une fonction est présente dans le code source à compiler, mais non renseignée par une telle directive, l'analyse n'est pas effectuée.
Un fichier source peut utiliser plusieurs fois cette directive en mélangeant les deux formes. Cette directive ne peut être utilisée qu'en dehors de toute fonction.
Un warning devra être émis à l'utilisateur s'il spécifie une fonction dans cette directive qui n'existe pas dans le code source. De plus, un warning sera émis si une fonction est spécifiée plusieurs fois dans l'ensemble des directives

## Sources

|Nom|Source|
|-|-|
|`Control Flow Graph`|[GCC Documentation](https://gcc.gnu.org/onlinedocs/gccint/Control-Flow.html)|
|`Basic Blocks`|[GCC Documentation](https://gcc.gnu.org/onlinedocs/gccint/Basic-Blocks.html)|
|`GENERIC`|[GCC Documentation](https://gcc.gnu.org/onlinedocs/gccint/GENERIC.html)|
|`GIMPLE`|[GCC Documentation](https://gcc.gnu.org/onlinedocs/gccint/GIMPLE.html)|
