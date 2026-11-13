// ============================================================
//  Application console - Agence de voyage (Cotonou)
//  Gestion des passagers, vols, reservations, paiements
//  Code compatible C++98 / C++11 / C++17 (tous compilateurs)
// ============================================================
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <cstdlib>
using namespace std;

// ------------------------------------------------------------
//  Fonctions utilitaires de saisie et d'affichage
// ------------------------------------------------------------
string nettoyer(const string& s) {
    string::size_type debut = s.find_first_not_of(" \t\r\n");
    if (debut == string::npos) return "";
    string::size_type fin = s.find_last_not_of(" \t\r\n");
    return s.substr(debut, fin - debut + 1);
}

string enMajuscules(string s) {
    for (string::size_type i = 0; i < s.size(); i++) {
        if (s[i] >= 'a' && s[i] <= 'z') s[i] = (char)(s[i] - 'a' + 'A');
    }
    return s;
}

string lireTexte(const string& message) {
    string ligne;
    while (true) {
        cout << message;
        if (!getline(cin, ligne)) {
            cout << "\nFin de la saisie. Au revoir.\n";
            exit(0);
        }
        ligne = nettoyer(ligne);
        if (!ligne.empty()) return ligne;
        cout << "  [Erreur] Le champ ne peut pas etre vide.\n";
    }
}

int lireEntier(const string& message, int min, int max) {
    while (true) {
        string ligne = lireTexte(message);
        istringstream flux(ligne);
        int valeur;
        char reste;
        if (!(flux >> valeur) || (flux >> reste)) {
            cout << "  [Erreur] Veuillez saisir un nombre entier valide.\n";
        } else if (valeur < min || valeur > max) {
            cout << "  [Erreur] La valeur doit etre comprise entre " << min << " et " << max << ".\n";
        } else {
            return valeur;
        }
    }
}

double lireReel(const string& message, double min) {
    while (true) {
        string ligne = lireTexte(message);
        istringstream flux(ligne);
        double valeur;
        char reste;
        if (!(flux >> valeur) || (flux >> reste)) {
            cout << "  [Erreur] Veuillez saisir un nombre valide.\n";
        } else if (valeur < min) {
            cout << "  [Erreur] La valeur doit etre superieure ou egale a " << min << ".\n";
        } else {
            return valeur;
        }
    }
}

bool estChiffre(char c) { return c >= '0' && c <= '9'; }

bool dateValide(const string& d) {          // format JJ/MM/AAAA
    if (d.size() != 10 || d[2] != '/' || d[5] != '/') return false;
    int positions[8] = {0, 1, 3, 4, 6, 7, 8, 9};
    for (int i = 0; i < 8; i++) {
        if (!estChiffre(d[positions[i]])) return false;
    }
    int jour  = (d[0] - '0') * 10 + (d[1] - '0');
    int mois  = (d[3] - '0') * 10 + (d[4] - '0');
    int annee = (d[6] - '0') * 1000 + (d[7] - '0') * 100 + (d[8] - '0') * 10 + (d[9] - '0');
    if (annee < 2024 || annee > 2100 || mois < 1 || mois > 12 || jour < 1) return false;
    int joursParMois[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    bool bissextile = (annee % 4 == 0 && annee % 100 != 0) || (annee % 400 == 0);
    int maxJours = joursParMois[mois - 1];
    if (mois == 2 && bissextile) maxJours = 29;
    return jour <= maxJours;
}

bool heureValide(const string& h) {         // format HH:MM
    if (h.size() != 5 || h[2] != ':') return false;
    if (!estChiffre(h[0]) || !estChiffre(h[1]) || !estChiffre(h[3]) || !estChiffre(h[4])) return false;
    int heures  = (h[0] - '0') * 10 + (h[1] - '0');
    int minutes = (h[3] - '0') * 10 + (h[4] - '0');
    return heures < 24 && minutes < 60;
}

string lireDate(const string& message) {
    while (true) {
        string s = lireTexte(message);
        if (dateValide(s)) return s;
        cout << "  [Erreur] Date invalide (format attendu : JJ/MM/AAAA).\n";
    }
}

string lireHeure(const string& message) {
    while (true) {
        string s = lireTexte(message);
        if (heureValide(s)) return s;
        cout << "  [Erreur] Heure invalide (format attendu : HH:MM).\n";
    }
}

bool confirmer(const string& message) {
    while (true) {
        string s = enMajuscules(lireTexte(message + " (O/N) : "));
        if (s == "O") return true;
        if (s == "N") return false;
        cout << "  [Erreur] Repondez par O ou N.\n";
    }
}

string formaterMontant(double valeur) {
    ostringstream flux;
    flux << fixed << setprecision(0) << valeur << " FCFA";
    return flux.str();
}

// ------------------------------------------------------------
//  Enumerations
// ------------------------------------------------------------
enum StatutReservation { EN_ATTENTE, VALIDEE, ANNULEE };
enum StatutRemboursement { AUCUN, EN_ATTENTE_REMBOURSEMENT };

string statutReservationEnTexte(StatutReservation s) {
    if (s == EN_ATTENTE) return "EN ATTENTE";
    if (s == VALIDEE) return "VALIDEE";
    return "ANNULEE";
}

string statutRemboursementEnTexte(StatutRemboursement s) {
    if (s == AUCUN) return "Aucun";
    return "En attente de remboursement";
}

string modePaiementEnTexte(int mode) {
    if (mode == 1) return "Especes";
    if (mode == 2) return "Carte bancaire";
    return "Mobile Money";
}

// Declarations anticipees
class Reservation;
class Paiement;
class Vol;

// ------------------------------------------------------------
//  Classe Passager
// ------------------------------------------------------------
class Passager {
private:
    int id;
    string nom;
    string prenom;
    string telephone;
    string passeport;

public:
    Passager(int id, const string& nom, const string& prenom,
             const string& telephone, const string& passeport) {
        this->id = id;
        this->nom = nom;
        this->prenom = prenom;
        this->telephone = telephone;
        this->passeport = passeport;
    }

    int getId() const { return id; }
    string getNom() const { return nom; }
    string getPrenom() const { return prenom; }
    string getTelephone() const { return telephone; }
    string getPasseport() const { return passeport; }
    string getNomComplet() const { return prenom + " " + nom; }

    void afficher() const {
        cout << "  Passager #" << id << " : " << getNomComplet()
             << " | Tel : " << telephone
             << " | Passeport : " << passeport << endl;
    }
};

// ------------------------------------------------------------
//  Classe Reservation
// ------------------------------------------------------------
class Reservation {
private:
    int id;
    Passager* passager;      // association : 1 passager
    Vol* vol;                // association : 1 vol
    Paiement* paiement;      // association : 0..1 paiement
    StatutReservation statut;

public:
    Reservation(int id, Passager* passager, Vol* vol) {
        this->id = id;
        this->passager = passager;
        this->vol = vol;
        this->paiement = NULL;
        this->statut = EN_ATTENTE;
    }

    int getId() const { return id; }
    Passager* getPassager() const { return passager; }
    Vol* getVol() const { return vol; }
    Paiement* getPaiement() const { return paiement; }
    StatutReservation getStatut() const { return statut; }

    bool estAnnulee() const { return statut == ANNULEE; }
    bool estValidee() const { return statut == VALIDEE; }
    bool estPayee() const { return paiement != NULL; }

    void setPaiement(Paiement* p) { paiement = p; }
    void valider() { statut = VALIDEE; }
    void annuler() { statut = ANNULEE; }

    void afficher() const;   // definie plus bas
};

// ------------------------------------------------------------
//  Classe Vol
// ------------------------------------------------------------
class Vol {
private:
    string numero;
    string depart;
    string destination;
    string date;
    string heure;
    int capacite;
    double prix;
    vector<Reservation*> reservations;   // association : 0..* reservations

public:
    Vol(const string& numero, const string& depart, const string& destination,
        const string& date, const string& heure, int capacite, double prix) {
        this->numero = numero;
        this->depart = depart;
        this->destination = destination;
        this->date = date;
        this->heure = heure;
        this->capacite = capacite;
        this->prix = prix;
    }

    string getNumero() const { return numero; }
    string getDepart() const { return depart; }
    string getDestination() const { return destination; }
    string getDate() const { return date; }
    string getHeure() const { return heure; }
    int getCapacite() const { return capacite; }
    double getPrix() const { return prix; }

    void ajouterReservation(Reservation* r) { reservations.push_back(r); }

    int placesReservees() const;                       // reservations non annulees
    int placesConfirmees() const;                      // reservations validees
    int placesDisponibles() const { return capacite - placesReservees(); }
    bool estComplet() const { return placesDisponibles() <= 0; }
    bool estReservePar(const Passager* p) const;
    void afficher() const;
};

// ------------------------------------------------------------
//  Classe Paiement
// ------------------------------------------------------------
class Paiement {
private:
    int id;
    Reservation* reservation;
    double montant;
    int mode;                                  // 1 = especes, 2 = carte, 3 = mobile money
    StatutRemboursement statutRemboursement;
    double montantRembourse;

public:
    Paiement(int id, Reservation* reservation, double montant, int mode) {
        this->id = id;
        this->reservation = reservation;
        this->montant = montant;
        this->mode = mode;
        this->statutRemboursement = AUCUN;
        this->montantRembourse = 0;
    }

    int getId() const { return id; }
    double getMontant() const { return montant; }
    double getMontantRembourse() const { return montantRembourse; }
    StatutRemboursement getStatutRemboursement() const { return statutRemboursement; }

    // Paiement annule : un remboursement partiel est prevu
    void marquerARembourser(double taux) {
        statutRemboursement = EN_ATTENTE_REMBOURSEMENT;
        montantRembourse = montant * taux;
    }

    void afficher() const {
        cout << "    Paiement #" << id << " : " << formaterMontant(montant)
             << " par " << modePaiementEnTexte(mode)
             << " | Remboursement : " << statutRemboursementEnTexte(statutRemboursement);
        if (statutRemboursement != AUCUN) {
            cout << " (" << formaterMontant(montantRembourse) << ")";
        }
        cout << endl;
    }
};

// ------------------------------------------------------------
//  Methodes de Vol et Reservation (definies apres les classes)
// ------------------------------------------------------------
int Vol::placesReservees() const {
    int n = 0;
    for (size_t i = 0; i < reservations.size(); i++) {
        if (!reservations[i]->estAnnulee()) n++;
    }
    return n;
}

int Vol::placesConfirmees() const {
    int n = 0;
    for (size_t i = 0; i < reservations.size(); i++) {
        if (reservations[i]->estValidee()) n++;
    }
    return n;
}

bool Vol::estReservePar(const Passager* p) const {
    for (size_t i = 0; i < reservations.size(); i++) {
        if (!reservations[i]->estAnnulee() && reservations[i]->getPassager() == p) return true;
    }
    return false;
}

void Vol::afficher() const {
    cout << "  " << left << setw(7) << numero
         << setw(11) << depart << "-> "
         << setw(11) << destination
         << date << " " << heure
         << " | " << setw(12) << formaterMontant(prix)
         << "| Places libres : " << placesDisponibles() << "/" << capacite
         << " (confirmees : " << placesConfirmees() << ")" << endl;
}

void Reservation::afficher() const {
    cout << "  Reservation #" << id << " [" << statutReservationEnTexte(statut) << "]" << endl;
    cout << "    Passager : " << passager->getNomComplet()
         << " (passeport " << passager->getPasseport() << ")" << endl;
    cout << "    Vol      : " << vol->getNumero() << " " << vol->getDepart()
         << " -> " << vol->getDestination()
         << " le " << vol->getDate() << " a " << vol->getHeure() << endl;
    cout << "    Prix     : " << formaterMontant(vol->getPrix()) << endl;
    if (paiement != NULL) {
        paiement->afficher();
    } else {
        cout << "    Paiement : aucun" << endl;
    }
}

// ------------------------------------------------------------
//  Classe AgenceVoyage : gere toutes les operations
// ------------------------------------------------------------
const double TAUX_REMBOURSEMENT = 0.90;   // 10 % de frais d'annulation

class AgenceVoyage {
private:
    vector<Passager*> passagers;
    vector<Vol*> vols;
    vector<Reservation*> reservations;
    vector<Paiement*> paiements;
    int prochainIdPassager;
    int prochainIdReservation;
    int prochainIdPaiement;

    // Interdit la copie (l'agence possede des pointeurs)
    AgenceVoyage(const AgenceVoyage&);
    AgenceVoyage& operator=(const AgenceVoyage&);

    Passager* trouverPassager(int id) const {
        for (size_t i = 0; i < passagers.size(); i++)
            if (passagers[i]->getId() == id) return passagers[i];
        return NULL;
    }

    Vol* trouverVol(const string& numero) const {
        string n = enMajuscules(numero);
        for (size_t i = 0; i < vols.size(); i++)
            if (vols[i]->getNumero() == n) return vols[i];
        return NULL;
    }

    Reservation* trouverReservation(int id) const {
        for (size_t i = 0; i < reservations.size(); i++)
            if (reservations[i]->getId() == id) return reservations[i];
        return NULL;
    }

    Passager* choisirPassager() const {
        if (passagers.empty()) {
            cout << "  [Erreur] Aucun passager enregistre. Enregistrez-en un d'abord (option 1)." << endl;
            return NULL;
        }
        int id = lireEntier("ID du passager : ", 1, 1000000);
        Passager* p = trouverPassager(id);
        if (p == NULL) cout << "  [Erreur] Passager introuvable." << endl;
        return p;
    }

    Reservation* choisirReservation() const {
        if (reservations.empty()) {
            cout << "  [Erreur] Aucune reservation enregistree." << endl;
            return NULL;
        }
        int id = lireEntier("Numero de reservation : ", 1, 1000000);
        Reservation* r = trouverReservation(id);
        if (r == NULL) cout << "  [Erreur] Reservation introuvable." << endl;
        return r;
    }

public:
    AgenceVoyage() {
        prochainIdPassager = 1;
        prochainIdReservation = 1;
        prochainIdPaiement = 1;
        // Vols de demonstration
        vols.push_back(new Vol("BJ101", "Cotonou", "Paris",   "15/12/2026", "22:30", 3, 450000));
        vols.push_back(new Vol("BJ202", "Cotonou", "Abidjan", "20/12/2026", "08:15", 2, 150000));
        vols.push_back(new Vol("BJ303", "Cotonou", "Lagos",   "05/01/2027", "14:00", 5, 95000));
    }

    ~AgenceVoyage() {
        for (size_t i = 0; i < paiements.size(); i++) delete paiements[i];
        for (size_t i = 0; i < reservations.size(); i++) delete reservations[i];
        for (size_t i = 0; i < vols.size(); i++) delete vols[i];
        for (size_t i = 0; i < passagers.size(); i++) delete passagers[i];
    }

    // 1. Enregistrer un passager
    void enregistrerPassager() {
        cout << "\n--- Enregistrer un passager ---" << endl;
        string nom = lireTexte("Nom : ");
        string prenom = lireTexte("Prenom : ");
        string tel = lireTexte("Telephone : ");
        string passeport = enMajuscules(lireTexte("Numero de passeport : "));
        for (size_t i = 0; i < passagers.size(); i++) {
            if (passagers[i]->getPasseport() == passeport) {
                cout << "  [Erreur] Un passager avec ce passeport existe deja (ID "
                     << passagers[i]->getId() << ")." << endl;
                return;
            }
        }
        Passager* p = new Passager(prochainIdPassager++, nom, prenom, tel, passeport);
        passagers.push_back(p);
        cout << "  Passager enregistre avec l'ID " << p->getId() << "." << endl;
    }

    // 2. Ajouter un vol
    void ajouterVol() {
        cout << "\n--- Ajouter un vol ---" << endl;
        string numero = enMajuscules(lireTexte("Numero du vol : "));
        if (trouverVol(numero) != NULL) {
            cout << "  [Erreur] Ce numero de vol existe deja." << endl;
            return;
        }
        string depart = lireTexte("Ville de depart : ");
        string destination = lireTexte("Destination : ");
        if (enMajuscules(depart) == enMajuscules(destination)) {
            cout << "  [Erreur] Le depart et la destination doivent etre differents." << endl;
            return;
        }
        string date = lireDate("Date (JJ/MM/AAAA) : ");
        string heure = lireHeure("Heure (HH:MM) : ");
        int capacite = lireEntier("Capacite (1 a 1000) : ", 1, 1000);
        double prix = lireReel("Prix du billet (FCFA) : ", 1);
        vols.push_back(new Vol(numero, depart, destination, date, heure, capacite, prix));
        cout << "  Vol " << numero << " ajoute avec succes." << endl;
    }

    // 3. Afficher les vols disponibles
    void afficherVolsDisponibles() const {
        cout << "\n--- Vols disponibles ---" << endl;
        bool trouve = false;
        for (size_t i = 0; i < vols.size(); i++) {
            if (!vols[i]->estComplet()) {
                vols[i]->afficher();
                trouve = true;
            }
        }
        if (!trouve) cout << "  Aucun vol disponible." << endl;
    }

    // 4. Rechercher un vol
    void rechercherVol() const {
        cout << "\n--- Rechercher un vol ---" << endl;
        cout << "1. Par numero" << endl;
        cout << "2. Par destination" << endl;
        cout << "3. Par ville de depart" << endl;
        int choix = lireEntier("Votre choix : ", 1, 3);
        string cle = enMajuscules(lireTexte("Mot-cle : "));
        bool trouve = false;
        for (size_t i = 0; i < vols.size(); i++) {
            string champ;
            if (choix == 1) champ = enMajuscules(vols[i]->getNumero());
            else if (choix == 2) champ = enMajuscules(vols[i]->getDestination());
            else champ = enMajuscules(vols[i]->getDepart());
            if (champ.find(cle) != string::npos) {
                vols[i]->afficher();
                trouve = true;
            }
        }
        if (!trouve) cout << "  Aucun vol ne correspond a la recherche." << endl;
    }

    // 5. Effectuer une reservation
    void effectuerReservation() {
        cout << "\n--- Effectuer une reservation ---" << endl;
        Passager* p = choisirPassager();                     // Regle 1
        if (p == NULL) return;
        afficherVolsDisponibles();
        Vol* v = trouverVol(lireTexte("Numero du vol : "));
        if (v == NULL) {
            cout << "  [Erreur] Vol introuvable." << endl;
            return;
        }
        if (v->estComplet()) {                               // Regle 2
            cout << "  [Erreur] Ce vol est complet." << endl;
            return;
        }
        if (v->estReservePar(p)) {                           // Regle 3
            cout << "  [Erreur] Ce passager a deja une reservation active sur ce vol." << endl;
            return;
        }
        Reservation* r = new Reservation(prochainIdReservation++, p, v);   // Regle 4
        reservations.push_back(r);
        v->ajouterReservation(r);
        cout << "  Reservation #" << r->getId()
             << " creee (EN ATTENTE). Elle sera validee apres le paiement." << endl;
    }

    // 6. Enregistrer un paiement
    void enregistrerPaiement() {
        cout << "\n--- Enregistrer un paiement ---" << endl;
        Reservation* r = choisirReservation();
        if (r == NULL) return;
        if (r->estAnnulee()) {                               // Regle 5
            cout << "  [Erreur] Reservation annulee : le paiement est impossible." << endl;
            return;
        }
        if (r->estPayee()) {
            cout << "  [Erreur] Cette reservation est deja payee." << endl;
            return;
        }
        double du = r->getVol()->getPrix();
        cout << "  Montant a payer : " << formaterMontant(du) << endl;
        double montant = lireReel("Montant verse (FCFA) : ", 0);
        if (montant != du) {
            cout << "  [Erreur] Le montant doit etre exactement " << formaterMontant(du) << "." << endl;
            return;
        }
        cout << "Mode de paiement : 1. Especes  2. Carte  3. Mobile Money" << endl;
        int mode = lireEntier("Votre choix : ", 1, 3);
        Paiement* pay = new Paiement(prochainIdPaiement++, r, montant, mode);
        paiements.push_back(pay);
        r->setPaiement(pay);
        r->valider();                                        // Regle 6
        cout << "  Paiement enregistre. Reservation #" << r->getId()
             << " VALIDEE : la place est definitivement reservee." << endl;
    }

    // 7. Annuler une reservation
    void annulerReservation() {
        cout << "\n--- Annuler une reservation ---" << endl;
        Reservation* r = choisirReservation();
        if (r == NULL) return;
        if (r->estAnnulee()) {
            cout << "  [Erreur] Cette reservation est deja annulee." << endl;
            return;
        }
        r->afficher();
        if (!confirmer("Confirmer l'annulation")) {
            cout << "  Annulation abandonnee." << endl;
            return;
        }
        r->annuler();
        if (r->estPayee()) {                                 // Regle 7
            r->getPaiement()->marquerARembourser(TAUX_REMBOURSEMENT);
            cout << "  Reservation annulee. Remboursement prevu : "
                 << formaterMontant(r->getPaiement()->getMontantRembourse())
                 << " (" << (int)(TAUX_REMBOURSEMENT * 100) << " %)."
                 << " Statut : " << statutRemboursementEnTexte(r->getPaiement()->getStatutRemboursement())
                 << "." << endl;
        } else {
            cout << "  Reservation annulee (aucun paiement, donc aucun remboursement)." << endl;
        }
    }

    // 8. Consulter les reservations
    void consulterReservations() const {
        cout << "\n--- Liste des reservations ---" << endl;
        if (reservations.empty()) {
            cout << "  Aucune reservation." << endl;
            return;
        }
        for (size_t i = 0; i < reservations.size(); i++) {
            Reservation* r = reservations[i];
            cout << "  #" << setw(3) << r->getId() << " | " << left << setw(22)
                 << r->getPassager()->getNomComplet() << " | " << setw(7)
                 << r->getVol()->getNumero() << " | " << setw(11)
                 << statutReservationEnTexte(r->getStatut()) << " | "
                 << (r->estPayee() ? "Payee" : "Non payee") << endl;
        }
    }

    // 9. Afficher les informations d'une reservation
    void afficherReservation() const {
        cout << "\n--- Informations d'une reservation ---" << endl;
        Reservation* r = choisirReservation();
        if (r != NULL) r->afficher();
    }

    // Menu principal
    void executer() {
        int choix = 0;
        do {
            cout << "\n========== AGENCE DE VOYAGE - COTONOU ==========" << endl;
            cout << " 1. Enregistrer un passager" << endl;
            cout << " 2. Ajouter un vol" << endl;
            cout << " 3. Afficher les vols disponibles" << endl;
            cout << " 4. Rechercher un vol" << endl;
            cout << " 5. Effectuer une reservation" << endl;
            cout << " 6. Enregistrer un paiement" << endl;
            cout << " 7. Annuler une reservation" << endl;
            cout << " 8. Consulter les reservations" << endl;
            cout << " 9. Afficher les informations d'une reservation" << endl;
            cout << "10. Quitter" << endl;
            choix = lireEntier("Votre choix : ", 1, 10);
            switch (choix) {
                case 1:  enregistrerPassager(); break;
                case 2:  ajouterVol(); break;
                case 3:  afficherVolsDisponibles(); break;
                case 4:  rechercherVol(); break;
                case 5:  effectuerReservation(); break;
                case 6:  enregistrerPaiement(); break;
                case 7:  annulerReservation(); break;
                case 8:  consulterReservations(); break;
                case 9:  afficherReservation(); break;
                case 10: cout << "Au revoir !" << endl; break;
            }
        } while (choix != 10);
    }
};

// ------------------------------------------------------------
//  Programme principal
// ------------------------------------------------------------
int main() {
    AgenceVoyage agence;
    agence.executer();
    return 0;
}
