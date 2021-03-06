#ifndef STAN_LANG_GRAMMARS_FUNCTIONS_GRAMMAR_DEF_HPP
#define STAN_LANG_GRAMMARS_FUNCTIONS_GRAMMAR_DEF_HPP

#include <stan/lang/ast.hpp>
#include <stan/lang/grammars/functions_grammar.hpp>
#include <stan/lang/grammars/semantic_actions.hpp>
#include <stan/lang/grammars/whitespace_grammar.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <string>
#include <vector>

BOOST_FUSION_ADAPT_STRUCT(stan::lang::function_decl_def,
                          (stan::lang::expr_type, return_type_)
                          (std::string, name_)
                          (std::vector<stan::lang::arg_decl>, arg_decls_)
                          (stan::lang::statement, body_) )

BOOST_FUSION_ADAPT_STRUCT(stan::lang::arg_decl,
                          (stan::lang::expr_type, arg_type_)
                          (std::string, name_)
                          (stan::lang::statement, body_) )

namespace stan {

  namespace lang {

  template <typename Iterator>
  functions_grammar<Iterator>::functions_grammar(variable_map& var_map,
                                                 std::stringstream& error_msgs)
      : functions_grammar::base_type(functions_r),
        var_map_(var_map),
        functions_declared_(),
        functions_defined_(),
        error_msgs_(error_msgs),
        statement_g(var_map_, error_msgs_),
        bare_type_g(var_map_, error_msgs_) {
      using boost::spirit::qi::_1;
      using boost::spirit::qi::char_;
      using boost::spirit::qi::eps;
      using boost::spirit::qi::lexeme;
      using boost::spirit::qi::lit;
      using boost::spirit::qi::_pass;
      using boost::spirit::qi::_val;
      using boost::spirit::qi::labels::_a;
      using boost::spirit::qi::labels::_b;

      functions_r.name("function declarations and definitions");
      functions_r
        %= (lit("functions") > lit("{"))
        >> *function_r
        > lit('}')
        > eps[validate_declarations_f(_pass,
                                      boost::phoenix::ref(functions_declared_),
                                      boost::phoenix::ref(functions_defined_),
                                      boost::phoenix::ref(error_msgs_))];

      // locals: _a = allow sampling, _b = origin (function, rng/lp)
      function_r.name("function declaration or definition");
      function_r
        %= bare_type_g[set_void_function_f(_1, _b, _pass,
                                           boost::phoenix::ref(error_msgs_))]
        > identifier_r
          [set_allows_sampling_origin_f(_1, _a, _b)]
          [validate_prob_fun_f(_1, _pass, boost::phoenix::ref(error_msgs_))]
        > lit('(')
        > arg_decls_r
        > close_arg_decls_r
        > eps
          [validate_pmf_pdf_variate_f(_val, _pass,
                                      boost::phoenix::ref(error_msgs_))]
        > eps[scope_lp_f(boost::phoenix::ref(var_map_))]
        > statement_g(_a, _b, true)
        > eps[unscope_variables_f(_val, boost::phoenix::ref(var_map_))]
        > eps[validate_return_type_f(_val, _pass,
                                     boost::phoenix::ref(error_msgs_))]
        > eps[add_function_signature_f(_val, _pass,
                                       boost::phoenix::ref(functions_declared_),
                                       boost::phoenix::ref(functions_defined_),
                                       boost::phoenix::ref(error_msgs_))];

      close_arg_decls_r.name("argument declaration or close paren )"
                             " to end argument declarations");
      close_arg_decls_r %= lit(')');

      arg_decls_r.name("function argument declaration sequence");
      arg_decls_r
        %= arg_decl_r % ','
        | eps;

      arg_decl_r.name("function argument declaration");
      arg_decl_r
        %= bare_type_g[validate_non_void_arg_f(_1, _pass,
                                       boost::phoenix::ref(error_msgs_))]
        > identifier_r
        > eps[add_fun_var_f(_val, _pass,
                            boost::phoenix::ref(var_map_),
                            boost::phoenix::ref(error_msgs_))];

      identifier_r.name("identifier");
      identifier_r
        %= lexeme[char_("a-zA-Z")
                   >> *char_("a-zA-Z0-9_.")];
    }

  }
}
#endif

