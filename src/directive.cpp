#include "directive.h"

void register_pragmas(void* gcc_data, void* user_data) {
	c_register_pragma("instrument","function", handle_pragma_function);
}

static void handle_pragma_function(struct cpp_reader* useless) {
	enum cpp_ttype token;
	tree t;
	token = pragma_lex(&t);
	bool close_paren_needed_p = false;
	if (cfun) {
		error ("PAS DE PRAGMA DANS LES FONCTIONS");
		return;
	}
	if (token == CPP_OPEN_PAREN) {
		close_paren_needed_p = true;
		token = pragma_lex(&t);
	}
	if (token != CPP_NAME) {
		GCC_BAD (" Mdr c'est pas un nom de fonction");
		return;
	}
	else {
		tree args = NULL_TREE;
		do {
			while (token == CPP_COMMA)
				token = pragma_lex(&t);
			
		}
		while (token == CPP_NAME || token == CPP_COMMA);
	}	
	if (close_paren_needed_p) {
		if (token == CPP_CLOSE_PAREN)
			token = pragma_lex (&t);
		else
			GCC_BAD ("Ferme tes parentheses");
	}
	if (token != CPP_EOF) {
		error ("Il est ou le retour a la ligne apres ton pragma ?");
		return;
	}
		
}
