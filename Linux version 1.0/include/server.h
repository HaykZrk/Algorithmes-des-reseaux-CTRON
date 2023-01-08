#include "common.h"

#define EMPTY -1
#define HEAD 0
#define TRAIL_DOWN -2

#define NO_WIN -1
#define J1_WIN 1
#define J2_WIN 2
#define DRAW 0

typedef struct player_info {
    int id;
    int dir;
    int posx, posy;
    int trail;
} player_info;

/**
 * @brief Valide les changements de direction
 * @param old_dir La direction actuelle
 * @param new_dir La direction demandée
 * @return Si new_dir est valide alors new_dir sinon old_dir
*/
int mise_a_jour_direction (int old_dir, int new_dir);

/**
 * @brief Initialise le plateau, place les joueurs et les murs
 * @param di Pointe vers le display_info qui sera initialisé
 * @param pi1 Pointe vers les infos du joueur 1 qui sera initialisé
 * @param pi2 Pointe vers les infos du joueur 2 qui sera initialisé
 * @return Void
*/
void init_game (display_info *di, player_info *pi1, player_info *pi2);

/**
 * @brief Mets à jour le tableau en fonction des directions des joueurs
 * @param di Pointe vers le display_info qui sera modifié
 * @param pi1 Pointe vers les infos du joueur 1 qui sera mis à jour
 * @param pi2 Pointe vers les infos du joueur 2 qui sera mis à jour
 * @return Si aucune collision n'est détectée dans le nouveau plateau alors -1, sinon l'ID du gagnant
*/
int mise_a_jour_board(display_info *di, player_info *pi1, player_info *pi2);

/**
 * @brief Vérifie la présence d'une collision dans le plateau
 * @param di Pointe vers le display_info
 * @param pi1 Pointe vers les infos du joueur 1
 * @param pi2 Pointe vers les infos du joueur 2
 * @return S'il n'y a pas de collision alors -1 sinon l'ID du gagnant (O en cas de match nul)
*/
int test_collision(display_info *di, player_info *pi1, player_info *pi2);

/**
 * @brief Ferme le serveur proprement en envoyant les données aux clients et en fermant les fichiers
 * @param s_srv Socket du serveur
 * @param s_cl1 Socket du client 1
 * @param s_cl2 Socket du client 2 (-1 s'il n'y a qu'un client)
 * @param di Pointe vers le display_info
 * @param gagnant ID du gagnant (il doit y un gagnant ou match nul pour fermer le serveur)
*/
void fin_propre(int s_srv, int s_cl1, int s_cl2, display_info *di, int gagnant);

/**
 * @brief Réactive la traînée d'un joueur qui l'avait désactivée
 * @param di Pointe vers le display_info qui sera modifié
 * @param player_id L'ID du joueur qui réactive sa trainée
*/
void remettre_trainee(display_info *di, int player_id);