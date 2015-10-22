#ifndef INTERPRETEUR_H
#define INTERPRETEUR_H

#include "Symbole.h"
#include "Lecteur.h"
#include "Exceptions.h"
#include "TableSymboles.h"
#include "ArbreAbstrait.h"

class Interpreteur {
public:
	Interpreteur(ifstream & fichier);   // Construit un interpréteur pour interpreter
	                                    //  le programme dans  fichier 
                                      
	void analyse();                     // Si le contenu du fichier est conforme à la grammaire,
	                                    //   cette méthode se termine normalement et affiche un message "Syntaxe correcte".
                                      //   la table des symboles (ts) et l'arbre abstrait (arbre) auront été construits
	                                    // Sinon, une exception sera levée

	inline const TableSymboles & getTable () const  { return m_table;    } // accesseur	
	inline Noeud* getArbre () const { return m_arbre; }                    // accesseur
	void traduitEnCPP(ofstream & fichier, unsigned int indentation) const;
	
private:
    Lecteur        m_lecteur;  // Le lecteur de symboles utilisé pour analyser le fichier
    TableSymboles  m_table;    // La table des symboles valués
    Noeud*         m_arbre;    // L'arbre abstrait
    map<string,Noeud *> * m_listProc;

    // Implémentation de la grammaire
    Noeud*  programme();   //   <programme> ::= {procedure <CHAINE>() <seqInst> finproc} procedure principale() <seqInst> finproc FIN_FICHIER
    Noeud*  seqInst();	   //     <seqInst> ::= <inst> { <inst> }
    Noeud*  inst();	       //        <inst> ::= <affectation> ; | <instSi> | <instTantQue> | <instRepeter> ; | <instPour> | <instEcrire> ; | <instLire> ;
    Noeud*  affectation(); // <affectation> ::= <variable> = <expression> 
    Noeud*  expression();  //  <expression> ::= <terme> { + <terme> | - <terme> }
    Noeud*  terme();	   //       <terme> ::= <facteur> { * <facteur> | / <facteur> }
    Noeud*  facteur();     //     <facteur> ::= <entier> | <variable> | - <expBool> | non <expBool> | ( <expBool> )
    Noeud*  expBool();     //     <expBool> ::= <relationET> { ou <relationET> }
    Noeud*  relationET();  //  <relationET> ::= <relation> { et <relation> }
    Noeud*  relation();    //    <relation> ::= <expression> { <opRel> <expression> }

    Noeud*  instSi() throw (SyntaxeException);      //      <instSi> ::= si ( <expression> ) <seqInst> { sinonsi ( <expression> ) <seqInst> } [sinon <seqInst>] finsi
    Noeud*  instTantQue(); // <instTantQue> ::= tantque ( <expression> ) <seqInst> fintantque
    Noeud*  instRepeter(); // <instRepeter> ::= repeter <seqInst> jusqua ( <expression> )
    Noeud*  instPour();    //    <instPour> ::= pour ( [ <affectation> ] ) ; <expression> ; ( [ <affectation> ] ) <seqInst>	 finpour
    Noeud*  instEcrire();  //  <instEcrire> ::= ecrire ( <expression> | <chaine> | { , <expression> | <chaine> } )
    Noeud*  instLire();    //    <instLire> ::= lire ( <variable> { , <variable> } )
    Noeud*  instProcedure();    //  <instProcedure> ::= appel <CHAINE>()

    // outils pour simplifier l'analyse syntaxique
    void tester (const string & symboleAttendu) const throw (SyntaxeException);   // Si symbole courant != symboleAttendu, on lève une exception
    void testerEtAvancer(const string & symboleAttendu) throw (SyntaxeException); // Si symbole courant != symboleAttendu, on lève une exception, sinon on avance
    void erreur (const string & mess) const throw (SyntaxeException);             // Lève une exception "contenant" le message mess
};

#endif /* INTERPRETEUR_H */
