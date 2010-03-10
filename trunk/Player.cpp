#include <stdlib.h>
#include <math.h>
#include <curses.h>
#include "Agent.h"
#include "Player.h"
#include "Level.h"
#include "Game.h"

Player::Player(Level *loc, Game *parent) : Agent(loc->get_upstair_x(), loc->get_upstair_y(), 0, loc, parent) {
    
}

Player::Player(int x, int y, float f, Level *loc, Game *parent) : Agent(x, y, f, loc, parent) {
    
}

int Player::take_turn() {
    int input;
    input = getch();
    switch(input) {
        case 'j':
            walk(-1, 0);
            break;
        case 'l':
            walk(1, 0);
            break;
        case 'i':
            walk(0, -1);
            break;
        case ',':
            walk(0, 1);
            break;
        case 'u':
            walk(-1, -1);
            break;
        case 'm':
            walk(-1, 1);
            break;
        case 'o':
            walk(1, -1);
            break;
        case '.':
            walk(1, 1);
            break;
        case 'h':
            turn(float(-PI/4));
            break;
        case ';':
            turn(float(PI/4));
            break;
		case 'k':
			use();
			break;
        default:
            break;
    }
    
    mutual_fov();
    return input;
}

/*
int Player::calculate_visibility() {
    location->clear_visibility();
	for (int i = -vision; i <= vision; i++) {
		for (int j = -vision; j <= vision; j++) {
			if (position.x+i >= map_width || position.y+j >= map_height ||
				position.x+i < 0 || position.y+j < 0) continue;

			float angle = atan2(float(j),float(i));
			if (angle < 0) angle += float(2*PI);
			float anglediff = angle - facing;
			if (anglediff > PI) anglediff -= float(2*PI);
//			if (fabs(anglediff) > PI/3 && !(i==0 && j==0)) continue;

			if (location->is_visible(position.x, position.y, position.x+i, position.y+j))
				location->mark_visible(position.x+i, position.y+j);
		}
	}
    return 0;
}
*/

void Player::mutual_fov() {
    location->clear_visibility();
    Agent::mutual_fov();
    for(int i = 0; i < n_visible_corners; i++) {
        location->mark_visible(visible_corners[i].x, visible_corners[i].y);
        location->mark_visible(visible_corners[i].x-1, visible_corners[i].y);
        location->mark_visible(visible_corners[i].x, visible_corners[i].y-1);
        location->mark_visible(visible_corners[i].x-1, visible_corners[i].y-1);
    }
}

void Player::use() {
	if (location->is_upstair(position.x, position.y)) {
		if (game->ascend()) {
			position.x = location->get_downstair_x();
			position.y = location->get_downstair_y();
		}
	}
	else if (location->is_downstair(position.x, position.y)) {
		if (game->descend()) {
			position.x = location->get_upstair_x();
			position.y = location->get_upstair_y();
		}
	}
}