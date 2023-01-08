/**
 * @file server.c
 * @author Hayk ZARIKIAN et Alexandre DUBERT
 * @brief Fichier server.c qui envoie les données de jeu TRON aux joueurs.
 * @version 0.1
 * @date 2022-12-06
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "server.h"

int main (int argc, char* argv[]) {
    int s, s_client1, s_client2;
    SAI addr_srv, addr_client1, addr_client2;
    socklen_t len_addr_client1, len_addr_client2;
    fd_set rfds, tmp;
    struct timeval timer_refresh;
    struct client_input cl_in;
    int dir;
    struct client_init_infos c_init_info;
    player_info p1_info, p2_info;
    struct display_info di;
    int gagnant;
    char entree_term[10];
    int refresh_rate;

    if (argc != 3) {
        fprintf(stderr, "./server [port_serveur] [refresh_rate]\n");
        exit(EXIT_FAILURE);
    }

    /* Création de la socket serveur */    
    s = socket(AF_INET, SOCK_STREAM, 0);
    CHECK(s);

    addr_srv.sin_family = AF_INET;
    addr_srv.sin_port = htons(atoi(argv[1]));
    addr_srv.sin_addr.s_addr = htonl(INADDR_ANY);
    memset(&(addr_srv.sin_zero), '\0', 8); 
    
    CHECK(bind(s, (SA*)&addr_srv, sizeof(SAI)) != -1);
    CHECK(listen(s,2) != -1);

    /* Connexion des clients */
    s_client1 = accept(s, (SA*)&addr_client1, &len_addr_client1);
    s_client2 = -1;
    CHECK(s_client1);
    printf("Client 1 connecté\n");
    CHECK(recv(s_client1, &c_init_info, sizeof(struct client_init_infos), 0));
    if (c_init_info.nb_players == 1) {
        s_client2 = accept(s, (SA*)&addr_client2, &len_addr_client2);
        CHECK(s_client2);
        printf("Client 2 connecté\n");
        CHECK(recv(s_client2, &c_init_info, sizeof(struct client_init_infos), 0));
    }

    init_game(&di, &p1_info, &p2_info);

    FD_ZERO(&rfds);
    FD_SET(0, &rfds);
    FD_SET(s_client1, &rfds);
    if (s_client2 != -1) FD_SET(s_client2, &rfds);

    timer_refresh.tv_sec = 0;
    refresh_rate = atoi(argv[2]) * 1000;

    gagnant = NO_WIN;

    while (gagnant == NO_WIN) {
        tmp = rfds;
        timer_refresh.tv_usec = refresh_rate;

        CHECK(send(s_client1, &di, sizeof(struct display_info), 0) != -1);

        if (s_client2 == -1) {
            CHECK(select(s_client1 + 1, &tmp, NULL, NULL, &timer_refresh) != -1);
        } else {
            CHECK(send(s_client2, &di, sizeof(struct display_info), 0) != -1);
            CHECK(select(s_client2 + 1, &tmp, NULL, NULL, &timer_refresh) != -1);
        }

        /* Données dans l'entrée standard */
        if (FD_ISSET(0, &tmp)) {
            memset(entree_term, '\0', 10);
            CHECK(read(0, entree_term, 10) != -1);
            if (!strcmp(entree_term, "quit\n")) {
                fin_propre(s, s_client1, s_client2, &di, 0);
            } else if (!strcmp(entree_term, "restart\n")) {
                init_game(&di, &p1_info, &p2_info);
            } else if (!strcmp(entree_term, "help\n")) {
                printf("restart : recommence la partie.\n");
                printf("quit : quitte le jeu.\n");
            } else {
                printf("Commande inconnue, \"help\" pour obtenir de l'aide.\n");
            }
        }

        /* Données dans la socket du client 1 */
        if (FD_ISSET(s_client1, &tmp)) {
            memset(&cl_in, '\0', sizeof(struct client_input));
            CHECK(recv(s_client1, &cl_in, sizeof(struct client_input), 0) != -1);
            
            if (s_client2 == -1 && cl_in.id == 2) {
                /* Traitement de l'entrée du joueur 2 (si 2 joueurs sur 1 terminal) */
                if (cl_in.input == TRAIL_UP) {
                    p2_info.trail = (p2_info.trail + 1) % 2;
                    if (p2_info.trail) {
                        remettre_trainee(&di, 2);
                        gagnant = test_collision(&di, &p1_info, &p2_info);
                        if (gagnant != NO_WIN) fin_propre(s, s_client1, s_client2, &di, gagnant);
                    }
                } else {
                    dir = mise_a_jour_direction(p2_info.dir, (int) cl_in.input);
                    p2_info.dir = dir;
                }
            } else {
                /* Traitement de l'entrée du joueur 1 */
                if (cl_in.input == TRAIL_UP) {
                    p1_info.trail = (p1_info.trail + 1) % 2;
                    if (p1_info.trail) {
                        remettre_trainee(&di, 1);
                        gagnant = test_collision(&di, &p1_info, &p2_info);
                        if (gagnant != NO_WIN) fin_propre(s, s_client1, s_client2, &di, gagnant);
                    }
                } else {
                    dir = mise_a_jour_direction(p1_info.dir, (int) cl_in.input);
                    p1_info.dir = dir;
                }
            }
        } 
        
        /* Données dans la socket du client 2 */
        if (s_client2 != -1 && FD_ISSET(s_client2, &tmp)) {
            memset(&cl_in, '\0', sizeof(struct client_input));
            CHECK(recv(s_client2, &cl_in, sizeof(struct client_input), 0) != -1);

            /* Traitement de l'entrée du joueur 2 */
            if (cl_in.input == TRAIL_UP) {
                p2_info.trail = (p2_info.trail + 1) % 2;
                if (p2_info.trail) {
                    remettre_trainee(&di, 2);
                    gagnant = test_collision(&di, &p1_info, &p2_info);
                    if (gagnant != NO_WIN) fin_propre(s, s_client1, s_client2, &di, gagnant);
                }
            } else {
                dir = mise_a_jour_direction(p2_info.dir, (int) cl_in.input);
                p2_info.dir = dir;
            }
        }

        /* Mise à jour du plateau et de la condition d'arrêt */
        gagnant = mise_a_jour_board(&di, &p1_info, &p2_info);
    }

    fin_propre(s, s_client1, s_client2, &di, gagnant);

    return EXIT_SUCCESS;
}

void init_game (display_info *di, player_info *pi1, player_info *pi2) {
    int i, j;

    pi1->id = 1;
    pi1->dir = RIGHT;
    pi1->posx = XMAX / 3;
    pi1->posy = 2 * YMAX / 3;
    pi1->trail = 1;

    pi2->id = 2;
    pi2->dir = LEFT;
    pi2->posx = 2 * XMAX / 3;
    pi2->posy = YMAX / 3;
    pi2->trail = 1;

    for (j = 0; j < XMAX; j++) {
        for (i = 0; i < YMAX; i++) {
            if (i == 0 || i == YMAX-1 || j == 0 || j == XMAX-1) {
                di->board[j][i] = WALL;
            } else {
                di->board[j][i] = EMPTY;
            }
        }
    } 

    di->board[pi1->posx][pi1->posy] = HEAD;
    di->board[pi2->posx][pi2->posy] = HEAD + 1;

    di->winner = NO_WIN;
}

int mise_a_jour_direction (int old_dir, int new_dir) {
    switch (new_dir) {
        case UP:
            if (old_dir == DOWN) return DOWN;
            break;
        case DOWN:
            if (old_dir == UP) return UP;
            break;
        case RIGHT:
            if (old_dir == LEFT) return LEFT;
            break;
        case LEFT:
            if (old_dir == RIGHT) return RIGHT;
            break;
    }

    return new_dir;
}

int mise_a_jour_board(display_info *di, player_info *pi1, player_info *pi2) {
    int p1_oldx, p1_oldy, p2_oldx, p2_oldy;
    int gagnant;
    display_info nv_di;

    nv_di = *di;

    p1_oldx = pi1->posx;
    p1_oldy = pi1->posy;

    switch (pi1->dir) {
        case UP:
            pi1->posy = p1_oldy - 1;
            break;
        case DOWN:
            pi1->posy = p1_oldy + 1;
            break;
        case RIGHT:
            pi1->posx = p1_oldx + 1;
            break;
        case LEFT:
            pi1->posx = p1_oldx - 1;
            break;
    }

    p2_oldx = pi2->posx;
    p2_oldy = pi2->posy;

    switch (pi2->dir) {
        case UP:
            pi2->posy = p2_oldy - 1;
            break;
        case DOWN:
            pi2->posy = p2_oldy + 1;
            break;
        case RIGHT:
            pi2->posx = p2_oldx + 1;
            break;
        case LEFT:
            pi2->posx = p2_oldx - 1;
            break;
    }

    nv_di.board[p1_oldx][p1_oldy] = pi1->trail == 1 ? TRAIL_INDEX_SHIFT : TRAIL_DOWN;
    nv_di.board[p2_oldx][p2_oldy] = pi2->trail == 1 ? TRAIL_INDEX_SHIFT + 1 : TRAIL_DOWN - 1;

    gagnant = test_collision(&nv_di, pi1, pi2);
    if (gagnant == NO_WIN) {
        *di = nv_di;
        di->board[pi1->posx][pi1->posy] = HEAD;
        di->board[pi2->posx][pi2->posy] = HEAD + 1;
    }

    return gagnant;
}

int test_collision(display_info *di, player_info *pi1, player_info *pi2) {
    int coll_j1, coll_j2;

    switch (di->board[pi1->posx][pi1->posy]) {
        case WALL:
            coll_j1 = 1;
            break;
        case TRAIL_INDEX_SHIFT:
            coll_j1 = 1;
            break;
        case TRAIL_INDEX_SHIFT + 1:
            coll_j1 = 1;
            break;
        case HEAD + 1:
            coll_j1 = 1;
            break;
        default:
            coll_j1 = 0;
    }

    switch (di->board[pi2->posx][pi2->posy]) {
        case WALL:
            coll_j2 = 1;
            break;
        case TRAIL_INDEX_SHIFT:
            coll_j2 = 1;
            break;
        case TRAIL_INDEX_SHIFT + 1:
            coll_j2 = 1;
            break;
        case HEAD:
            coll_j2 = 1;
            break;
        default:
            coll_j2 = 0;
    }

    if (coll_j1 && coll_j2) return DRAW;
    if (coll_j1) return J2_WIN;
    if (coll_j2) return J1_WIN;
    return NO_WIN;
}

void fin_propre(int s_srv, int s_cl1, int s_cl2, display_info *di, int gagnant) {
    gagnant = gagnant == NO_WIN ? DRAW : gagnant;
    di->winner = gagnant;
    CHECK(send(s_cl1, di, sizeof(struct display_info), 0) != -1);
    if (s_cl2 != -1) 
        CHECK(send(s_cl2, di, sizeof(struct display_info), 0) != -1);
    close(s_cl1);
    if (s_cl2 != -1) close(s_cl2);
    close(s_srv);

    exit(EXIT_SUCCESS);
}

void remettre_trainee(display_info *di, int player_id) {
    int i, j;
    
    for (j = 0; j < XMAX; j++) {
        for (i = 0; i < YMAX; i++) {
            if (di->board[j][i] == TRAIL_DOWN - player_id + 1)
                di->board[j][i] = TRAIL_INDEX_SHIFT + player_id - 1;
        }
    }
}