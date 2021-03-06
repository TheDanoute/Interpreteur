#ifndef ARBREABSTRAIT_H
#define ARBREABSTRAIT_H

// Contient toutes les déclarations de classes nécessaires
//  pour représenter l'arbre abstrait

#include <vector>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <map>

using namespace std;

#include "Symbole.h"
#include "Exceptions.h"

////////////////////////////////////////////////////////////////////////////////
class Noeud {
// Classe abstraite dont dériveront toutes les classes servant à représenter l'arbre abstrait
// Remarque : la classe ne contient aucun constructeur
  public:
    virtual int  executer() =0 ; // Méthode pure (non implémentée) qui rend la classe abstraite
    virtual void ajoute(Noeud* instruction) { throw OperationInterditeException(); }
    virtual ~Noeud() {} // Présence d'un destructeur virtuel conseillée dans les classes abstraites
    virtual void traduitEnCPP(ofstream & fichier,unsigned int indentation)const=0;
};
////////////////////////////////////////////////////////////////////////////////
class NoeudProcedures : public Noeud {
public:
	NoeudProcedures();
	~NoeudProcedures() {}
	void traduitEnCPP(ofstream & ficher,unsigned int indentation)const;
	void ajoute(string nom,Noeud * procedure);
	inline map<string,Noeud *> * getPointer(){return &m_procedures;}
	void procedurePrincipale(Noeud * procedure);
	int executer();
private:
	Noeud* m_procedurePrincipale;
	map<string,Noeud *> m_procedures;
};

////////////////////////////////////////////////////////////////////////////////
class NoeudSeqInst : public Noeud {
// Classe pour représenter un noeud "sequence d'instruction"
//  qui a autant de fils que d'instructions dans la séquence
  public:
     NoeudSeqInst();   // Construit une séquence d'instruction vide
    ~NoeudSeqInst() {} // A cause du destructeur virtuel de la classe Noeud
    int executer();    // Exécute chaque instruction de la séquence
    void ajoute(Noeud* instruction);  // Ajoute une instruction à la séquence
    void traduitEnCPP(ofstream & ficher,unsigned int indentation)const;
  private:
    vector<Noeud *> m_instructions; // pour stocker les instructions de la séquence
};

////////////////////////////////////////////////////////////////////////////////
class NoeudAffectation : public Noeud {
// Classe pour représenter un noeud "affectation"
//  composé de 2 fils : la variable et l'expression qu'on lui affecte
  public:
     NoeudAffectation(Noeud* variable, Noeud* expression); // construit une affectation
    ~NoeudAffectation() {} // A cause du destructeur virtuel de la classe Noeud
    int executer();        // Exécute (évalue) l'expression et affecte sa valeur à la variable
    void traduitEnCPP(ofstream & ficher,unsigned int indentation)const;
    void traduitEnCPPFor(ofstream & ficher,unsigned int indentation)const;

  private:
    Noeud* m_variable;
    Noeud* m_expression;
};

////////////////////////////////////////////////////////////////////////////////
class NoeudOperateurBinaire : public Noeud {
// Classe pour représenter un noeud "opération binaire" composé d'un opérateur
//  et de 2 fils : l'opérande gauche et l'opérande droit
  public:
    NoeudOperateurBinaire(Symbole operateur, Noeud* operandeGauche, Noeud* operandeDroit);
    // Construit une opération binaire : operandeGauche operateur OperandeDroit
   ~NoeudOperateurBinaire() {} // A cause du destructeur virtuel de la classe Noeud
    int executer();            // Exécute (évalue) l'opération binaire)
    void traduitEnCPP(ofstream & ficher,unsigned int indentation)const;

  private:
    Symbole m_operateur;
    Noeud*  m_operandeGauche;
    Noeud*  m_operandeDroit;
};

////////////////////////////////////////////////////////////////////////////////
class NoeudInstSi : public Noeud {
// Classe pour représenter un noeud "instruction si"
//  et ses 2 fils : la condition du si et la séquence d'instruction associée
  public:
    NoeudInstSi(Noeud* condition, Noeud* sequence);
    void ajouterSinonsi(Noeud * instSinonsi);
    void ajouterSinon(Noeud * sequence);
     // Construit une "instruction si" avec sa condition et sa séquence d'instruction
   ~NoeudInstSi() {} // A cause du destructeur virtuel de la classe Noeud
    int executer();  // Exécute l'instruction si : si condition vraie on exécute la séquence
    void traduitEnCPP(ofstream & ficher,unsigned int indentation)const;

  private:
    Noeud*  m_condition;
    Noeud*  m_sequence;
    vector<Noeud*>m_sinonsi;
    Noeud* m_sinon;
};

class NoeudInstSinonsi : public Noeud {
// Classe pour représenter un noeud "instruction si"
//  et ses 2 fils : la condition du si et la séquence d'instruction associée
  public:
    NoeudInstSinonsi(Noeud* condition, Noeud* sequence);
     // Construit une "instruction si" avec sa condition et sa séquence d'instruction
   ~NoeudInstSinonsi() {} // A cause du destructeur virtuel de la classe Noeud
    int executer();  // Exécute l'instruction si : si condition vraie on exécute la séquence
    void traduitEnCPP(ofstream & ficher,unsigned int indentation)const;

  private:
    Noeud*  m_condition;
    Noeud*  m_sequence;
};

////////////////////////////////////////////////////////////////////////////////
class NoeudInstTantQue : public Noeud {
// Classe pour représenter un noeud "instruction tantque"
// et ses 2 fils : la condition du tanque et la séquence d'instruction associée
public:
	NoeudInstTantQue(Noeud* condition, Noeud* sequence);
	// Construit une "instruction tantque" avec sa condition et sa séquence d'instruction
	~NoeudInstTantQue() {}// A cause du destructeur virtuel de la classe Noeud
	int executer(); // Exécute l'instruction tantque : si condition vraie on exécute la séquence
	void traduitEnCPP(ofstream & ficher,unsigned int indentation)const;

private:
	Noeud* m_condition;
	Noeud* m_sequence;
};

////////////////////////////////////////////////////////////////////////////////
class NoeudInstPour : public Noeud {
// Classe pour représenter un noeud "instruction pour"
// et ses potentiels 4 fils : la condition du tanque et la séquence d'instruction associée et les 2 affectations potentielles
public:
	NoeudInstPour(Noeud* condition, Noeud* sequence, Noeud* affect = nullptr, Noeud* affect2 = nullptr);
	// Construit une "instruction pour" avec sa condition et sa séquence d'instruction et ses 2 affectations potentielles
	~NoeudInstPour() {}	// A cause du destruction virtuel de la classe Noeud
	int executer();	// Exécute l'instruction pour : si condition vraie on exécute la séquence
	void traduitEnCPP(ofstream & ficher,unsigned int indentation)const;

private:
	Noeud* m_condition;
	Noeud* m_sequence;
	Noeud* m_affect;
	Noeud* m_affect2 ;
};

class NoeudInstRepeter : public Noeud {
// Classe pour représenter un noeud "instruction tantque"
// et ses 2 fils : la condition du tanque et la séquence d'instruction associée
public:
	NoeudInstRepeter(Noeud* condition, Noeud* sequence);
	// Construit une "instruction tantque" avec sa condition et sa séquence d'instruction
	~NoeudInstRepeter() {}// A cause du destructeur virtuel de la classe Noeud
	int executer(); // Exécute l'instruction tantque : si condition vraie on exécute la séquence
	void traduitEnCPP(ofstream & ficher,unsigned int indentation)const;

private:
	Noeud* m_condition;
	Noeud* m_sequence;
};

class NoeudInstEcrire : public Noeud {
// Classe pour représenter un noeud "instruction tantque"
// et ses 2 fils : la condition du tanque et la séquence d'instruction associée
public:
	NoeudInstEcrire();
	// Construit une "instruction tantque" avec sa condition et sa séquence d'instruction
	~NoeudInstEcrire() {}// A cause du destructeur virtuel de la classe Noeud
	void ajouterNoeud(Noeud * n);
	int executer(); // Exécute l'instruction tantque : si condition vraie on exécute la séquence
	void traduitEnCPP(ofstream & ficher,unsigned int indentation)const;

private:
	vector<Noeud *>m_elements;
};

class NoeudInstLire : public Noeud {
// Classe pour représenter un noeud "instruction tantque"
// et ses 2 fils : la condition du tanque et la séquence d'instruction associée
public:
	NoeudInstLire();
	// Construit une "instruction tantque" avec sa condition et sa séquence d'instruction
	~NoeudInstLire() {}// A cause du destructeur virtuel de la classe Noeud
	void ajouter(Noeud* variable);
	int executer(); // Exécute l'instruction tantque : si condition vraie on exécute la séquence
	void traduitEnCPP(ofstream & ficher,unsigned int indentation)const;

private:
	vector<Noeud*>m_variables;
};

class NoeudChaine : public Noeud {
public:
	NoeudChaine(string chaine);
	// Construit une "instruction tantque" avec sa condition et sa séquence d'instruction
	~NoeudChaine() {}// A cause du destructeur virtuel de la classe Noeud
	int executer(); // Exécute l'instruction tantque : si condition vraie on exécute la séquence
	void traduitEnCPP(ofstream & fichier,unsigned int indentation)const;

private:
	string m_chaine;
};


#endif /* ARBREABSTRAIT_H */
