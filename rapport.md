# Projet de Compilation Avancée
Gavoille Clément - Skutnik Jean-Baptiste

## Introduction

Le framework `MPI` met en place des fonctions permettant aux programmes de communiquer lors de l'exécution pour faciliter la parallélisation de calculs. Cependant introduire des communications dans un code peut entraîner des ralentissements, voir des arrêts indéfinis lors d'une erreur ou d'un mauvais agencement des fonctions de communication par l'utilisateur.

Ce projet propose de créer un plugin `GCC`, qui lors de la compilation, va vérifier que les fonctions collectives `MPI` sont traversées par tous les cas possibles d'exécution du programme, assurant ainsi que le programme ne sera jamais dans une impasse.

Pour ce faire, l'utilisation de l'`API GCC` est indispensable, permettant d'interagir avec les étapes de compilation, en introduisant une 'passe' qui implémente certaines fonctions clés.

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

Les noeuds contenant des collectives `MPI` à analyser sont regroupés en ensembles, représentés en mémoire par des `bitmaps`, de l'`API GCC`.

Dans le code, la fonction `mpi_calls` effectue ce traitement. Elle renvoie un tableau de `bitmaps` de taille égale au nombre de collectives `MPI` définies dans le programme. Dans la `bitmap` d'index `k`, les bits d'index `i` valent `1` si le `basic block` d'index `i` contient la collective d'index `k` correspondant.

Ces ensembles sont alors utilisés dans le calcul de leur frontières de post-dominance. On calcule la frontière de post-dominance d'un noeud de la manière suivante:

$$
On\ note\ n\ post\ domine\ z\ par\ n \\triangleright z
$$
$$
PDF(n) = \\{z\ |\ \\forall (m,z) \\in \\Omega,\ m \\to z\ |\ n \\triangleright m,\ n \\ntriangleright z\\}
$$

À l'aide des frontières de post-dominance des noeuds, la frontière de post-dominance de l'ensemble est calculée suivant la formule:

$$
PDF(N) = \\{z\ |\ z \\in \\cup\_{n \\in N}\ PDF(n),\ z \\in \\cup\_{k \\in \\bar{N}}\ PDF(k) \\}
$$

La frontière de post-dominance d'un ensemble nous permet de déterminer les noeuds à partir desquels il existe un chemin ne passant pas par un noeud de l'ensemble. La fonction `compute_pdf_sets` renvoie un objet similaire à `mpi_calls` contenant cette information.

### Détermination des noeuds à risque

Une fois l'ensemble des noeuds à tester déterminé, il s'agit de vérifier que les divergences potentielles existent. Pour ce faire, un parcours en profondeur du graphe depuis les noeuds sélectionnés va être effectué. 

L'algorithme effectuant le parcours est le suivant, en pseudo-code:
```
std::vector<unsigned int> suite = {}
typedef stack_el std::pair<basic_block, unsigned int>
std::vector<stack_el> pile = {}

suite = parcours_simple(bb)

tant que pile.size() > 0:
	current_bb, index = pile.pop()
	si collective dans current_bb:
		si collective != suite[index]:
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

Cet algorithme, et les parcours en profondeur en général, peuvent rester bloqués lors de l'analyse de boucles ; et dans des cas d'usage réels des boucles sont garanties.

Nous avons pris la décision de ne pas analyser les boucles.

Le framework `GCC` délimitant et catégorisant les boucles à été utilisé pour déterminer l'appartenance d'un `basic_block` à une boucle, mais le système de prédictions de `GCC` n'a pas été utilisé pour faire des suppositions. À la place, le programme évite l'analyse si un noeud à risque est dans une boucle.

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

## Partie 2: Gestion des directives

### Définition et format

L'utilisation des directives dans le programme permet à l'utilisateur de contrôler facilement le comportement du plugin, en utilisant un format de messages standardisé dans `GCC`.

Dans ce plugin, les directives définies servent à spécifier les fonctions à analyser. Elles prennent les formes suivantes:
```
#pragma mpicoll check f1
#pragma mpicoll check (f2, f3)
```

Les directives introduites dans le plugin suivent les règles suivantes:
- Elles ne peuvent apparaître dans un corps de fonction ; 
- Plusieurs directives peuvent apparaître, mais chaque fonction doit être spécifiée une seule fois ; 
- Une fonction peut être spécifiée uniquement si elle est présente dans le code source.

Enfreindre une de ces règles provoquera un avertissement pour l'utilisateur.

### Gestion des directives

Les directives sont interprêtées par deux fonctions définies par le plugin : 

- `handle_pragma_function`: lit les directives et enregistre les fonctions dans une zone mémoire interne. Les directives erronées ou redondantes génèrent un avertissement à l'utilisateur.
- `wrap_mpicoll`: génère un avertissement pour toutes les fonctions dans la zone mémoire à la fin de l'exécution de `GCC`, indiquant que leur définition n'a pas été trouvée dans le code.

Lors de l'exécution de la passe, la fonction `gate` du plugin prend en compte les fonctions enregistrées pendant `handle_pragma_function` pour ne lancer l'exécution que sur les fonctions spécifiées.

## Sources

|Nom|Source|
|-|-|
|`Control Flow Graph`|[GCC Documentation](https://gcc.gnu.org/onlinedocs/gccint/Control-Flow.html)|
|`Basic Blocks`|[GCC Documentation](https://gcc.gnu.org/onlinedocs/gccint/Basic-Blocks.html)|
|`GENERIC`|[GCC Documentation](https://gcc.gnu.org/onlinedocs/gccint/GENERIC.html)|
|`GIMPLE`|[GCC Documentation](https://gcc.gnu.org/onlinedocs/gccint/GIMPLE.html)|
