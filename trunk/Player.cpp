#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <curses.h>
#include "Agent.h"
#include "Player.h"
#include "Level.h"
#include "Game.h"

Player::Player(Level *loc, Game *parent) : Agent(loc->get_upstair_x(), loc->get_upstair_y(), 0, loc, parent) {
    is_player = true;
    walk_mode = TURNING;
    symbol = '@';
    hp = max_hp = 20;
    attack_strength = 2;
    ranged_accuracy = 5;
    init_keys();
	inventory = new Inventory(game->get_inventory_win(), this);
}

void Player::init_keys() {
    default_keys();
}

void Player::default_keys() {
    keys.walk_west = 'j';
    keys.walk_east = 'l';
    keys.walk_north = 'i';
    keys.walk_south = ',';
    keys.walk_nw = 'u';
    keys.walk_ne = 'o';
    keys.walk_sw = 'm';
    keys.walk_se = '.';
    keys.turn_left = 'h';
    keys.turn_right = ';';
    keys.use = 'k';
    keys.change_walk_mode = '/';
	keys.inventory = 's';
	keys.close = 'c';
}

void Player::die() {
    
}

void Player::close_door() {
    int n_open_doors = 0;
    Position door;
    for(int y = -1; y <= 1; y++)
        for(int x = -1; x <= 1; x++)
            if(x != 0 || y != 0)
                if(location->is_open_door(position.x+x, position.y+y)) {
                    n_open_doors++;
                    door.x = position.x+x;
                    door.y = position.y+y;
                }
    int result;
    #define NO_DOORS 0
    #define NO_DOOR_THAT_DIRECTION 1
    #define DOOR_OBSTRUCTED 2
    #define DOOR_CLOSED_ALREADY 3
    #define DOOR_CLOSE_SUCCESS 4
    if(n_open_doors == 0) {
        result = NO_DOORS;
    }
    else if(n_open_doors == 1) {
        if(location->close_door(door.x, door.y))
            result = DOOR_CLOSE_SUCCESS;
        else
            result = DOOR_OBSTRUCTED;
    } else {
        bool input_ok;
        game->write_message("Close a door in what direction?");
        do {
            input_ok = true;
            int input = getch();
            if(input == keys.walk_west) {
                door.x = position.x-1;
                door.y = position.y;
            } else if(input == keys.walk_east) {
                door.x = position.x+1;
                door.y = position.y;
            } else if(input == keys.walk_north) {
                door.x = position.x;
                door.y = position.y-1;
            } else if(input == keys.walk_south) {
                door.x = position.x;
                door.y = position.y+1;
            } else if(input == keys.walk_nw) {
                door.x = position.x-1;
                door.y = position.y-1;
            } else if(input == keys.walk_sw) {
                door.x = position.x-1;
                door.y = position.y+1;
            } else if(input == keys.walk_ne) {
                door.x = position.x+1;
                door.y = position.y-1;
            } else if(input == keys.walk_se) {
                door.x = position.x+1;
                door.y = position.y+1;
            } else {
                input_ok = false;
                game->write_message("I didn't catch that.  What direction?");
            }
        } while(!input_ok);
        if(location->close_door(door.x, door.y))
            result = DOOR_CLOSE_SUCCESS;
        else if(location->is_closed_door(door.x, door.y))
            result = DOOR_CLOSED_ALREADY;
        else if(location->is_open_door(door.x, door.y))
            result = DOOR_OBSTRUCTED;
        else
            result = NO_DOOR_THAT_DIRECTION;
    }
    char msg[80] = "";
    switch(result) {
        case NO_DOORS:
            strcat(msg, "There are no doors you can close.");
            break;
        case NO_DOOR_THAT_DIRECTION:
            strcat(msg, "There is no door in that direction.");
            break;
        case DOOR_OBSTRUCTED:
            strcat(msg, "You try to close a door, but there something in the way.");
            break;
        case DOOR_CLOSED_ALREADY:
            strcat(msg, "That door is already closed.");
            break;
        case DOOR_CLOSE_SUCCESS:
            strcat(msg, "You close a door.");
            break;
        default:
            strcat(msg, "yay?");
            break;
    }
    game->write_message(msg);
}

int Player::get_melee_damage() {
    return attack_strength +
        ((Weapon *)(inventory->get_current_melee_weapon()))->damage();
}

int Player::get_ranged_damage() {
    return ((Weapon *)(inventory->get_current_ranged_weapon()))->damage();
}

//Get input from the keyboard and act on input.  Then recalculate player's fov.
int Player::take_turn() {
    int input;
    input = getch();
    if(input == keys.walk_west) 
        walk(-1, 0);
    else if(input == keys.walk_east)
        walk(1, 0);
    else if(input == keys.walk_north)
        walk(0, -1);
    else if(input == keys.walk_south)
        walk(0, 1);
    else if(input == keys.walk_nw)
        walk(-1, -1);
    else if(input == keys.walk_sw)
        walk(-1, 1);
    else if(input == keys.walk_ne)
        walk(1, -1);
    else if(input == keys.walk_se)
        walk(1, 1);
    else if(input == keys.turn_left)
        turn(float(-PI/4));
    else if(input == keys.turn_right)
        turn(float(PI/4));
	else if(input == keys.use)
		use();
	else if(input == keys.change_walk_mode)
        toggle_walk_mode();
	else if(input == keys.inventory)
		inventory->open();
	else if(input == keys.close)
        close_door();
    else {
        
    }
    
    mutual_fov();
    return input;
}

void Player::walk(int x, int y) {
    if(walk_mode == TURNING)
        Agent::walk_turn(x, y);
    else if(walk_mode == STRAFING)
        Agent::walk(x, y);
}

void Player::toggle_walk_mode() {
    if(walk_mode == TURNING)
        walk_mode = STRAFING;
    else if(walk_mode == STRAFING)
        walk_mode = TURNING;
}

//Calculate FOV as for any agent, then update the map with this information.
void Player::mutual_fov() {
    location->clear_visibility();
    location->clear_dangerousness();
    Agent::mutual_fov();
    for(int i = 0; i < n_visible_corners; i++) {
        location->mark_visible(visible_corners[i].x, visible_corners[i].y);
        location->mark_visible(visible_corners[i].x-1, visible_corners[i].y);
        location->mark_visible(visible_corners[i].x, visible_corners[i].y-1);
        location->mark_visible(visible_corners[i].x-1, visible_corners[i].y-1);
    }
}

char *Player::get_name() {
    return "you";
}

//Use the thing where the player is standing: stairs...
void Player::use() {
	if (location->is_upstair(position.x, position.y)) {
		if (game->ascend()) {
            set_position(location->get_downstair_x(), location->get_downstair_y());
            location->add_agent(this);
		}
	}
	else if (location->is_downstair(position.x, position.y)) {
		if (game->descend()) {
            set_position(location->get_upstair_x(), location->get_upstair_y());
            location->add_agent(this);
		}
	}
	else if (location->contains_item(position.x, position.y)) {
		inventory->add_item(location->get_item(position.x, position.y));
		location->remove_item(position.x, position.y);
	}
}