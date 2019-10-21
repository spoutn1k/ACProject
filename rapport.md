# Projet de Compilation Avancée
Gavoille Clément - Skutnik Jean-Baptiste

## Introduction


## Partie 1: Vérification de la séquence d'appel aux fonctions collectives MPI

La passe de compilation travaille sur une représentation du code interne à `GCC`, qui prend la forme d'un graphe orienté, nommé `Control Flow Graph`, qui représente le code et les différentes suites d'instructions pouvant être exécutées lors de l'exécution d'un programme. 
Les liaisons entre les différents noeuds correspondent aux tests logiques effectués lors de l'exécutiondu programme. 

La lecture de ce graphe est indépendante du langage du code source dont il résulte. Les portions de code sont transformées en déclarations de forme normée `GENERIC`, mais l'analyse est faite sur des simplifications de cette norme: le format `GIMPLE`.
Cette structure de graphe est nommée `Control Flow Graph`, ou `CFG` dans la suite de ce rapport. Les noeuds qui le composent sont eux appelés des `Basic Blocks`.

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

### Affichage d'un warning à l'utilisateur

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
