#include "directive.h"

std::vector<const char*> funcname;

void register_pragmas(void* gcc_data, void* user_data) {
	c_register_pragma("mpicoll","check", handle_pragma_function);
}

void handle_pragma_function(struct cpp_reader* useless) {
	enum cpp_ttype token;
	tree t;
	token = pragma_lex(&t);
	bool close_paren_needed_p = false;
	if (cfun) {
		printf ("PAS DE PRAGMA DANS LES FONCTIONS");
		return;
	}
	if (token == CPP_OPEN_PAREN) {
		close_paren_needed_p = true;
		token = pragma_lex(&t);
	}
	if (token != CPP_NAME) {
		printf (" Mdr c'est pas un nom de fonction");
		return;
	}
	else {
		do {
			while (token == CPP_COMMA)
				token = pragma_lex(&t);
			funcname.push_back(IDENTIFIER_POINTER(t));
			token = pragma_lex(&t);
		}
		while (token == CPP_NAME || token == CPP_COMMA);
	}	
	if (close_paren_needed_p) {
		if (token == CPP_CLOSE_PAREN)
			token = pragma_lex (&t);
		else
			printf ("Ferme tes parentheses");
	}
	if (token != CPP_EOF) {
		printf ("Il est ou le retour a la ligne apres ton pragma ?");
		return;
	}
}
