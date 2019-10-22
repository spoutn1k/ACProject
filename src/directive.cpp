#include "directive.h"
#include <diagnostic.h>

std::vector<const char*> funcname;

void register_pragmas(void* gcc_data, void* user_data) {
	c_register_pragma("mpicoll", "check", handle_pragma_function);
}

bool is_registered(const char* fname) {
	for (const char* func: funcname) {
		if (!strcmp(func, fname))
			return true;
	}
	return false;
}

void mark_processed(const char* fname) {
	for (auto it = funcname.begin(); it != funcname.end();) {
		if (!strcmp(*it, fname))
            funcname.erase(it);
		else
			++it;
	}
}

void handle_pragma_function(struct cpp_reader* useless) {
	enum cpp_ttype token;
	tree t;
	token = pragma_lex(&t);
	bool close_paren_needed_p = false;

	if (cfun) {
		warning(0, "Token is prohibited in a function");
		return;
	}

	if (token == CPP_OPEN_PAREN) {
		close_paren_needed_p = true;
		token = pragma_lex(&t);
	}

	if (token != CPP_NAME) {
		warning(0, "Token is not a function name");
		return;
	}

	else {
		do {
			while (token == CPP_COMMA)
				token = pragma_lex(&t);

			if (is_registered(IDENTIFIER_POINTER(t)))
				warning(0, "%s is already registered for MPI check", IDENTIFIER_POINTER(t));
			else
				funcname.push_back(IDENTIFIER_POINTER(t));

			token = pragma_lex(&t);
		}
		while (token == CPP_NAME || token == CPP_COMMA);
	}	

	if (close_paren_needed_p) {
		if (token == CPP_CLOSE_PAREN)
			token = pragma_lex (&t);
		else
			warning(0, "Missing closing parenthesis");
	}

	if (token != CPP_EOF) {
		warning(0, "Missing carriage return after statement");
		return;
	}
}

void wrap_mpicoll(void* gcc_data, void* user_data) {
	for (const char* func: funcname)
		warning(0, "function `%s` marked as mpi_check target but was not found in source code", func);
}
