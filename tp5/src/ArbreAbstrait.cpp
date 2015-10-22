#include <stdlib.h>
#include "ArbreAbstrait.h"
#include "Symbole.h"
#include "SymboleValue.h"
#include "Exceptions.h"
#include <typeinfo>

////////////////////////////////////////////////////////////////////////////////
// NoeudSeqInst
////////////////////////////////////////////////////////////////////////////////

NoeudSeqInst::NoeudSeqInst() : m_instructions() {
}

int NoeudSeqInst::executer() {
  for (unsigned int i = 0; i < m_instructions.size(); i++)
    m_instructions[i]->executer(); // on exécute chaque instruction de la séquence
  return 0; // La valeur renvoyée ne représente rien !
}

void NoeudSeqInst::ajoute(Noeud* instruction) {
  if (instruction!=nullptr) m_instructions.push_back(instruction);
}

void NoeudSeqInst::traduitEnCPP(ostream & cout,unsigned int indentation)const{
	for (unsigned int i = 0; i < m_instructions.size(); i++){
		cout << setw(4*indentation) << "";
	    m_instructions[i]->traduitEnCPP(cout,indentation);
	}
}
////////////////////////////////////////////////////////////////////////////////
// NoeudAffectation
////////////////////////////////////////////////////////////////////////////////

NoeudAffectation::NoeudAffectation(Noeud* variable, Noeud* expression)
: m_variable(variable), m_expression(expression) {
}

int NoeudAffectation::executer() {
  int valeur = m_expression->executer(); // On exécute (évalue) l'expression
  ((SymboleValue*) m_variable)->setValeur(valeur); // On affecte la variable
  return 0; // La valeur renvoyée ne représente rien !
}

void NoeudAffectation::traduitEnCPP(ostream & cout,unsigned int indentation)const{
	if(!((SymboleValue *)m_variable)->setDefiniTCPP())
		cout << "int ";
	m_variable->traduitEnCPP(cout,indentation);
	cout << "=";
	m_expression->traduitEnCPP(cout,indentation);
	cout << ";" << endl;
}

////////////////////////////////////////////////////////////////////////////////
// NoeudOperateurBinaire
////////////////////////////////////////////////////////////////////////////////

NoeudOperateurBinaire::NoeudOperateurBinaire(Symbole operateur, Noeud* operandeGauche, Noeud* operandeDroit)
: m_operateur(operateur), m_operandeGauche(operandeGauche), m_operandeDroit(operandeDroit) {
}

int NoeudOperateurBinaire::executer() {
  int og, od, valeur;
  if (m_operandeGauche != nullptr) og = m_operandeGauche->executer(); // On évalue l'opérande gauche
  if (m_operandeDroit != nullptr) od = m_operandeDroit->executer(); // On évalue l'opérande droit
  // Et on combine les deux opérandes en fonctions de l'opérateur
  if (this->m_operateur == "+") valeur = (og + od);
  else if (this->m_operateur == "-") valeur = (og - od);
  else if (this->m_operateur == "*") valeur = (og * od);
  else if (this->m_operateur == "==") valeur = (og == od);
  else if (this->m_operateur == "!=") valeur = (og != od);
  else if (this->m_operateur == "<") valeur = (og < od);
  else if (this->m_operateur == ">") valeur = (og > od);
  else if (this->m_operateur == "<=") valeur = (og <= od);
  else if (this->m_operateur == ">=") valeur = (og >= od);
  else if (this->m_operateur == "et") valeur = (og && od);
  else if (this->m_operateur == "ou") valeur = (og || od);
  else if (this->m_operateur == "non") valeur = (!og);
  else if (this->m_operateur == "/") {
    if (od == 0) throw DivParZeroException();
    valeur = og / od;
  }
  return valeur; // On retourne la valeur calculée
}

void NoeudOperateurBinaire::traduitEnCPP(ostream & cout,unsigned int indentation)const{
	if (m_operandeGauche != nullptr)
		m_operandeGauche->traduitEnCPP(cout,indentation);
	cout << m_operateur.getChaine();
	if (m_operandeDroit != nullptr)
		m_operandeDroit->traduitEnCPP(cout,indentation);
}

////////////////////////////////////////////////////////////////////////////////
// NoeudInstSi
////////////////////////////////////////////////////////////////////////////////

NoeudInstSinonsi::NoeudInstSinonsi(Noeud* condition, Noeud* sequence)
: m_condition(condition), m_sequence(sequence) {
}

int NoeudInstSinonsi::executer(){
	int final=0;
	if (m_condition->executer()) {
		final=1;
		m_sequence->executer();
	}
	return final;
}

void NoeudInstSinonsi::traduitEnCPP(ostream & cout,unsigned int indentation)const{
	cout << "else if ( " ;
	m_condition->traduitEnCPP(cout,indentation);
	cout << " ) {" << endl;
	m_sequence->traduitEnCPP(cout,indentation+1);
	cout << setw(4*indentation) << "" << "} " ;
}

NoeudInstSi::NoeudInstSi(Noeud* condition, Noeud* sequence)
: m_condition(condition), m_sequence(sequence) {
	m_sinon=nullptr;
}
void NoeudInstSi::ajouterSinonsi(Noeud * instSinonsi){
	m_sinonsi.push_back(instSinonsi);
}
void NoeudInstSi::ajouterSinon(Noeud * sequence){
	m_sinon=sequence;
}

int NoeudInstSi::executer() {
  if (m_condition->executer())
	  m_sequence->executer();
  else {
	  int condition = 0;
	  for(auto instSinonSi : m_sinonsi){
		  if (condition<1)
			  condition+=instSinonSi->executer();
	  }
	  if(condition < 1 && m_sinon!=nullptr)
		  m_sinon->executer();
	  }
  return 0; // La valeur renvoyée ne représente rien !
}

void NoeudInstSi::traduitEnCPP(ostream & cout,unsigned int indentation)const{
	cout << setw(4*indentation) << "" << "if (";
	m_condition->traduitEnCPP(cout,indentation);
	cout << ") {" << endl;
	m_sequence->traduitEnCPP(cout,indentation+1);
	cout << setw(4*indentation) << "" << "} " ;
	if (m_sinonsi.size()!=0)
		for(auto instSinonSi : m_sinonsi)
			instSinonSi->traduitEnCPP(cout,indentation);
	if (m_sinon!=nullptr){
		cout << "else {" << endl;
		m_sinon->traduitEnCPP(cout,indentation+1);
		cout << setw(4*indentation) << "" <<"}" << endl;
	}
}
////////////////////////////////////////////////////////////////////////////////
// NoeudInstTantQue
////////////////////////////////////////////////////////////////////////////////

NoeudInstTantQue::NoeudInstTantQue(Noeud* condition, Noeud* sequence)
: m_condition(condition), m_sequence(sequence) {
}

int NoeudInstTantQue::executer() {
	while (m_condition->executer()) m_sequence->executer();
	return 0; // La valeur renvoyée ne représente rien !
}

void NoeudInstTantQue::traduitEnCPP(ostream & cout,unsigned int indentation)const{
	cout << setw(4*indentation) << "" << "while (" ;
	m_condition->traduitEnCPP(cout,indentation);
	cout << ") {" << endl;
	m_sequence->traduitEnCPP(cout,indentation+1);
	cout << setw(4*indentation) << "" << "}" << endl;
}
///////////////////////////////////////////////////////////////////////////////
// NoeudInstPour
////////////////////////////////////////////////////////////////////////////////

NoeudInstPour::NoeudInstPour(Noeud* condition, Noeud* sequence, Noeud* affect, Noeud* affect2)
: m_condition(condition), m_sequence(sequence), m_affect(affect), m_affect2(affect2) {

}

int NoeudInstPour::executer() {
	if (m_affect!=nullptr)
		m_affect->executer();
	while (m_condition->executer()) {
		m_sequence->executer();
	if(m_affect2!=nullptr)
		m_affect2->executer();
	}
	return 0; // La valeur renvoyée ne représente rien !
}

void NoeudInstPour::traduitEnCPP(ostream & cout,unsigned int indentation)const{
	cout << setw(4*indentation) << "" << "for( ";
	if (m_affect!=nullptr)
		m_affect->traduitEnCPP(cout,indentation);
	cout << ";" ;
	m_condition->traduitEnCPP(cout,indentation);
	cout << ";" ;
	if(m_affect2!=nullptr)
		m_affect2->traduitEnCPP(cout,indentation);
	cout << ") {" << endl;
	m_sequence->traduitEnCPP(cout,indentation);
	cout << setw(4*indentation) << "" << "}" << endl;
}
///////////////////////////////////////////////////////////////////////////////
// NoeudInstRepeter
////////////////////////////////////////////////////////////////////////////////

NoeudInstRepeter::NoeudInstRepeter(Noeud* condition, Noeud* sequence)
: m_condition(condition), m_sequence(sequence) {
}

int NoeudInstRepeter::executer() {
	do {
		m_sequence->executer();
	}
	while (!m_condition->executer());
	return 0; // La valeur renvoyée ne représente rien !
}

void  NoeudInstRepeter::traduitEnCPP(ostream & cout,unsigned int indentation)const{
	cout << setw(4*indentation) << "" << "do {" << endl;
	m_sequence->traduitEnCPP(cout,indentation);
	cout << setw(4*indentation) << "" << "}" << endl << setw(4*indentation) << "while (!( ";
	m_condition->traduitEnCPP(cout,indentation);
	cout << " ));" << endl << setw(4*indentation) << "" << "}" << endl;
}

///////////////////////////////////////////////////////////////////////////////
// NoeudInstEcrire
////////////////////////////////////////////////////////////////////////////////

NoeudInstEcrire::NoeudInstEcrire(){
}

void NoeudInstEcrire::ajouterNoeud(Noeud* variable){
	m_elements.push_back(variable);
}

int NoeudInstEcrire::executer(){
	for(auto elem : m_elements){
		if(typeid(*elem)==typeid(NoeudChaine)){
			elem->executer();
		} else {
			cout << elem->executer();
		}
	}
	return 0;
}

void NoeudInstEcrire::traduitEnCPP(ostream & cout,unsigned int indentation)const{
	cout << setw(4*indentation) << "" << "cout";
	for(auto elem : m_elements){
		cout << " << " ;
		elem->traduitEnCPP(cout,indentation);
	}
	cout << " ;" << endl;
}

///////////////////////////////////////////////////////////////////////////////
// NoeudInstLire
////////////////////////////////////////////////////////////////////////////////

NoeudInstLire::NoeudInstLire(){
}

void NoeudInstLire::ajouter(Noeud* variable){
	m_variables.push_back(variable);
}
int NoeudInstLire::executer(){
	for(auto variable : m_variables){
		int temp;
		cin >> temp;
		((SymboleValue*) variable)->setValeur(temp);
	}
	return 0;
}

void NoeudInstLire::traduitEnCPP(ostream & cout,unsigned int indentation)const{
	for(auto variable : m_variables){
		if(!((SymboleValue *)variable)->setDefiniTCPP())
			cout << "int " << ((SymboleValue *)variable)->getChaine() << ";" << endl ;
	}
	cout << setw(4*indentation) << "" <<"cin";
	for(auto variable : m_variables){
		cout << " >> ";
		variable->traduitEnCPP(cout,indentation);
	}
	cout << " ;" << endl;
}

NoeudChaine::NoeudChaine(string chaine) {
	if(chaine!="finligne"){
		chaine.erase(chaine.begin());
		chaine.erase(chaine.end()-1);
	}
	m_chaine=chaine;
}

int NoeudChaine::executer(){
	if (m_chaine=="finligne")
		cout << '\n';
	else cout << m_chaine ;// Exécute l'instruction tantque : si condition vraie on exécute la séquence
	return 0;
}

void NoeudChaine::traduitEnCPP(ostream & cout,unsigned int indentation)const{
	if (m_chaine=="finligne")
		cout << "endl";
	else cout << m_chaine ;
}
