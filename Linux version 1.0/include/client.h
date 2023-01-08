/**
 * @file client.h
 * @author Hayk ZARIKIAN et Alexandre DUBERT 
 * @brief Fichier client.h déclare les fonctions utilisées dans le fichier client.c
 * @version 0.1
 * @date 2022-11-30
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef __CLIENT_H__
#define __CLIENT_H__

#include <ncurses.h>
#include <termios.h>
#include "../include/common.h"

void tune_terminal ();

void init_graphics ();

void display_character (int color, int y, int x, char character);

int sock_factory (char *ip_serveur, int port_serveur);

int change_direction (char touche_clavier, struct client_input *input, int nb_joueur, char *ip_serveur);

#endif