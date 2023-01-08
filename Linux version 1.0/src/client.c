/**
 * @file client.c
 * @author Hayk ZARIKIAN et Alexandre DUBERT
 * @brief Fichier client.c qui permet de se connocter au serveur pour jouer au jeu TRON.
 * @version 0.1
 * @date 2022-11-30
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "../include/client.h"

#define BLUE_ON_BLUE        50
#define YELLOW_ON_YELLOW    51
#define BLUE_ON_BLACK       0
#define YELLOW_ON_BLACK     1

/**
 * @brief Fonctionne principale qui assure la bonne connexion du client au serveur 
 *        en protocole TCP_IP pour jouer au jeu TRON.
 * 
 * @param[in] argc : Nombre d'argument
 * @param[in] argv : [EXECUTABLE] [IP_SERVEUR] [PORT_SERVEUR] [NB_JOUEURS]
 * @return[out] int : Exit avec succès 
 */
int main(int argc, char **argv)
{

    if (argc != 4)
    {
        fprintf (stderr, "Usage : %s [IP_SERVEUR] [PORT_SERVEUR] [NB_JOUEURS]\n", argv[0]);
        exit (EXIT_FAILURE);
    }

    char *ip_serveur = malloc (sizeof (char) * strlen (argv[1]));
    if (ip_serveur == NULL)
    {
        fprintf (stderr, "Echec allocation !\n");
        exit (EXIT_FAILURE);
    }

    strcpy (ip_serveur, argv[1]);
    int port_serveur = atoi(argv [2]);
    int nb_joueur = atoi (argv[3]);

    if (nb_joueur != 2 && nb_joueur != 1)
    {
        fprintf (stderr, "Nombre de joueur max = 2 !\n");
        exit (EXIT_FAILURE);
    }

    int sockfdServeur = sock_factory(ip_serveur, port_serveur);
    struct client_init_infos client_init;
    client_init.nb_players = nb_joueur;

    CHECK (send (sockfdServeur, &client_init, sizeof (struct client_init_infos), 0) != -1);
    
    fd_set rfds, tmp;
    FD_ZERO (&rfds);
    FD_SET (0, &rfds);
    FD_SET (sockfdServeur, &rfds);

    display_info plateau_jeu;
    plateau_jeu.winner = -1;
    tune_terminal();
    init_graphics();
    char touche_clavier;

    struct client_input input;

    while (plateau_jeu.winner == -1)
    {
        tmp = rfds;
        CHECK((select (sockfdServeur +1, &tmp, NULL, NULL, NULL) != -1));
        /*
            Envois des informations (structure client_input) de direction vers le serveur.
        */
        if (FD_ISSET (0, &tmp))
        {
            ssize_t n_lus;
            n_lus = read (0, &touche_clavier, 1);
            CHECK (n_lus != -1);
            if (change_direction (touche_clavier, &input, nb_joueur, ip_serveur)) {
                CHECK (send (sockfdServeur, &input, sizeof (struct client_input), 0) != -1);
            }
        }
        /*
            Réception des informations du plateau de jeu (structure display_info) du serveur.
        */
        if (FD_ISSET (sockfdServeur, &tmp))
        {
            memset(&plateau_jeu, '\0', sizeof(display_info));
            CHECK (recv (sockfdServeur, &plateau_jeu, sizeof (display_info), 0) != -1);
            clear ();
            for (int x = 0; x < XMAX; x++)
                for (int y = 0; y < YMAX; y++)
                {
                    if (x == 0 || x == XMAX-1)
                    {
                        if (plateau_jeu.board[x][y] == WALL)
                            display_character (WALL, y, x, ACS_VLINE);
                    }
                    else
                    {
                        if (plateau_jeu.board[x][y] == BLUE_ON_BLUE)
                            display_character (BLUE_ON_BLUE, y, x, ACS_HLINE);
                        else if (plateau_jeu.board[x][y] == YELLOW_ON_YELLOW)
                            display_character (YELLOW_ON_YELLOW, y, x, ACS_HLINE);
                        else if (plateau_jeu.board[x][y] == WALL)
                            display_character (WALL, y, x, ACS_HLINE);
                        else if (plateau_jeu.board[x][y] == BLUE_ON_BLACK)
                            display_character (BLUE_ON_BLACK, y, x, 'O');
                        else if (plateau_jeu.board[x][y] == YELLOW_ON_BLACK)
                            display_character (BLUE_ON_BLACK, y, x, 'O');
                    }
                }
            mvaddstr (0, XMAX/2 - strlen("C-TRON")/2, "C-TRON");
            refresh();
        }
    }
    /*
        Affichage du vainqueur et fermeture du jeu.
    */
    clear ();
    if (plateau_jeu.winner == 0)
        mvaddstr (YMAX/2, XMAX/2 - strlen("END GAME : No winner")/2, "END GAME : No winner");
    else {
            mvaddstr (YMAX/2, XMAX/2 - strlen("END GAME : Winner is ")/2, "END GAME : Winner is ");
            printw ("%d", plateau_jeu.winner);
    }
    refresh ();
    sleep (3);

    endwin ();
    CHECK (close (sockfdServeur) != -1);
    return EXIT_SUCCESS;
}

/**
 * @brief Fonction qui crée le point de communication qui prend en argument
 *        l'IP serveur et le port du serveur.
 * 
 * @param[in] ip_serveur char* : L'adresse IP du serveur
 * @param[in] port_serveur int : Le port du serveur
 * @return[out] int : Retourne le file descriptor
 */
int sock_factory (char *ip_serveur, int port_serveur)
{
    int sockfdServeur;
    SAI addrServeur;
    socklen_t addrLen = sizeof (SAI);

    memset (&addrServeur, '\0', sizeof (SAI));
    sockfdServeur = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    CHECK ((sockfdServeur = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)));
    
    addrServeur.sin_family = AF_INET;
    addrServeur.sin_port = htons(port_serveur);
    if (!strcmp (ip_serveur, "0.0.0.0"))
        addrServeur.sin_addr.s_addr = htonl(INADDR_ANY);
    else 
        addrServeur.sin_addr.s_addr = inet_addr (ip_serveur);

    CHECK (connect (sockfdServeur, (SA*)&addrServeur, addrLen) != -1);

    return sockfdServeur;
}

/**
 * @brief Fonction qui récupère la touche appuyé pour insérer la direction dans la structure
 *        input avec l'identificant du joueur.
 * 
 * @param[in] touche_clavier char : Touche appuyée
 * @param[in] input struct client_input : Direction enregistrée avec l'identifiant du joueur
 * @param[in] nb_joueur int : Nombre de joueur 
 * @param[in] ip_serveur char* : IP du serveur
 * @returns[in] input struct client_input : Affectation des valeurs par adresse de la structure input
 * @return[out] int : Retourne 1 si une touche valide a été appuyée
 */
int change_direction (char touche_clavier, struct client_input *input, int nb_joueur, char* ip_serveur)
{
    switch (touche_clavier)
    {
    case 'z':
        input->id = 1;
        input->input = UP;
        return 1;
    case 'q':
        input->id = 1;
        input->input = LEFT;
        return 1;
    case 's':
        input->id = 1;
        input->input = DOWN;
        return 1;
    case 'd':
        input->id = 1;
        input->input = RIGHT;
        return 1;
    case ' ':
        input->id = 1;
        input->input = TRAIL_UP;
        return 1;
    }

    if (nb_joueur == 2 && !strcmp (ip_serveur, "0.0.0.0"))
    {
        switch (touche_clavier)
        {
        case 'i':
            input->id = 2;
            input->input = UP;
            return 1; 
        case 'j':
            input->id = 2;
            input->input = LEFT;
            return 1;
        case 'k':
            input->id = 2;
            input->input = DOWN;
            return 1;
        case 'l':
            input->id = 2;
            input->input = RIGHT;
            return 1;
        case 'm':
            input->id = 2;
            input->input = TRAIL_UP;
            return 1;
        }
    }
    return 0;
} 

/**
 * @brief Initialisation terminal.
 * 
 */
void tune_terminal()
{
    fflush (stdin);
    struct termios term;
    tcgetattr(0, &term);
    term.c_iflag &= ~ICANON;
    term.c_lflag &= ~ICANON;
    term.c_cc[VMIN] = 0;
    term.c_cc[VTIME] = 0;
    tcsetattr(0, TCSANOW, &term);
}

/**
 * @brief Inititialisation graphique.
 * 
 */
void init_graphics()
{
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    timeout(100);
    start_color();
    init_pair(BLUE_ON_BLUE, COLOR_BLUE, COLOR_BLUE);
    init_pair(YELLOW_ON_YELLOW, COLOR_YELLOW, COLOR_YELLOW);
    init_pair(WALL, COLOR_WHITE, COLOR_WHITE);
}

/**
 * @brief Affichage d'un caractère à la position x et y.
 * 
 * @param[in] color : Couleur choisie
 * @param[in] y : Position y
 * @param[in] x : Position x
 * @param[in] character : Caractère d'affichage
 */
void display_character(int color, int y, int x, char character) {
    attron(COLOR_PAIR(color));
    mvaddch(y, x, character);
    attroff(COLOR_PAIR(color));
}