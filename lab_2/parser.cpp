// ============================================================================
// parser.cpp - Recursive descent parser that BUILDS the AST 
// ----------------------------------------------------------------------------
// Author: Derek Willis (Fall 2025)
// ============================================================================

#include "parser.h"
#include "lexer.h"
#include "debug.h"
#include <stdexcept>
#include <string>
using namespace std;

// Provided by the lexer (Flex)
extern int yylex();
extern char* yytext;
extern FILE* yyin;


// Single-token lookahead
static int lookahead = 0;

// Advance to the next token
static void next() {
    lookahead = yylex();
    if (gDebug) {
        if (lookahead == TOK_EOF) dbg("next: TOK_EOF");
        else dbg(string("next: ") + tokenName(lookahead) + " (" + yytext + ")");
    }
}

// Match a specific token and return its lexeme, or throw with the given message.
static string expect(int tok, const char* msgIfMismatch) {
    if (lookahead == tok) {
        string lex = yytext;
        if (gDebug) dbg(string("match ") + tokenName(tok) + " (" + lex + ")");
        next();
        return lex;
    }
    if (gDebug) {
        dbg(string("mismatch: got ") + tokenName(lookahead) +
            ", expected " + tokenName(tok));
    }
    throw runtime_error(msgIfMismatch);
}


// TODO: define and implement parseSentence, parseNounPhrase, parseAdjectivePhrase,
//        parseVerbPhrase

// <adjective phrase> --> (ARTICLE | POSSESSIVE) ADJECTIVE
// Errors:
// "<adjective phrase> did not start with an article or possessive."
// "<adjective phrase> did not have an adjective."

unique_ptr<AdjectivePhrase> parseAdjectivePhrase(){
    dbgLine("enter <adjective phrase>");
    DebugIndent _scope;

    // FIRST check
    if (lookahead != ARTICLE && lookahead != POSSESSIVE){
        throw runtime_error(
            "<adjective phrase> did not start with an article or possessive."
        );
    }

    auto node = make_unique<AdjectivePhrase>();

    // Determiner (ARTICLE | POSSESSIVE)
    if (lookahead == ARTICLE){
        node->detType = AdjectivePhrase::DetType::Article;
        node->detLexeme = expect(ARTICLE, "<adjective phrase> did not start with an article or possessive.");
    } else{//POSSESSIVE
        node->detType = AdjectivePhrase::DetType::Possessive;
    node->detLexeme = expect(POSSESSIVE, "<adjective phrase> did not start with an article or possessive.");
    }

    // ADJECTIVE
    node->adjLexeme = expect(ADJECTIVE, "<adjective phrase> did not have an adjective.");

    return node;
}


// Entry point: initialize, parse, enforce EOF
unique_ptr<Sentence> parseStart() {
    next();                      // prime lookahead
    auto root = parseSentence(); // may throw on first syntax error
    if (lookahead != TOK_EOF) {
        throw runtime_error("Extra input after complete sentence.");
    }
    return root;
}


// parseNounPhrase()
// Spec: <noun phrase> → <adjective phrase> NOUN
// 1. dbgLine("enter <noun phrase>"); DebugIndent _scope;
// 2. FIRST check: ARTICLE or POSSESSIVE, else
// throw "<noun phrase> did not start with an article or possessive."
// (Tip: do this FIRST, before constructing children.)
// 3. auto node = make_unique<NounPhrase>();
// 4. node->adj = parseAdjectivePhrase();
// 5. node->nounLexeme = expect(NOUN, "<noun phrase> did not have a noun.");
// 6. return node.

unique_ptr<NounPhrase> parseNounPhrase(){
    dbgLine("enter <noun phrase>");
    DebugIndent _scope;

    // FIRST check
    if (lookahead != ARTICLE && lookahead != POSSESSIVE){
        throw runtime_error(
            "<noun phrase> did not start with an article or possessive."
        );
    }

    auto node = make_unique<NounPhrase>();

    node->adj = parseAdjectivePhrase();

    node->nounLexeme = expect(NOUN, "<noun phrase> did not have a noun.");

    return node;
}


// parseVerbPhrase()
// Spec: <verb phrase> → VERB | ADVERB <verb phrase> (normalize to ADVERB* then VERB)
// 1. dbgLine("enter <verb phrase>"); DebugIndent _scope;
// 2. FIRST check: VERB or ADVERB, else
// throw "<verb phrase> did not start with a verb or an adverb."
// 3. auto node = make_unique<VerbPhrase>();
// 4. While lookahead == ADVERB:
// o node->adverbs.push_back(expect(ADVERB, "..."))
// o (Use a precise message only where required; FIRST errors are already covered above.)
// 5. node->verbLexeme = expect(VERB, "<verb phrase> did not start with a verb or an adverb.");
// 6. return node.

unique_ptr<VerbPhrase> parseVerbPhrase(){
    dbgLine("enter <verb phrase>");
    DebugIndent _scope;

    // FIRST check
    if (lookahead != VERB && lookahead != ADVERB){
        throw runtime_error(
            "<verb phrase> did not start with a verb or an adverb."
        );
    }

    auto node = make_unique<VerbPhrase>();

    while (lookahead==ADVERB){
        node->adverbs.push_back(expect(ADVERB, "<verb phrase> did not start with adv"));
    }

    node->verbLexeme = expect(VERB, "<verb phrase> did not start with a verb or an adverb.");

    return node;
}


// parseSentence()
// Spec: <sentence> → <noun phrase> <verb phrase> <noun phrase>
// 1. dbgLine("enter <sentence>"); DebugIndent _scope;
// 2. FIRST check: ARTICLE or POSSESSIVE, else
// throw "<sentence> did not start with an article or possessive."
// 3. auto node = make_unique<Sentence>();
// 4. node->subjectNP = parseNounPhrase();
// 5. node->verbP = parseVerbPhrase();
// 6. node->objectNP = parseNounPhrase();
// 7. return node.

unique_ptr<Sentence> parseSentence(){
    dbgLine("enter <sentence>");
    DebugIndent _scope;

    // FIRST check
    if (lookahead != ARTICLE && lookahead != POSSESSIVE){
        throw runtime_error(
            "<sentence> did not start with an article or possessive."
        );
    }

    auto node = make_unique<Sentence>();
    node->subjectNP = parseNounPhrase();
    node->verbP = parseVerbPhrase();
    node->objectNP = parseNounPhrase();

    return node;
}
