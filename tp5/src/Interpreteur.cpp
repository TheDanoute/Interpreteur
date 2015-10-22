#include "Interpreteur.h"
#include <stdlib.h>
#include <iostream>
#include <sstream>
using namespace std;

Interpreteur::Interpreteur(ifstream & fichier) :
m_lecteur(fichier), m_table(), m_arbre(nullptr) {
}

void Interpreteur::analyse() {
  m_arbre = programme(); // on lance l'analyse de la première règle
}

void Interpreteur::tester(const string & symboleAttendu) const throw (SyntaxeException) {
  // Teste si le symbole courant est égal au symboleAttendu... Si non, lève une exception
  static char messageWhat[256];
  if (m_lecteur.getSymbole() != symboleAttendu) {
    sprintf(messageWhat,
            "Ligne %d, Colonne %d - Erreur de syntaxe - Symbole attendu : %s - Symbole trouvé : %s",
            m_lecteur.getLigne(), m_lecteur.getColonne(),
            symboleAttendu.c_str(), m_lecteur.getSymbole().getChaine().c_str());
    throw SyntaxeException(messageWhat);
  }
}

void Interpreteur::testerEtAvancer(const string & symboleAttendu) throw (SyntaxeException) {
  // Teste si le symbole courant est égal au symboleAttendu... Si oui, avance, Sinon, lève une exception
  tester(symboleAttendu);
  m_lecteur.avancer();
}

void Interpreteur::erreur(const string & message) const throw (SyntaxeException) {
  // Lève une exception contenant le message et le symbole courant trouvé
  // Utilisé lorsqu'il y a plusieurs symboles attendus possibles...
  static char messageWhat[256];
  sprintf(messageWhat,
          "Ligne %d, Colonne %d - Erreur de syntaxe - %s - Symbole trouvé : %s",
          m_lecteur.getLigne(), m_lecteur.getColonne(), message.c_str(), m_lecteur.getSymbole().getChaine().c_str());
  throw SyntaxeException(messageWhat);
}

Noeud* Interpreteur::programme() {
  // <programme> ::= procedure principale() <seqInst> finproc FIN_FICHIER
  testerEtAvancer("procedure");
  testerEtAvancer("principale");
  testerEtAvancer("(");
  testerEtAvancer(")");
  Noeud* sequence = seqInst();
  testerEtAvancer("finproc");
  tester("<FINDEFICHIER>");
  return sequence;
}

Noeud* Interpreteur::seqInst() {
  // <seqInst> ::= <inst> { <inst> }
  NoeudSeqInst* sequence = new NoeudSeqInst();
  do {
    sequence->ajoute(inst());
  } while (m_lecteur.getSymbole() == "<VARIABLE>" || m_lecteur.getSymbole() == "si" || m_lecteur.getSymbole() == "tantque" || m_lecteur.getSymbole() == "repeter" || m_lecteur.getSymbole() == "pour" || m_lecteur.getSymbole() == "ecrire" || m_lecteur.getSymbole() == "lire");
  // Tant que le symbole courant est un début possible d'instruction...
  // Il faut compléter cette condition chaque fois qu'on rajoute une nouvelle instruction
  return sequence;
}

Noeud* Interpreteur::inst() {
  // <inst> ::= <affectation>  ; | <instSi>
  try {
  if (m_lecteur.getSymbole() == "<VARIABLE>") {
    Noeud *affect = affectation();
    testerEtAvancer(";");
    return affect;
  }
  else if (m_lecteur.getSymbole() == "si")
	  return instSi();
  else if (m_lecteur.getSymbole() == "tantque")
	  return instTantQue();
  else if (m_lecteur.getSymbole() == "repeter")
  	  return instRepeter();
  else if (m_lecteur.getSymbole() == "pour")
  	  return instPour();
  else if (m_lecteur.getSymbole() == "ecrire")
  	  return instEcrire();
  else if (m_lecteur.getSymbole() == "lire")
  	  return instLire();
  else erreur("Instruction incorrecte");
} catch (SyntaxeException & e) {
	cout << e. what() << endl;
	while(m_lecteur.getSymbole()!="<VARIABLE>" && m_lecteur.getSymbole()!="si" && m_lecteur.getSymbole()!="tantque" && m_lecteur.getSymbole()!="repeter" && m_lecteur.getSymbole()!="pour" && m_lecteur.getSymbole()!="ecrire" && m_lecteur.getSymbole()!="lire") {
	m_lecteur.avancer();
	}
	m_arbre=nullptr;
}
}

Noeud* Interpreteur::affectation() {
  // <affectation> ::= <variable> = <expression> 
  tester("<VARIABLE>");
  Noeud* var = m_table.chercheAjoute(m_lecteur.getSymbole()); // La variable est ajoutée à la table eton la mémorise
  m_lecteur.avancer();
  testerEtAvancer("=");
  Noeud* exp = expression();             // On mémorise l'expression trouvée
  return new NoeudAffectation(var, exp); // On renvoie un noeud affectation
}


Noeud* Interpreteur::relation() {
	// <relation> ::= <expression> { <opRel> <expression> }
	Noeud* expr = expression();
	while  (m_lecteur.getSymbole() == "=="   || m_lecteur.getSymbole() == "!=" ||
			m_lecteur.getSymbole() == "<"    || m_lecteur.getSymbole() == "<=" ||
			m_lecteur.getSymbole() == ">"    || m_lecteur.getSymbole() == ">=") {
		Symbole opRel = m_lecteur.getSymbole();
		m_lecteur.avancer();
		Noeud* factDroit = expression();
		expr = new NoeudOperateurBinaire(opRel,expr,factDroit);
	}
	return expr;
}

Noeud* Interpreteur::relationET() {
	// <relationET> ::= <relation> { et <relation> }
	Noeud* rlt = relation();
	while (m_lecteur.getSymbole()=="et") {
		Symbole et = m_lecteur.getSymbole();
		m_lecteur.avancer();
		Noeud* factDroit = relation();
		rlt = new NoeudOperateurBinaire(et,rlt,factDroit);
	}
	return rlt;
}

Noeud* Interpreteur::expBool() {
	// <expBool> ::= <relationET> { ou <relationET> }
	Noeud* rltET = relationET();
	while (m_lecteur.getSymbole()=="ou") {
		Symbole ou = m_lecteur.getSymbole();
		m_lecteur.avancer();
		Noeud* factDroit = relationET();
		rltET = new NoeudOperateurBinaire(ou,rltET,factDroit);
	}
	return rltET;
}

Noeud* Interpreteur::terme() {
	// <terme> ::= <facteur> { * <facteur> | / <facteur> }
	Noeud* fact = facteur();
	while (m_lecteur.getSymbole()=="*" || m_lecteur.getSymbole()=="/") {
		Symbole operateur = m_lecteur.getSymbole();
		m_lecteur.avancer();
		Noeud* factDroit = facteur();
		fact = new NoeudOperateurBinaire(operateur,fact,factDroit);
	}
	return fact;
}

Noeud* Interpreteur::expression() {
  // <expression> ::= <terme> { + <terme> | - <terme> }
  Noeud* trm = terme();
  while (m_lecteur.getSymbole()=="+" || m_lecteur.getSymbole()=="-") {
	  Symbole operateur = m_lecteur.getSymbole();
	  m_lecteur.avancer();
	  Noeud* factDroit = terme();
	  trm = new NoeudOperateurBinaire(operateur,trm,factDroit);
  }
  return trm;

}

Noeud* Interpreteur::facteur() {
  // <facteur> ::= <entier> | <variable> | - <expBool> | non <expBool> | ( <expBool>)
  // avant <facteur> ::= <entier> | <variable> | - <facteur> | non <facteur> | ( <expression> )
  Noeud* fact = nullptr;
  if (m_lecteur.getSymbole() == "<VARIABLE>" || m_lecteur.getSymbole() == "<ENTIER>") {
    fact = m_table.chercheAjoute(m_lecteur.getSymbole()); // on ajoute la variable ou l'entier à la table
    m_lecteur.avancer();
  } else if (m_lecteur.getSymbole() == "-") { // - <expBool>
    m_lecteur.avancer();
    // on représente le moins unaire (- expBool) par une soustraction binaire (0 - expBool)
    fact = new NoeudOperateurBinaire(Symbole("-"), m_table.chercheAjoute(Symbole("0")), expBool());
  } else if (m_lecteur.getSymbole() == "non") { // non <facteur>
    m_lecteur.avancer();
    // on représente le moins unaire (- expBool) par une soustractin binaire (0 - expBool)
    fact = new NoeudOperateurBinaire(Symbole("non"), expBool(), nullptr);
  } else if (m_lecteur.getSymbole() == "(") { // expression parenthésée
    m_lecteur.avancer();
    fact = expBool();
    testerEtAvancer(")");
  } else
    erreur("Facteur incorrect");
  return fact;
}

Noeud* Interpreteur::instSi() throw (SyntaxeException){
  // <instSi> ::= si ( <expression> ) <seqInst> { sinonsi ( <expression> ) <seqInst> } [sinon <seqInst>] finsi
  try {
  testerEtAvancer("si");
  testerEtAvancer("(");
  Noeud* condition = expBool(); // On mémorise la condition
  testerEtAvancer(")");
  Noeud* sequence = seqInst();     // On mémorise la séquence d'instruction
  NoeudInstSi * instSi = new NoeudInstSi(condition,sequence);
  while(m_lecteur.getSymbole() != "finsi"){
	  if(m_lecteur.getSymbole()=="sinonsi"){
		  testerEtAvancer("sinonsi");
		  testerEtAvancer("(");
		  Noeud* condition = expBool();
		  testerEtAvancer(")");
		  Noeud* sequence = seqInst();
		  instSi->ajouterSinonsi(new NoeudInstSinonsi(condition,sequence));
	  } else if(m_lecteur.getSymbole()=="sinon"){
		  testerEtAvancer("sinon");
		  Noeud* sequence = seqInst();
		  instSi->ajouterSinon(sequence);
	  } else {
		  throw new SyntaxeException("Erreur : Sinonsi, sinon attendu");
	  }
  }
  testerEtAvancer("finsi");
  return instSi;
} catch (SyntaxeException & e) {
	cout << e. what() << endl;
	while(m_lecteur.getSymbole()!="<VARIABLE>" && m_lecteur.getSymbole()!="si" && m_lecteur.getSymbole()!="tantque" && m_lecteur.getSymbole()!="repeter" && m_lecteur.getSymbole()!="pour" && m_lecteur.getSymbole()!="ecrire" && m_lecteur.getSymbole()!="lire") {
	m_lecteur.avancer();
	}
	m_arbre=nullptr;
	}
}

Noeud* Interpreteur::instTantQue() {
	// <instTantQue> ::= tantque ( <expression> ) <seqInst> fintantque
	try {
	testerEtAvancer("tantque");
	testerEtAvancer("(");
	Noeud* condition = expBool();// On mémorise la condition
	testerEtAvancer(")");
	Noeud* sequence = seqInst();	// On mémorise la séquence d'instruction
	testerEtAvancer("fintantque");
	return new NoeudInstTantQue(condition, sequence);	// Et on renvoie un noeud Instruction TantQue
	} catch (SyntaxeException & e) {
		cout << e.what() << endl;
		while(m_lecteur.getSymbole()!="<VARIABLE>" && m_lecteur.getSymbole()!="si" && m_lecteur.getSymbole()!="tantque" && m_lecteur.getSymbole()!="repeter" && m_lecteur.getSymbole()!="pour" && m_lecteur.getSymbole()!="ecrire" && m_lecteur.getSymbole()!="lire") {
		m_lecteur.avancer();
			}
		m_arbre=nullptr;
	}
}

Noeud* Interpreteur::instRepeter() {
	// <instRepeter> ::= repeter <seqInst> jusqua ( <expression> )
	try {
	testerEtAvancer("repeter");
	Noeud* sequence = seqInst();
	testerEtAvancer("jusqua");
	testerEtAvancer("(");
	Noeud* condition = expBool();// On mémorise la condition
	testerEtAvancer(")");
	return new NoeudInstRepeter(condition,sequence);
	} catch (SyntaxeException & e) {
		cout << e.what() << endl;
		while(m_lecteur.getSymbole()!="<VARIABLE>" && m_lecteur.getSymbole()!="si" && m_lecteur.getSymbole()!="tantque" && m_lecteur.getSymbole()!="repeter" && m_lecteur.getSymbole()!="pour" && m_lecteur.getSymbole()!="ecrire" && m_lecteur.getSymbole()!="lire") {
		m_lecteur.avancer();
		}
		m_arbre=nullptr;
	}
}

Noeud* Interpreteur::instPour() {
	// <instPour> ::= pour ( [ 	<affectation> 	]	 ;	 <expression> 	;	[ 	<affectation> 	]	) <seqInst>	 finpour
	try {
	testerEtAvancer("pour");
	testerEtAvancer("(");
	Noeud * affect = nullptr;
	if(m_lecteur.getSymbole()!=";")
		affect = affectation();	// On mémorise l'affectation s'il y en a une
	testerEtAvancer(";");
	Noeud* condition = expBool();	// On mémorise la condition
	testerEtAvancer(";");
	Noeud* affect2 = nullptr;
	if(m_lecteur.getSymbole()!=")")
		affect2 = affectation();	// On mémorise l'affectation2 s'il y en a une
	testerEtAvancer(")");
	Noeud* sequence = seqInst();		// On mémorise la séquence d'instruction
	testerEtAvancer("finpour");
	return new NoeudInstPour(condition,sequence,affect,affect2);
	} catch (SyntaxeException & e) {
		cout << e.what() << endl;
		while(m_lecteur.getSymbole()!="<VARIABLE>" && m_lecteur.getSymbole()!="si" && m_lecteur.getSymbole()!="tantque" && m_lecteur.getSymbole()!="repeter" && m_lecteur.getSymbole()!="pour" && m_lecteur.getSymbole()!="ecrire" && m_lecteur.getSymbole()!="lire") {
		m_lecteur.avancer();
		}
		m_arbre=nullptr;
	}
}

Noeud* Interpreteur::instEcrire() {
	// <instEcrire> ::= ecrire ( <expression> | <chaine> | { , <expression> | <chaine> } )
	try {
	testerEtAvancer("ecrire");
	testerEtAvancer("(");
	NoeudInstEcrire * final = new NoeudInstEcrire;
	while(m_lecteur.getSymbole()!=")"){
		if(m_lecteur.getSymbole()=="<CHAINE>"){
			final->ajouterNoeud(new NoeudChaine(m_lecteur.getSymbole().getChaine()));
			m_lecteur.avancer();
		} else if (m_lecteur.getSymbole()=="finligne"){
			final->ajouterNoeud(new NoeudChaine("finligne"));
			m_lecteur.avancer();
		} else if (m_lecteur.getSymbole()!=","){
			Noeud* valeur = expression();
			final->ajouterNoeud(valeur);
		} else {
			m_lecteur.avancer();
		}
	}
	testerEtAvancer(")");
	return final;
	} catch (SyntaxeException & e) {
		cout << e.what() << endl;
		while(m_lecteur.getSymbole()!="<VARIABLE>" && m_lecteur.getSymbole()!="si" && m_lecteur.getSymbole()!="tantque" && m_lecteur.getSymbole()!="repeter" && m_lecteur.getSymbole()!="pour" && m_lecteur.getSymbole()!="ecrire" && m_lecteur.getSymbole()!="lire") {
		m_lecteur.avancer();
		}
		m_arbre=nullptr;
	}
}

Noeud* Interpreteur::instLire() {
	//  <instLire> ::= lire ( <variable> { , <variable> } )
	try {
	testerEtAvancer("lire");
	testerEtAvancer("(");
	NoeudInstLire* instLire = new NoeudInstLire;
	while(m_lecteur.getSymbole()!=")"){
		if(m_lecteur.getSymbole()=="<VARIABLE>"){
			Noeud* var = m_table.chercheAjoute(m_lecteur.getSymbole());
			instLire->ajouter(var);
		}
		m_lecteur.avancer();
	}
	testerEtAvancer(")");
	return instLire;
	} catch (SyntaxeException & e) {
		cout << e.what() << endl;
		while(m_lecteur.getSymbole()!="<VARIABLE>" && m_lecteur.getSymbole()!="si" && m_lecteur.getSymbole()!="tantque" && m_lecteur.getSymbole()!="repeter" && m_lecteur.getSymbole()!="pour" && m_lecteur.getSymbole()!="ecrire" && m_lecteur.getSymbole()!="lire") {
		m_lecteur.avancer();
		}
		m_arbre=nullptr;
	}
}
void Interpreteur::traduitEnCPP(ostream & cout, unsigned int indentation) const {
	cout << setw(4*indentation) << "" << "int main() {" << endl; // Début d'un programme C++
	// Ecrire en C++ la déclaration des variables présentes dans le programme...
	// ... variables dont on retrouvera la nom en parcourant la table des symboles !
	// Par exemple, si le programme contient i,j,k, il faudra écrire : int i; int j; int k; ...
	getArbre()->traduitEnCPP(cout, indentation+1); // lance l'opération traduitEnCPP sur la racine
	cout << setw(4*(indentation+1))<< ""<< "return 0;" << endl;
	cout << setw(4*indentation) << "" << "}" << endl; // Fin d'un programme C++
}


